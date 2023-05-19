#pragma once

import glmath;
#include <memory>
#include <string>
#include <vector>

namespace KUMA {
	class RenderPass;
	struct Viewport;
	struct Scissor;
	class CommandBuffer;

	class SwapChainDescriptor {
    public:
        SwapChainDescriptor();
        MATHGL::Vector2i       Resolution;
        unsigned int   Samples;
        int            ColorBits;
        int            DepthBits;
        int            StencilBits;
        unsigned int   SwapBuffers;
        bool           Fullscreen;
    };

    enum class ResourceType {
        Undefined,
        Buffer,
        Texture,
        Sampler,
    };

    enum class Format
    {
        Undefined,          //!< Undefined format.

        /* --- Alpha channel color formats --- */
        A8UNorm,            //!< Alpha channel format: alpha 8-bit normalized unsigned integer component.

        /* --- Red channel color formats --- */
        R8UNorm,            //!< Ordinary color format: red 8-bit normalized unsigned integer component.
        R8SNorm,            //!< Ordinary color format: red 8-bit normalized signed integer component.
        R8UInt,             //!< Ordinary color format: red 8-bit unsigned integer component.
        R8SInt,             //!< Ordinary color format: red 8-bit signed integer component.

        R16UNorm,           //!< Ordinary color format: red 16-bit normalized unsigned interger component.
        R16SNorm,           //!< Ordinary color format: red 16-bit normalized signed interger component.
        R16UInt,            //!< Ordinary color format: red 16-bit unsigned interger component.
        R16SInt,            //!< Ordinary color format: red 16-bit signed interger component.
        R16Float,           //!< Ordinary color format: red 16-bit floating point component.

        R32UInt,            //!< Ordinary color format: red 32-bit unsigned interger component.
        R32SInt,            //!< Ordinary color format: red 32-bit signed interger component.
        R32Float,           //!< Ordinary color format: red 32-bit floating point component.

        R64Float,           //!< Ordinary color format: red 64-bit floating point component. \note Only supported with: Vulkan.

        /* --- RG color formats --- */
        RG8UNorm,           //!< Ordinary color format: red, green 8-bit normalized unsigned integer components.
        RG8SNorm,           //!< Ordinary color format: red, green 8-bit normalized signed integer components.
        RG8UInt,            //!< Ordinary color format: red, green 8-bit unsigned integer components.
        RG8SInt,            //!< Ordinary color format: red, green 8-bit signed integer components.

        RG16UNorm,          //!< Ordinary color format: red, green 16-bit normalized unsigned interger components.
        RG16SNorm,          //!< Ordinary color format: red, green 16-bit normalized signed interger components.
        RG16UInt,           //!< Ordinary color format: red, green 16-bit unsigned interger components.
        RG16SInt,           //!< Ordinary color format: red, green 16-bit signed interger components.
        RG16Float,          //!< Ordinary color format: red, green 16-bit floating point components.

        RG32UInt,           //!< Ordinary color format: red, green 32-bit unsigned interger components.
        RG32SInt,           //!< Ordinary color format: red, green 32-bit signed interger components.
        RG32Float,          //!< Ordinary color format: red, green 32-bit floating point components.

        RG64Float,          //!< Ordinary color format: red, green 64-bit floating point components. \note Only supported with: Vulkan.

        /* --- RGB color formats --- */
        RGB8UNorm,          //!< Ordinary color format: red, green, blue 8-bit normalized unsigned integer components. \note Only supported with: OpenGL, Vulkan.
        RGB8UNorm_sRGB,     //!< Ordinary color format: red, green, blue 8-bit normalized unsigned integer components in non-linear sRGB color space. \note Only supported with: OpenGL, Vulkan.
        RGB8SNorm,          //!< Ordinary color format: red, green, blue 8-bit normalized signed integer components. \note Only supported with: OpenGL, Vulkan.
        RGB8UInt,           //!< Ordinary color format: red, green, blue 8-bit unsigned integer components. \note Only supported with: OpenGL, Vulkan.
        RGB8SInt,           //!< Ordinary color format: red, green, blue 8-bit signed integer components. \note Only supported with: OpenGL, Vulkan.

