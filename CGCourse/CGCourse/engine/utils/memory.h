#pragma once

#include <cstdio>
#include <cstdint>

namespace KUMA::MEMORY {
	class Memory {
	public:
		static void* Allocate(const std::size_t size);
		static void* Reallocate(void* const ptr, const std::size_t size);
		static void Deallocate(void* const ptr);
	};
}