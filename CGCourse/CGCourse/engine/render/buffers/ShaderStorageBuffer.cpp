#include "ShaderStorageBuffer.h"

using namespace KUMA;
using namespace KUMA::RENDER;

ShaderStorageBuffer::ShaderStorageBuffer(AccessSpecifier type) {
	glGenBuffers(1, &bufferID);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, bufferID);
	glBufferData(GL_SHADER_STORAGE_BUFFER, 0, nullptr, static_cast<GLenum>(type));
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, bufferID);
}

ShaderStorageBuffer::~ShaderStorageBuffer() {
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}

void ShaderStorageBuffer::bind(unsigned val) {
	bindingPoint = val;
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, val, bufferID);
}

void ShaderStorageBuffer::unbind() {
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, bindingPoint, 0);
}
