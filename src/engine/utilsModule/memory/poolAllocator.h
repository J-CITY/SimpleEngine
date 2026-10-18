#pragma once
// Adapted from mtrebi/memory-allocators, MIT; see LICENSE.mtrebi and README.md.
#include "heapAllocator.h"
#include <memory>
#include <vector>

namespace IKIGAI::MEMORY {
// Intrusive free list, fixed slot size, optional growth by equally sized pages.
// Synchronization belongs to the resource allocator wrapper, not this policy.
class PoolAllocator final : public IAllocator {
	struct FreeNode {
		FreeNode* next;
	};
	struct Page {
		std::byte* data;
		std::vector<unsigned char> live;
	};

public:
	PoolAllocator(std::size_t objectSize, std::size_t alignment, std::size_t objectsPerPage = 128, bool grow = true)
		: mAlignment(std::max(Detail::ValidateAlignment(alignment), alignof(FreeNode)))
		, mStride(Detail::AlignUp(std::max(objectSize, sizeof(FreeNode)), mAlignment))
		, mObjectSize(objectSize)
		, mObjectsPerPage(objectsPerPage)
		, mGrow(grow) {
		if (!objectSize || !objectsPerPage)
			throw std::invalid_argument("Pool size and capacity must be nonzero");
	}
	~PoolAllocator() override {
		for (auto& page : mPages)
			mHeap.deallocate(page.data);
	}
	PoolAllocator(const PoolAllocator&) = delete;
	PoolAllocator& operator=(const PoolAllocator&) = delete;
	void* allocate(std::size_t bytes) override {
		return allocate(bytes, mAlignment);
	}
	void* allocate(std::size_t bytes, std::size_t alignment) override {
		Detail::ValidateAlignment(alignment);
		if (bytes > mObjectSize || alignment > mAlignment)
			throw std::bad_alloc();
		if (!mFree) {
			if (!mGrow && !mPages.empty())
				throw std::bad_alloc();
			addPage();
		}
		auto* result = mFree;
		mFree = mFree->next;
		auto [page, index] = locate(result);
		page->live[index] = 1;
		++mLive;
		mPeak = std::max(mPeak, mLive * mStride);
		return result;
	}
	void deallocate(void* ptr) override {
		if (!ptr)
			return;
		auto [page, index] = locate(ptr);
		if (!page || !page->live[index])
			throw std::invalid_argument("Foreign pointer or double free in PoolAllocator");
		page->live[index] = 0;
		mFree = ::new (ptr) FreeNode { mFree };
		--mLive;
	}
	void reset() {
		if (mLive)
			throw std::logic_error("Pool reset requires all objects to be destroyed");
		mFree = nullptr;
		for (auto& page : mPages)
			linkPage(page);
	}
	bool owns(const void* ptr) const noexcept {
		const auto address = reinterpret_cast<std::uintptr_t>(ptr);
		for (const auto& page : mPages) {
			const auto base = reinterpret_cast<std::uintptr_t>(page.data);
			if (address >= base && address - base < mStride * mObjectsPerPage && (address - base) % mStride == 0)
				return true;
		}
		return false;
	}
	AllocatorStats stats() const noexcept {
		return { mPages.size() * mStride * mObjectsPerPage, mLive * mStride, mPeak, mLive };
	}

private:
	std::pair<Page*, std::size_t> locate(const void* ptr) noexcept {
		const auto address = reinterpret_cast<std::uintptr_t>(ptr);
		for (auto& page : mPages) {
			const auto base = reinterpret_cast<std::uintptr_t>(page.data);
			if (address >= base && address - base < mStride * mObjectsPerPage && (address - base) % mStride == 0)
				return { &page, (address - base) / mStride };
		}
		return { nullptr, 0 };
	}
	void linkPage(Page& page) noexcept {
		for (std::size_t i = mObjectsPerPage; i > 0; --i)
			mFree = ::new (page.data + (i - 1) * mStride) FreeNode { mFree };
	}
	void addPage() {
		Page page { nullptr, std::vector<unsigned char>(mObjectsPerPage, 0) };
		page.data = static_cast<std::byte*>(mHeap.allocate(Detail::CheckedMultiply(mStride, mObjectsPerPage), mAlignment));
		try {
			mPages.push_back(std::move(page));
		} catch (...) {
			mHeap.deallocate(page.data);
			throw;
		}
		linkPage(mPages.back());
	}
	HeapAllocator mHeap;
	std::vector<Page> mPages;
	FreeNode* mFree = nullptr;
	std::size_t mAlignment, mStride, mObjectSize, mObjectsPerPage, mLive = 0, mPeak = 0;
	bool mGrow;
};
}
