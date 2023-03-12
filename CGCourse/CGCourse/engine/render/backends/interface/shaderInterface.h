#pragma once

#include <unordered_map>
#include <string>


namespace KUMA::RENDER {
	class PushConstantInterface;
	class UniformBufferInterface;
	class UniformVkInterface;
	struct UniformInform;

	class ShaderInterface {
	public:

		virtual ~ShaderInterface() = default;

		virtual void bind() = 0;
		virtual void unbind() = 0;
		virtual int getId() = 0;
		virtual void setUniform(const UniformBufferInterface& uniform) = 0;
		virtual void setPushConstant(const PushConstantInterface& uniform) = 0;

		virtual const std::unordered_map<std::string, KUMA::RENDER::UniformInform>& getUniformsInfo() const = 0;
	};
}

