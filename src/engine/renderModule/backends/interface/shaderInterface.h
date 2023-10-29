#pragma once

#include <unordered_map>
#include <string>
#include <optional>


namespace IKIGAI::RENDER {
	class PushConstantInterface;
	class UniformBufferInterface;
	class UniformVkInterface;
	struct UniformInform;

	class ShaderInterface {
	public:
		std::optional<std::string> fragmentPath;
		std::optional<std::string> vertexPath;

		virtual ~ShaderInterface() = default;

		virtual void bind() = 0;
		virtual void unbind() = 0;
		virtual int getId() = 0;
		virtual void setUniform(const UniformBufferInterface& uniform) = 0;
		virtual void setPushConstant(const PushConstantInterface& uniform) = 0;

		virtual const std::unordered_map<std::string, IKIGAI::RENDER::UniformInform>& getUniformsInfo() const = 0;
	};
}

