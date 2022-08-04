#include "uniformBuffer.h"

using namespace KUMA;
using namespace KUMA::RENDER;


UniformBuffer::UniformBuffer(unsigned sz, unsigned bindingPoint, unsigned offset, AccessSpecifier accessSpecifier) {
	glGenBuffers(1, &ID);
	glBindBuffer(GL_UNIFORM_BUFFER, ID);
	glBufferData(GL_UNIFORM_BUFFER, sz, NULL, static_cast<GLint>(accessSpecifier));
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
	glBindBufferRange(GL_UNIFORM_BUFFER, bindingPoint, ID, offset, sz);
}

UniformBuffer::~UniformBuffer() {
	glDeleteBuffers(1, &ID);
}

void UniformBuffer::bind() const {
	glBindBuffer(GL_UNIFORM_BUFFER, ID);
}

void UniformBuffer::unbind() const {
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

unsigned UniformBuffer::getID() const {
	return ID;
}

void UniformBuffer::BindBlockToShader(RESOURCES::Shader& p_shader, uint32_t p_uniformBlockLocation, uint32_t p_bindingPoint) {
	glUniformBlockBinding(p_shader.getId(), p_uniformBlockLocation, p_bindingPoint);
}

void UniformBuffer::BindBlockToShader(RESOURCES::Shader& p_shader, const std::string& p_name, uint32_t p_bindingPoint) {
	glUniformBlockBinding(p_shader.getId(), GetBlockLocation(p_shader, p_name), p_bindingPoint);
}

uint32_t UniformBuffer::GetBlockLocation(RESOURCES::Shader& p_shader, const std::string& p_name) {
	return glGetUniformBlockIndex(p_shader.getId(), p_name.c_str());
}

