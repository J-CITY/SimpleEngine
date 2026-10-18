#include "memory.h"
#include <cstring>

namespace IKIGAI::MEMORY {
namespace {
	auto& heap() {
		using Allocator = SynchronizedAllocator<OptionalProfiledAllocator<HeapAllocator, Category::General>>;
		static Allocator allocator;
		return allocator;
	}
}
void* Memory::Allocate(std::size_t bytes) {
	return heap().allocate(bytes, alignof(std::max_align_t));
}
void Memory::Deallocate(void* ptr) {
	heap().deallocate(ptr);
}
void* Memory::Reallocate(void* ptr, std::size_t bytes) {
	if (!ptr)
		return Allocate(bytes);
	if (!bytes) {
		Deallocate(ptr);
		return nullptr;
	}
	// Allocation happens first: failure preserves the original memory/record.
	void* result = Allocate(bytes);
	std::memcpy(result, ptr, std::min(bytes, HeapAllocator::size(ptr)));
	Deallocate(ptr);
	return result;
}
}