        RGB16UNorm,         //!< Ordinary color format: red, green, blue 16-bit normalized unsigned interger components. \note Only supported with: OpenGL, Vulkan.
        RGB16SNorm,         //!< Ordinary color format: red, green, blue 16-bit normalized signed interger components. \note Only supported with: OpenGL, Vulkan.
        RGB16UInt,          //!< Ordinary color format: red, green, blue 16-bit unsigned interger components. \note Only supported with: OpenGL, Vulkan.
        RGB16SInt,          //!< Ordinary color format: red, green, blue 16-bit signed interger components. \note Only supported with: OpenGL, Vulkan.
        RGB16Float,         //!< Ordinary color format: red, green, blue 16-bit floating point components. \note Only supported with: OpenGL, Vulkan.

        RGB32UInt,          //!< Ordinary color format: red, green, blue 32-bit unsigned interger components. \note As texture format only supported with: OpenGL, Vulkan, Direct3D 11, Direct3D 12.
        RGB32SInt,          //!< Ordinary color format: red, green, blue 32-bit signed interger components. \note As texture format only supported with: OpenGL, Vulkan, Direct3D 11, Direct3D 12.
        RGB32Float,         //!< Ordinary color format: red, green, blue 32-bit floating point components. \note As texture format only supported with: OpenGL, Vulkan, Direct3D 11, Direct3D 12.

        RGB64Float,         //!< Ordinary color format: red, green, blue 64-bit floating point components. \note Only supported with: Vulkan.

        /* --- RGBA color formats --- */
        RGBA8UNorm,         //!< Ordinary color format: red, green, blue, alpha 8-bit normalized unsigned integer components.
        RGBA8UNorm_sRGB,    //!< Ordinary color format: red, green, blue, alpha 8-bit normalized unsigned integer components in non-linear sRGB color space.
        RGBA8SNorm,         //!< Ordinary color format: red, green, blue, alpha 8-bit normalized signed integer components.
        RGBA8UInt,          //!< Ordinary color format: red, green, blue, alpha 8-bit unsigned integer components.
        RGBA8SInt,          //!< Ordinary color format: red, green, blue, alpha 8-bit signed integer components.

        RGBA16UNorm,        //!< Ordinary color format: red, green, blue, alpha 16-bit normalized unsigned interger components.
        RGBA16SNorm,        //!< Ordinary color format: red, green, blue, alpha 16-bit normalized signed interger components.
        RGBA16UInt,         //!< Ordinary color format: red, green, blue, alpha 16-bit unsigned interger components.
        RGBA16SInt,         //!< Ordinary color format: red, green, blue, alpha 16-bit signed interger components.
        RGBA16Float,        //!< Ordinary color format: red, green, blue, alpha 16-bit floating point components.

        RGBA32UInt,         //!< Ordinary color format: red, green, blue, alpha 32-bit unsigned interger components.
        RGBA32SInt,         //!< Ordinary color format: red, green, blue, alpha 32-bit signed interger components.
        RGBA32Float,        //!< Ordinary color format: red, green, blue, alpha 32-bit floating point components.

        RGBA64Float,        //!< Ordinary color format: red, green, blue, alpha 64-bit floating point components. \note Only supported with: Vulkan.

        /* --- BGRA color formats --- */
        BGRA8UNorm,         //!< Ordinary color format: blue, green, red, alpha 8-bit normalized unsigned integer components.
        BGRA8UNorm_sRGB,    //!< Ordinary color format: blue, green, red, alpha 8-bit normalized unsigned integer components in non-linear sRGB color space.
        BGRA8SNorm,         //!< Ordinary color format: blue, green, red, alpha 8-bit normalized signed integer components. \note Only supported with: Vulkan.
        BGRA8UInt,          //!< Ordinary color format: blue, green, red, alpha 8-bit unsigned integer components. \note Only supported with: Vulkan.
        BGRA8SInt,          //!< Ordinary color format: blue, green, red, alpha 8-bit signed integer components. \note Only supported with: Vulkan.

