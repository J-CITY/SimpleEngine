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

        enum class FrameBufferStatus
        {
            FRAMEBUFFER_COMPLETE = 0x8CD5,
            FRAMEBUFFER_UNDEFINED = 0x8219,
            FRAMEBUFFER_INCOMPLETE_ATTACHMENT = 0x8CD6,
            FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT = 0x8CD7,
            FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER = 0x8CDB,
            FRAMEBUFFER_INCOMPLETE_READ_BUFFER = 0x8CDC,
            FRAMEBUFFER_UNSUPPORTED = 0x8CDD,
            FRAMEBUFFER_INCOMPLETE_MULTISAMPLE = 0x8D56,
            FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS = 0x8DA8
        };

        class DepthBuffer
        {
            using BindableId = unsigned int;
            BindableId id;

        public:

            enum class Format {
                DEPTH_COMPONENT = 0x1902,
                DEPTH_COMPONENT16 = 0x81A5,
                DEPTH_COMPONENT24 = 0x81A6,
                DEPTH_COMPONENT32 = 0x81A7
            };
            DepthBuffer(unsigned w, unsigned h, Format format= Format::DEPTH_COMPONENT) {
                glGenRenderbuffers(1, &id);
                bind();
                glRenderbufferStorage(GL_RENDERBUFFER, static_cast<GLenum>(format), w, h);
            }

            void init(unsigned w, unsigned h, Format format = Format::DEPTH_COMPONENT) {
                bind();
                glRenderbufferStorage(GL_RENDERBUFFER, static_cast<GLenum>(format), w, h);
            }

            void bind() {
                glBindRenderbuffer(GL_RENDERBUFFER, id);
            }
            void unbind() {
                glBindRenderbuffer(GL_RENDERBUFFER, 0);
            }
            BindableId getId() {
                return id;
            }

            void attachCubeMapSide(RESOURCES::CubeMap& cubemap, unsigned sideId, unsigned mip) {
                glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + sideId, cubemap.id, mip);
            }
        };

        class FrameBuffer {
            enum class AttachmentType : uint8_t {
                NONE,
                TEXTURE,
                CUBEMAP,
            };

            using BindableId = unsigned int;

            AttachmentType currentAttachment = AttachmentType::NONE;
            
        public:
            BindableId id = 0;
            FrameBuffer();
            ~FrameBuffer();
            FrameBuffer(const FrameBuffer&) = delete;
            FrameBuffer(FrameBuffer&&) noexcept;
            FrameBuffer& operator=(const FrameBuffer&) = delete;
            FrameBuffer& operator=(FrameBuffer&&) noexcept;



            size_t getWidth() const;
            size_t getHeight() const;
            void bind() const;
            void unbind() const;

            BindableId GetNativeHandle() const;

            FrameBufferStatus getStatus() {
                return static_cast<FrameBufferStatus>(glCheckNamedFramebufferStatus(id, GL_FRAMEBUFFER));
            }

            void attachTexture(RESOURCES::Texture& texture, Attachment attachment = Attachment::COLOR_ATTACHMENT0) {
                currentAttachment = AttachmentType::TEXTURE;
                GLenum mode = attachmentTable[int(attachment)];
                GLint textureId = texture.getId();
                bind();
                glFramebufferTexture2D(GL_FRAMEBUFFER, mode, GL_TEXTURE_2D, textureId, 0);
            }

            void attachCubeMapSide(RESOURCES::CubeMap& cubemap, unsigned sideId) {
                glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + sideId, cubemap.id, 0);
            }

            void attachCubeMap(RESOURCES::CubeMap& cubemap, Attachment attachment = Attachment::COLOR_ATTACHMENT0) {
                GLenum mode = attachmentTable[int(attachment)];
                GLint cubemapId = cubemap.id;//cubemap.getNativeHandle();
                bind();
                glFramebufferTexture(GL_FRAMEBUFFER, mode, cubemapId, 0);
            }

            void attachDepth(DepthBuffer& rboDepth) {
                bind();
                glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboDepth.getId());
            }

            std::vector<GLenum> attachmentTable {
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

            void setOupbutBuffers(const std::vector<Attachment>& outBuffers) {
                std::vector<GLenum> buffers;
                for (auto e : outBuffers) {
                    buffers.push_back(attachmentTable[int(e)]);
                }
                glDrawBuffers(3, buffers.data());
            };

            static void Unbind() {
                glBindFramebuffer(GL_FRAMEBUFFER, 0);
            }

            static void CopyDepth(const FrameBuffer& from, const FrameBuffer& to, unsigned w, unsigned h) {
                glBindFramebuffer(GL_READ_FRAMEBUFFER, from.id);
                glBindFramebuffer(GL_DRAW_FRAMEBUFFER, to.id); // пишем в заданный по умолчанию фреймбуфер
                glBlitFramebuffer(0, 0, w, h, 0, 0, w, h, GL_DEPTH_BUFFER_BIT, GL_NEAREST);
            }
        };
	}
}
