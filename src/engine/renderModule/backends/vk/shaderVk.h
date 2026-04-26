#pragma once

#ifdef VULKAN_BACKEND

#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>
#include <vulkan/vulkan_raii.hpp>

#include "commandHandler.h"
#include "../interface/shaderInterface.h"
#include "../interface/uniformBufferInterface.h"
#include "../interface/reflectionStructs.h"

namespace IKIGAI::RENDER {
	class TextureVk;
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

		vk::raii::DescriptorSetLayout mDescriptorSetLayout = nullptr;
		vk::raii::PipelineLayout mPipelineLayout = nullptr;
		vk::raii::ShaderModule mVertexShaderModule = nullptr;
		vk::raii::ShaderModule mFragmentShaderModule = nullptr;
		std::vector<vk::DescriptorSetLayoutBinding> mRequiredDescriptorBindings;

		ShaderVk(std::map<ShaderType, std::string> shaderCode);

		void _getReflection(std::string path, ShaderType type);
		std::tuple<vk::raii::PipelineLayout, vk::raii::DescriptorSetLayout, std::vector<vk::DescriptorSetLayoutBinding>> createPipelineLayout();
		static std::shared_ptr<ShaderVk> CreateFromPath(std::map<ShaderType, std::string> path);
		void bind() override{};
		void unbind() override{};
		int getId() override { return 0; };
		void setUniform(const UniformBufferInterface& uniform) override{};
		void setPushConstant(const PushConstantInterface& uniform) override{};
		void recompile(const ShaderResource& res) override {}
		const std::unordered_map<std::string, IKIGAI::RENDER::UniformInform>& getUniformsInfo() const override
		{
			return std::unordered_map<std::string, IKIGAI::RENDER::UniformInform>();
		};
	};
}
#endif
