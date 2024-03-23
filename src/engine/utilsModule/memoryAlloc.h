#pragma once

#include <cstdio>

namespace IKIGAI::UTILS {
	class Memory {
	public:
		static void* Allocate(const std::size_t size);
		static void* Reallocate(void* const ptr, const std::size_t size);
		static void Deallocate(void* const ptr);
	};
}