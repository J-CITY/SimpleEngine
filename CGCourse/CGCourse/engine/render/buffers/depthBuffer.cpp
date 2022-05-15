#include "depthBuffer.h"


#include <cassert>
#include <GL/glew.h>


#include "../../resourceManager/ServiceManager.h"
#include "../../utils/debug/logger.h"
#include "../../window/window.h"

using namespace KUMA;
using namespace KUMA::RENDER;

FrameBuffer::FrameBuffer() {
    glGenFramebuffers(1, &id);
}

FrameBuffer::~FrameBuffer() {
    if (id != 0) {
        glDeleteFramebuffers(1, &id);
    }
    id = 0;
}

FrameBuffer::FrameBuffer(FrameBuffer&& framebuffer) noexcept {
    this->id = framebuffer.id;
    this->currentAttachment = framebuffer.currentAttachment;
    framebuffer.currentAttachment = AttachmentType::NONE;
    framebuffer.id = 0;
}

FrameBuffer& FrameBuffer::operator=(FrameBuffer&& framebuffer) noexcept {
    if (id != 0) {
        glDeleteFramebuffers(1, &id);
    }
    id = 0;

    this->id = framebuffer.id;
    this->currentAttachment = framebuffer.currentAttachment;
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
