#pragma once
// Adapted from mtrebi/memory-allocators, MIT; see LICENSE.mtrebi and README.md.
#include "heapAllocator.h"

namespace IKIGAI::MEMORY {
class LinearAllocator final : public IAllocator {
public:
	explicit LinearAllocator(std::size_t capacity)
		: mCapacity(capacity) {
		mData = static_cast<std::byte*>(mHeap.allocate(capacity));
	}
	~LinearAllocator() override {
		mHeap.deallocate(mData);
	}
	LinearAllocator(const LinearAllocator&) = delete;
	LinearAllocator& operator=(const LinearAllocator&) = delete;
	void* allocate(std::size_t bytes) override {
		return allocate(bytes, alignof(std::max_align_t));
	}
	void* allocate(std::size_t bytes, std::size_t alignment) override {
		const auto padding = Detail::Padding(reinterpret_cast<std::uintptr_t>(mData + mOffset), alignment);
		const auto required = Detail::CheckedAdd(padding, std::max(bytes, std::size_t { 1 }));
		if (required > mCapacity - mOffset)
			throw std::bad_alloc();
		void* result = mData + mOffset + padding;
		mOffset += required;
		mPeak = std::max(mPeak, mOffset);
		++mCount;
		return result;
	}
	// Individual frees intentionally do not reclaim arena storage.
	void deallocate(void*) override { }
	void reset() noexcept {
		mOffset = 0;
		mCount = 0;
	}
	AllocatorStats stats() const noexcept {
		return { mCapacity, mOffset, mPeak, mCount };
	}

private:
	HeapAllocator mHeap;
	std::byte* mData = nullptr;
	std::size_t mCapacity, mOffset = 0, mPeak = 0, mCount = 0;
};
}
