#pragma once


#include "../interface/shaderInterface.h"
#include <memory>

namespace IKIGAI::RENDER {
	struct RenderSettings {
		enum class Backend {
			OPENGL, VULKAN, DIRECTX12
		};
		Backend backend = Backend::OPENGL;

	};


	class DriverInterface {
	public:
		virtual ~DriverInterface() = default;

		virtual int init() = 0;
		virtual void begin() = 0;
		virtual void end() = 0;

		virtual void setViewport(const ShaderInterface& shader, float x, float y, float w, float h) = 0;
		virtual void setScissor(const ShaderInterface& shader, int x, int y, unsigned w, unsigned h) = 0;
		virtual void drawIndexed(std::shared_ptr<ShaderInterface> shader, size_t indexCount) = 0;
		virtual void draw(std::shared_ptr<ShaderInterface> shader, size_t vertexCount) = 0;
		virtual void submit() = 0;
		virtual void cleanup() = 0;

		inline static RenderSettings settings;
	};
}

