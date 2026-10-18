#include <chrono>
#include <deque>
#include <iostream>
#include <map>
#include <string>
#include <unordered_map>
#include "utilsModule/containers/chunkedVector.h"
#include "utilsModule/containers/flatMap.h"
#include "utilsModule/containers/internedNameMap.h"
#include "utilsModule/containers/smallVector.h"

using namespace IKIGAI;
struct CountingHeap : MEMORY::IAllocator {
    MEMORY::HeapAllocator heap;
    std::size_t count = 0, bytes = 0, peak = 0;
    void* allocate(std::size_t n) override { return allocate(n, alignof(std::max_align_t)); }
    void* allocate(std::size_t n, std::size_t a) override {
        auto* ptr = heap.allocate(n, a); ++count; bytes += n; peak = std::max(peak, bytes); return ptr;
    }
    void deallocate(void* ptr) override { bytes -= MEMORY::HeapAllocator::size(ptr); heap.deallocate(ptr); }
};
volatile std::size_t checksumSink = 0;
template<class F> void Measure(const char* name, std::size_t size, const std::shared_ptr<CountingHeap>& domain, F&& fn) {
    const auto start = std::chrono::steady_clock::now();
    const auto checksum = fn();
    const auto elapsed = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::steady_clock::now() - start).count();
    checksumSink = checksum;
    std::cout << name << ',' << size << ',' << elapsed << ',' << checksum << ',' << domain->count << ',' << domain->peak << '\n';
}
template<class Map> void Maps(const char* name, std::size_t size) {
    auto domain = std::make_shared<CountingHeap>();
    std::vector<std::string> keys;
    for (std::size_t i = 0; i < size; ++i) keys.push_back("engine_uniform_" + std::to_string(i));
    Map values{typename Map::allocator_type(domain)};
    Measure((std::string(name) + "_build").c_str(), size, domain, [&] {
        for (std::size_t i = 0; i < size; ++i) values[keys[i]] = i;
        return values.size();
    });
    Measure((std::string(name) + "_hit").c_str(), size, domain, [&] {
        std::size_t sum = 0; for (std::size_t i = 0; i < 100000; ++i) sum += values.at(keys[i % size]); return sum;
    });
    Measure((std::string(name) + "_miss").c_str(), size, domain, [&] {
        std::size_t sum = 0; for (int i = 0; i < 100000; ++i) sum += values.contains("missing_uniform"); return sum;
    });
    Measure((std::string(name) + "_churn").c_str(), size, domain, [&] {
        std::size_t sum = 0; for (std::size_t i = 0; i < 10000; ++i) { auto& key = keys[i % size]; sum += values.erase(key); values[key] = i; } return sum;
    });
}
template<class Vector> void Sequence(const char* name, std::size_t size, bool reserve) {
    auto domain = std::make_shared<CountingHeap>(); Vector values{typename Vector::allocator_type(domain)};
    Measure(name, size, domain, [&] {
        if constexpr (requires { values.reserve(size); }) { if (reserve) values.reserve(size); }
        for (std::size_t i = 0; i < size; ++i) values.push_back(i);
        std::size_t sum = 0; for (int pass = 0; pass < 50; ++pass) for (auto value : values) sum += value; return sum;
    });
}
template<class Vector> void ShortLists(const char* name, std::size_t size) {
    auto domain = std::make_shared<CountingHeap>();
    Measure(name, size, domain, [&] {
        std::size_t sum = 0;
        for (std::size_t pass = 0; pass < 10000; ++pass) {
            Vector values{typename Vector::allocator_type(domain)};
            for (std::size_t i = 0; i < size; ++i) values.push_back(i + pass);
            for (auto value : values) sum += value;
        }
        return sum;
    });
}
int main() {
    std::cout << "case,size,elapsed_ns,checksum,allocations_cumulative,peak_requested_bytes\n";
    for (std::size_t size : {8, 32, 128, 1024}) {
        Maps<UTILS::FlatMap<std::string, std::size_t>>("flat", size);
        Maps<std::map<std::string, std::size_t, std::less<>, MEMORY::StlAllocator<std::pair<const std::string, std::size_t>>>>("tree", size);
        Maps<std::unordered_map<std::string, std::size_t, std::hash<std::string>, std::equal_to<>, MEMORY::StlAllocator<std::pair<const std::string, std::size_t>>>>("hash", size);
    }
    Sequence<UTILS::ChunkedVector<std::size_t>>("chunks", 20000, false);
    Sequence<std::vector<std::size_t, MEMORY::StlAllocator<std::size_t>>>("vector_reserved", 20000, true);
    Sequence<std::vector<std::size_t, MEMORY::StlAllocator<std::size_t>>>("vector_growth", 20000, false);
    Sequence<std::deque<std::size_t, MEMORY::StlAllocator<std::size_t>>>("deque", 20000, false);
    for (std::size_t size : {0, 1, 4, 8, 9, 32}) {
        ShortLists<UTILS::SmallVector<std::size_t, 8>>("small_vector_8", size);
        ShortLists<std::vector<std::size_t, MEMORY::StlAllocator<std::size_t>>>("short_std_vector", size);
    }
    auto domain = std::make_shared<CountingHeap>();
    UTILS::StringPool strings(domain, 4096);
    Measure("intern_unique", 10000, domain, [&] { for (int i = 0; i < 10000; ++i) strings.intern("uniform_" + std::to_string(i)); return strings.size(); });
    Measure("intern_repeat", 10000, domain, [&] { std::size_t sum = 0; for (int i = 0; i < 100000; ++i) sum += strings.intern("uniform_42").entry().value(); return sum; });
    auto symbols = std::make_shared<CountingHeap>();
    UTILS::InternedNameMap<std::size_t> names(std::make_shared<UTILS::SynchronizedStringPool>(symbols), symbols);
    std::vector<std::string> queries;
    std::vector<UTILS::Identifier> ids;
    for (int i = 0; i < 128; ++i) { queries.push_back("uniform_" + std::to_string(i)); names[queries.back()] = i; ids.push_back(names.identifier(queries.back())); }
    Measure("symbol_cached_lookup", 128, symbols, [&] { std::size_t sum = 0; for (int i = 0; i < 100000; ++i) sum += names.at(ids[(i * 73) % 128]); return sum; });
    Measure("symbol_string_lookup", 128, symbols, [&] { std::size_t sum = 0; for (int i = 0; i < 100000; ++i) sum += names.at(queries[(i * 73) % 128]); return sum; });
}
