#include "depthBuffer.h"


#include <cassert>
#include <GL/glew.h>


#include "../../resourceManager/ServiceManager.h"
#include "../../utils/debug/logger.h"
#include "../../window/window.h"

using namespace KUMA;
using namespace KUMA::RENDER;

GLenum AttachmentTable []
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

FrameBuffer::FrameBuffer() {
    glGenFramebuffers(1, &id);
}

void FrameBuffer::OnTextureAttach(const RESOURCES::Texture& texture, Attachment attachment) {
    GLenum mode = AttachmentTable[int(attachment)];
    GLint textureId = texture.id;

    bind();
    glFramebufferTexture2D(GL_FRAMEBUFFER, mode, texture.textureType, textureId, 0);
}

void FrameBuffer::OnCubeMapAttach(const RESOURCES::CubeMap& cubemap, Attachment attachment) {
    GLenum mode = AttachmentTable[int(attachment)];
    GLint cubemapId = cubemap.id;//cubemap.getNativeHandle();

    bind();
    glFramebufferTexture(GL_FRAMEBUFFER, mode, cubemapId, 0);
}

void FrameBuffer::FreeFrameBuffer() {
    this->DetachRenderTarget();
    if (this->id != 0) {
        glDeleteFramebuffers(1, &id);
    }
    this->id = 0;
}

void FrameBuffer::CopyFrameBufferContents(int screenWidth, int screenHeight) const {
    glBindFramebuffer(GL_READ_FRAMEBUFFER, this->id);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    glBlitFramebuffer(0, 0, (GLint)this->GetWidth(), (GLint)this->GetHeight(), 0, 0, screenWidth, screenHeight, GL_COLOR_BUFFER_BIT, GL_NEAREST);
}

void FrameBuffer::Validate() const {
    bind();
    auto status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE)
        LOG_ERROR("FrameBuffer framebuffer validation failed: incomplete");
}

void FrameBuffer::DetachRenderTarget() {
    if (this->currentAttachment == AttachmentType::TEXTURE)
        delete attachedTexture;
    else if (this->currentAttachment == AttachmentType::CUBEMAP)
        delete attachedCubeMap;

    this->currentAttachment = AttachmentType::NONE;

}

void FrameBuffer::DetachExtraTarget(Attachment attachment) {
    glFramebufferTexture(GL_FRAMEBUFFER, AttachmentTable[(int)attachment], 0, 0);
}

bool FrameBuffer::HasTextureAttached() const {
    return this->currentAttachment == AttachmentType::TEXTURE;
}

bool FrameBuffer::HasCubeMapAttached() const {
    return this->currentAttachment == AttachmentType::CUBEMAP;
}

//void FrameBuffer::UseDrawBuffers(std::vector<Attachment>& attachments) const {
//    std::array<GLenum, 20> attachmentTypes{ };
//    assert(attachments.size() <= attachmentTypes.size());
//
//    this->Bind();
//    for (size_t i = 0; i < attachments.size(); i++) {
//        attachmentTypes[i] = AttachmentTable[(int)attachments[i]];
//    }
//    glDrawBuffers((GLsizei)attachments.size(), attachmentTypes.data());
//}

void FrameBuffer::UseOnlyDepth() const {
    bind();
    //glDrawBuffer(GL_NONE);
}

size_t FrameBuffer::GetWidth() const {
    
    if (this->currentAttachment == AttachmentType::TEXTURE && attachedTexture)
        return attachedTexture->width;
    if (this->currentAttachment == AttachmentType::CUBEMAP && attachedCubeMap)
        return attachedCubeMap->width;

    return 0;
}

size_t FrameBuffer::GetHeight() const {
    if (this->currentAttachment == AttachmentType::TEXTURE && attachedTexture)
        return attachedTexture->height;
    if (this->currentAttachment == AttachmentType::CUBEMAP && attachedCubeMap)
        return attachedCubeMap->height;
    return 0;
}

FrameBuffer::~FrameBuffer() {
    this->FreeFrameBuffer();
}

FrameBuffer::FrameBuffer(FrameBuffer&& framebuffer) noexcept {
    this->id = framebuffer.id;
    this->currentAttachment = framebuffer.currentAttachment;
    this->attachedTexture = framebuffer.attachedTexture;
    this->attachedCubeMap = framebuffer.attachedCubeMap;
    framebuffer.currentAttachment = AttachmentType::NONE;
    framebuffer.id = 0;
}

FrameBuffer& FrameBuffer::operator=(FrameBuffer&& framebuffer) noexcept {
    this->FreeFrameBuffer();

    this->id = framebuffer.id;
    this->currentAttachment = framebuffer.currentAttachment;
    this->attachedTexture = framebuffer.attachedTexture;
    this->attachedCubeMap = framebuffer.attachedCubeMap;
    framebuffer.currentAttachment = AttachmentType::NONE;
    framebuffer.id = 0;
    return *this;
}

void FrameBuffer::bind() const {
    glBindFramebuffer(GL_FRAMEBUFFER, id);
}

void FrameBuffer::unbind() const {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

FrameBuffer::BindableId FrameBuffer::GetNativeHandle() const {
    return id;
}

void FrameBuffer::CopyFrameBufferContents(const FrameBuffer& framebuffer) const {
    glBindFramebuffer(GL_READ_FRAMEBUFFER, this->id);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, framebuffer.GetNativeHandle());
    glBlitFramebuffer(0, 0, (GLint)this->GetWidth(), (GLint)this->GetHeight(), 0, 0, (GLint)framebuffer.GetWidth(), (GLint)framebuffer.GetHeight(), GL_COLOR_BUFFER_BIT, GL_NEAREST);
}