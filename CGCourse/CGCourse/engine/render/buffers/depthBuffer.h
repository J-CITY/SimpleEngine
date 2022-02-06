#pragma once
#include <array>

#include "../../resourceManager/resource/texture.h"

namespace KUMA {
	namespace RENDER {

        enum class Attachment {
            COLOR_ATTACHMENT0,
            COLOR_ATTACHMENT1,
            COLOR_ATTACHMENT2,
            COLOR_ATTACHMENT3,
            COLOR_ATTACHMENT4,
            COLOR_ATTACHMENT5,
            COLOR_ATTACHMENT6,
            COLOR_ATTACHMENT9,
            COLOR_ATTACHMENT10,
            COLOR_ATTACHMENT11,
            COLOR_ATTACHMENT12,
            COLOR_ATTACHMENT13,
            COLOR_ATTACHMENT14,
            COLOR_ATTACHMENT15,
            DEPTH_ATTACHMENT,
            STENCIL_ATTACHMENT,
            DEPTH_STENCIL_ATTACHMENT,
        };

        class FrameBuffer {
            enum class AttachmentType : uint8_t {
                NONE,
                TEXTURE,
                CUBEMAP,
            };

            using BindableId = unsigned int;

            
            AttachmentType currentAttachment = AttachmentType::NONE;

            //std::aligned_storage_t<24> attachmentStorage;
            void OnTextureAttach(const RESOURCES::Texture& texture, Attachment attachment);
            void OnCubeMapAttach(const RESOURCES::CubeMap& cubemap, Attachment attachment);
            void FreeFrameBuffer();
        public:
            BindableId id = 0;
            FrameBuffer();
            ~FrameBuffer();
            FrameBuffer(const FrameBuffer&) = delete;
            FrameBuffer(FrameBuffer&&) noexcept;
            FrameBuffer& operator=(const FrameBuffer&) = delete;
            FrameBuffer& operator=(FrameBuffer&&) noexcept;
            void CopyFrameBufferContents(const FrameBuffer& framebuffer) const;
            void CopyFrameBufferContents(int screenWidth, int screenHeight) const;
            void Validate() const;
            void DetachRenderTarget();
            void DetachExtraTarget(Attachment attachment);
            bool HasTextureAttached() const;
            bool HasCubeMapAttached() const;
            //void UseDrawBuffers(ArrayView<Attachment> attachments) const;
            void UseOnlyDepth() const;
            size_t GetWidth() const;
            size_t GetHeight() const;
            void bind() const;
            void unbind() const;
            BindableId GetNativeHandle() const;
            RESOURCES::Texture* attachedTexture = nullptr;
            RESOURCES::CubeMap* attachedCubeMap = nullptr;
            void AttachTexture(RESOURCES::Texture& texture, Attachment attachment = Attachment::COLOR_ATTACHMENT0) {
                DetachRenderTarget();
                attachedTexture = new RESOURCES::Texture(texture);
                this->currentAttachment = AttachmentType::TEXTURE;
                this->OnTextureAttach(*attachedTexture, attachment);

            }

            void AttachTextureExtra(RESOURCES::Texture& texture, Attachment attachment) {
                this->OnTextureAttach(texture, attachment);
            }

            void AttachCubeMapExtra(RESOURCES::CubeMap& cubemap, Attachment attachment) {
                this->OnCubeMapAttach(cubemap, attachment);
            }

            void AttachCubeMap(RESOURCES::CubeMap& cubemap, Attachment attachment = Attachment::COLOR_ATTACHMENT0) {
                
                DetachRenderTarget();
                attachedCubeMap = new RESOURCES::CubeMap(cubemap);
                this->currentAttachment = AttachmentType::CUBEMAP;
                this->OnCubeMapAttach(*attachedCubeMap, attachment);

            }

            std::vector<GLenum> AttachmentTable
            {
                GL_COLOR_ATTACHMENT0,
                GL_COLOR_ATTACHMENT1,
                GL_COLOR_ATTACHMENT2,
                GL_COLOR_ATTACHMENT3,
                GL_COLOR_ATTACHMENT4,
                GL_COLOR_ATTACHMENT5,
                GL_COLOR_ATTACHMENT6,
                GL_COLOR_ATTACHMENT9,
                GL_COLOR_ATTACHMENT10,
                GL_COLOR_ATTACHMENT11,
                GL_COLOR_ATTACHMENT12,
                GL_COLOR_ATTACHMENT13,
                GL_COLOR_ATTACHMENT14,
                GL_COLOR_ATTACHMENT15,
                GL_DEPTH_ATTACHMENT,
                GL_STENCIL_ATTACHMENT,
                GL_DEPTH_STENCIL_ATTACHMENT,
            };
            void UseDrawBuffers(std::vector<Attachment> attachments) const {
                std::array<GLenum, 20> attachmentTypes;
                
                bind();
                for (size_t i = 0; i < attachments.size(); i++) {
                    attachmentTypes[i] = AttachmentTable[(int)attachments[i]];
                }
                //glDrawBuffers((GLsizei)attachments.size(), attachmentTypes.data());
            }

            //template<template<typename, typename> typename Resource, typename Factory>
            //RESOURCES::Texture* GetAttachedTexture() const {
            //    if (!this->HasTextureAttached())
            //        return Resource<Texture, Factory>{ };
            //
            //    const auto& texture = *std::launder(reinterpret_cast<const Resource<Texture, Factory>*>(&this->attachmentStorage));
            //
            //    return texture;
            //}
            //
            //template<template<typename, typename> typename Resource, typename Factory>
            //RESOURCES::CubeMap* GetAttachedCubeMap() const {
            //    if (!this->HasCubeMapAttached())
            //        return Resource<CubeMap, Factory>{ };
            //
            //    const auto& cubemap = *std::launder(reinterpret_cast<const Resource<CubeMap, Factory>*>(&this->attachmentStorage));
            //
            //    return cubemap;
            //}
        };

	}
}
