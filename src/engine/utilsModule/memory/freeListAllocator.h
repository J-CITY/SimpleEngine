#pragma once
// Adapted from mtrebi/memory-allocators, MIT; see LICENSE.mtrebi and README.md.
#include "heapAllocator.h"
#include <vector>

namespace IKIGAI::MEMORY {
class FreeListAllocator final : public IAllocator {
	struct FreeNode {
		std::size_t bytes;
		FreeNode* next;
	};
	struct Header {
		std::size_t blockBytes, padding;
	};
	struct Page {
		std::byte* data;
		std::size_t bytes;
	};

public:
	enum class PlacementPolicy { FirstFit, BestFit };
	explicit FreeListAllocator(std::size_t pageBytes = 64 * 1024, PlacementPolicy policy = PlacementPolicy::FirstFit, bool grow = true)
		: mPageBytes(Detail::AlignUp(std::max(pageBytes, sizeof(FreeNode)), alignof(FreeNode)))
		, mPolicy(policy)
		, mGrow(grow) { }
	~FreeListAllocator() override {
		for (auto& page : mPages)
			mHeap.deallocate(page.data);
	}
	FreeListAllocator(const FreeListAllocator&) = delete;
	FreeListAllocator& operator=(const FreeListAllocator&) = delete;
	void* allocate(std::size_t bytes) override {
		return allocate(bytes, alignof(std::max_align_t));
	}
	void* allocate(std::size_t bytes, std::size_t alignment) override {
		alignment = std::max(Detail::ValidateAlignment(alignment), alignof(Header));
		bytes = std::max(bytes, std::size_t { 1 });
		FreeNode* selected = nullptr;
		FreeNode* selectedPrevious = nullptr;
		std::size_t selectedPadding = 0, selectedRequired = 0;
		auto find = [&] {
			FreeNode* previous = nullptr;
			for (auto* node = mHead; node; previous = node, node = node->next) {
				const auto address = reinterpret_cast<std::uintptr_t>(node);
				const auto padding = sizeof(Header) + Detail::Padding(address + sizeof(Header), alignment);
				const auto required = Detail::AlignUp(Detail::CheckedAdd(bytes, padding), alignof(FreeNode));
				if (required <= node->bytes && (!selected || node->bytes - required < selected->bytes - selectedRequired)) {
					selected = node;
					selectedPrevious = previous;
					selectedPadding = padding;
					selectedRequired = required;
					if (mPolicy == PlacementPolicy::FirstFit)
						break;
				}
			}
		};
		find();
		if (!selected) {
			if (!mGrow && !mPages.empty())
				throw std::bad_alloc();
			const auto minimum = Detail::CheckedAdd(Detail::CheckedAdd(bytes, sizeof(Header)), alignment - 1);
			if (!mGrow && minimum > mPageBytes)
				throw std::bad_alloc();
			addPage(std::max(mPageBytes, Detail::AlignUp(minimum, alignof(FreeNode))));
			find();
		}
		const auto blockBytes = selected->bytes;
		auto* next = selected->next;
		if (blockBytes - selectedRequired >= sizeof(FreeNode)) {
			auto* remainder = ::new (reinterpret_cast<std::byte*>(selected) + selectedRequired) FreeNode { blockBytes - selectedRequired, next };
			if (selectedPrevious)
				selectedPrevious->next = remainder;
			else
				mHead = remainder;
		}
		else {
			selectedRequired = blockBytes;
			if (selectedPrevious)
				selectedPrevious->next = next;
			else
				mHead = next;
		}
		auto* result = reinterpret_cast<std::byte*>(selected) + selectedPadding;
		::new (result - sizeof(Header)) Header { selectedRequired, selectedPadding };
		mUsed += selectedRequired;
		mPeak = std::max(mPeak, mUsed);
		++mLive;
		return result;
	}
	void deallocate(void* ptr) override {
		if (!ptr)
			return;
		const auto header = *(static_cast<Header*>(ptr) - 1);
		auto* begin = static_cast<std::byte*>(ptr) - header.padding;
		auto* node = ::new (begin) FreeNode { header.blockBytes, nullptr };
		insert(node);
		mUsed -= header.blockBytes;
		--mLive;
	}
	void reset() {
		if (mLive)
			throw std::logic_error("Free-list reset requires all allocations to be freed");
		mHead = nullptr;
		for (auto& page : mPages)
			insert(::new (page.data) FreeNode { page.bytes, nullptr });
	}
	AllocatorStats stats() const noexcept {
		return { mReserved, mUsed, mPeak, mLive };
	}

private:
	// Adjacent independent malloc blocks must never be coalesced together.
	bool samePage(const void* a, const void* b) const noexcept {
		const auto x = reinterpret_cast<std::uintptr_t>(a), y = reinterpret_cast<std::uintptr_t>(b);
		for (const auto& page : mPages) {
			const auto start = reinterpret_cast<std::uintptr_t>(page.data);
			if (x >= start && x - start < page.bytes && y >= start && y - start < page.bytes)
				return true;
		}
		return false;
	}
	void insert(FreeNode* node) noexcept {
		FreeNode* previous = nullptr;
		auto* next = mHead;
		while (next && reinterpret_cast<std::uintptr_t>(next) < reinterpret_cast<std::uintptr_t>(node)) {
			previous = next;
			next = next->next;
		}
		node->next = next;
		if (previous)
			previous->next = node;
		else
			mHead = node;
		if (next && samePage(node, next) && reinterpret_cast<std::byte*>(node) + node->bytes == reinterpret_cast<std::byte*>(next)) {
			node->bytes += next->bytes;
			node->next = next->next;
		}
		if (previous && samePage(previous, node) && reinterpret_cast<std::byte*>(previous) + previous->bytes == reinterpret_cast<std::byte*>(node)) {
			previous->bytes += node->bytes;
			previous->next = node->next;
		}
	}
	void addPage(std::size_t bytes) {
		auto* data = static_cast<std::byte*>(mHeap.allocate(bytes));
		try {
			mPages.push_back({ data, bytes });
		} catch (...) {
			mHeap.deallocate(data);
			throw;
		}
		mReserved += bytes;
		insert(::new (data) FreeNode { bytes, nullptr });
	}
	HeapAllocator mHeap;
	std::vector<Page> mPages;
	FreeNode* mHead = nullptr;
	std::size_t mPageBytes, mReserved = 0, mUsed = 0, mPeak = 0, mLive = 0;
	PlacementPolicy mPolicy;
	bool mGrow;
};
}
