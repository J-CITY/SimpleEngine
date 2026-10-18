#pragma once
#include "profilerModule/gpuMemoryProfiler.h"
#ifdef DX12_BACKEND
#include <d3d12.h>
namespace IKIGAI::RENDER {
#if IKIGAI_ENABLE_MEMORY_TRACKING
void TrackGpuMemoryDx12(ID3D12Object*, const PROFILER::GpuAllocationInfo&) noexcept;
void SetGpuMemoryKindDx12(ID3D12Object*, PROFILER::GpuResourceKind, const char* name = nullptr) noexcept;
void MarkGpuMemoryPendingDx12(ID3D12Object*) noexcept;
void TrackGpuObjectDx12(ID3D12Object*, ID3D12Device*, PROFILER::GpuResourceKind, const char* name = nullptr) noexcept;
#else
inline void TrackGpuMemoryDx12(ID3D12Object*, const PROFILER::GpuAllocationInfo&) noexcept {}
inline void SetGpuMemoryKindDx12(ID3D12Object*, PROFILER::GpuResourceKind, const char* = nullptr) noexcept {}
inline void MarkGpuMemoryPendingDx12(ID3D12Object*) noexcept {}
inline void TrackGpuObjectDx12(ID3D12Object*, ID3D12Device*, PROFILER::GpuResourceKind, const char* = nullptr) noexcept {}
#endif
}
#endif
