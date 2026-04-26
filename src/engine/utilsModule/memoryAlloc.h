#pragma once

#include <cstdio>

namespace IKIGAI::UTILS {
	class IAllocator {
	public:
		virtual ~IAllocator() = default;
		virtual void* allocate(size_t size) = 0;
		virtual void deallocate(void* ptr) = 0;
	};

	class Memory {
	public:
		static void* Allocate(const std::size_t size);
		static void* Reallocate(void* const ptr, const std::size_t size);
		static void Deallocate(void* const ptr);
	};
}