#include "storageBufferGl.h"
#ifdef OPENGL_BACKEND

using namespace IKIGAI::RENDER;

ShaderStorageBufferGl::ShaderStorageBufferGl(IKIGAI::RENDER::AccessSpecifier type) {
	glGenBuffers(1, &bufferID);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, bufferID);
	glBufferData(GL_SHADER_STORAGE_BUFFER, 0, nullptr, static_cast<GLenum>(type));
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, bufferID);
}

ShaderStorageBufferGl::~ShaderStorageBufferGl() {
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}

void ShaderStorageBufferGl::bind(unsigned val) {
	bindingPoint = val;
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, val, bufferID);
}

void ShaderStorageBufferGl::unbind() {
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, bindingPoint, 0);
}

#endif
