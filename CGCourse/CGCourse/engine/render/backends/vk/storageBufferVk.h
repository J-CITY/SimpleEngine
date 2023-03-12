#pragma once

#ifdef VULKAN_BACKEND
#include <vector>
#include <memory>
#include <stdexcept>
#include <vulkan/vulkan_core.h>
#include "helpers.h"
#include "shaderVk.h"
#include "driverVk.h"
#include "../../../resourceManager/ServiceManager.h"
namespace KUMA::RENDER {
	class ShaderInterface;
	template<typename T>
	class ShaderStorageBufferVk {
	public:
		int				 m_vertexCount;
		VkBuffer		 m_vertexBuffer;
		VkDeviceMemory	 m_vertexBufferMemory;

		std::vector<VkDescriptorSet> descriptorSets;
		VkDescriptorPool pool;
		const int MAX_SIZE = 1000;

		int setId = 0;
		ShaderStorageBufferVk(std::shared_ptr<ShaderInterface> shader, std::string name/*AccessSpecifier p_accessSpecifier*/)
		{
			//TODO: VkDescriptorSetLayout for ssbo
			VkDeviceSize bufferSize = sizeof(T) * MAX_SIZE;

			VkBuffer staging_buffer;
			VkDeviceMemory staging_buffer_memory;

			BufferSettings buffer_settings;
			buffer_settings.size = bufferSize;
			buffer_settings.properties = 
				//VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | 
				VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | 
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
			buffer_settings.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
			UtilityVk::CreateBuffer(buffer_settings, &m_vertexBuffer, &m_vertexBufferMemory);


			auto _shader = reinterpret_cast<ShaderVk*>(shader.get());
			setId = _shader->nameToSet[name];
			createDescriptorSet(
				_shader->descriptorSetLayout[_shader->nameToSet[name]],
				_shader->setSizes[_shader->nameToSet[name]]);
		}

		void createDescriptorPool() {

			auto render = UtilityVk::GetDriver();
			VkDescriptorPoolSize vpPoolSize = {};
			vpPoolSize.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
			vpPoolSize.descriptorCount = static_cast<uint32_t>(render->m_SwapChain.SwapChainImagesSize());

			std::vector<VkDescriptorPoolSize> descriptorPoolSizes = { vpPoolSize };

			VkDescriptorPoolCreateInfo poolCreateInfo = {};
			poolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
			poolCreateInfo.maxSets = static_cast<uint32_t>(render->m_SwapChain.SwapChainImagesSize());
			poolCreateInfo.poolSizeCount = static_cast<uint32_t>(descriptorPoolSizes.size());
			poolCreateInfo.pPoolSizes = descriptorPoolSizes.data();

			VkResult result = vkCreateDescriptorPool(render->m_MainDevice.LogicalDevice, &poolCreateInfo, nullptr, &pool);
			if (result != VK_SUCCESS) {
				throw std::runtime_error("Failed to create a Descriptor Pool!");
			}
		}

		void createDescriptorSet(VkDescriptorSetLayout descriptorSetsLayout, int setSize) {

			auto render = UtilityVk::GetDriver(); createDescriptorPool();

			auto swapchain_size = static_cast<uint32_t>(render->m_SwapChain.SwapChainImagesSize());
			descriptorSets.resize(swapchain_size);

			std::vector<VkDescriptorSetLayout> setLayouts(swapchain_size, descriptorSetsLayout);

			VkDescriptorSetAllocateInfo allocate_info = {};
			allocate_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
			allocate_info.descriptorPool = pool;
			allocate_info.descriptorSetCount = static_cast<uint32_t>(swapchain_size);
			allocate_info.pSetLayouts = setLayouts.data();

			VkResult res = vkAllocateDescriptorSets(render->m_MainDevice.LogicalDevice, &allocate_info, descriptorSets.data());
			if (res != VK_SUCCESS) {
				throw std::runtime_error("Failed to allocate the descriptor sets for the View and Projection matrices!");
			}

			for (size_t i = 0; i < swapchain_size; ++i)
			{
				VkDescriptorBufferInfo vpBufferInfo = {};
				vpBufferInfo.buffer = m_vertexBuffer;
				vpBufferInfo.offset = 0;
				vpBufferInfo.range = sizeof(T) * MAX_SIZE;

				VkWriteDescriptorSet vpSetWrite = {};
				vpSetWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				vpSetWrite.dstSet = descriptorSets[i];
				vpSetWrite.dstBinding = 0;
				vpSetWrite.dstArrayElement = 0;
				vpSetWrite.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
				vpSetWrite.descriptorCount = 1;
				vpSetWrite.pBufferInfo = &vpBufferInfo;

				std::vector<VkWriteDescriptorSet> setWrites = { vpSetWrite };

				vkUpdateDescriptorSets(render->m_MainDevice.LogicalDevice, static_cast<uint32_t>(setWrites.size()), setWrites.data(), 0, nullptr);
			}


		}


		~ShaderStorageBufferVk()
		{

		}
		void bind(const ShaderInterface& shader)
		{
			//VkBuffer vertexBuffers[] = { m_vertexBuffer };
			//VkDeviceSize offsets[] = { 0 };

			//vkCmdBindVertexBuffers(static_cast<const ShaderVk&>(shader).m_CommandHandler.m_CommandBuffers[dynamic_cast<DriverVk*>(DriverVkInstance::GetInstance())->imageIndex], 0, 1, vertexBuffers, offsets);

		}
		void unbind()
		{

		}


		inline void SendBlocks(std::vector<T> data) {

			auto render = UtilityVk::GetDriver(); void* objectData;
			//vmaMapMemory(_allocator, get_current_frame().objectBuffer._allocation, &objectData);
			vkMapMemory(render->m_MainDevice.LogicalDevice,
				m_vertexBufferMemory, 0, sizeof(T) * MAX_SIZE, 0, &objectData);

			T* objectSSBO = (T*)objectData;

			for (int i = 0; i < data.size(); i++) {
				objectSSBO[i] = data[i];
			}

			//vmaUnmapMemory(_allocator, get_current_frame().objectBuffer._allocation);
			vkUnmapMemory(render->m_MainDevice.LogicalDevice,
				m_vertexBufferMemory);

		}

	private:
	};
}
#endif