        /* --- Packed formats --- */
        RGB10A2UNorm,       //!< Packed color format: red, green, blue 10-bit and alpha 2-bit normalized unsigned integer components.
        RGB10A2UInt,        //!< Packed color format: red, green, blue 10-bit and alpha 2-bit unsigned integer components.
        RG11B10Float,       //!< Packed color format: red, green 11-bit and blue 10-bit unsigned floating point, i.e. 6-bit mantissa for red and green, 5-bit mantissa for blue, and 5-bit exponent for all components.
        RGB9E5Float,        //!< Packed color format: red, green, blue 9-bit unsigned floating-point with shared 5-bit exponent, i.e. 9-bit mantissa for each component and one 5-bit exponent for all components.

        /* --- Depth-stencil formats --- */
        D16UNorm,           //!< Depth-stencil format: depth 16-bit normalized unsigned integer component.
        D24UNormS8UInt,     //!< Depth-stencil format: depth 24-bit normalized unsigned integer component, and 8-bit unsigned integer stencil component.
        D32Float,           //!< Depth-stencil format: depth 32-bit floating point component.
        D32FloatS8X24UInt,  //!< Depth-stencil format: depth 32-bit floating point component, and 8-bit unsigned integer stencil components (where the remaining 24 bits are unused).
        //S8UInt,             //!< Stencil only format: 8-bit unsigned integer stencil component. \note Only supported with: OpenGL, Vulkan, Metal.

          /* --- Block compression (BC) formats --- */
          BC1UNorm,           //!< Compressed color format: S3TC BC1 compressed RGBA with normalized unsigned integer components in 64-bit per 4x4 block.
          BC1UNorm_sRGB,      //!< Compressed color format: S3TC BC1 compressed RGBA with normalized unsigned integer components in 64-bit per 4x4 block in non-linear sRGB color space.
          BC2UNorm,           //!< Compressed color format: S3TC BC2 compressed RGBA with normalized unsigned integer components in 128-bit per 4x4 block.
          BC2UNorm_sRGB,      //!< Compressed color format: S3TC BC2 compressed RGBA with normalized unsigned integer components in 128-bit per 4x4 block in non-linear sRGB color space.
          BC3UNorm,           //!< Compressed color format: S3TC BC3 compressed RGBA with normalized unsigned integer components in 128-bit per 4x4 block.
          BC3UNorm_sRGB,      //!< Compressed color format: S3TC BC3 compressed RGBA with normalized unsigned integer components in 128-bit per 4x4 block in non-linear sRGB color space.
          BC4UNorm,           //!< Compressed color format: S3TC BC4 compressed red channel with normalized unsigned integer component in 64-bit per 4x4 block.
          BC4SNorm,           //!< Compressed color format: S3TC BC4 compressed red channel with normalized signed integer component 64-bit per 4x4 block.
          BC5UNorm,           //!< Compressed color format: S3TC BC5 compressed red and green channels with normalized unsigned integer components in 64-bit per 4x4 block.
          BC5SNorm,           //!< Compressed color format: S3TC BC5 compressed red and green channels with normalized signed integer components in 128-bit per 4x4 block.
    };

    enum class TextureType {
        Texture1D,
        Texture2D,
        Texture3D,
        TextureCube,
        Texture1DArray,
        Texture2DArray,
        TextureCubeArray,
        Texture2DMS,
        Texture2DMSArray,
    };

    class VertexAttribute {
    public:

        VertexAttribute();
        //VertexAttribute(String^ name, Format format, unsigned int location);
        //VertexAttribute(String^ name, Format format, unsigned int location, unsigned int instanceDivisor);
        //VertexAttribute(String^ name, Format format, unsigned int location, unsigned int instanceDivisor, SystemValue systemValue);
        //VertexAttribute(String^ semanticName, unsigned int semanticIndex, Format format, unsigned int location, unsigned int instanceDivisor);
        //VertexAttribute(String^ name, Format format, unsigned int location, unsigned int offset, unsigned int stride, unsigned int slot, unsigned int instanceDivisor);
        //VertexAttribute(String^ semanticName, unsigned int semanticIndex, Format format, unsigned int location, unsigned int offset, unsigned int stride, unsigned int slot, unsigned int instanceDivisor);

        unsigned int Size;

        std::string Name;
        Format         Format;
        unsigned int   Location;
        unsigned int   SemanticIndex;
        //SystemValue    SystemValue;
        unsigned int   Slot;
        unsigned int   Offset;
        unsigned int   Stride;
        unsigned int   InstanceDivisor;

    };

