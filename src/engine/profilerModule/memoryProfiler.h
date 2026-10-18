#pragma once
#include "gpuMemoryProfiler.h"
#include "processMemory.h"
#include "utilsModule/memory/allocator.h"
#include <array>
#include <atomic>
#include <mutex>
#include <optional>
#include <unordered_map>
#include <vector>

#ifndef IKIGAI_ENABLE_MEMORY_TRACKING
#define IKIGAI_ENABLE_MEMORY_TRACKING 0
#endif

namespace IKIGAI::PROFILER {
struct MemoryAllocation {
	const void* address;
	const void* allocator;
	std::size_t bytes, alignment;
	MEMORY::Category category;
};
struct MemoryTotals {
	std::size_t bytes = 0, peakBytes = 0, allocations = 0;
};
struct MemorySnapshot {
	GpuMemorySnapshot gpu;
	ProcessMemorySnapshot process;
	std::array<MemoryTotals, static_cast<std::size_t>(MEMORY::Category::Count)> categories { };
	std::vector<MemoryAllocation> allocations;
	std::size_t droppedRecords = 0;
};

// The ledger uses the system allocator directly, never the decorated engine
// allocators. The process-lifetime instance also permits late resource teardown.
class MemoryProfiler {
public:
	static MemoryProfiler& instance() {
		static auto* profiler = new MemoryProfiler;
		return *profiler;
	}
	void record(MemoryAllocation allocation) noexcept {
#if IKIGAI_ENABLE_MEMORY_TRACKING
		try {
			std::lock_guard lock(mMutex);
			if (!mAllocations.emplace(allocation.address, allocation).second) {
				++mDropped;
				return;
			}
			auto& totals = mTotals[static_cast<std::size_t>(allocation.category)];
			totals.bytes += allocation.bytes;
			totals.peakBytes = std::max(totals.peakBytes, totals.bytes);
			++totals.allocations;
		} catch (...) {
			++mDropped;
		}
#else
		(void)allocation;
#endif
	}
	std::optional<MemoryAllocation> take(const void* address, const void* allocator) noexcept {
#if IKIGAI_ENABLE_MEMORY_TRACKING
		std::lock_guard lock(mMutex);
		auto it = mAllocations.find(address);
		if (it == mAllocations.end() || it->second.allocator != allocator)
			return { };
		const auto allocation = it->second;
		auto& totals = mTotals[static_cast<std::size_t>(it->second.category)];
		totals.bytes -= it->second.bytes;
		--totals.allocations;
		mAllocations.erase(it);
		return allocation;
#else
		(void)address;
		(void)allocator;
		return { };
#endif
	}
	// Arenas release allocations as a group; remove their ledger entries before
	// addresses can be reused. Other allocators' records are untouched.
	void releaseAllocator(const void* allocator) noexcept {
#if IKIGAI_ENABLE_MEMORY_TRACKING
		std::lock_guard lock(mMutex);
		for (auto it = mAllocations.begin(); it != mAllocations.end();) {
			if (it->second.allocator != allocator) {
				++it;
				continue;
			}
			auto& totals = mTotals[static_cast<std::size_t>(it->second.category)];
			totals.bytes -= it->second.bytes;
			--totals.allocations;
			it = mAllocations.erase(it);
		}
#else
		(void)allocator;
#endif
	}
	MemorySnapshot captureSnapshot() const {
		MemorySnapshot snapshot;
#if IKIGAI_ENABLE_MEMORY_TRACKING
		{
			std::lock_guard lock(mMutex);
			snapshot.categories = mTotals;
			snapshot.droppedRecords = mDropped.load();
			snapshot.allocations.reserve(mAllocations.size());
			for (const auto& [address, allocation] : mAllocations)
				snapshot.allocations.push_back(allocation);
		}
		snapshot.gpu = GpuMemoryProfiler::instance().captureSnapshot();
		snapshot.process = CaptureProcessMemory();
#endif
		return snapshot;
	}

private:
#if IKIGAI_ENABLE_MEMORY_TRACKING
	mutable std::mutex mMutex;
	std::unordered_map<const void*, MemoryAllocation> mAllocations;
	std::array<MemoryTotals, static_cast<std::size_t>(MEMORY::Category::Count)> mTotals { };
	std::atomic<std::size_t> mDropped = 0;
#endif
};
}
