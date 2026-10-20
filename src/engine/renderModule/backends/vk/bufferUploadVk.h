#pragma once
#ifdef VULKAN_BACKEND
#include "vmaVk.h"
#include <cstddef>

namespace IKIGAI::RENDER {
size_t CheckedBufferSizeVk(size_t count, size_t stride);
void ReserveBufferVk(VmaBuffer&, size_t& capacity, size_t bytes, vk::BufferUsageFlags usage);
void UploadBufferVk(const VmaBuffer&, const void* source, size_t bytes, size_t offset = 0);
}
#endif
