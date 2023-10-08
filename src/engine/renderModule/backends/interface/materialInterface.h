#pragma once

#include <memory>
#include "shaderInterface.h"


namespace IKIGAI::RENDER {
	class TextureInterface;
	class MaterialInterface {
	public:
		virtual ~MaterialInterface() = default;

		virtual void bind(std::shared_ptr<TextureInterface> defaultTexture, bool useTextures) = 0;
		virtual void unbind() = 0;

		virtual unsigned getGPUInstances() const = 0;
		virtual bool hasShader() const = 0;
		virtual std::shared_ptr<ShaderInterface> getShader() const = 0;

		virtual bool isDeferred() const = 0;
		virtual bool isBlendable() const = 0;
		virtual bool isCastShadow() const = 0;
		virtual bool isShadowBaked() const = 0;
		virtual bool isBackfaceCulling() const = 0;
		virtual bool isFrontfaceCulling() const = 0;
		virtual bool isDepthTest() const = 0;
		virtual bool isDepthWriting() const = 0;
		virtual bool isColorWriting() const = 0;

		virtual void setIsDeferred(bool v, std::shared_ptr<ShaderInterface> shader) = 0;

		std::string mPath;
	};
}
