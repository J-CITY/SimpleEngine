#pragma once

#ifdef VULKAN_BACKEND
#include <variant>
#include <memory>

#include "shaderVk.h"
#include "textureVk.h"
#include "uniformBufferVk.h"
#include <resourceModule/serviceManager.h>
#include "../interface/materialInterface.h"
#include "resourceModule/serializerInterface.h"


namespace IKIGAI::RENDER {
	class UniformBuffer;

	class MaterialVk : public MaterialInterface {
	public:
		void bind(std::shared_ptr<TextureInterface> defaultTexture, bool useTextures) override{};
		void unbind() override{};
		std::shared_ptr<ShaderInterface> getShader() const override { return nullptr; };
		void setShader(std::shared_ptr<ShaderInterface> shader) override{};
		bool hasShader() const override { return false; };
		MaterialResource getDescriptor() override {
			return MaterialResource();
		};
		void create(const MaterialResource& res) override {
			//TODO
		};
	};
}
#endif
