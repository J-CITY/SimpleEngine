#pragma once
#include "profilerModule/gpuMemoryProfiler.h"
#if defined(OPENGL_BACKEND) && IKIGAI_ENABLE_MEMORY_TRACKING
namespace IKIGAI::RENDER {
struct TextureMemoryLayoutGl {
    uint32_t width = 0, height = 0, depth = 1, layers = 1, levels = 1;
    uint32_t bytesPerTexel = 4;
    bool volume = false;
};
uint32_t TextureBytesPerTexelGl(unsigned internalFormat);
void TrackBufferMemoryGl(PROFILER::GpuAllocationToken&, unsigned target, unsigned id, PROFILER::GpuResourceKind);
void TrackTextureMemoryGl(PROFILER::GpuAllocationToken&, unsigned target, unsigned id, const TextureMemoryLayoutGl&, const char* name);
void TrackRenderbufferMemoryGl(PROFILER::GpuAllocationToken&, unsigned id);
}
#endif
