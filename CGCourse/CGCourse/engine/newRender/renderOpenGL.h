#pragma once
#include <stdexcept>

#include "render.h"

namespace KUMA {
	namespace GL_SYSTEM {
		class GlManager;
	}


    class OpenGLCommandBuffer final : public CommandBuffer {
    public:

        OpenGLCommandBuffer() {};
        void begin() override {};
        void end() override {};


        void clear(long flags, const ClearValue& clearValue) {
            
        }


        void draw(std::uint32_t numVertices, std::uint32_t firstVertex) {
            glDrawArrays(
                renderState_.drawMode,
                static_cast<GLint>(firstVertex),
                static_cast<GLsizei>(numVertices)
            );
        }

        void drawIndexed(std::uint32_t numIndices, std::uint32_t firstIndex) {
            const GLintptr indices = (renderState_.indexBufferOffset + firstIndex * renderState_.indexBufferStride);
            glDrawElements(
                renderState_.drawMode,
                static_cast<GLsizei>(numIndices),
                renderState_.indexBufferDataType,
                reinterpret_cast<const GLvoid*>(indices)
            );
        }
        //
        //void Execute(CommandBuffer& deferredCommandBuffer) override;
        //
        //void UpdateBuffer(
        //    Buffer& dstBuffer,
        //    std::uint64_t   dstOffset,
        //    const void* data,
        //    std::uint16_t   dataSize
        //) override;
        //
        //void CopyBuffer(
        //    Buffer& dstBuffer,
        //    std::uint64_t   dstOffset,
        //    Buffer& srcBuffer,
        //    std::uint64_t   srcOffset,
        //    std::uint64_t   size
        //) override;
        //
        //void CopyBufferFromTexture(
        //    Buffer& dstBuffer,
        //    std::uint64_t           dstOffset,
        //    Texture& srcTexture,
        //    const TextureRegion& srcRegion,
        //    std::uint32_t           rowStride = 0,
        //    std::uint32_t           layerStride = 0
        //) override;
        //
        //void FillBuffer(
        //    Buffer& dstBuffer,
        //    std::uint64_t   dstOffset,
        //    std::uint32_t   value,
        //    std::uint64_t   fillSize = Constants::wholeSize
        //) override;
        //
        //void CopyTexture(
        //    Texture& dstTexture,
        //    const TextureLocation& dstLocation,
        //    Texture& srcTexture,
        //    const TextureLocation& srcLocation,
        //    const Extent3D& extent
        //) override;
        //
        //void CopyTextureFromBuffer(
        //    Texture& dstTexture,
        //    const TextureRegion& dstRegion,
        //    Buffer& srcBuffer,
        //    std::uint64_t           srcOffset,
        //    std::uint32_t           rowStride = 0,
        //    std::uint32_t           layerStride = 0
        //) override;
        //
        //void GenerateMips(Texture& texture) override;
        //void GenerateMips(Texture& texture, const TextureSubresource& subresource) override;
        //
        //void SetViewport(const Viewport& viewport) override;
        //void SetViewports(std::uint32_t numViewports, const Viewport* viewports) override;
        //
        //void SetScissor(const Scissor& scissor) override;
        //void SetScissors(std::uint32_t numScissors, const Scissor* scissors) override;
        //
        //void SetVertexBuffer(Buffer& buffer) override;
        //void SetVertexBufferArray(BufferArray& bufferArray) override;
        //
        //void SetIndexBuffer(Buffer& buffer) override;
        //void SetIndexBuffer(Buffer& buffer, const Format format, std::uint64_t offset = 0) override;
        //
        //void SetResourceHeap(
        //    ResourceHeap& resourceHeap,
        //    std::uint32_t           firstSet = 0,
        //    const PipelineBindPoint bindPoint = PipelineBindPoint::Undefined
        //) override;
        //
        //void SetResource(Resource& resource, std::uint32_t slot, long bindFlags, long stageFlags = StageFlags::AllStages) override;
        //
        //void ResetResourceSlots(
        //    const ResourceType  resourceType,
        //    std::uint32_t       firstSlot,
        //    std::uint32_t       numSlots,
        //    long                bindFlags,
        //    long                stageFlags = StageFlags::AllStages
        //) override;
        //
        //void BeginRenderPass(
        //    RenderTarget& renderTarget,
        //    const RenderPass* renderPass = nullptr,
        //    std::uint32_t       numClearValues = 0,
        //    const ClearValue* clearValues = nullptr
        //) override;
        //
        //void EndRenderPass() override;
        //
        //void Clear(long flags, const ClearValue& clearValue = {}) override;
        //void ClearAttachments(std::uint32_t numAttachments, const AttachmentClear* attachments) override;
        //
        //void SetPipelineState(PipelineState& pipelineState) override;
        //void SetBlendFactor(const ColorRGBAf& color) override;
        //void SetStencilReference(std::uint32_t reference, const StencilFace stencilFace = StencilFace::FrontAndBack) override;
        //
        //void SetUniform(
        //    UniformLocation location,
        //    const void* data,
        //    std::uint32_t   dataSize
        //) override;
        //
        //void SetUniforms(
        //    UniformLocation location,
        //    std::uint32_t   count,
        //    const void* data,
        //    std::uint32_t   dataSize
        //) override;
        //
        //void BeginQuery(QueryHeap& queryHeap, std::uint32_t query = 0) override;
        //void EndQuery(QueryHeap& queryHeap, std::uint32_t query = 0) override;
        //
        //void BeginRenderCondition(QueryHeap& queryHeap, std::uint32_t query = 0, const RenderConditionMode mode = RenderConditionMode::Wait) override;
        //void EndRenderCondition() override;
        //
        //void BeginStreamOutput(std::uint32_t numBuffers, Buffer* const* buffers) override;
        //void EndStreamOutput() override;
        //void Draw(std::uint32_t numVertices, std::uint32_t firstVertex) override;
        //
        //void DrawIndexed(std::uint32_t numIndices, std::uint32_t firstIndex) override;
        //void DrawIndexed(std::uint32_t numIndices, std::uint32_t firstIndex, std::int32_t vertexOffset) override;
        //
        //void DrawInstanced(std::uint32_t numVertices, std::uint32_t firstVertex, std::uint32_t numInstances) override;
        //void DrawInstanced(std::uint32_t numVertices, std::uint32_t firstVertex, std::uint32_t numInstances, std::uint32_t firstInstance) override;
        //
        //void DrawIndexedInstanced(std::uint32_t numIndices, std::uint32_t numInstances, std::uint32_t firstIndex) override;
        //void DrawIndexedInstanced(std::uint32_t numIndices, std::uint32_t numInstances, std::uint32_t firstIndex, std::int32_t vertexOffset) override;
        //void DrawIndexedInstanced(std::uint32_t numIndices, std::uint32_t numInstances, std::uint32_t firstIndex, std::int32_t vertexOffset, std::uint32_t firstInstance) override;
        //
        //void DrawIndirect(Buffer& buffer, std::uint64_t offset) override;
        //void DrawIndirect(Buffer& buffer, std::uint64_t offset, std::uint32_t numCommands, std::uint32_t stride) override;
        //
        //void DrawIndexedIndirect(Buffer& buffer, std::uint64_t offset) override;
        //void DrawIndexedIndirect(Buffer& buffer, std::uint64_t offset, std::uint32_t numCommands, std::uint32_t stride) override;
        //
        //void Dispatch(std::uint32_t numWorkGroupsX, std::uint32_t numWorkGroupsY, std::uint32_t numWorkGroupsZ) override;
        //void DispatchIndirect(Buffer& buffer, std::uint64_t offset) override;
        //
        //void PushDebugGroup(const char* name) override;
        //void PopDebugGroup() override;
        //
        //bool IsImmediateCmdBuffer() const override;

    };

    class OpenGLRenderSystem : public RenderSystem {
        //std::unique_ptr<GLCommandQueue> commandQueue_;
        std::vector<std::unique_ptr<OpenGLCommandBuffer>> commandBuffers_;
        
        CommandBuffer* createCommandBuffer(const CommandBufferDescriptor& desc) {
        	commandBuffers_.push_back(std::make_unique<OpenGLCommandBuffer>());
        	return commandBuffers_.back().get();
        }
    };
}
