#include "depthBuffer.h"
#include <GL/glew.h>

using namespace KUMA;
using namespace KUMA::RENDER;

DepthBuffer::DepthBuffer(unsigned w, unsigned h, Format format) {
	glGenRenderbuffers(1, &id);
	bind();
	glRenderbufferStorage(GL_RENDERBUFFER, static_cast<GLenum>(format), w, h);
}

void DepthBuffer::init(unsigned w, unsigned h, Format format) {
	bind();
	glRenderbufferStorage(GL_RENDERBUFFER, static_cast<GLenum>(format), w, h);
}

void DepthBuffer::bind() const {
	glBindRenderbuffer(GL_RENDERBUFFER, id);
}

void DepthBuffer::unbind() const {
	glBindRenderbuffer(GL_RENDERBUFFER, 0);
}

DepthBuffer::BindableId DepthBuffer::getId() const {
	return id;
}

void DepthBuffer::attachCubeMapSide(RESOURCES::CubeMap& cubemap, unsigned sideId, unsigned mip) {
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + sideId, cubemap.getId(), mip);
}

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

FrameBuffer::BindableId FrameBuffer::getId() const {
	return id;
}

FrameBufferStatus FrameBuffer::getStatus() const {
	return static_cast<FrameBufferStatus>(glCheckNamedFramebufferStatus(id, GL_FRAMEBUFFER));
}

void FrameBuffer::attachTexture(RESOURCES::Texture& texture, Attachment attachment) {
	currentAttachment = AttachmentType::TEXTURE;
	GLenum mode = attachmentTable[int(attachment)];
	GLint textureId = texture.getId();
	bind();
	glFramebufferTexture2D(GL_FRAMEBUFFER, mode, GL_TEXTURE_2D, textureId, 0);
}

void FrameBuffer::attachCubeMapSide(RESOURCES::CubeMap& cubemap, unsigned sideId) {
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + sideId, cubemap.getId(), 0);
}

void FrameBuffer::attachCubeMap(RESOURCES::CubeMap& cubemap, Attachment attachment) {
	GLenum mode = attachmentTable[int(attachment)];
	GLint cubemapId = cubemap.getId();//cubemap.getNativeHandle();
	bind();
	glFramebufferTexture(GL_FRAMEBUFFER, mode, cubemapId, 0);
}

void FrameBuffer::attachDepth(DepthBuffer& rboDepth) {
	bind();
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboDepth.getId());
}

void FrameBuffer::setOupbutBuffers(const std::vector<Attachment>& outBuffers) const {
	std::vector<GLenum> buffers;
	for (auto e : outBuffers) {
		buffers.push_back(attachmentTable[int(e)]);
	}
	glDrawBuffers(buffers.size(), buffers.data());
}

void FrameBuffer::Unbind() {
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void FrameBuffer::CopyDepth(const FrameBuffer& from, const FrameBuffer& to, unsigned w, unsigned h) {
	glBindFramebuffer(GL_READ_FRAMEBUFFER, from.id);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, to.id); // пишем в заданный по умолчанию фреймбуфер
	glBlitFramebuffer(0, 0, w, h, 0, 0, w, h, GL_DEPTH_BUFFER_BIT, GL_NEAREST);
}
