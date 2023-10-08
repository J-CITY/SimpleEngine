#include "frameBufferGl.h"
#ifdef OPENGL_BACKEND
#include "textureGl.h"

#include <gl/glew.h>
#include <iostream>

using namespace IKIGAI;
using namespace IKIGAI::RENDER;

FrameBufferGl::FrameBufferGl() = default;

FrameBufferGl::~FrameBufferGl() {
	glDeleteFramebuffers(1, &id);
}

void FrameBufferGl::create() {
	glGenFramebuffers(1, &id);
}

void FrameBufferGl::create(std::vector<std::shared_ptr<TextureGl>> textures, std::shared_ptr<TextureGl> depthTexture) {
	glGenFramebuffers(1, &id);
	bind();

	int i = 0;
	std::vector<unsigned> attachments;
	attachments.resize(textures.size());
	for (auto t : textures) {
		attachments[i] = GL_COLOR_ATTACHMENT0 + i;
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, t->id, 0);
		i++;
	}
	if (attachments.size()) {
		glDrawBuffers(attachments.size(), attachments.data());
	}
	if (!depthTexture) {
		const unsigned int SCR_WIDTH = textures[0]->width;
		const unsigned int SCR_HEIGHT = textures[0]->height;
		unsigned int rboDepth;
		glGenRenderbuffers(1, &rboDepth);
		glBindRenderbuffer(GL_RENDERBUFFER, rboDepth);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, SCR_WIDTH, SCR_HEIGHT);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboDepth);
		// finally check if framebuffer is complete
		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
			std::cout << "Framebuffer not complete!" << std::endl;
	}
	else {
		if (depthTexture->type == TextureType::TEXTURE_2D) {
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthTexture->id, 0);
		}
		else
		{
			glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depthTexture->id, 0);
		}
		glDrawBuffer(GL_NONE);
		glReadBuffer(GL_NONE);
		int status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
		if (status != GL_FRAMEBUFFER_COMPLETE) {
			std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!";
			throw 0;
		}
	}
	unbind();
}

void FrameBufferGl::bind()
{
	glBindFramebuffer(GL_FRAMEBUFFER, id);
}

void FrameBufferGl::unbind()
{
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void FrameBufferGl::CopyDepth(const FrameBufferGl& from, const FrameBufferGl& to, unsigned w, unsigned h) {
	glBindFramebuffer(GL_READ_FRAMEBUFFER, from.id);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, to.id);
	glBlitFramebuffer(0, 0, w, h, 0, 0, w, h, GL_DEPTH_BUFFER_BIT, GL_NEAREST);
}
#endif
