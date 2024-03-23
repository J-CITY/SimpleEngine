#include "storageBufferGl.h"
#ifdef OPENGL_BACKEND

using namespace IKIGAI::RENDER;

ShaderStorageBufferGl::ShaderStorageBufferGl(IKIGAI::RENDER::AccessSpecifier type) {
	glGenBuffers(1, &bufferID);
#ifndef USING_GLES
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, bufferID);
	glBufferData(GL_SHADER_STORAGE_BUFFER, 0, nullptr, static_cast<GLenum>(type));
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, bufferID);
#endif
}

ShaderStorageBufferGl::~ShaderStorageBufferGl() {
#ifndef USING_GLES
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
#endif
}

void ShaderStorageBufferGl::bind(unsigned val) {
	bindingPoint = val;
#ifndef USING_GLES
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, val, bufferID);
#endif
}

void ShaderStorageBufferGl::unbind() {
#ifndef USING_GLES
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, bindingPoint, 0);
#endif
}

#endif
