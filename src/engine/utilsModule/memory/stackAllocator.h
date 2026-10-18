#pragma once
// Adapted from mtrebi/memory-allocators, MIT; see LICENSE.mtrebi and README.md.
#include "heapAllocator.h"
#include <vector>

namespace IKIGAI::MEMORY {
class StackAllocator final : public IAllocator {
	struct Entry {
		void* ptr;
		std::size_t previousOffset;
	};

public:
	struct Marker {
		const StackAllocator* owner;
		std::size_t offset, count, generation;
	};
	explicit StackAllocator(std::size_t capacity)
		: mCapacity(capacity) {
		mData = static_cast<std::byte*>(mHeap.allocate(capacity));
	}
	~StackAllocator() override {
		mHeap.deallocate(mData);
	}
	StackAllocator(const StackAllocator&) = delete;
	StackAllocator& operator=(const StackAllocator&) = delete;
	void* allocate(std::size_t bytes) override {
		return allocate(bytes, alignof(std::max_align_t));
	}
	void* allocate(std::size_t bytes, std::size_t alignment) override {
		const auto padding = Detail::Padding(reinterpret_cast<std::uintptr_t>(mData + mOffset), alignment);
		const auto required = Detail::CheckedAdd(padding, std::max(bytes, std::size_t { 1 }));
		if (required > mCapacity - mOffset)
			throw std::bad_alloc();
		void* ptr = mData + mOffset + padding;
		mEntries.push_back({ ptr, mOffset });
		mOffset += required;
		mPeak = std::max(mPeak, mOffset);
		return ptr;
	}
	void deallocate(void* ptr) override {
		if (!ptr)
			return;
		if (mEntries.empty() || mEntries.back().ptr != ptr)
			throw std::logic_error("StackAllocator requires LIFO deallocation");
		mOffset = mEntries.back().previousOffset;
		mEntries.pop_back();
	}
	Marker marker() const noexcept {
		return { this, mOffset, mEntries.size(), mGeneration };
	}
	void rewind(Marker marker) {
		if (marker.owner != this || marker.generation != mGeneration || marker.count > mEntries.size() || marker.offset > mOffset)
			throw std::invalid_argument("Invalid stack marker");
		if (marker.count < mEntries.size() && mEntries[marker.count].previousOffset != marker.offset)
			throw std::invalid_argument("Stale stack marker");
		mEntries.resize(marker.count);
		mOffset = marker.offset;
	}
	void reset() noexcept {
		mEntries.clear();
		mOffset = 0;
		++mGeneration;
	}
	AllocatorStats stats() const noexcept {
		return { mCapacity, mOffset, mPeak, mEntries.size() };
	}

private:
	HeapAllocator mHeap;
	std::byte* mData = nullptr;
	std::vector<Entry> mEntries;
	std::size_t mCapacity, mOffset = 0, mPeak = 0, mGeneration = 0;
};
}
