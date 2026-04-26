#include "storageBufferGl.h"
#ifdef OPENGL_BACKEND
#include <coreModule/graphicsWrapper.hpp>


IKIGAI::RENDER::StorageBufferGl::StorageBufferGl(const void* data, size_t sz, size_t stride) : StorageBufferInterface(sz, stride) {
#ifndef USING_GLES
	glGenBuffers(1, &mId);
	if (data) {
		StorageBufferGl::setData(data, sz, stride);
	}
#endif
}

IKIGAI::RENDER::StorageBufferGl::~StorageBufferGl() {
#ifndef USING_GLES
	glDeleteBuffers(1, &mId);
#endif
}

void IKIGAI::RENDER::StorageBufferGl::setData(const void* data, size_t sz, size_t stride) {
#ifndef USING_GLES
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, mId);
	glBufferData(GL_SHADER_STORAGE_BUFFER, sz * stride, data, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
#endif
}

void IKIGAI::RENDER::StorageBufferGl::setSubData(const void* data, size_t sz, size_t offset) {
#ifndef USING_GLES
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, mId);
	glBufferSubData(GL_SHADER_STORAGE_BUFFER, offset, sz, data);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
#endif
}

IKIGAI::RENDER::StorageBufferGl::Id IKIGAI::RENDER::StorageBufferGl::getId() const {
	return mId;
}

//void ShaderStorageBufferGl::bind(unsigned val) {
//#ifndef USING_GLES
//	mBindId = val;
//	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, val, mId);
//#endif
//}
//
//void ShaderStorageBufferGl::unbind() {
//#ifndef USING_GLES
//	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, mBindId, 0);
//#endif
//}

#endif