    struct BufferDescriptor {
        unsigned                   size = 0;
        unsigned                   stride = 0;
        Format                          format = Format::Undefined;
        long                            bindFlags = 0;
        long                            cpuAccessFlags = 0;
        long                            miscFlags = 0;
        std::vector<VertexAttribute>    vertexAttribs;
    };

    class Buffer {
    public:
        ResourceType getResourceType() const { return ResourceType::Buffer; }
        virtual BufferDescriptor getDesc() const = 0;
    protected:
        Buffer(long bindFlags);
    };

    class TextureDescriptor {
    public:
        TextureDescriptor();
        TextureType    Type;
        //BindFlags      BindFlags;
        //MiscFlags      MiscFlags;
        Format         Format;
        //Extent3D^ Extent;
        unsigned int   ArrayLayers;
        unsigned int   MipLevels;
        unsigned int   Samples;

    };

    struct BindingDescriptor {
        BindingDescriptor() = default;
        BindingDescriptor(const BindingDescriptor&) = default;
        
        inline BindingDescriptor(
            ResourceType    type,
            long            bindFlags,
            long            stageFlags,
            std::uint32_t   slot,
            std::uint32_t   arraySize = 1)
            :
            type{ type },
            bindFlags{ bindFlags },
            stageFlags{ stageFlags },
            slot{ slot },
            arraySize{ arraySize }
        {
        }

        //! Constructors with all attributes.
        inline BindingDescriptor(
            const std::string& name,
            ResourceType        type,
            long                bindFlags,
            long                stageFlags,
            std::uint32_t       slot,
            std::uint32_t       arraySize = 1)
            :
            name{ name },
            type{ type },
            bindFlags{ bindFlags },
            stageFlags{ stageFlags },
            slot{ slot },
            arraySize{ arraySize }
        {
        }
        
        std::string     name;
        
        ResourceType    type = ResourceType::Undefined;
        long            bindFlags = 0;
        long            stageFlags = 0;
        std::uint32_t   slot = 0;
        std::uint32_t   arraySize = 1;
    };
    
    struct PipelineLayoutDescriptor {
        std::vector<BindingDescriptor> bindings;
    };

    class PipelineLayout {
    public:
        virtual std::uint32_t GetNumBindings() const = 0;

    };

    struct CommandBufferFlags {
        enum {
            Secondary = (1 << 0),
            MultiSubmit = (1 << 1),
            ImmediateSubmit = (1 << 2),
        };
    };

    class CommandQueue {
    public:
        
        virtual void Submit(CommandBuffer& commandBuffer) = 0;
        virtual void Submit(std::uint32_t numCommandBuffers, CommandBuffer* const* commandBuffers);
        //virtual bool QueryResult(
        //    QueryHeap& queryHeap,
        //    std::uint32_t   firstQuery,
        //    std::uint32_t   numQueries,
        //    void* data,
        //    std::size_t     dataSize
        //) = 0;
        
        //virtual void Submit(Fence& fence) = 0;
        //virtual bool WaitFence(Fence& fence, std::uint64_t timeout) = 0;
        //virtual void WaitIdle() = 0; GLCommandQueue

    protected:

        CommandQueue() = default;

    };


    struct CommandBufferDescriptor
    {
        CommandBufferDescriptor() = default;
        CommandBufferDescriptor(const CommandBufferDescriptor&) = default;
        CommandBufferDescriptor& operator = (const CommandBufferDescriptor&) = default;

        //! Constructs the command buffer descriptor with the specified flags.
        inline CommandBufferDescriptor(long flags) :
            flags{ flags }
        {
        }

        //! Constructs the command buffer descriptor with the specified flags and number of native buffers.
        inline CommandBufferDescriptor(long flags, std::uint32_t numNativeBuffers) :
            flags{ flags },
            numNativeBuffers{ numNativeBuffers }
        {
        }
        long            flags = 0;
        std::uint32_t   numNativeBuffers = 2;
    };

    struct GraphicsPipelineDescriptor {
        const PipelineLayout* pipelineLayout = nullptr;
        //const ShaderProgram* shaderProgram = nullptr;
        const RenderPass* renderPass = nullptr;
        //PrimitiveTopology       primitiveTopology = PrimitiveTopology::TriangleList;
        std::vector<Viewport>   viewports;
        std::vector<Scissor>    scissors;
        
