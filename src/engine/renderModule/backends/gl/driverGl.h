#pragma once
#include <bitset>
#include <memory>

#include "renderModule/backends/interface/storageBufferInterface.h"
#include "renderModule/backends/interface/textureInterface.h"
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
		class StorageBufferGl;
		class UniformBufferGl;
		class TextureGl;
		class IndexBufferInterface;
		class IndexBufferGl;
		class MeshInterface;
		class ShaderGl;

		class DriverGl : public DriverInterface {
		public:
			DriverGl();
			~DriverGl() override;

			std::shared_ptr<VertexBufferGl> mVertexBuffer;
			std::shared_ptr<IndexBufferGl> mIndexBuffer;
			std::shared_ptr<ShaderGl> mShader;

			unsigned mWidth = 0;
			unsigned mHeight = 0;

			MATH::Vector4f mClearColor = {0.0f, 0.0f, 0.0f, 1.0f};
			float mDepthValue = 1.0f;
			float mStencilValue = 1.0f;

			RasterizationMode mRasterization = RasterizationMode::FILL;
			PrimitiveMode mPrimitiveMode = PrimitiveMode::TRIANGLES;
			std::optional<Viewport> mViewport;
			std::optional<Scissor> mScissor;
			CullFace mCullFace = CullFace::NONE;
			TriangleOrientation mTriangleOrientation = TriangleOrientation::CW;
			std::optional<Depth> mDepth;
			std::optional<Blending> mBlend;
			std::optional<Stencil> mStencil;
			bool mMSAA = false;

			void setVertexBuffer(std::shared_ptr<VertexBufferInterface> buffer) override;
			void setIndexBuffer(std::shared_ptr<IndexBufferInterface> buffer) override;
			void setShader(std::shared_ptr<ShaderInterface> shader) override;
			void onResize();// override;

			void setMSAA(bool value);
			void setPrimitiveMode(PrimitiveMode param);
			void setRasterization(RasterizationMode param);
			void setViewport(const Viewport& param);
			void resetViewport();
			void setScissor(const Scissor& param);
			void resetScissor();
			void setBlend(const Blending& param);
			void resetBlend();
			void setDepth(const Depth& param);
			void resetDepth();
			void setStencil(const Stencil& param);
			void resetStencil();
			void setCull(CullFace param);
			void setClearColor(const MATH::Vector4f& color);
			void setClearColor(float r, float g, float b, float a);
			void clear(bool clearColor, bool clearDepth, bool clearStencil);
			void draw(uint32_t vertex_count, uint32_t vertex_offset, uint32_t instance_count);
			void drawIndexed(uint32_t index_count, uint32_t index_offset, uint32_t instance_count);
			void draw(const MeshInterface& mesh, PrimitiveMode primitive, uint32_t instances);
			void setTexture(size_t bind, std::shared_ptr<TextureInterface> data);
			void setUniformBuffer(size_t bind, std::shared_ptr<UniformBufferInterface> data);
			void setStorageBuffer(size_t bind, std::shared_ptr<StorageBufferInterface> data);
			void setTexture(const std::string& name, std::shared_ptr<TextureInterface> data);
			void setUniformBuffer(const std::string& name, std::shared_ptr<UniformBufferInterface> data);
			void setStorageBuffer(const std::string& name, std::shared_ptr<StorageBufferInterface> data);
			void applyState();

			//void begin() override {};
			//void end() override {};
			//
			//virtual void setViewport(const ShaderInterface& shader, float x, float y, float w, float h) override;
			//virtual void setScissor(const ShaderInterface& shader, int x, int y, unsigned w, unsigned h) override;
			//
			//virtual void drawIndexed(std::shared_ptr<ShaderInterface> shader, size_t indexCount) override;
			//
			//virtual void draw(std::shared_ptr<ShaderInterface> shader, size_t vertexCount) override {
			//
			//}
			//void submit() override {};
			//void cleanup() override {}
			//
			/////
			//void setClearColor(float red, float green, float blue, float alpha = 1.0f);
			//void clear(bool colorBuffer = true, bool depthBuffer = true, bool stencilBuffer = true);
			//void setRasterizationLinesWidth(float width);
			//void setRasterizationMode(RasterizationMode rasterizationMode);
			//void setCapability(RenderingCapability capability, bool value);
			//[[nodiscard]] bool getCapability(RenderingCapability capability) const;
			//void setStencilAlgorithm(ComparaisonAlgorithm algorithm, int32_t reference, uint32_t mask);
			//void setDepthAlgorithm(ComparaisonAlgorithm algorithm);
			//void setStencilMask(uint32_t mask);
			//void setStencilOperations(Operation stencilFail = Operation::KEEP, Operation depthFail = Operation::KEEP,
			//	Operation bothPass = Operation::KEEP);
			//void setCullFace(CullFace cullFace);
			//void setDepthWriting(bool enable);
			//void setColorWriting(bool enableRed, bool enableGreen, bool enableBlue, bool enableAlpha);
			//void setColorWriting(bool enable);
			//void setViewPort(uint32_t x, uint32_t y, uint32_t width, uint32_t height);
			//void readPixels(uint32_t x, uint32_t y, uint32_t width, uint32_t height, PixelDataFormat format, PixelDataType type, void* data);
			//bool getBool(GLenum parameter);
			//bool getBool(GLenum parameter, uint32_t index);
			//int getInt(GLenum parameter);
			//int getInt(GLenum parameter, uint32_t index);
			//float getFloat(GLenum parameter);
			//float getFloat(GLenum parameter, uint32_t index);
			//double getDouble(GLenum parameter);
			//double getDouble(GLenum parameter, uint32_t index);
			//int64_t getInt64(GLenum parameter);
			//int64_t getInt64(GLenum parameter, uint32_t index);
			//std::string getString(GLenum parameter);
			//std::string getString(GLenum parameter, uint32_t index);
			//void clearFrameInfo();
			//void draw(const MeshInterface& mesh, PrimitiveMode primitiveMode = PrimitiveMode::TRIANGLES, uint32_t instances = 1);
			//uint8_t fetchGLState();
			//void applyStateMask(uint8_t mask);
			//void setState(uint8_t state);
			//[[nodiscard]] const FrameInfo& getFrameInfo() const;
			//void useDepthBufferMask(bool value);
			//void useBlendFactors(BlendFactor src, BlendFactor dist);
			//void useReversedDepth(bool value);
			//void useDepthFunction(DepthFunction function);
			//void useCulling(bool value, bool counterClockWise, bool cullBack);
			//void drawIndices(PrimitiveMode primitive, size_t indexCount, size_t indexOffset);
			//void drawIndicesBaseVertex(PrimitiveMode primitive, size_t indexCount, size_t indexOffset, size_t baseVertex);
			//void drawIndicesBaseVertexInstanced(PrimitiveMode primitive, size_t indexCount, size_t indexOffset, size_t baseVertex, size_t instanceCount, size_t baseInstance);
			//void setPatchSize(int sz) const;

			std::bitset<static_cast<int>(Dirty::SIZE)> mDirty;

			void setDirty(Dirty flag) {
				mDirty.set(static_cast<int>(flag), true);
			}
			void clearDirty(Dirty flag) {
				mDirty.set(static_cast<int>(flag), false);
			}
			bool isDirty(Dirty flag) const {
				return mDirty.test(static_cast<int>(flag));
			}

			std::map<size_t, std::shared_ptr<TextureGl>> mTextures{};
			std::map<size_t, std::shared_ptr<UniformBufferGl>> mUniformBuffers{};
			std::map<size_t, std::shared_ptr<StorageBufferGl>> mStorageBuffers{};
		private:
			void init() override;

			FrameInfo mFrameInfo;
			uint8_t mState = 0;

			void initGlew();
#ifndef OCULUS
			static void __stdcall GLDebugMessageCallback(uint32_t source, uint32_t type, uint32_t id, uint32_t severity, int32_t length, const char* message, const void* userParam);

		public:
			void begin() override{};
			void end() override{};
			void submit() override;;
			void cleanup() override{};
			void setBlending(const Blending& value) override{};
			void resetBlending() override{};
			void setTriangleOrientation(TriangleOrientation value) override{};

		private:
#endif
		};
	}
}

#endif


