#pragma once
#include "allocator.h"
#include <cstdlib>
#include <cstring>

namespace IKIGAI::MEMORY {
// malloc backend corresponding to upstream CAllocator; a header makes aligned
// deallocation and the legacy reallocate facade independent of platform APIs.
class HeapAllocator final : public IAllocator {
	struct Header {
		void* base;
		std::size_t size;
	};

public:
	void* allocate(std::size_t bytes) override {
		return allocate(bytes, alignof(std::max_align_t));
	}
	void* allocate(std::size_t bytes, std::size_t alignment) override {
		alignment = std::max(Detail::ValidateAlignment(alignment), alignof(Header));
		const auto total = Detail::CheckedAdd(Detail::CheckedAdd(std::max(bytes, std::size_t { 1 }), sizeof(Header)), alignment - 1);
		void* base = std::malloc(total);
		if (!base)
			throw std::bad_alloc();
		auto address = reinterpret_cast<std::uintptr_t>(base) + sizeof(Header);
		address += Detail::Padding(address, alignment);
		auto* header = reinterpret_cast<Header*>(address) - 1;
		::new (header) Header { base, bytes };
		return reinterpret_cast<void*>(address);
	}
	void deallocate(void* ptr) override {
		if (ptr)
			std::free((static_cast<Header*>(ptr) - 1)->base);
	}
	static std::size_t size(const void* ptr) {
		return ptr ? (static_cast<const Header*>(ptr) - 1)->size : 0;
	}
};
}
