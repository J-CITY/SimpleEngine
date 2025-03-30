#pragma once


#include "../interface/shaderInterface.h"
#include <memory>

namespace IKIGAI::RENDER {
	class IndexBufferInterface;
	class VertexBufferInterface;
	class ShaderInterface;

	struct RenderSettings {
		enum class Backend {
			OPENGL, VULKAN, DIRECTX12
		};
		Backend backend = Backend::OPENGL;

	};


	class DriverInterface {
	protected:
		enum class Dirty {
			VIEWPORT = 0,
			SCISSOR,
			PRIMITIVE_MODE,
			RASTERIZATION_MODE,
			VERTEX_BUFFER,
			INDEX_BUFFER,

			DEPTH,
			STENCIL,
			BLENDING,
			CULLING,
			TRIANGULATION_ORDER,

			PIPELINE,

			SIZE
		};

		struct FrameInfo {
			uint64_t mBatchCount = 0;
			uint64_t mInstanceCount = 0;
			uint64_t mPolyCount = 0;
		};
	public:
		virtual ~DriverInterface() = default;

		virtual void init() = 0;
		virtual void begin() = 0;
		virtual void end() = 0;

		virtual void submit() = 0;
		virtual void cleanup() = 0;

		//virtual void resize(size_t width, size_t height) = 0;

		virtual void setPrimitiveMode(PrimitiveMode topology) = 0;
		virtual void setRasterization(RasterizationMode mode) = 0;
		virtual void setViewport(const Viewport& viewport) = 0;
		virtual void resetViewport() = 0;
		virtual void setScissor(const Scissor& scissor) = 0;
		virtual void resetScissor() = 0;
		virtual void setShader(std::shared_ptr<ShaderInterface> shader) = 0;
		virtual void setVertexBuffer(std::shared_ptr<VertexBufferInterface> buffer) = 0;
		virtual void setIndexBuffer(std::shared_ptr<IndexBufferInterface> buffer) = 0;
		virtual void setBlending(const Blending& value) = 0;
		virtual void resetBlending() = 0;
		virtual void setDepth(const Depth& depth) = 0;
		virtual void resetDepth() = 0;
		virtual void setStencil(const Stencil& stencil) = 0;
		virtual void resetStencil() = 0;
		virtual void setCull(CullFace cull_mode) = 0;
		virtual void setTriangleOrientation(TriangleOrientation value) = 0;
		virtual void clear(bool clearColor, bool clearDepth, bool clearStencil) = 0;
		virtual void setClearColor(const MATH::Vector4f& color) = 0;
		virtual void setClearColor(float r, float g, float b, float a) = 0;
		virtual void draw(uint32_t count, uint32_t offset, uint32_t instance) = 0;
		virtual void drawIndexed(uint32_t count, uint32_t offset, uint32_t instance) = 0;
		

		inline static RenderSettings settings;
	};
}

