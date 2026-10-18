#pragma once
#include "utilsModule/memory/resourceAllocator.h"
#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <initializer_list>
#include <iterator>
#include <limits>
#include <stdexcept>
#include <type_traits>
#include <utility>

namespace IKIGAI::UTILS {
// Contiguous storage with N inline slots. Growth relocates elements. Allocators
// must use raw pointers and nonthrowing destruction/deallocation, as for vector.
template<class T, std::size_t N, class Allocator = MEMORY::StlAllocator<T>>
class SmallVector {
    using Traits = std::allocator_traits<Allocator>;
    static_assert(std::is_same_v<typename Traits::value_type, T>);
    static_assert(std::is_same_v<typename Traits::pointer, T*>);
    static_assert(std::is_move_constructible_v<T> || std::is_copy_constructible_v<T>);
    static_assert(N <= static_cast<std::size_t>(PTRDIFF_MAX) / sizeof(T));
    // MSVC allocator_traits::construct has no conditional noexcept specifier.
    // Inspect the allocator hook (if any), otherwise construction uses T itself.
    static constexpr bool NothrowRelocation = [] {
        using Argument = decltype(std::move_if_noexcept(std::declval<T&>()));
        if constexpr (requires(Allocator& a, T* p, Argument value) { a.construct(p, std::forward<Argument>(value)); })
            return noexcept(std::declval<Allocator&>().construct(std::declval<T*>(), std::declval<Argument>()));
        else return std::is_nothrow_constructible_v<T, Argument>;
    }();
public:
    using value_type = T;
    using allocator_type = Allocator;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;
    using reference = T&;
    using const_reference = const T&;
    using pointer = T*;
    using const_pointer = const T*;
    using iterator = T*;
    using const_iterator = const T*;
    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;
    static constexpr size_type inline_capacity = N;

    SmallVector() : SmallVector(Allocator()) {}
    explicit SmallVector(const Allocator& allocator) noexcept(std::is_nothrow_copy_constructible_v<Allocator>) : mAllocator(allocator) {}
    explicit SmallVector(size_type count, const Allocator& allocator = Allocator()) : SmallVector(allocator) {
        try { resize(count); } catch (...) { resetStorage(); throw; }
    }
    SmallVector(size_type count, const T& value, const Allocator& allocator = Allocator()) : SmallVector(allocator) {
        try { resize(count, value); } catch (...) { resetStorage(); throw; }
    }
    template<std::input_iterator It>
    SmallVector(It first, It last, const Allocator& allocator = Allocator()) : SmallVector(allocator) {
        try {
            if constexpr (std::forward_iterator<It>) {
                const auto count = std::distance(first, last);
                if (count < 0) throw std::length_error("SmallVector reversed range");
                reserve(static_cast<size_type>(count));
            }
            for (; first != last; ++first) emplace_back(*first);
        } catch (...) { resetStorage(); throw; }
    }
    SmallVector(std::initializer_list<T> values, const Allocator& allocator = Allocator())
        : SmallVector(values.begin(), values.end(), allocator) {}
    SmallVector(const SmallVector& other) requires std::is_copy_constructible_v<T>
        : SmallVector(other, Traits::select_on_container_copy_construction(other.mAllocator)) {}
    SmallVector(const SmallVector& other, const Allocator& allocator) requires std::is_copy_constructible_v<T>
        : SmallVector(other.begin(), other.end(), allocator) {}
    // Copy the allocator so that the emptied source retains a usable domain.
    SmallVector(SmallVector&& other) noexcept(
        std::is_nothrow_copy_constructible_v<Allocator> &&
        noexcept(std::declval<const Allocator&>() == std::declval<const Allocator&>()) &&
        (N == 0 || NothrowRelocation))
        : SmallVector(std::move(other), other.mAllocator) {}
    SmallVector(SmallVector&& other, const Allocator& allocator) : SmallVector(allocator) {
        try { moveFrom(other); } catch (...) { resetStorage(); throw; }
    }
    ~SmallVector() { resetStorage(); }

