#include "uniformBuffer.h"

using namespace KUMA;
using namespace KUMA::RENDER;


void UniformBuffer::BindBlockToShader(RESOURCES::Shader& p_shader, uint32_t p_uniformBlockLocation, uint32_t p_bindingPoint) {
	glUniformBlockBinding(p_shader.id, p_uniformBlockLocation, p_bindingPoint);
}

void UniformBuffer::BindBlockToShader(RESOURCES::Shader& p_shader, const std::string& p_name, uint32_t p_bindingPoint) {
	glUniformBlockBinding(p_shader.id, GetBlockLocation(p_shader, p_name), p_bindingPoint);
}

uint32_t UniformBuffer::GetBlockLocation(RESOURCES::Shader& p_shader, const std::string& p_name) {
	return glGetUniformBlockIndex(p_shader.id, p_name.c_str());
}

