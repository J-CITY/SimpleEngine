#version 450
#extension GL_KHR_vulkan_glsl : enable

layout(location = 0) out vec2 outUV;

void main() {
	outUV = vec2((gl_VertexIndex << 1) & 2, gl_VertexIndex & 2);
	gl_Position = vec4(outUV * 2.0 - 1.0, 0.0, 1.0);
}
