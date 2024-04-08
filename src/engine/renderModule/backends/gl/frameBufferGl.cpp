#include "frameBufferGl.h"
#ifdef OPENGL_BACKEND
#include "textureGl.h"

#include <coreModule/graphicsWrapper.hpp>
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
void checkGlError(const char* op, ...) {
	//if (IS_DEBUG_MODE) {
	va_list params;
	char buf[BUFSIZ];

	va_start(params, op);
	vsprintf(buf, op, params);
	for (GLint error = glGetError(); error; error = glGetError()) {
		switch (error) {
		case GL_INVALID_ENUM:
			std::cout << "after %s() glError: GL_INVALID_ENUM \n" << buf;
			break;
		case GL_INVALID_VALUE:
			std::cout << "after %s() glError: GL_INVALID_VALUE \n" << buf;
			break;
		case GL_INVALID_OPERATION:
			std::cout << "after %s() glError: GL_INVALID_OPERATION \n" << buf;
			break;
		case GL_INVALID_FRAMEBUFFER_OPERATION:
			std::cout << "after %s() glError: GL_INVALID_FRAMEBUFFER_OPERATION \n" << buf;
			break;
		case GL_OUT_OF_MEMORY:
			std::cout << "after %s() glError: GL_OUT_OF_MEMORY \n" << buf;
			break;
		default:
			break;
		}
	}
	va_end(params);
	//}
}
void FrameBufferGl::create(std::vector<std::shared_ptr<TextureGl>> textures, std::shared_ptr<TextureGl> depthTexture) {
	glGenFramebuffers(1, &id);
	std::cout << id << std::endl;
	checkGlError("glGenFramebuffers");
	bind();
	checkGlError("glBindFramebuffer");

	int i = 0;
	std::vector<unsigned> attachments;
	attachments.resize(textures.size());
	for (auto t : textures) {
		attachments[i] = GL_COLOR_ATTACHMENT0 + i;
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, t->id, 0);
		checkGlError("glFramebufferTexture2D");
		i++;
	}
	if (attachments.size()) {
#ifndef USING_GLES
		glDrawBuffers(attachments.size(), attachments.data());
#endif
	}
	if (!depthTexture) {
		const unsigned int SCR_WIDTH = textures[0]->getWidth();
		const unsigned int SCR_HEIGHT = textures[0]->getHeight();
		std::cout << SCR_WIDTH << " " << SCR_HEIGHT << std::endl;

		//GLuint depth_texture;
		//glGenTextures(1, &depth_texture);
		//glBindTexture(GL_TEXTURE_2D, depth_texture);
		//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		//glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SCR_WIDTH, SCR_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, NULL);
		//glBindTexture(GL_TEXTURE_2D, 0);
		//glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depth_texture, 0);

		unsigned int rboDepth;
		glGenRenderbuffers(1, &rboDepth);
		checkGlError("glGenRenderbuffers");
		glBindRenderbuffer(GL_RENDERBUFFER, rboDepth);
		checkGlError("glBindRenderbuffer");
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, SCR_WIDTH, SCR_HEIGHT);
		checkGlError("glRenderbufferStorage");
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboDepth);
		checkGlError("glFramebufferRenderbuffer");

		// finally check if framebuffer is complete
		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
			std::cout << "Framebuffer not complete!" << glCheckFramebufferStatus(GL_FRAMEBUFFER) << std::endl;
	}
	else {
		if (depthTexture->getType() == TextureType::TEXTURE_2D) {
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthTexture->id, 0);
		}
		else
		{
#ifndef USING_GLES
			glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depthTexture->id, 0);
#endif
		}
#ifndef USING_GLES
		glDrawBuffer(GL_NONE);
		glReadBuffer(GL_NONE);
#endif
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
#ifndef USING_GLES
	glBindFramebuffer(GL_READ_FRAMEBUFFER, from.id);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, to.id);
	glBlitFramebuffer(0, 0, w, h, 0, 0, w, h, GL_DEPTH_BUFFER_BIT, GL_NEAREST);
#endif
}
#endif