        //DepthDescriptor         depth;
        //StencilDescriptor       stencil;
        //RasterizerDescriptor    rasterizer;
        //BlendDescriptor         blend;
        //TessellationDescriptor  tessellation;
    };

    /*struct TextureRegion
    {
        TextureRegion() = default;
        TextureRegion(const TextureRegion&) = default;
        
        inline TextureRegion(const Offset3D& offset, const Extent3D& extent) :
            offset{ offset },
            extent{ extent }
        {
        }
        
        inline TextureRegion(const TextureSubresource& subresource, const Offset3D& offset, const Extent3D& extent) :
            subresource{ subresource },
            offset{ offset },
            extent{ extent }
        {
        }
        
        TextureSubresource  subresource;
        
        Offset3D            offset;
        Extent3D            extent;
    }*/;

    class Texture {
    public:

        ResourceType GetResourceType() const;

        //std::uint32_t GetMemoryFootprint() const;

        //std::uint32_t GetMemoryFootprint(const Extent3D& extent, const TextureSubresource& subresource) const;

        inline TextureType GetType() const
        {
            return type_;
        }

        inline long GetBindFlags() const
        {
            return bindFlags_;
        }
        virtual TextureDescriptor GetDesc() const = 0;
        //virtual Extent3D GetMipExtent(std::uint32_t mipLevel) const = 0;
        //virtual Format GetFormat() const = 0;

    protected:
        Texture(const TextureType type, long bindFlags);

    private:

        TextureType type_;
        long        bindFlags_ = 0;

    };

    class BufferArray {
    public:
        BufferArray(BufferArray* native);
    private:

        BufferArray* native_ = nullptr;
        
    };

    struct Viewport {
        Viewport() = default;
        float x = 0.0f;
        float y = 0.0f;
        float width = 0.0f;
        float height = 0.0f;
        float minDepth = 0.0f;
        float maxDepth = 1.0f;
    };

    struct Scissor {
        Scissor() = default;
        std::int32_t x = 0; //!< Left-top X coordinate.
        std::int32_t y = 0; //!< Left-top Y coordinate.
        std::int32_t width = 0; //!< Right-bottom width.
        std::int32_t height = 0; //!< Right-bottom height.
    };

    class ResourceHeap {
    public:
        virtual std::uint32_t GetNumDescriptorSets() const = 0;
    };

    struct TextureLocation
    {
        TextureLocation() = default;
        TextureLocation(const TextureLocation&) = default;
        
        //inline TextureLocation(const Offset3D& offset, std::uint32_t arrayLayer = 0, std::uint32_t mipLevel = 0) :
        //    offset{ offset },
        //    arrayLayer{ arrayLayer },
        //    mipLevel{ mipLevel }
        //{
        //}
        
        //Offset3D        offset;
        
        std::uint32_t   arrayLayer = 0;
        
        std::uint32_t   mipLevel = 0;
    };
    using UniformLocation = std::int32_t;

    class PipelineState {
    public:
        //PipelineState(LLGL::PipelineState* native);
    private:

        //LLGL::PipelineState* native_ = nullptr;
    };

    class RenderPass {
    public:

        //RenderPass(LLGL::RenderPass* native);

    private:
        //LLGL::RenderPass* native_ = nullptr;
    };

    class ClearValue {
    public:

        ClearValue();

        //MATHGL::Vector4 Color;
        float              Depth;
        unsigned int       Stencil;

    };

    class RenderTarget {
    public:
        //RenderTarget(LLGL::RenderTarget* native);

        //Extent2D^ Resolution
        //{
        //    Extent2D^ get();
        //}

        unsigned int NumColorAttachments;

        bool HasDepthAttachment;

        bool HasStencilAttachment;

        RenderPass RenderPass;

    private:

        //LLGL::RenderTarget* native_ = nullptr;
        //SharpLLGL::RenderPass^ renderPass_ = nullptr;
    };

    class CommandBuffer {
    public:
        virtual void begin() = 0;
        virtual void end() = 0;

