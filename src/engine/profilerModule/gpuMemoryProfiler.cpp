#include "gpuMemoryProfiler.h"
#include <algorithm>
#include <atomic>
#include <unordered_map>
#include <utility>

namespace IKIGAI::PROFILER {
namespace {
#if IKIGAI_ENABLE_MEMORY_TRACKING
	struct State {
		std::mutex mutex;
		std::unordered_map<std::uint64_t, GpuAllocationRecord> records;
		std::vector<std::weak_ptr<GpuMemorySampler>> samplers;
		GpuMemoryTotals totals;
		std::array<GpuMemoryTotals, static_cast<std::size_t>(GpuBackend::Count)> backends { };
		std::array<GpuMemoryTotals, static_cast<std::size_t>(GpuResourceKind::Count)> kinds { };
		std::array<bool, static_cast<std::size_t>(GpuBackend::Count)> available { };
		std::uint64_t nextId = 1;
	};
	State& state() {
		static auto* value = new State;
		return *value;
	}
	std::atomic<std::uint64_t> dropped = 0;
	bool valid(const GpuAllocationInfo& info) {
		return static_cast<std::size_t>(info.backend) < static_cast<std::size_t>(GpuBackend::Count)
			&& static_cast<std::size_t>(info.kind) < static_cast<std::size_t>(GpuResourceKind::Count)
			&& static_cast<unsigned>(info.measurement) <= static_cast<unsigned>(GpuMeasurement::Unavailable);
	}
	void add(GpuMemoryTotals& total, const GpuAllocationRecord& record) {
		auto& bytes = record.info.measurement == GpuMeasurement::ReportedAllocation ? total.reportedBytes : total.estimatedBytes;
		if (record.info.measurement == GpuMeasurement::Unavailable)
			++total.unknownSizeAllocations;
		else
			bytes += record.info.bytes;
		++total.allocations;
		total.peakBytes = std::max(total.peakBytes, total.reportedBytes + total.estimatedBytes);
		if (record.pendingRelease) {
			total.pendingBytes += record.info.bytes;
			++total.pendingAllocations;
		}
	}
	void remove(GpuMemoryTotals& total, const GpuAllocationRecord& record) {
		auto& bytes = record.info.measurement == GpuMeasurement::ReportedAllocation ? total.reportedBytes : total.estimatedBytes;
		if (record.info.measurement == GpuMeasurement::Unavailable)
			--total.unknownSizeAllocations;
		else
			bytes -= record.info.bytes;
		--total.allocations;
		if (record.pendingRelease) {
			total.pendingBytes -= record.info.bytes;
			--total.pendingAllocations;
		}
	}
	void add(State& s, const GpuAllocationRecord& r) {
		add(s.totals, r);
		add(s.backends[static_cast<std::size_t>(r.info.backend)], r);
		add(s.kinds[static_cast<std::size_t>(r.info.kind)], r);
	}
	void remove(State& s, const GpuAllocationRecord& r) {
		remove(s.totals, r);
		remove(s.backends[static_cast<std::size_t>(r.info.backend)], r);
		remove(s.kinds[static_cast<std::size_t>(r.info.kind)], r);
	}
#endif
}

void GpuMemorySampler::disable() {
	std::lock_guard lock(mMutex);
	mCallback = { };
}
std::vector<GpuHeapSnapshot> GpuMemorySampler::capture() {
	std::lock_guard lock(mMutex);
	return mCallback ? mCallback() : std::vector<GpuHeapSnapshot> { };
}
GpuMemoryProfiler& GpuMemoryProfiler::instance() {
	static GpuMemoryProfiler profiler;
	return profiler;
}
std::uint64_t GpuMemoryProfiler::record(const GpuAllocationInfo& info) noexcept {
#if IKIGAI_ENABLE_MEMORY_TRACKING
	try {
		if (!valid(info)) {
			++dropped;
			return 0;
		}
		GpuAllocationRecord record;
		record.info = info;
		record.info.name = nullptr; // Names are owned, never pointers into assets.
		if (record.info.measurement == GpuMeasurement::Unavailable)
			record.info.bytes = 0;
		if (info.name)
			record.name = info.name;
		auto& s = state();
		std::lock_guard lock(s.mutex);
		record.id = s.nextId++;
		const auto id = record.id;
		auto [it, inserted] = s.records.emplace(id, std::move(record));
		if (!inserted) {
			++dropped;
			return 0;
		}
		add(s, it->second);
		s.available[static_cast<std::size_t>(info.backend)] = true;
		return id;
	} catch (...) {
		++dropped;
	}
#else
	(void)info;
#endif
	return 0;
}
void GpuMemoryProfiler::update(std::uint64_t id, const GpuAllocationInfo& info) noexcept {
#if IKIGAI_ENABLE_MEMORY_TRACKING
	try {
		if (!valid(info)) {
			++dropped;
			return;
		}
		std::string name;
		if (info.name)
			name = info.name;
		auto& s = state();
		std::lock_guard lock(s.mutex);
		auto it = s.records.find(id);
		if (it == s.records.end())
			return;
		remove(s, it->second);
		it->second.info = info;
		it->second.info.name = nullptr;
		if (it->second.info.measurement == GpuMeasurement::Unavailable)
			it->second.info.bytes = 0;
		if (info.name)
			it->second.name = std::move(name);
		add(s, it->second);
		s.available[static_cast<std::size_t>(info.backend)] = true;
	} catch (...) {
		++dropped;
	}
#else
	(void)id;
	(void)info;
#endif
}
void GpuMemoryProfiler::release(std::uint64_t id) noexcept {
#if IKIGAI_ENABLE_MEMORY_TRACKING
	if (!id)
		return;
	auto& s = state();
	std::lock_guard lock(s.mutex);
	auto it = s.records.find(id);
	if (it == s.records.end())
		return;
	remove(s, it->second);
	s.records.erase(it);
#else
	(void)id;
#endif
}
void GpuMemoryProfiler::markPending(std::uint64_t id) noexcept {
#if IKIGAI_ENABLE_MEMORY_TRACKING
	if (!id)
		return;
	auto& s = state();
	std::lock_guard lock(s.mutex);
	auto it = s.records.find(id);
	if (it == s.records.end() || it->second.pendingRelease)
		return;
	remove(s, it->second);
	it->second.pendingRelease = true;
	add(s, it->second);
#else
	(void)id;
#endif
}
void GpuMemoryProfiler::setName(std::uint64_t id, const char* name) noexcept {
#if IKIGAI_ENABLE_MEMORY_TRACKING
	try {
		if (!id)
			return;
		auto& s = state();
		std::lock_guard lock(s.mutex);
		auto it = s.records.find(id);
		if (it != s.records.end())
			it->second.name = name ? name : "";
	} catch (...) {
		++dropped;
	}
#else
	(void)id;
	(void)name;
#endif
}
void GpuMemoryProfiler::registerSampler(const std::shared_ptr<GpuMemorySampler>& sampler) {
#if IKIGAI_ENABLE_MEMORY_TRACKING
	auto& s = state();
	std::lock_guard lock(s.mutex);
	std::erase_if(s.samplers, [](const auto& source) { return source.expired(); });
	s.samplers.push_back(sampler);
#else
	(void)sampler;
#endif
}
GpuMemorySnapshot GpuMemoryProfiler::captureSnapshot() const {
	GpuMemorySnapshot snapshot;
#if IKIGAI_ENABLE_MEMORY_TRACKING
	snapshot.trackingEnabled = true;
	std::vector<std::shared_ptr<GpuMemorySampler>> samplers;
	{
		auto& s = state();
		std::lock_guard lock(s.mutex);
		snapshot.totals = s.totals;
		snapshot.backends = s.backends;
		snapshot.kinds = s.kinds;
		snapshot.allocationTrackingAvailable = s.available;
#ifdef VULKAN_BACKEND
		snapshot.allocationTrackingAvailable[static_cast<std::size_t>(GpuBackend::Vulkan)] = true;
#endif
#ifdef OPENGL_BACKEND
		snapshot.allocationTrackingAvailable[static_cast<std::size_t>(GpuBackend::OpenGL)] = true;
#endif
#ifdef DX12_BACKEND
		snapshot.allocationTrackingAvailable[static_cast<std::size_t>(GpuBackend::DirectX12)] = true;
#endif
#ifdef METAL_BACKEND
		snapshot.allocationTrackingAvailable[static_cast<std::size_t>(GpuBackend::Metal)] = true;
#endif
		snapshot.droppedRecords = dropped.load();
		snapshot.allocations.reserve(s.records.size());
		for (const auto& [id, record] : s.records)
			snapshot.allocations.push_back(record);
		for (const auto& weak : s.samplers)
			if (auto source = weak.lock())
				samplers.push_back(std::move(source));
	}
	for (const auto& source : samplers) {
		try {
			auto heaps = source->capture();
			snapshot.heaps.insert(snapshot.heaps.end(), heaps.begin(), heaps.end());
		} catch (...) {
			++snapshot.failedSamplers;
		}
	}
#endif
	return snapshot;
}

void GpuMemoryProfiler::noteDroppedRecord() noexcept {
#if IKIGAI_ENABLE_MEMORY_TRACKING
	++dropped;
#endif
}

GpuAllocationToken::GpuAllocationToken(GpuAllocationToken&& other) noexcept {
#if IKIGAI_ENABLE_MEMORY_TRACKING
	mId = std::exchange(other.mId, 0);
#else
	(void)other;
#endif
}
GpuAllocationToken& GpuAllocationToken::operator=(GpuAllocationToken&& other) noexcept {
	if (this != &other) {
		reset();
#if IKIGAI_ENABLE_MEMORY_TRACKING
		mId = std::exchange(other.mId, 0);
#endif
	}
	return *this;
}
void GpuAllocationToken::track(const GpuAllocationInfo& info) noexcept {
#if IKIGAI_ENABLE_MEMORY_TRACKING
	if (mId)
		GpuMemoryProfiler::instance().update(mId, info);
	else
		mId = GpuMemoryProfiler::instance().record(info);
#else
	(void)info;
#endif
}
void GpuAllocationToken::reset() noexcept {
#if IKIGAI_ENABLE_MEMORY_TRACKING
	GpuMemoryProfiler::instance().release(std::exchange(mId, 0));
#endif
}
void GpuAllocationToken::markPending() noexcept {
#if IKIGAI_ENABLE_MEMORY_TRACKING
	GpuMemoryProfiler::instance().markPending(mId);
#endif
}
void GpuAllocationToken::setName(const char* name) noexcept {
#if IKIGAI_ENABLE_MEMORY_TRACKING
	GpuMemoryProfiler::instance().setName(mId, name);
#else
	(void)name;
#endif
}
std::uint64_t GpuAllocationToken::id() const noexcept {
#if IKIGAI_ENABLE_MEMORY_TRACKING
	return mId;
#else
	return 0;
#endif
}
}
