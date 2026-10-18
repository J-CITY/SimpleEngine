#pragma once
#ifdef VULKAN_BACKEND
#include <vulkan/vulkan.hpp>
#include "../interface/vertexLayout.h"
namespace IKIGAI::RENDER {
inline vk::Format NativeVertexFormat(VertexFormat format) {
    static constexpr vk::Format formats[] = {vk::Format::eR32Sfloat,vk::Format::eR32G32Sfloat,vk::Format::eR32G32B32Sfloat,vk::Format::eR32G32B32A32Sfloat,vk::Format::eR32Sint,vk::Format::eR32G32Sint,vk::Format::eR32G32B32Sint,vk::Format::eR32G32B32A32Sint,vk::Format::eR32Uint,vk::Format::eR32G32Uint,vk::Format::eR32G32B32Uint,vk::Format::eR32G32B32A32Uint,vk::Format::eR8G8B8A8Unorm,vk::Format::eR8G8B8A8Snorm};
    GetVertexFormatInfo(format);
    return formats[static_cast<unsigned>(format)];
}
}
#endif

