#pragma once

#ifdef VULKAN_BACKEND
#include <stdexcept>
#include <vector>
#include <vulkan/vulkan.h>
#include "driverVk.h"
#include "../interface/uniformBufferInterface.h"
#include <resourceModule/serviceManager.h>
#include "textureVk.h"
#include "shaderVk.h"
namespace IKIGAI::RENDER {
	class UniformVkInterface : public UniformBufferInterface {
	public:
		virtual const std::vector<VkDescriptorSet>& getDesctiptorSets() const = 0;

	};

	class UniformTexturesVk : public UniformVkInterface {
	public:
		std::vector<std::shared_ptr<TextureInterface>> textures;

		std::vector<VkDescriptorSet> descriptorSets;
		VkDescriptorPool pool;
		int setId = 0;
		UniformTexturesVk(std::shared_ptr<ShaderInterface> shader, std::string name, std::vector<std::shared_ptr<TextureInterface>> textures) {
			auto _shader = std::dynamic_pointer_cast<ShaderVk>(shader);
			setId = _shader->nameToSet[name];
			createDescriptorSet(
				_shader->descriptorSetLayout[setId],
				textures);
		}

		UniformTexturesVk(std::shared_ptr<ShaderInterface> shader, int _setId, std::vector<std::shared_ptr<TextureInterface>> textures) {
			auto _shader = std::dynamic_pointer_cast<ShaderVk>(shader);
			setId = _setId;
			createDescriptorSet(
				_shader->descriptorSetLayout[setId],
				textures);
		}

		virtual int getSetId() const override {
			return setId;
		}

		virtual const std::vector<VkDescriptorSet>& getDesctiptorSets() const override {
			return descriptorSets;
		}

		void createDescriptorPool(int texturesSz) {
			auto render = UtilityVk::GetDriver();

			std::vector<VkDescriptorPoolSize> pool_sizes;
			for (int i = 0; i < texturesSz; i++) {
				VkDescriptorPoolSize position_pool_size = {};
				position_pool_size.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
				position_pool_size.descriptorCount = static_cast<uint32_t>(render->m_SwapChain.SwapChainImagesSize());
				pool_sizes.push_back(position_pool_size);
			}

			VkDescriptorPoolCreateInfo pool_create_info = {};
			pool_create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
			pool_create_info.maxSets = static_cast<uint32_t>(render->m_SwapChain.SwapChainImagesSize());
			pool_create_info.poolSizeCount = static_cast<uint32_t>(pool_sizes.size());
			pool_create_info.pPoolSizes = pool_sizes.data();

			VkResult result = vkCreateDescriptorPool(render->m_MainDevice.LogicalDevice,
				&pool_create_info, nullptr, &pool);
			if (result != VK_SUCCESS) {
				throw std::runtime_error("Failed to create the Input Descriptor Pool!");
			}
		}

