#pragma once
#include "resourceAllocator.h"

namespace IKIGAI::MEMORY {
class Memory {
public:
	static void* Allocate(std::size_t size);
	static void* Reallocate(void* ptr, std::size_t size);
	static void Deallocate(void* ptr);
};
}

namespace IKIGAI::UTILS {
using Memory = MEMORY::Memory;
template <class T, class... Args> std::shared_ptr<T> AllocateResource(IAllocator* allocator, std::function<void(T*)> customDeleter, Args&&... args) {
	return MEMORY::MakeSharedWithDeleter<T>(allocator, std::move(customDeleter), std::forward<Args>(args)...);
}
}
