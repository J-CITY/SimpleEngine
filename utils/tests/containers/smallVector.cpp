#include "utilsModule/containers/smallVector.h"
#include <array>
#include <iostream>
#include <memory>
#include <numeric>
#include <random>
#include <sstream>
#include <span>
#include <string>
#include <unordered_map>
#include <vector>

namespace {
using namespace IKIGAI;
using namespace IKIGAI::UTILS;
using namespace IKIGAI::MEMORY;
#define CHECK(...) do { if (!(__VA_ARGS__)) throw std::runtime_error("SmallVector check at line " + std::to_string(__LINE__) + ": " #__VA_ARGS__); } while (false)
template<class Exception, class F> void Throws(F&& fn) {
    try { fn(); } catch (const Exception&) { return; }
    throw std::runtime_error("SmallVector expected exception");
}
struct Domain final : IAllocator {
    HeapAllocator heap;
    std::unordered_map<void*, std::size_t> live;
    std::size_t allocations = 0;
    int failAfter = -1;
    ~Domain() { if (!live.empty()) std::terminate(); }
    void* allocate(std::size_t n) override { return allocate(n, alignof(std::max_align_t)); }
    void* allocate(std::size_t n, std::size_t alignment) override {
        if (failAfter == 0) throw std::bad_alloc();
        if (failAfter > 0) --failAfter;
        auto* ptr = heap.allocate(n, alignment);
        try { live.emplace(ptr, n); } catch (...) { heap.deallocate(ptr); throw; }
        ++allocations; return ptr;
    }
    void deallocate(void* ptr) override {
        if (!live.erase(ptr)) std::terminate();
        heap.deallocate(ptr);
    }
};
template<class T, bool Copy = false, bool Move = false, bool Swap = false>
struct TestAllocator {
    using value_type = T;
    using propagate_on_container_copy_assignment = std::bool_constant<Copy>;
    using propagate_on_container_move_assignment = std::bool_constant<Move>;
    using propagate_on_container_swap = std::bool_constant<Swap>;
    template<class U> struct rebind { using other = TestAllocator<U, Copy, Move, Swap>; };
    std::shared_ptr<Domain> domain;
    explicit TestAllocator(std::shared_ptr<Domain> d) : domain(std::move(d)) {}
    template<class U> TestAllocator(const TestAllocator<U, Copy, Move, Swap>& other) : domain(other.domain) {}
    T* allocate(std::size_t n) { return static_cast<T*>(domain->allocate(n * sizeof(T), alignof(T))); }
    void deallocate(T* ptr, std::size_t) { domain->deallocate(ptr); }
    template<class U> bool operator==(const TestAllocator<U, Copy, Move, Swap>& other) const noexcept { return domain == other.domain; }
};
struct alignas(128) Value {
    static inline int live = 0, copiesUntilThrow = -1, assignmentsUntilThrow = -1;
    int value;
    static void MaybeThrow(int& remaining) {
        if (remaining == 0) throw std::runtime_error("injected failure");
        if (remaining > 0) --remaining;
    }
    explicit Value(int v = 0) : value(v) { if (v < 0) throw std::runtime_error("constructor"); ++live; }
    Value(const Value& other) : value(other.value) { MaybeThrow(copiesUntilThrow); ++live; }
    Value(Value&& other) noexcept(false) : value(other.value) { MaybeThrow(copiesUntilThrow); other.value = -2; ++live; }
    Value& operator=(const Value& other) { MaybeThrow(assignmentsUntilThrow); value = other.value; return *this; }
    Value& operator=(Value&& other) { MaybeThrow(assignmentsUntilThrow); value = other.value; other.value = -2; return *this; }
    ~Value() { --live; }
};
struct ThrowingMoveOnly {
    static inline int live = 0, movesUntilThrow = -1;
    int value;
    explicit ThrowingMoveOnly(int v) : value(v) { ++live; }
    ThrowingMoveOnly(const ThrowingMoveOnly&) = delete;
    ThrowingMoveOnly(ThrowingMoveOnly&& other) : value(other.value) {
        other.value = -1; Value::MaybeThrow(movesUntilThrow); ++live;
    }
    ~ThrowingMoveOnly() { --live; }
};
struct CopyOnly {
    int value;
    explicit CopyOnly(int v) : value(v) {}
    CopyOnly(const CopyOnly&) = default;
    CopyOnly(CopyOnly&&) = delete;
    CopyOnly& operator=(const CopyOnly&) = delete;
};

void StorageAndApi() {
    static_assert(std::contiguous_iterator<SmallVector<int, 4>::iterator>);
    static_assert(std::ranges::contiguous_range<SmallVector<int, 4>>);
    static_assert(std::is_same_v<decltype(std::declval<const SmallVector<int, 4>&>()[0]), const int&>);
    static_assert(!std::is_copy_constructible_v<SmallVector<std::unique_ptr<int>, 4>>);
    static_assert(std::is_nothrow_move_constructible_v<SmallVector<int, 4>>);
    static_assert(std::is_nothrow_move_assignable_v<SmallVector<int, 4>>);
    static_assert(std::is_nothrow_move_constructible_v<SmallVector<Value, 0>>);
    static_assert(!std::is_nothrow_move_constructible_v<SmallVector<Value, 4>>);
    static_assert(sizeof(SmallVector<std::array<int, 1024>, 0>) < sizeof(std::array<int, 1024>));
    auto domain = std::make_shared<Domain>();
    using V = SmallVector<int, 4>;
    {
        struct Aggregate { V values; } aggregate{};
        CHECK(aggregate.values.empty() && aggregate.values.is_inline());
        V values{StlAllocator<int>(domain)};
        CHECK(values.capacity() == 4 && values.is_inline() && values.begin() == values.end());
        auto* data = values.data();
        domain->failAfter = 0;
        for (int i = 0; i < 4; ++i) values.push_back(i);
        CHECK(values.data() == data && domain->allocations == 0);
        Throws<std::bad_alloc>([&] { values.push_back(4); });
        CHECK(values.size() == 4 && values.data() == data && values.back() == 3);
        domain->failAfter = -1;
        values.push_back(values.front()); // inline -> heap with aliased argument
        CHECK(!values.is_inline() && values.back() == 0 && domain->allocations == 1);
        std::span<int> view(values); CHECK(view.size() == 5 && view.data() == values.data());
        values.reserve(16); data = values.data(); values.push_back(8); CHECK(values.data() == data);
        values.resize(3); values.shrink_to_fit(); CHECK(values.is_inline() && domain->live.empty());
        values.resize(8, values[1]); CHECK(values.back() == 1 && values.front() == 0);
        values.resize(10); CHECK(values.back() == 0);
        values.shrink_to_fit(); CHECK(values.capacity() == values.size());
        data = values.data(); auto capacity = values.capacity();
        values.clear(); CHECK(values.data() == data && values.capacity() == capacity);
        values.shrink_to_fit(); CHECK(values.is_inline() && domain->live.empty());
        Throws<std::out_of_range>([&] { values.at(0); });
        Throws<std::length_error>([&] { values.reserve(std::numeric_limits<std::size_t>::max()); });
        Throws<std::length_error>([&] { values.resize(std::numeric_limits<std::size_t>::max()); });
        values = {3, 1, 4}; std::sort(values.begin(), values.end()); CHECK(values[0] == 1 && values.back() == 4);
        CHECK(*values.crbegin() == 4 && std::accumulate(values.cbegin(), values.cend(), 0) == 8);
        values.assign(values.begin() + 1, values.end()); CHECK(values.size() == 2 && values[0] == 3);
        values.assign(4, values[1]); CHECK(values.size() == 4 && values.front() == 4);
        auto copy = values; CHECK(copy.is_inline() && copy.size() == 4);
        copy = copy; copy = std::move(copy); CHECK(copy.size() == 4);
        SmallVector<int, 4> count(3); CHECK(count.size() == 3 && count.back() == 0);
        SmallVector<int, 4> fill(3, 12); CHECK(fill.back() == 12);
        std::istringstream input("8 9 10 11 12");
        SmallVector<int, 4> streamed{std::istream_iterator<int>(input), std::istream_iterator<int>()};
        CHECK(streamed.size() == 5 && streamed.front() == 8 && streamed.back() == 12);
        SmallVector<int, 0> zero{StlAllocator<int>(domain)};
        CHECK(zero.is_inline() && zero.capacity() == 0);
        zero.push_back(17); CHECK(!zero.is_inline() && zero[0] == 17);
        zero.clear(); zero.shrink_to_fit(); CHECK(zero.is_inline() && zero.capacity() == 0);
    }
    CHECK(domain->live.empty());
    {
        SmallVector<Value, 2> aligned{StlAllocator<Value>(domain)};
        aligned.emplace_back(1); CHECK(reinterpret_cast<std::uintptr_t>(aligned.data()) % 128 == 0);
        aligned.emplace_back(2); aligned.emplace_back(3);
        CHECK(reinterpret_cast<std::uintptr_t>(aligned.data()) % 128 == 0);
        aligned.resize(1); aligned.shrink_to_fit(); CHECK(aligned.is_inline() && aligned[0].value == 1);
        CHECK(Value::live == 1);
        SmallVector<CopyOnly, 1> nonassignable;
        nonassignable.emplace_back(1); nonassignable.emplace_back(2); nonassignable.reserve(10);
        auto moved = std::move(nonassignable); CHECK(moved[1].value == 2 && nonassignable.empty());
        SmallVector<std::unique_ptr<int>, 2> pointers;
        pointers.emplace_back(std::make_unique<int>(7)); pointers.emplace(pointers.begin(), std::make_unique<int>(9));
        pointers.emplace(pointers.begin(), std::make_unique<int>(11)); pointers.erase(pointers.begin() + 1);
        auto movedPointers = std::move(pointers); CHECK(*movedPointers[0] == 11 && *movedPointers[1] == 7);
        pointers.emplace_back(std::make_unique<int>(12)); CHECK(*pointers.back() == 12);
    }
    CHECK(Value::live == 0 && domain->live.empty());
    SmallVector<std::string, 2> alias{"first", "second"};
    alias.emplace(alias.begin(), alias.back()); CHECK(alias[0] == "second" && alias[2] == "second");
    alias.reserve(10); alias.emplace(alias.begin() + 1, alias.back()); CHECK(alias[1] == "second");
    alias.resize(alias.capacity(), "tail"); alias.push_back(std::move(alias[1])); CHECK(alias.back() == "second");
}

void RandomOperations() {
    SmallVector<int, 8> values;
    std::vector<int> reference;
    std::mt19937 random(91);
    for (int step = 0; step < 8000; ++step) {
        const auto index = static_cast<std::size_t>(random()) % (values.size() + 1);
        const auto value = static_cast<int>(random() % 1000);
        switch (random() % 8) {
        case 0: values.push_back(value); reference.push_back(value); break;
        case 1: values.insert(values.begin() + index, value); reference.insert(reference.begin() + index, value); break;
        case 2: if (!values.empty()) { auto i = index % values.size(); values.erase(values.begin() + i); reference.erase(reference.begin() + i); } break;
        case 3: { auto end = index + random() % (values.size() - index + 1); values.erase(values.begin() + index, values.begin() + end); reference.erase(reference.begin() + index, reference.begin() + end); break; }
        case 4: { auto size = random() % 25; values.resize(size, value); reference.resize(size, value); break; }
        case 5: values.reserve(random() % 40); break;
        case 6: values.shrink_to_fit(); break;
        case 7: if (!values.empty()) { values.pop_back(); reference.pop_back(); } break;
        }
        CHECK(values.size() == reference.size());
        CHECK(std::equal(values.begin(), values.end(), reference.begin()));
    }
}

void DomainsAndSwap() {
    auto a = std::make_shared<Domain>(), b = std::make_shared<Domain>();
    for (int left : {0, 1, 2, 4}) for (int right : {0, 1, 2, 4}) {
        using V = SmallVector<Value, 2>;
        V x{StlAllocator<Value>(a)}, y{StlAllocator<Value>(b)};
        for (int i = 0; i < left; ++i) x.emplace_back(10 + i);
        for (int i = 0; i < right; ++i) y.emplace_back(20 + i);
        const auto allocations = a->allocations + b->allocations;
        x.swap(y);
        CHECK(x.size() == right && y.size() == left);
        CHECK(x.get_allocator().domain() == b && y.get_allocator().domain() == a);
        if (right) CHECK(x.front().value == 20);
        if (left) CHECK(y.front().value == 10);
        CHECK(a->allocations + b->allocations == allocations);
        x = y; CHECK(x.get_allocator().domain() == b && x.size() == left);
        x = std::move(y); CHECK(x.get_allocator().domain() == a && y.empty());
        y.emplace_back(42); CHECK(y.back().value == 42);
    }
    CHECK(a->live.empty() && b->live.empty() && Value::live == 0);
    for (int left : {1, 5}) for (int right : {1, 5}) {
        using A = TestAllocator<Value>;
        using V = SmallVector<Value, 2, A>;
        V x{A(a)}, y{A(b)};
        for (int i = 0; i < left; ++i) x.emplace_back(10 + i);
        for (int i = 0; i < right; ++i) y.emplace_back(20 + i);
        Throws<std::invalid_argument>([&] { x.swap(y); });
        y = x; CHECK(y.get_allocator().domain == b && y.size() == left);
        y = std::move(x); CHECK(y.get_allocator().domain == b && y[0].value == 10 && x.empty());
        V moved(std::move(y), A(a)); CHECK(moved[0].value == 10 && y.empty());
        V copy(moved, A(b)); CHECK(copy.size() == left && copy.get_allocator().domain == b);
        V same{A(a)}; same.emplace_back(71); moved.swap(same); CHECK(moved[0].value == 71 && same.size() == left);
    }
    {
        using A = TestAllocator<int, true, true, true>;
        SmallVector<int, 2, A> x(5, 8, A(a)), y(1, 9, A(b));
        y = x; CHECK(y.get_allocator().domain == a && y.size() == 5);
        SmallVector<int, 2, A> moved(1, 7, A(b)); moved = std::move(x);
        CHECK(moved.get_allocator().domain == a && moved[0] == 8 && x.empty());
    }
    CHECK(a->live.empty() && b->live.empty() && Value::live == 0);
}

void Exceptions() {
    auto domain = std::make_shared<Domain>();
    // Exercise each construction position, including after some old values copied.
    for (int failure = 0; failure < 5; ++failure) {
        SmallVector<Value, 2> values{StlAllocator<Value>(domain)};
        values.emplace_back(10); values.emplace_back(20);
        auto* original = values.data();
        Value::copiesUntilThrow = failure;
        try { values.push_back(values[0]); } catch (const std::runtime_error&) {
            CHECK(values.size() == 2 && values.data() == original && values[0].value == 10 && values[1].value == 20);
        }
        Value::copiesUntilThrow = -1;
        CHECK(Value::live == static_cast<int>(values.size()));
    }
    CHECK(Value::live == 0 && domain->live.empty());
    for (int failure = 0; failure < 7; ++failure) {
        SmallVector<Value, 2> values{StlAllocator<Value>(domain)};
        values.emplace_back(10); values.emplace_back(20);
        auto* original = values.data();
        Value::copiesUntilThrow = failure;
        try { values.resize(6, values[0]); } catch (const std::runtime_error&) {
            CHECK(values.size() == 2 && values.data() == original && values[1].value == 20);
        }
        Value::copiesUntilThrow = -1;
        CHECK(Value::live == static_cast<int>(values.size()));
    }
    for (int failure = 0; failure < 3; ++failure) {
        SmallVector<Value, 2> values{StlAllocator<Value>(domain)};
        values.emplace_back(10); values.emplace_back(20); values.reserve(10);
        auto* original = values.data();
        Value::copiesUntilThrow = failure;
        try { values.shrink_to_fit(); } catch (const std::runtime_error&) {
            CHECK(values.data() == original && values.size() == 2 && values[0].value == 10);
        }
        Value::copiesUntilThrow = -1;
    }
    for (int failure = 0; failure < 4; ++failure) {
        SmallVector<Value, 4> values{StlAllocator<Value>(domain)};
        values.emplace_back(10); Value fill(20);
        Value::copiesUntilThrow = failure;
        try { values.resize(4, fill); } catch (const std::runtime_error&) {
            CHECK(values.size() == 1 && values[0].value == 10 && values.is_inline());
        }
        Value::copiesUntilThrow = -1;
        CHECK(Value::live == static_cast<int>(values.size()) + 1);
    }
    {
        SmallVector<Value, 2> values{StlAllocator<Value>(domain)};
        values.emplace_back(10); values.emplace_back(20);
        auto* original = values.data();
        Throws<std::runtime_error>([&] { values.emplace(values.begin(), -1); });
        CHECK(values.data() == original && values.size() == 2 && values[0].value == 10);
        Value::copiesUntilThrow = 1;
        Throws<std::runtime_error>([&] { values.reserve(10); });
        Value::copiesUntilThrow = -1;
        CHECK(values.data() == original && values[1].value == 20 && Value::live == 2);
    }
    {
        SmallVector<Value, 4> values{StlAllocator<Value>(domain)};
        values.emplace_back(10); values.emplace_back(20); values.emplace_back(30);
        Value::assignmentsUntilThrow = 1;
        Throws<std::runtime_error>([&] { values.emplace(values.begin(), 5); });
        Value::assignmentsUntilThrow = -1; CHECK(Value::live == 3 && values.size() == 3);
        values.clear(); values.emplace_back(1); values.emplace_back(2);
        Value::assignmentsUntilThrow = 0;
        Throws<std::runtime_error>([&] { values.erase(values.begin()); });
        Value::assignmentsUntilThrow = -1; CHECK(values.size() == 2 && Value::live == 2);
        Throws<std::runtime_error>([&] { values.emplace_back(-1); }); CHECK(values.size() == 2);
        Value::copiesUntilThrow = 0;
        Throws<std::runtime_error>([&] { SmallVector<Value, 4> copy(values); });
        Throws<std::runtime_error>([&] { SmallVector<Value, 4> moved(std::move(values)); });
        Value::copiesUntilThrow = -1; CHECK(Value::live == 2 && values.size() == 2);
    }
    // Inline swaps may throw; neither allocation domain may lose its heap block.
    for (int left : {1, 2, 4}) for (int right : {1, 2, 4}) for (int failure = 0; failure < 4; ++failure) {
        auto otherDomain = std::make_shared<Domain>();
        SmallVector<Value, 2> x{StlAllocator<Value>(domain)}, y{StlAllocator<Value>(otherDomain)};
        for (int i = 0; i < left; ++i) x.emplace_back(10 + i);
        for (int i = 0; i < right; ++i) y.emplace_back(20 + i);
        Value::copiesUntilThrow = failure;
        try { x.swap(y); } catch (const std::runtime_error&) {
            CHECK(x.get_allocator().domain() == domain && y.get_allocator().domain() == otherDomain);
        }
        Value::copiesUntilThrow = -1;
        CHECK(Value::live == static_cast<int>(x.size() + y.size()));
    }
    // Copy assignment staging succeeds, but committing to inline storage fails.
    for (int failure = 0; failure < 5; ++failure) {
        SmallVector<Value, 2> x{StlAllocator<Value>(domain)}, y{StlAllocator<Value>(domain)};
        x.emplace_back(1); x.emplace_back(2); y.emplace_back(9);
        Value::copiesUntilThrow = failure;
        try { y = x; } catch (const std::runtime_error&) {}
        Value::copiesUntilThrow = -1;
        CHECK(x.size() == 2 && x[0].value == 1 && Value::live == static_cast<int>(x.size() + y.size()));
    }
    CHECK(Value::live == 0 && domain->live.empty());
    {
        SmallVector<ThrowingMoveOnly, 2> values;
        values.emplace_back(1); values.emplace_back(2);
        ThrowingMoveOnly::movesUntilThrow = 1;
        Throws<std::runtime_error>([&] { values.emplace_back(3); });
        ThrowingMoveOnly::movesUntilThrow = -1;
        CHECK(values.size() == 2 && ThrowingMoveOnly::live == 2);
        values.clear(); values.emplace_back(4); CHECK(values[0].value == 4);
    }
    CHECK(ThrowingMoveOnly::live == 0);
    // A failing allocator construct hook must prevent a noexcept move promise.
    struct HookAllocator : std::allocator<int> {
        void construct(int*, int&&) { throw std::runtime_error("allocator construct hook"); }
    };
    static_assert(!std::is_nothrow_move_constructible_v<SmallVector<int, 2, HookAllocator>>);
}
}
void TestSmallVector() { StorageAndApi(); RandomOperations(); DomainsAndSwap(); Exceptions(); }
