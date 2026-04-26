#pragma once

#ifdef VULKAN_BACKEND
#include <vector>
#include <vulkan/vulkan_raii.hpp>
#include "../interface/vertexBufferInterface.h"

namespace IKIGAI::RENDER {
	class VertexBufferVk : public VertexBufferInterface {
		vk::raii::Buffer mBuffer = nullptr;
		vk::raii::DeviceMemory mDeviceMemory = nullptr;
	public:
		VertexBufferVk(void* data, size_t size, size_t stride);

		template<class T>
		VertexBufferVk(const std::vector<T>& vertices): VertexBufferVk((void*)vertices.data(), vertices.size(), sizeof(T)) {
			
		}

		~VertexBufferVk() override;

		void setData(const void* data, size_t sz, size_t stride) override;

		void bind() override;

		void unbind() override;

		vk::raii::Buffer& getBuffer() {
			return mBuffer;
		}
	};
}
#endif
