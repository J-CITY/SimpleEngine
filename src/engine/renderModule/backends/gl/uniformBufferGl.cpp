#include "uniformBufferGl.h"
#ifdef OPENGL_BACKEND
#include <coreModule/graphicsWrapper.hpp>

IKIGAI::RENDER::UniformBufferGl::UniformBufferGl(const void* data, size_t sz) : UniformBufferInterface(sz) {
	glGenBuffers(1, &mId);
//#ifndef USING_GLES
	if (data) {
		UniformBufferGl::setData(data, sz);
	}
//#endif
}

IKIGAI::RENDER::UniformBufferGl::~UniformBufferGl() {
	glDeleteBuffers(1, &mId);
}

void IKIGAI::RENDER::UniformBufferGl::setData(const void* data, size_t sz, size_t offset) {
	bind();
	glBufferSubData(GL_UNIFORM_BUFFER, offset, sz, data);
	unbind();
}

void IKIGAI::RENDER::UniformBufferGl::bind() {
	glBindBuffer(GL_UNIFORM_BUFFER, mId);
}

void IKIGAI::RENDER::UniformBufferGl::bindToShader(unsigned slot) {
	glBindBufferRange(GL_UNIFORM_BUFFER, slot, mId, 0, static_cast<long long>(mSizeByte));
}

void IKIGAI::RENDER::UniformBufferGl::unbind() {
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

unsigned IKIGAI::RENDER::UniformBufferGl::getId() const {
	return mId;
}
#endif