		void createDescriptorSet(VkDescriptorSetLayout descriptorSetsLayout, std::vector<std::shared_ptr<TextureInterface>>& textures) {

			auto render = UtilityVk::GetDriver();
			createDescriptorPool(textures.size());
			auto swapchain_size = static_cast<uint32_t>(render->m_SwapChain.SwapChainImagesSize());
			descriptorSets.resize(swapchain_size);

			std::vector<VkDescriptorSetLayout> inputSetLayouts(swapchain_size, descriptorSetsLayout);

			VkDescriptorSetAllocateInfo input_allocate_info = {};
			input_allocate_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
			input_allocate_info.descriptorPool = pool;
			input_allocate_info.descriptorSetCount = static_cast<uint32_t>(swapchain_size);
			input_allocate_info.pSetLayouts = inputSetLayouts.data();

			VkResult result = vkAllocateDescriptorSets(render->m_MainDevice.LogicalDevice, &input_allocate_info, descriptorSets.data());
			if (result != VK_SUCCESS) {
				throw std::runtime_error("Failed to allocate Input Attachment Descriptor Sets!");
			}

			for (size_t i = 0; i < swapchain_size; i++) {
				std::vector<VkWriteDescriptorSet> setWrites;
				std::list<VkDescriptorImageInfo> save;

				int binding = 0;
				for (auto tex : textures) {

					int id = reinterpret_cast<TextureVk*>(tex.get())->ImageView.size() == 1 ? 0 : i;
					VkDescriptorImageInfo positionImageInfo = {};
					positionImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
					positionImageInfo.imageView = reinterpret_cast<TextureVk*>(tex.get())->ImageView[id];
					positionImageInfo.sampler = reinterpret_cast<TextureVk*>(tex.get())->Sampler[id];
					save.push_back(positionImageInfo);

					VkWriteDescriptorSet positionWrite = {};
					positionWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
					positionWrite.dstSet = descriptorSets[i];
					positionWrite.dstBinding = binding;
					positionWrite.dstArrayElement = 0;
					positionWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
					positionWrite.descriptorCount = 1;
					positionWrite.pImageInfo = &save.back();

					binding++;
					setWrites.push_back(positionWrite);
				}
				vkUpdateDescriptorSets(render->m_MainDevice.LogicalDevice,
					static_cast<uint32_t>(setWrites.size()),
					setWrites.data(), 0,
					nullptr);
			}
		}
	};

	template<class T>
	class PushConstantVk : public PushConstantInterface {
	public:
		T data;
	};

	template<class T>
	class UniformBufferVk : public UniformVkInterface {
	public:
		std::vector<VkDeviceMemory> uniformBuffersMemory;
		std::vector<VkBuffer> uniformBuffers;
		//std::vector<void*> uniformBuffersMapped;

		std::vector<VkDescriptorSet> descriptorSets;
		VkDescriptorPool pool;

		int setId = 0;

		UniformBufferVk(std::shared_ptr<ShaderInterface> shader, std::string name, int size = 1) {
			auto _shader = std::dynamic_pointer_cast<ShaderVk>(shader);
			create(size > 1 ? _shader->setSizes[_shader->nameToSet[name]] : sizeof(T));

			//shader->createDescriptorSet(shader->nameToSet[name], uniformBuffers);
			setId = _shader->nameToSet[name];
			createDescriptorSet(
				_shader->descriptorSetLayout[_shader->nameToSet[name]],
				_shader->setSizes[_shader->nameToSet[name]]);
		}

		UniformBufferVk(std::shared_ptr<ShaderInterface> shader, std::string name, const std::vector<unsigned char>& vec) {
			auto render = UtilityVk::GetDriver();

			BufferSettings buffer_settings;
			buffer_settings.size = sizeof(unsigned char) * vec.size();
			buffer_settings.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
			buffer_settings.properties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

			uniformBuffers.resize(render->m_SwapChain.SwapChainImagesSize());
			uniformBuffersMemory.resize(render->m_SwapChain.SwapChainImagesSize());
			//uniformBuffersMapped.resize(MAX_FRAMES_IN_FLIGHT);

			for (size_t i = 0; i < render->m_SwapChain.SwapChainImagesSize(); i++) {
				UtilityVk::CreateBuffer(buffer_settings, &uniformBuffers[i], &uniformBuffersMemory[i]);
			}
			////////////////

			auto _shader = std::dynamic_pointer_cast<ShaderVk>(shader);
			//shader->createDescriptorSet(shader->nameToSet[name], uniformBuffers);
			setId = _shader->nameToSet[name];
			createDescriptorSet(
				_shader->descriptorSetLayout[_shader->nameToSet[name]],
				_shader->setSizes[_shader->nameToSet[name]]);
		}

		UniformBufferVk() = default;

		virtual int getSetId()  const override {
			return setId;
		}

		virtual const std::vector<VkDescriptorSet>& getDesctiptorSets() const override {
			return descriptorSets;
		}

