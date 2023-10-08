#pragma once
#ifdef OPENGL_BACKEND
#include <memory>
#include <string>

#include "vertexBufferGl.h"
#include "../interface/driverInterface.h"
#include "../interface/renderEnums.h"

namespace IKIGAI
{
	namespace RENDER
	{
		class MeshInterface;

		class DriverGl : public DriverInterface {
		public:
			struct FrameInfo {
				uint64_t mBatchCount = 0;
				uint64_t mInstanceCount = 0;
				uint64_t mPolyCount = 0;
			};
			DriverGl() = default;
			~DriverGl() = default;

			void begin() override {};
			void end() override {};

			virtual void setViewport(const ShaderInterface& shader, float x, float y, float w, float h) override;
			virtual void setScissor(const ShaderInterface& shader, int x, int y, unsigned w, unsigned h) override;

			virtual void drawIndexed(std::shared_ptr<ShaderInterface> shader, size_t indexCount) override;

			virtual void draw(std::shared_ptr<ShaderInterface> shader, size_t vertexCount) override {

			}
			void submit() override {};
			void cleanup() override {}
			int init() override;

			///
			void setClearColor(float red, float green, float blue, float alpha = 1.0f);
			void clear(bool colorBuffer = true, bool depthBuffer = true, bool stencilBuffer = true);
			void setRasterizationLinesWidth(float width);
			void setRasterizationMode(RasterizationMode rasterizationMode);
			void setCapability(RenderingCapability capability, bool value);
			[[nodiscard]] bool getCapability(RenderingCapability capability) const;
			void setStencilAlgorithm(ComparaisonAlgorithm algorithm, int32_t reference, uint32_t mask);
			void setDepthAlgorithm(ComparaisonAlgorithm algorithm);
			void setStencilMask(uint32_t mask);
			void setStencilOperations(Operation stencilFail = Operation::KEEP, Operation depthFail = Operation::KEEP,
				Operation bothPass = Operation::KEEP);
			void setCullFace(CullFace cullFace);
			void setDepthWriting(bool enable);
			void setColorWriting(bool enableRed, bool enableGreen, bool enableBlue, bool enableAlpha);
			void setColorWriting(bool enable);
			void setViewPort(uint32_t x, uint32_t y, uint32_t width, uint32_t height);
			void readPixels(uint32_t x, uint32_t y, uint32_t width, uint32_t height, PixelDataFormat format, PixelDataType type, void* data);
			bool getBool(GLenum parameter);
			bool getBool(GLenum parameter, uint32_t index);
			int getInt(GLenum parameter);
			int getInt(GLenum parameter, uint32_t index);
			float getFloat(GLenum parameter);
			float getFloat(GLenum parameter, uint32_t index);
			double getDouble(GLenum parameter);
			double getDouble(GLenum parameter, uint32_t index);
			int64_t getInt64(GLenum parameter);
			int64_t getInt64(GLenum parameter, uint32_t index);
			std::string getString(GLenum parameter);
			std::string getString(GLenum parameter, uint32_t index);
			void clearFrameInfo();
			void draw(const MeshInterface& mesh, PrimitiveMode primitiveMode = PrimitiveMode::TRIANGLES, uint32_t instances = 1);
			uint8_t fetchGLState();
			void applyStateMask(uint8_t mask);
			void setState(uint8_t state);
			[[nodiscard]] const FrameInfo& getFrameInfo() const;
			void useDepthBufferMask(bool value);
			void useBlendFactors(BlendFactor src, BlendFactor dist);
			void useReversedDepth(bool value);
			void useDepthFunction(DepthFunction function);
			void useCulling(bool value, bool counterClockWise, bool cullBack);
			void drawIndices(PrimitiveMode primitive, size_t indexCount, size_t indexOffset);
			void drawIndicesBaseVertex(PrimitiveMode primitive, size_t indexCount, size_t indexOffset, size_t baseVertex);
			void drawIndicesBaseVertexInstanced(PrimitiveMode primitive, size_t indexCount, size_t indexOffset, size_t baseVertex, size_t instanceCount, size_t baseInstance);
			void setPatchSize(int sz) const;


		private:
			FrameInfo mFrameInfo;
			uint8_t mState = 0;

			void initGlew();
			static void __stdcall GLDebugMessageCallback(uint32_t source, uint32_t type, uint32_t id, uint32_t severity, int32_t length, const char* message, const void* userParam);
		};
	}
}

#endif