    SmallVector& operator=(const SmallVector& other) requires std::is_copy_constructible_v<T> {
        if (this == &other) return *this;
        if constexpr (Traits::propagate_on_container_copy_assignment::value) {
            SmallVector copy(other, other.mAllocator);
            resetStorage(); mAllocator = other.mAllocator; moveFrom(copy);
        } else {
            SmallVector copy(other, mAllocator);
            resetStorage(); moveFrom(copy);
        }
        return *this;
    }
    SmallVector& operator=(SmallVector&& other) noexcept(
        (Traits::propagate_on_container_move_assignment::value || Traits::is_always_equal::value) &&
        (!Traits::propagate_on_container_move_assignment::value || std::is_nothrow_copy_assignable_v<Allocator>) &&
        noexcept(std::declval<const Allocator&>() == std::declval<const Allocator&>()) &&
        (N == 0 || NothrowRelocation)) {
        if (this == &other) return *this;
        resetStorage();
        if constexpr (Traits::propagate_on_container_move_assignment::value) mAllocator = other.mAllocator;
        moveFrom(other);
        return *this;
    }
    SmallVector& operator=(std::initializer_list<T> values) { assign(values.begin(), values.end()); return *this; }
    template<std::input_iterator It> void assign(It first, It last) {
        SmallVector copy(first, last, mAllocator); resetStorage(); moveFrom(copy);
    }
    void assign(size_type count, const T& value) {
        SmallVector copy(count, value, mAllocator); resetStorage(); moveFrom(copy);
    }
    void assign(std::initializer_list<T> values) { assign(values.begin(), values.end()); }

    allocator_type get_allocator() const { return mAllocator; }
    size_type size() const noexcept { return mSize; }
    size_type capacity() const noexcept { return mCapacity; }
    bool empty() const noexcept { return mSize == 0; }
    bool is_inline() const noexcept { return mData == inlineData(); }
    size_type max_size() const noexcept {
        return std::max(N, std::min(Traits::max_size(mAllocator), static_cast<size_type>(PTRDIFF_MAX) / sizeof(T)));
    }
    T* data() noexcept { return mData; }
    const T* data() const noexcept { return mData; }
    T& operator[](size_type i) noexcept { return mData[i]; }
    const T& operator[](size_type i) const noexcept { return mData[i]; }
    T& at(size_type i) { if (i >= mSize) throw std::out_of_range("SmallVector index"); return mData[i]; }
    const T& at(size_type i) const { if (i >= mSize) throw std::out_of_range("SmallVector index"); return mData[i]; }
    T& front() noexcept { assert(mSize); return mData[0]; }
    const T& front() const noexcept { assert(mSize); return mData[0]; }
    T& back() noexcept { assert(mSize); return mData[mSize - 1]; }
    const T& back() const noexcept { assert(mSize); return mData[mSize - 1]; }
    iterator begin() noexcept { return mData; }
    iterator end() noexcept { return mData + mSize; }
    const_iterator begin() const noexcept { return mData; }
    const_iterator end() const noexcept { return mData + mSize; }
    const_iterator cbegin() const noexcept { return begin(); }
    const_iterator cend() const noexcept { return end(); }
    reverse_iterator rbegin() noexcept { return reverse_iterator(end()); }
    reverse_iterator rend() noexcept { return reverse_iterator(begin()); }
    const_reverse_iterator rbegin() const noexcept { return const_reverse_iterator(end()); }
    const_reverse_iterator rend() const noexcept { return const_reverse_iterator(begin()); }
    const_reverse_iterator crbegin() const noexcept { return rbegin(); }
    const_reverse_iterator crend() const noexcept { return rend(); }