        virtual void execute(CommandBuffer& deferredCommandBuffer) = 0;
        virtual void updateBuffer(
            Buffer& dstBuffer,
            unsigned dstOffset,
            const void* data,
            unsigned dataSize
        ) = 0;

        virtual void CopyBuffer(
            Buffer& dstBuffer,
            std::uint64_t   dstOffset,
            Buffer& srcBuffer,
            std::uint64_t   srcOffset,
            std::uint64_t   size
        ) = 0;


        virtual void CopyBufferFromTexture(
            Buffer& dstBuffer,
            std::uint64_t           dstOffset,
            Texture& srcTexture,
            //const TextureRegion& srcRegion,
            std::uint32_t           rowStride = 0,
            std::uint32_t           layerStride = 0
        ) = 0;

        virtual void FillBuffer(
            Buffer& dstBuffer,
            std::uint64_t   dstOffset,
            std::uint32_t   value,
            std::uint64_t   fillSize = 0
        ) = 0;

        //virtual void CopyTexture(
        //    Texture& dstTexture,
        //    const TextureLocation& dstLocation,
        //    Texture& srcTexture,
        //    const TextureLocation& srcLocation,
        //    const Extent3D& extent
        //) = 0;
        //
        //virtual void CopyTextureFromBuffer(
        //    Texture& dstTexture,
        //    const TextureRegion& dstRegion,
        //    Buffer& srcBuffer,
        //    std::uint64_t           srcOffset,
        //    std::uint32_t           rowStride = 0,
        //    std::uint32_t           layerStride = 0
        //) = 0;

        //virtual void GenerateMips(Texture& texture) = 0;

        //virtual void GenerateMips(Texture& texture, const TextureSubresource& subresource) = 0;

        virtual void SetViewport(const Viewport& viewport) = 0;

        //virtual void SetViewports(std::uint32_t numViewports, const Viewport* viewports) = 0;

        virtual void SetScissor(const Scissor& scissor) = 0;

        //virtual void SetScissors(std::uint32_t numScissors, const Scissor* scissors) = 0;

        virtual void SetVertexBuffer(Buffer& buffer) = 0;

        virtual void SetVertexBufferArray(BufferArray& bufferArray) = 0;

        virtual void SetIndexBuffer(Buffer& buffer) = 0;

        virtual void SetIndexBuffer(Buffer& buffer, const Format format, std::uint64_t offset = 0) = 0;

        //virtual void SetResourceHeap(
        //    ResourceHeap& resourceHeap,
        //    std::uint32_t           firstSet = 0,
        //    const PipelineBindPoint bindPoint = PipelineBindPoint::Undefined
        //) = 0;
        //
        //virtual void SetResource(
        //    Resource& resource,
        //    std::uint32_t   slot,
        //    long            bindFlags,
        //    long            stageFlags = StageFlags::AllStages
        //) = 0;

        //virtual void ResetResourceSlots(
        //    const ResourceType  resourceType,
        //    std::uint32_t       firstSlot,
        //    std::uint32_t       numSlots,
        //    long                bindFlags,
        //    long                stageFlags = StageFlags::AllStages
        //) = 0;

        virtual void BeginRenderPass(
            RenderTarget& renderTarget,
            const RenderPass* renderPass = nullptr,
            std::uint32_t       numClearValues = 0,
            const ClearValue* clearValues = nullptr
        ) = 0;

        virtual void Clear(long flags, const ClearValue& clearValue = {}) = 0;

        //virtual void ClearAttachments(std::uint32_t numAttachments, const AttachmentClear* attachments) = 0;

        virtual void SetPipelineState(PipelineState& pipelineState) = 0;

        //virtual void SetBlendFactor(const ColorRGBAf& color) = 0;

        //virtual void SetStencilReference(std::uint32_t reference, const StencilFace stencilFace = StencilFace::FrontAndBack) = 0;

        virtual void SetUniform(
            UniformLocation location,
            const void* data,
            std::uint32_t   dataSize
        ) = 0;

        virtual void SetUniforms(
            UniformLocation location,
            std::uint32_t   count,
            const void* data,
            std::uint32_t   dataSize
        ) = 0;

        //virtual void BeginQuery(QueryHeap& queryHeap, std::uint32_t query = 0) = 0;
        //virtual void EndQuery(QueryHeap& queryHeap, std::uint32_t query = 0) = 0;

