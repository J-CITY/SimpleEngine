#pragma once
#ifdef VULKAN_BACKEND
#include <span>

#include <cstdint>
#include <vector>
#include <vulkan/vulkan.h>
#include "../interface/indexBufferInterface.h"
namespace KUMA::RENDER {
	class ShaderInterface;

	class IndexBufferVk : public IndexBufferInterface {
	public:
		VkDeviceAddress       m_device_address = 0;
		int				 m_indexCount;
		VkBuffer		 m_indexBuffer;
		VkDeviceMemory   m_indexBufferMemory;
		IndexBufferVk() = default;
		IndexBufferVk(std::span<uint32_t>& indices);

		void bind(const ShaderInterface& shader);
		virtual int getIndexCount() override
		{
			return m_indexCount;
		}
	};
}
#endif