    void reserve(size_type count) {
        if (count > max_size()) throw std::length_error("SmallVector capacity overflow");
        if (count > mCapacity) reallocate(count);
    }
    // Explicitly return to inline storage when size <= N. clear/erase do not.
    void shrink_to_fit() {
        if (is_inline()) return;
        if (mSize <= N) relocate(inlineData(), N);
        else if (mSize < mCapacity) reallocate(mSize);
    }
    void clear() noexcept { while (mSize) pop_back(); }
    void pop_back() noexcept { assert(mSize); Traits::destroy(mAllocator, mData + --mSize); }
    template<class... Args> T& emplace_back(Args&&... args) {
        if (mSize == mCapacity) {
            const auto index = mSize;
            growInsert(index, 1, [&](T* ptr, size_type) { Traits::construct(mAllocator, ptr, std::forward<Args>(args)...); });
            return mData[index];
        }
        Traits::construct(mAllocator, mData + mSize, std::forward<Args>(args)...);
        return mData[mSize++];
    }
    void push_back(const T& value) { emplace_back(value); }
    void push_back(T&& value) { emplace_back(std::move(value)); }
    template<class... Args> iterator emplace(const_iterator pos, Args&&... args) requires std::is_move_assignable_v<T> {
        const auto index = indexOf(pos);
        if (index == mSize) { emplace_back(std::forward<Args>(args)...); return mData + index; }
        if (mSize == mCapacity) {
            growInsert(index, 1, [&](T* ptr, size_type) { Traits::construct(mAllocator, ptr, std::forward<Args>(args)...); });
        } else {
            // Stage arguments before shifting: they may refer to an element.
            Temporary value(mAllocator, std::forward<Args>(args)...);
            Traits::construct(mAllocator, mData + mSize, std::move_if_noexcept(back()));
            ++mSize;
            try {
                std::move_backward(mData + index, mData + mSize - 2, mData + mSize - 1);
                mData[index] = std::move(*value.get());
            } catch (...) { pop_back(); throw; }
        }
        return mData + index;
    }
    iterator insert(const_iterator pos, const T& value) requires std::is_move_assignable_v<T> { return emplace(pos, value); }
    iterator insert(const_iterator pos, T&& value) requires std::is_move_assignable_v<T> { return emplace(pos, std::move(value)); }
    iterator erase(const_iterator pos) requires std::is_move_assignable_v<T> {
        assert(pos != end()); return erase(pos, pos + 1);
    }
    iterator erase(const_iterator first, const_iterator last) requires std::is_move_assignable_v<T> {
        const auto start = indexOf(first), finish = indexOf(last);
        assert(start <= finish);
        if (start == finish) return mData + start;
        std::move(mData + finish, mData + mSize, mData + start);
        const auto newSize = mSize - (finish - start);
        while (mSize > newSize) pop_back();
        return mData + start;
    }
    void resize(size_type count) {
        resizeWith(count, [&](T* ptr, size_type) { Traits::construct(mAllocator, ptr); });
    }
    void resize(size_type count, const T& value) {
        resizeWith(count, [&](T* ptr, size_type) { Traits::construct(mAllocator, ptr, value); });
    }