        //virtual void BeginRenderCondition(QueryHeap& queryHeap, std::uint32_t query = 0, const RenderConditionMode mode = RenderConditionMode::Wait) = 0;

        virtual void EndRenderCondition() = 0;

        virtual void BeginStreamOutput(std::uint32_t numBuffers, Buffer* const* buffers) = 0;

        virtual void EndStreamOutput() = 0;

        virtual void Draw(std::uint32_t numVertices, std::uint32_t firstVertex) = 0;

        virtual void DrawIndexed(std::uint32_t numIndices, std::uint32_t firstIndex) = 0;

        virtual void DrawIndexed(std::uint32_t numIndices, std::uint32_t firstIndex, std::int32_t vertexOffset) = 0;

        virtual void DrawInstanced(std::uint32_t numVertices, std::uint32_t firstVertex, std::uint32_t numInstances) = 0;

        virtual void DrawInstanced(std::uint32_t numVertices, std::uint32_t firstVertex, std::uint32_t numInstances, std::uint32_t firstInstance) = 0;

        virtual void DrawIndexedInstanced(std::uint32_t numIndices, std::uint32_t numInstances, std::uint32_t firstIndex) = 0;

        virtual void DrawIndexedInstanced(std::uint32_t numIndices, std::uint32_t numInstances, std::uint32_t firstIndex, std::int32_t vertexOffset) = 0;

        virtual void DrawIndexedInstanced(std::uint32_t numIndices, std::uint32_t numInstances, std::uint32_t firstIndex, std::int32_t vertexOffset, std::uint32_t firstInstance) = 0;

        virtual void DrawIndirect(Buffer& buffer, std::uint64_t offset) = 0;

        virtual void DrawIndirect(Buffer& buffer, std::uint64_t offset, std::uint32_t numCommands, std::uint32_t stride) = 0;

        virtual void DrawIndexedIndirect(Buffer& buffer, std::uint64_t offset) = 0;

        virtual void DrawIndexedIndirect(Buffer& buffer, std::uint64_t offset, std::uint32_t numCommands, std::uint32_t stride) = 0;

        virtual void Dispatch(std::uint32_t numWorkGroupsX, std::uint32_t numWorkGroupsY, std::uint32_t numWorkGroupsZ) = 0;

        virtual void DispatchIndirect(Buffer& buffer, std::uint64_t offset) = 0;

        virtual void PushDebugGroup(const char* name) = 0;
        virtual void PopDebugGroup() = 0;
        virtual void SetGraphicsAPIDependentState(const void* stateDesc, std::size_t stateDescSize) = 0;

    protected:

        CommandBuffer() = default;

    };
    

    class RenderSystem {
    public:
        static std::unique_ptr<RenderSystem> Load();

        //virtual SwapChain* createSwapChain(const SwapChainDescriptor& descriptor) = 0;
        //virtual void destroySwapChain(SwapChain& swapChain) = 0;
        //
        //virtual CommandQueue* getCommandQueue() = 0;
        //
        //virtual CommandBuffer* createCommandBuffer(const CommandBufferDescriptor& desc = {}) = 0;
        //virtual void destroyCommandBuffer(CommandBuffer& commandBuffer) = 0;
        //
        //virtual Buffer* createBuffer(const BufferDescriptor& desc, const void* initialData = nullptr) = 0;
        //
        //virtual BufferArray* CreateBufferArray(std::uint32_t numBuffers, Buffer* const* bufferArray) = 0;
        //virtual void Release(Buffer& buffer) = 0;
        //
        //virtual void Release(BufferArray& bufferArray) = 0;
        //virtual void WriteBuffer(Buffer& dstBuffer, std::uint64_t dstOffset, const void* data, std::uint64_t dataSize) = 0;
        //
        //virtual Sampler* CreateSampler(const SamplerDescriptor& desc) = 0;
        //virtual void Release(Sampler& sampler) = 0;
        //
        //virtual PipelineState* CreatePipelineState(const ComputePipelineDescriptor& desc, std::unique_ptr<Blob>* serializedCache = nullptr) = 0;
        //
        //virtual void Release(PipelineState& pipelineState) = 0;
    };

    
}
