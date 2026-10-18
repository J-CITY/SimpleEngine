#pragma once
#include "utilsModule/memory/resourceAllocator.h"
#include <algorithm>
#include <functional>
#include <iterator>
#include <tuple>
#include <vector>

namespace IKIGAI::UTILS {
// Ordered flat map (not a hash table). Mutations may invalidate all references.
// Iterators return pair<const Key&, Value&> proxies, protecting the key ordering.
template <class Key, class Value, class Compare = std::less<>, class Allocator = MEMORY::StlAllocator<std::pair<Key, Value>>> class FlatMap {
	using Item = std::pair<Key, Value>;
	using Storage = std::vector<Item, Allocator>;
	using Traits = std::allocator_traits<Allocator>;
	static constexpr bool SafeMove = std::is_nothrow_move_constructible_v<Item> && std::is_nothrow_move_assignable_v<Item>;
	static_assert(SafeMove || std::is_copy_constructible_v<Item>, "FlatMap requires nonthrowing relocation or copyable entries");
	static_assert(std::is_nothrow_swappable_v<Compare>, "Comparator swap must not throw");

public:
	using key_type = Key;
	using mapped_type = Value;
	using value_type = Item;
	using allocator_type = Allocator;
	using size_type = std::size_t;
	template <bool Const> class BasicIterator {
		friend class FlatMap;
		template <bool> friend class BasicIterator;
		using Owner = std::conditional_t<Const, const FlatMap, FlatMap>;
		Owner* mOwner = nullptr;
		size_type mIndex = 0;
		BasicIterator(Owner* owner, size_type index)
			: mOwner(owner)
			, mIndex(index) { }

	public:
		using difference_type = std::ptrdiff_t;
		using value_type = std::pair<Key, Value>;
		using reference = std::pair<const Key&, std::conditional_t<Const, const Value&, Value&>>;
		struct Arrow {
			reference value;
			const reference* operator->() const noexcept {
				return &value;
			}
		};
		using pointer = Arrow;
		using iterator_category = std::input_iterator_tag;
		BasicIterator() = default;
		template <bool C = Const>
			requires C
		BasicIterator(const BasicIterator<false>& other)
			: mOwner(other.mOwner)
			, mIndex(other.mIndex) { }
		reference operator*() const {
			auto& item = mOwner->mData[mIndex];
			return { item.first, item.second };
		}
		Arrow operator->() const {
			return { **this };
		}
		BasicIterator& operator++() {
			++mIndex;
			return *this;
		}
		BasicIterator operator++(int) {
			auto old = *this;
			++*this;
			return old;
		}
		BasicIterator& operator--() {
			--mIndex;
			return *this;
		}
		BasicIterator operator--(int) {
			auto old = *this;
			--*this;
			return old;
		}
		template <bool C> bool operator==(const BasicIterator<C>& other) const {
			return mOwner == other.mOwner && mIndex == other.mIndex;
		}
	};
	using iterator = BasicIterator<false>;
	using const_iterator = BasicIterator<true>;
	FlatMap()
		: FlatMap(Compare(), Allocator()) { }
	explicit FlatMap(const Compare& compare, const Allocator& allocator = Allocator())
		: mCompare(compare)
		, mData(allocator) { }
	explicit FlatMap(const Allocator& allocator)
		: FlatMap(Compare(), allocator) { }
	FlatMap(const FlatMap&) = default;
	FlatMap(FlatMap&&) = default;
	FlatMap& operator=(const FlatMap& other) {
		if (this == &other)
			return *this;
		auto allocator = get_allocator();
		if constexpr (Traits::propagate_on_container_copy_assignment::value)
			allocator = other.get_allocator();
		FlatMap copy(other.mCompare, allocator);
		copy.mData.assign(other.mData.begin(), other.mData.end());
		if constexpr (Traits::propagate_on_container_copy_assignment::value) {
			// Let the vector propagate its allocator before committing comparator.
			mData.clear();
			try {
				mData = copy.mData;
			} catch (...) {
				mData.clear();
				throw;
			}
			using std::swap;
			swap(mCompare, copy.mCompare);
		}
		else
			swap(copy);
		return *this;
	}
	FlatMap& operator=(FlatMap&& other) {
		if (this == &other)
			return *this;
		// Prepare the comparator before any entries move.
		Compare compare(other.mCompare);
		if constexpr (Traits::propagate_on_container_move_assignment::value)
			mData = std::move(other.mData);
		else if (get_allocator() == other.get_allocator())
			mData = std::move(other.mData);
		else {
			Storage replacement(get_allocator());
			replacement.reserve(other.size());
			for (auto& item : other.mData)
				replacement.push_back(std::move_if_noexcept(item));
			mData.swap(replacement);
			other.clear();
		}
		using std::swap;
		swap(mCompare, compare);
		return *this;
	}
	allocator_type get_allocator() const {
		return mData.get_allocator();
	}
	void swap(FlatMap& other) {
		if constexpr (!Traits::propagate_on_container_swap::value) {
			if (get_allocator() != other.get_allocator())
				throw std::invalid_argument("Incompatible allocation domains");
		}
		mData.swap(other.mData);
		using std::swap;
		swap(mCompare, other.mCompare);
	}
	friend void swap(FlatMap& a, FlatMap& b) {
		a.swap(b);
	}
	size_type size() const noexcept {
		return mData.size();
	}
	bool empty() const noexcept {
		return mData.empty();
	}
	size_type capacity() const noexcept {
		return mData.capacity();
	}
	void reserve(size_type n) {
		mData.reserve(n);
	}
	void clear() noexcept {
		mData.clear();
	}
	iterator begin() noexcept {
		return { this, 0 };
	}
	iterator end() noexcept {
		return { this, size() };
	}
	const_iterator begin() const noexcept {
		return { this, 0 };
	}
	const_iterator end() const noexcept {
		return { this, size() };
	}
	const_iterator cbegin() const noexcept {
		return begin();
	}
	const_iterator cend() const noexcept {
		return end();
	}
	template <class K> iterator find(const K& key) {
		auto i = lower(key);
		return { this, equalAt(i, key) ? i : size() };
	}
	template <class K> const_iterator find(const K& key) const {
		auto i = lower(key);
		return { this, equalAt(i, key) ? i : size() };
	}
	template <class K> bool contains(const K& key) const {
		return find(key) != end();
	}
	template <class K> Value& at(const K& key) {
		auto it = find(key);
		if (it == end())
			throw std::out_of_range("FlatMap key");
		return it->second;
	}
	template <class K> const Value& at(const K& key) const {
		auto it = find(key);
		if (it == end())
			throw std::out_of_range("FlatMap key");
		return it->second;
	}
	template <class K, class... Args> std::pair<iterator, bool> try_emplace(K&& key, Args&&... args) {
		if constexpr (!std::is_same_v<std::remove_cvref_t<K>, Key>) {
			Key normalized = static_cast<Key>(std::forward<K>(key));
			return try_emplace(std::move(normalized), std::forward<Args>(args)...);
		}
		else {
			const auto index = lower(key);
			if (equalAt(index, key))
				return { { this, index }, false };
			Item item(std::piecewise_construct, std::forward_as_tuple(std::forward<K>(key)), std::forward_as_tuple(std::forward<Args>(args)...));
			if constexpr (SafeMove) {
				mData.insert(mData.begin() + index, std::move(item));
			}
			else {
				Storage replacement(get_allocator());
				replacement.reserve(size() + 1);
				for (size_type i = 0; i < index; ++i)
					replacement.push_back(mData[i]);
				replacement.push_back(item);
				for (size_type i = index; i < size(); ++i)
					replacement.push_back(mData[i]);
				mData.swap(replacement);
			}
			return { { this, index }, true };
		}
	}
	template <class K, class V> std::pair<iterator, bool> insert_or_assign(K&& key, V&& value) {
		if constexpr (!std::is_same_v<std::remove_cvref_t<K>, Key>) {
			Key normalized = static_cast<Key>(std::forward<K>(key));
			return insert_or_assign(std::move(normalized), std::forward<V>(value));
		}
		else {
			auto it = find(key);
			if (it != end()) {
				it->second = std::forward<V>(value);
				return { it, false };
			}
			return try_emplace(std::forward<K>(key), std::forward<V>(value));
		}
	}
	Value& operator[](const Key& key) {
		return try_emplace(key).first->second;
	}
	Value& operator[](Key&& key) {
		return try_emplace(std::move(key)).first->second;
	}
	template <class K> size_type erase(const K& key) {
		const auto index = lower(key);
		if (!equalAt(index, key))
			return 0;
		if constexpr (SafeMove)
			mData.erase(mData.begin() + index);
		else {
			Storage replacement(get_allocator());
			replacement.reserve(size() - 1);
			for (size_type i = 0; i < index; ++i)
				replacement.push_back(mData[i]);
			for (size_type i = index + 1; i < size(); ++i)
				replacement.push_back(mData[i]);
			mData.swap(replacement);
		}
		return 1;
	}
	// Transactional bulk replacement; duplicates are errors, not silently lost.
	template <std::input_iterator It> void assign(It first, It last) {
		Storage replacement(get_allocator());
		for (; first != last; ++first) {
			auto&& item = *first;
			replacement.emplace_back(std::forward<decltype(item)>(item).first, std::forward<decltype(item)>(item).second);
		}
		std::sort(replacement.begin(), replacement.end(), [this](const Item& a, const Item& b) { return mCompare(a.first, b.first); });
		for (size_type i = 1; i < replacement.size(); ++i)
			if (!mCompare(replacement[i - 1].first, replacement[i].first))
				throw std::invalid_argument("Duplicate FlatMap key");
		mData.swap(replacement);
	}

private:
	template <class K> size_type lower(const K& key) const {
		return static_cast<size_type>(std::lower_bound(mData.begin(), mData.end(), key, [this](const Item& item, const K& value) {
			return mCompare(item.first, value);
		}) - mData.begin());
	}
	template <class K> bool equalAt(size_type index, const K& key) const {
		return index != size() && !mCompare(key, mData[index].first);
	}
	Compare mCompare;
	Storage mData;
};
}