    void swap(SmallVector& other) requires std::is_swappable_v<T> {
        if (this == &other) return;
        constexpr bool propagate = Traits::propagate_on_container_swap::value;
        if constexpr (!propagate) {
            if (mAllocator != other.mAllocator) throw std::invalid_argument("Incompatible allocation domains");
        } else {
            static_assert(std::is_nothrow_swappable_v<Allocator>, "Allocator swap must not throw");
            using std::swap; swap(mAllocator, other.mAllocator);
        }
        if (!is_inline() && !other.is_inline()) {
            using std::swap; swap(mData, other.mData); swap(mSize, other.mSize); swap(mCapacity, other.mCapacity);
            return;
        }
        try {
            if (is_inline() && other.is_inline()) {
                const auto common = std::min(mSize, other.mSize);
                for (size_type i = 0; i < common; ++i) { using std::swap; swap(mData[i], other.mData[i]); }
                SmallVector& shorter = mSize < other.mSize ? *this : other;
                SmallVector& longer = mSize < other.mSize ? other : *this;
                try {
                    for (auto i = common; i < longer.mSize; ++i) {
                        Traits::construct(shorter.mAllocator, shorter.mData + i, std::move_if_noexcept(longer.mData[i]));
                        ++shorter.mSize;
                    }
                } catch (...) { while (shorter.mSize > common) shorter.pop_back(); throw; }
                while (longer.mSize > common) longer.pop_back();
            } else {
                SmallVector& small = is_inline() ? *this : other;
                SmallVector& heap = is_inline() ? other : *this;
                auto* target = heap.inlineData();
                size_type constructed = 0;
                try {
                    for (; constructed < small.mSize; ++constructed)
                        Traits::construct(heap.mAllocator, target + constructed, std::move_if_noexcept(small.mData[constructed]));
                } catch (...) { destroyRange(heap.mAllocator, target, constructed); throw; }
                const auto smallSize = small.mSize;
                // After allocator propagation, heap owns small's old domain.
                destroyRange(propagate ? heap.mAllocator : small.mAllocator, small.mData, small.mSize);
                small.mData = heap.mData; small.mSize = heap.mSize; small.mCapacity = heap.mCapacity;
                heap.mData = target; heap.mSize = smallSize; heap.mCapacity = N;
            }
        } catch (...) {
            if constexpr (propagate) { using std::swap; swap(mAllocator, other.mAllocator); }
            throw;
        }
    }
    friend void swap(SmallVector& a, SmallVector& b) requires std::is_swappable_v<T> { a.swap(b); }

private:
    struct Temporary {
        Allocator& allocator;
        alignas(T) std::byte storage[sizeof(T)];
        T* get() noexcept { return reinterpret_cast<T*>(storage); }
        template<class... Args> Temporary(Allocator& a, Args&&... args) : allocator(a) {
            Traits::construct(allocator, get(), std::forward<Args>(args)...);
        }
        ~Temporary() { Traits::destroy(allocator, get()); }
    };
    T* inlineData() noexcept { return reinterpret_cast<T*>(mInline); }
    const T* inlineData() const noexcept { return reinterpret_cast<const T*>(mInline); }
    static void destroyRange(Allocator& allocator, T* data, size_type count) noexcept {
        while (count) Traits::destroy(allocator, data + --count);
    }
    void resetStorage() noexcept {
        clear();
        if (!is_inline()) Traits::deallocate(mAllocator, mData, mCapacity);
        mData = inlineData(); mCapacity = N;
    }
    size_type indexOf(const_iterator pos) const noexcept {
        // pos must belong to this container, including end().
        const auto index = pos - mData;
        assert(index >= 0 && static_cast<size_type>(index) <= mSize);
        return static_cast<size_type>(index);
    }
    size_type nextCapacity(size_type requested) const {
        const auto limit = max_size();
        if (requested > limit) throw std::length_error("SmallVector capacity overflow");
        return std::max(requested, mCapacity + std::min(mCapacity / 2 + 1, limit - mCapacity));
    }
    void relocate(T* target, size_type capacity) {
        size_type constructed = 0;
        try {
            for (; constructed < mSize; ++constructed)
                Traits::construct(mAllocator, target + constructed, std::move_if_noexcept(mData[constructed]));
        } catch (...) { destroyRange(mAllocator, target, constructed); throw; }
        destroyRange(mAllocator, mData, mSize);
        if (!is_inline()) Traits::deallocate(mAllocator, mData, mCapacity);
        mData = target; mCapacity = capacity;
    }
    void reallocate(size_type count) {
        auto* target = Traits::allocate(mAllocator, count);
        try { relocate(target, count); }
        catch (...) { Traits::deallocate(mAllocator, target, count); throw; }
    }
    template<class Construct> void growInsert(size_type index, size_type count, Construct&& construct) {
        if (count > max_size() - mSize) throw std::length_error("SmallVector size overflow");
        const auto capacity = nextCapacity(mSize + count);
        auto* target = Traits::allocate(mAllocator, capacity);
        size_type inserted = 0, prefix = 0, suffix = 0;
        try {
            // Construct the new values before moving any aliased arguments.
            for (; inserted < count; ++inserted) construct(target + index + inserted, inserted);
            for (; prefix < index; ++prefix)
                Traits::construct(mAllocator, target + prefix, std::move_if_noexcept(mData[prefix]));
            for (; suffix < mSize - index; ++suffix)
                Traits::construct(mAllocator, target + index + count + suffix, std::move_if_noexcept(mData[index + suffix]));
        } catch (...) {
            destroyRange(mAllocator, target + index + count, suffix);
            destroyRange(mAllocator, target, prefix);
            destroyRange(mAllocator, target + index, inserted);
            Traits::deallocate(mAllocator, target, capacity); throw;
        }
        destroyRange(mAllocator, mData, mSize);
        if (!is_inline()) Traits::deallocate(mAllocator, mData, mCapacity);
        mData = target; mCapacity = capacity; mSize += count;
    }
    template<class Construct> void resizeWith(size_type count, Construct&& construct) {
        if (count > max_size()) throw std::length_error("SmallVector size overflow");
        if (count <= mSize) { while (mSize > count) pop_back(); return; }
        if (count > mCapacity) { growInsert(mSize, count - mSize, construct); return; }
        const auto oldSize = mSize;
        try { while (mSize < count) { construct(mData + mSize, mSize - oldSize); ++mSize; } }
        catch (...) { while (mSize > oldSize) pop_back(); throw; }
    }
    void moveFrom(SmallVector& other) {
        assert(empty() && is_inline());
        if (!other.is_inline() && mAllocator == other.mAllocator) {
            mData = other.mData; mSize = other.mSize; mCapacity = other.mCapacity;
            other.mData = other.inlineData(); other.mSize = 0; other.mCapacity = N;
        } else {
            reserve(other.size());
            for (auto& value : other) emplace_back(std::move_if_noexcept(value));
            other.clear();
        }
    }
#if __has_cpp_attribute(msvc::no_unique_address)
    [[msvc::no_unique_address]]
#elif __has_cpp_attribute(no_unique_address)
    [[no_unique_address]]
#endif
    Allocator mAllocator;
    alignas(T) std::byte mInline[N ? sizeof(T) * N : 1];
    T* mData = inlineData();
    size_type mSize = 0;
    size_type mCapacity = N;
};
}