		void createDescriptorPool() {
			auto render = UtilityVk::GetDriver();

			VkDescriptorPoolSize vpPoolSize = {};
			vpPoolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			vpPoolSize.descriptorCount = static_cast<uint32_t>(render->m_SwapChain.SwapChainImagesSize());

			// MODEL (DYNAMIC UBO)
			/*VkDescriptorPoolSize modelPoolSize = {};
			modelPoolSize.type			  = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
			modelPoolSize.descriptorCount = static_cast<uint32_t>(m_modelDynamicUBO.size()); */

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
			createDescriptorPool();
			auto render = UtilityVk::GetDriver();

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
				vpBufferInfo.buffer = uniformBuffers[i];
				vpBufferInfo.offset = 0;
				vpBufferInfo.range = setSize;

				VkWriteDescriptorSet vpSetWrite = {};
				vpSetWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				vpSetWrite.dstSet = descriptorSets[i];
				vpSetWrite.dstBinding = 0;
				vpSetWrite.dstArrayElement = 0;
				vpSetWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
				vpSetWrite.descriptorCount = 1;
				vpSetWrite.pBufferInfo = &vpBufferInfo;

				std::vector<VkWriteDescriptorSet> setWrites = { vpSetWrite };

				vkUpdateDescriptorSets(render->m_MainDevice.LogicalDevice, static_cast<uint32_t>(setWrites.size()), setWrites.data(), 0, nullptr);
			}


		}

		void create(int size) {
			auto render = UtilityVk::GetDriver();

			BufferSettings buffer_settings;
			buffer_settings.size = size;
			buffer_settings.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
			buffer_settings.properties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

			uniformBuffers.resize(render->m_SwapChain.SwapChainImagesSize());
			uniformBuffersMemory.resize(render->m_SwapChain.SwapChainImagesSize());
			//uniformBuffersMapped.resize(MAX_FRAMES_IN_FLIGHT);

			for (size_t i = 0; i < render->m_SwapChain.SwapChainImagesSize(); i++) {
				UtilityVk::CreateBuffer(buffer_settings, &uniformBuffers[i], &uniformBuffersMemory[i]);
			}
		}

		void set(T& ubo) {
			auto render = UtilityVk::GetDriver();

			auto imageIndex = render->imageIndex;
			void* vp_data;
			vkMapMemory(render->m_MainDevice.LogicalDevice, uniformBuffersMemory[imageIndex], 0,
				sizeof(T), 0, &vp_data);
			memcpy(vp_data, &ubo, sizeof(T));
			vkUnmapMemory(render->m_MainDevice.LogicalDevice, uniformBuffersMemory[imageIndex]);
		}

		void setBytes(std::vector<unsigned char>& ubo) {
			auto render = UtilityVk::GetDriver();

			auto imageIndex = render->imageIndex;
			void* vp_data;
			vkMapMemory(render->m_MainDevice.LogicalDevice, uniformBuffersMemory[imageIndex], 0,
				sizeof(unsigned char) * ubo.size(), 0, &vp_data);
			memcpy(vp_data, ubo.data(), sizeof(unsigned char) * ubo.size());
			vkUnmapMemory(render->m_MainDevice.LogicalDevice, uniformBuffersMemory[imageIndex]);
		}

		void set(std::vector<T>& ubo) {
			auto render = UtilityVk::GetDriver();

			auto imageIndex = render->imageIndex;
			void* vp_data;
			vkMapMemory(render->m_MainDevice.LogicalDevice, uniformBuffersMemory[imageIndex], 0,
				sizeof(T) * ubo.size(), 0, &vp_data);
			memcpy(vp_data, ubo.data(), sizeof(T) * ubo.size());
			vkUnmapMemory(render->m_MainDevice.LogicalDevice, uniformBuffersMemory[imageIndex]);
		}
	};



}
#endif
