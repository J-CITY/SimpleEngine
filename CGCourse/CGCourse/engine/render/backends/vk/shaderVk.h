#pragma once

#ifdef VULKAN_BACKEND

#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>
#include <vulkan/vulkan.h>

#include "commandHandler.h"
#include "../interface/shaderInterface.h"
#include "../interface/uniformBufferInterface.h"
#include "../interface/reflectionStructs.h"

namespace KUMA::RENDER {
	class UniformVkInterface;

	struct VertexDescriptor {
		enum class Type { VEC2, VEC3, VEC4, MAT3, MAT4, FLOAT, INT, BOOL };
		struct Info {
			std::string name;
			Type type;
			size_t binding = 0;
			size_t location = 0;
			size_t offset = 0;
			size_t size = 0;
		};
		std::vector<Info> member;
		size_t size = 0;
	};

	class ShaderVk : public ShaderInterface {
	public:
		VkPipelineBindPoint piplineType = VK_PIPELINE_BIND_POINT_GRAPHICS;

		std::unordered_map<std::string, int> nameToSet;
		std::unordered_map<std::string, int> nameToBinding;


		inline static VkBool32 depthWriteEnable = VK_FALSE;
		inline static VkCullModeFlags cullMode = VK_CULL_MODE_FRONT_BIT;
		inline static VkFrontFace frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
		inline static std::optional<VkPushConstantRange> push_constant;

		std::vector<int> setSizes;
		CommandHandler m_CommandHandler;

		std::vector<VkDescriptorPool> pools;
		std::vector<VkDescriptorSetLayout> descriptorSetLayout;
		std::unordered_map<size_t, std::vector<VkDescriptorSet>> descriptorSets;
		VkPipelineLayout pipelineLayout;
		VkPipeline graphicsPipeline;

		ShaderVk(VkRenderPass renderPass,
			std::string vertex,
			std::string fragment,
			std::optional<std::string> geometry = std::nullopt,
			std::optional<std::string> tessControl = std::nullopt,
			std::optional<std::string> tesEval = std::nullopt);
		ShaderVk();

		void createCompurePipeline(std::string compudeShaderPath);
		const std::unordered_map<std::string, KUMA::RENDER::UniformInform>& getUniformsInfo() const;

		void setUniform(const UniformBufferInterface& uniform) override;
		void setUniform(const UniformVkInterface& uniform);

		size_t attachmentsCount = 1;
		void getReflection(std::string vertex, std::string fragment, bool needVertDescr = false);
		void create(VkRenderPass renderPass, std::string vertex, std::string fragment,
			std::optional<std::string> geometry = std::nullopt, std::optional<std::string> tessControl = std::nullopt,
			std::optional<std::string> tessEval = std::nullopt);
		VkShaderModule createShaderModule(const std::vector<char>& code);
		void createDescriptorSetLayout();
		void createDescriptorPool();
		void createDescriptorPool(std::vector<std::shared_ptr<TextureVk>> textures);
		void setDescriptorSet(int setId, std::vector<VkDescriptorSet> descriptorSet);
		void createDescriptorSet(int setId, const std::vector<VkBuffer>& uniformBuffers);
		void createDescriptorSet(int setId, std::vector<std::shared_ptr<TextureVk>> ubo);
		//void createDescriptorSets();

		void bindDescriptorSets();
		void setDescriptorSet(int setId, VkDescriptorSet descriptorSet);

		void setPushConstant(const PushConstantInterface& data) override;
		template<class T>
		void setPushConstant(T& data) {
			vkCmdPushConstants(m_CommandHandler.m_CommandBuffers[getImageIndex()], pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(T), &data);
		}

		std::optional<VkVertexInputBindingDescription> bindingDescription;
		std::vector<VkVertexInputAttributeDescription> attributeDescriptions{};

		void createVertexDescriptor(VertexDescriptor descriptor);

		//std::vector<UniformInform> uniforms;

		std::vector<VkSemaphore> sync;

		void bind() override;
		void unbind() override;

		//remove it
		int getImageIndex();

		virtual int getId()
		{
			return 0;
		}

		std::unordered_map<std::string, KUMA::RENDER::UniformInform> mUniforms;
	};
}
#endif
