#pragma once

#ifdef VULKAN_BACKEND
#include <span>
#include <vector>
#include <vulkan/vulkan.h>

#include "driverVk.h"
#include "shaderVk.h"
#include "../interface/vertexBufferInterface.h"
#include <resourceModule/serviceManager.h>

namespace IKIGAI::RENDER {
	template<class T>
	class VertexBufferVk : public VertexBufferInterface {
	public:
		VkDeviceAddress       m_device_address = 0;
		int				 m_vertexCount;
		VkBuffer		 m_vertexBuffer;
		VkDeviceMemory	 m_vertexBufferMemory;
		VertexBufferVk() = default;
		VertexBufferVk(std::span<T>& vertices) {
			auto render = UtilityVk::GetDriver();

			m_vertexCount = vertices.size();
			VkDeviceSize bufferSize = sizeof(T) * vertices.size();

			VkBuffer staging_buffer;
			VkDeviceMemory staging_buffer_memory;

			BufferSettings buffer_settings;
			buffer_settings.size = bufferSize;
			buffer_settings.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
			buffer_settings.properties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

			UtilityVk::CreateBuffer(buffer_settings, &staging_buffer, &staging_buffer_memory);
			
			void* data;
			vkMapMemory(render->m_MainDevice.LogicalDevice, staging_buffer_memory, 0, bufferSize, 0, &data);
			memcpy(data, vertices.data(), static_cast<size_t>(bufferSize));	
			vkUnmapMemory(render->m_MainDevice.LogicalDevice, staging_buffer_memory);

			buffer_settings.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
			buffer_settings.properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

			UtilityVk::CreateBuffer(buffer_settings, &m_vertexBuffer, &m_vertexBufferMemory);

			UtilityVk::CopyBufferCmd(staging_buffer, m_vertexBuffer, bufferSize);

			vkDestroyBuffer(render->m_MainDevice.LogicalDevice, staging_buffer, nullptr);
			vkFreeMemory(render->m_MainDevice.LogicalDevice, staging_buffer_memory, nullptr);

			//bool useInRt = true;
			//VkBufferDeviceAddressInfoKHR address_info;
			//DW_ZERO_MEMORY(address_info);
			//
			//address_info.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO_KHR;
			//address_info.buffer = m_vertexBuffer;
			//
			////if ((usage & VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT) == VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT)
			//if (useInRt)
			//	m_device_address = vkGetBufferDeviceAddress(dynamic_cast<DriverVk*>(DriverVkInstance::GetInstance())->m_MainDevice.LogicalDevice, &address_info);
		}

		void bind(const ShaderInterface& shader) override {
			auto render = UtilityVk::GetDriver();

			VkBuffer vertexBuffers[] = { m_vertexBuffer };
			VkDeviceSize offsets[] = { 0 };
			vkCmdBindVertexBuffers(static_cast<const ShaderVk&>(shader).m_CommandHandler.m_CommandBuffers[render->imageIndex], 0, 1, vertexBuffers, offsets);
		}

		virtual int getVertexCount() override {
			return m_vertexCount;
		}
	};
}
#endif
