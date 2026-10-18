#pragma once
#ifdef METAL_BACKEND
#import <Metal/Metal.h>
#include "profilerModule/gpuMemoryProfiler.h"

namespace IKIGAI::RENDER {
#if IKIGAI_ENABLE_MEMORY_TRACKING
void TrackGpuMemoryMetal(id<MTLResource>, PROFILER::GpuResourceKind, const char* name = nullptr) noexcept;
void TrackGpuObjectMetal(id, id<MTLDevice>, PROFILER::GpuResourceKind, const char* name = nullptr) noexcept;
std::shared_ptr<PROFILER::GpuMemorySampler> CreateMemorySamplerMetal(id<MTLDevice>);
#endif
inline id<MTLBuffer> AllocateBufferMetal(id<MTLDevice> device, NSUInteger bytes, MTLResourceOptions options,
    PROFILER::GpuResourceKind kind = PROFILER::GpuResourceKind::Buffer, const char* name = nullptr) {
    id<MTLBuffer> buffer = [device newBufferWithLength:bytes options:options];
#if IKIGAI_ENABLE_MEMORY_TRACKING
    TrackGpuMemoryMetal(buffer, kind, name);
#endif
    return buffer;
}
inline id<MTLTexture> AllocateTextureMetal(id<MTLDevice> device, MTLTextureDescriptor* descriptor,
    PROFILER::GpuResourceKind kind = PROFILER::GpuResourceKind::Texture, const char* name = nullptr) {
    id<MTLTexture> texture = [device newTextureWithDescriptor:descriptor];
#if IKIGAI_ENABLE_MEMORY_TRACKING
    TrackGpuMemoryMetal(texture, kind, name);
#endif
    return texture;
}
}
#endif
