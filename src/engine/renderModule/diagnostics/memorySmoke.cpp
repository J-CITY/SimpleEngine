#include "memorySmoke.h"
#include "profilerModule/memoryProfiler.h"
#include "renderModule/backends/interface/driverInterface.h"
#include <iostream>
#include <stdexcept>
#ifdef DX12_BACKEND
#include "renderModule/backends/dx12/d3dUtil.h"
#include "renderModule/backends/dx12/driverDx12.h"
#endif
#ifdef VULKAN_BACKEND
#include "renderModule/backends/vk/driverVk.h"
#include "renderModule/backends/vk/helpers.h"
#endif
#ifdef OPENGL_BACKEND
#include "renderModule/backends/gl/storageBufferGl.h"
#endif

namespace IKIGAI::RENDER {
void RunMemorySmoke() {
#if IKIGAI_ENABLE_MEMORY_TRACKING
	auto require = [](bool value, const char* message) {
		if (!value)
			throw std::runtime_error(message);
	};
	auto& profiler = PROFILER::GpuMemoryProfiler::instance();
#ifdef DX12_BACKEND
	if (DriverInterface::settings.backend == RenderSettings::Backend::DIRECTX12) {
		const auto before = profiler.captureSnapshot();
		auto resource = d3dUtil::CreateBuffer(256, D3D12_HEAP_TYPE_UPLOAD);
		auto copy = resource;
		resource.Reset();
		const auto allocated = profiler.captureSnapshot();
		require(allocated.totals.allocations == before.totals.allocations + 1, "DX12 copies duplicated or lost a GPU memory record");
		require(allocated.totals.reportedBytes >= before.totals.reportedBytes + 256, "DX12 allocation size was not recorded");
		auto* driver = static_cast<DriverDx12*>(DriverInterface::Get());
		driver->destroyDeferred(copy);
		copy.Reset();
		const auto queued = profiler.captureSnapshot();
		require(queued.totals.allocations == allocated.totals.allocations, "DX12 memory was released before deferred destruction");
		require(queued.totals.pendingAllocations == before.totals.pendingAllocations + 1, "DX12 deferred memory was not marked pending");
		require(!queued.heaps.empty(), "DX12 memory sampler returned no segments");
	}
#endif
#ifdef VULKAN_BACKEND
	if (DriverInterface::settings.backend == RenderSettings::Backend::VULKAN) {
		const auto before = profiler.captureSnapshot();
		auto original = UtilityVk::CreateBuffer(256, vk::BufferUsageFlagBits::eTransferSrc, UtilityVk::MemoryUsage::Staging);
		original.setMemoryName("Memory smoke staging");
		VmaBuffer moved = std::move(original);
		VmaBuffer assigned;
		assigned = std::move(moved);
		const auto allocated = profiler.captureSnapshot();
		require(allocated.totals.allocations == before.totals.allocations + 1, "VMA move duplicated or lost a GPU memory record");
		require(allocated.totals.reportedBytes >= before.totals.reportedBytes + 256, "VMA allocation size was not recorded");
		UtilityVk::GetDriver()->destroyDeferred(std::move(assigned));
		const auto queued = profiler.captureSnapshot();
		require(queued.totals.allocations == allocated.totals.allocations, "GPU memory was released before its deferred destruction");
		require(queued.totals.pendingAllocations == before.totals.pendingAllocations + 1, "Deferred GPU memory was not marked pending");
		require(!queued.heaps.empty(), "VMA heap sampler returned no heaps");
	}
#endif
#ifdef OPENGL_BACKEND
	if (DriverInterface::settings.backend == RenderSettings::Backend::OPENGL) {
		const auto before = profiler.captureSnapshot();
		{
			StorageBufferGl buffer(nullptr, 16, 4);
			const auto allocated = profiler.captureSnapshot();
			require(allocated.totals.allocations == before.totals.allocations + 1, "OpenGL buffer storage was not tracked");
			buffer.setData(nullptr, 64, 4);
			const auto resized = profiler.captureSnapshot();
			require(resized.totals.allocations == allocated.totals.allocations, "OpenGL buffer resize duplicated its record");
			require(resized.totals.estimatedBytes == before.totals.estimatedBytes + 256, "OpenGL buffer resize reported the wrong size");
		}
		require(profiler.captureSnapshot().totals.allocations == before.totals.allocations, "OpenGL buffer deletion left a GPU record");
	}
#endif
	const auto memory = PROFILER::MemoryProfiler::instance().captureSnapshot();
	const auto& gpu = memory.gpu;
	require(gpu.trackingEnabled && gpu.totals.allocations > 0, "GPU memory snapshot is empty or disabled");
	require(gpu.droppedRecords == 0 && gpu.failedSamplers == 0, "GPU memory bookkeeping is incomplete");
	std::cout << "GPU_MEMORY_SMOKE_OK allocations=" << gpu.totals.allocations << " reported=" << gpu.totals.reportedBytes
			  << " estimated=" << gpu.totals.estimatedBytes << " pending=" << gpu.totals.pendingBytes << " heaps=" << gpu.heaps.size() << std::endl;
	std::cout << "MEMORY_SYSTEM_SMOKE resident=" << memory.process.residentBytes.value_or(0)
			  << " ram=" << memory.process.totalPhysicalBytes.value_or(0) << " unknown_gpu_sizes=" << gpu.totals.unknownSizeAllocations << std::endl;
#else
	throw std::runtime_error("--smoke-memory requires IKIGAI_ENABLE_MEMORY_TRACKING=ON");
#endif
}
}
