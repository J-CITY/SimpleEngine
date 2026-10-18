#pragma once

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <new>
#include <stdexcept>

namespace IKIGAI::MEMORY {
enum class Category { General, Node, Component, Texture, Mesh, Model, Shader, Material, Audio, Skeleton, Font, Count };

struct AllocatorStats {
	std::size_t reserved = 0;
	std::size_t used = 0;
	std::size_t peak = 0;
	std::size_t allocations = 0;
};

class IAllocator {
public:
	virtual ~IAllocator() = default;
	virtual void* allocate(std::size_t bytes) = 0;
	// Existing user allocators remain source compatible. Over-aligned objects
	// require an allocator which explicitly implements this overload.
	virtual void* allocate(std::size_t bytes, std::size_t alignment) {
		if (alignment > alignof(std::max_align_t))
			throw std::bad_alloc();
		return allocate(bytes);
	}
	virtual void deallocate(void* ptr) = 0;
	virtual AllocatorStats stats() const {
		return { };
	}
	// Allows layered arenas to avoid counting a tracked upstream page a second
	// time as logical suballocations. This property is immutable for a domain.
	virtual bool tracksAllocations() const noexcept {
		return false;
	}
};

namespace Detail {
	inline std::size_t CheckedAdd(std::size_t a, std::size_t b) {
		if (b > std::numeric_limits<std::size_t>::max() - a)
			throw std::bad_alloc();
		return a + b;
	}
	inline std::size_t CheckedMultiply(std::size_t a, std::size_t b) {
		if (a && b > std::numeric_limits<std::size_t>::max() / a)
			throw std::bad_alloc();
		return a * b;
	}
	inline std::size_t ValidateAlignment(std::size_t alignment) {
		if (!alignment || (alignment & (alignment - 1)))
			throw std::invalid_argument("Alignment must be a power of two");
		return alignment;
	}
	inline std::size_t AlignUp(std::size_t value, std::size_t alignment) {
		ValidateAlignment(alignment);
		return CheckedAdd(value, alignment - 1) & ~(alignment - 1);
	}
	inline std::size_t Padding(std::uintptr_t address, std::size_t alignment) {
		ValidateAlignment(alignment);
		return (alignment - (address & (alignment - 1))) & (alignment - 1);
	}
}
}

namespace IKIGAI::UTILS {
using IAllocator = MEMORY::IAllocator;
}
