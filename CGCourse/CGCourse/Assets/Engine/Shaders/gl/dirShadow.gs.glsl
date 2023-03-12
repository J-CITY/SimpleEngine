#version 460 core

layout(triangles, invocations = 5) in;
layout(triangle_strip, max_vertices = 3) out;

//layout (std140, binding = 1) 
uniform EngineDirShadowUBO {
    mat4 lightSpaceMatrices[16];
} engine_DirShadowUBO ;

void main() {
	for (int i = 0; i < 3; ++i) {
		gl_Position = engine_DirShadowUBO.lightSpaceMatrices[gl_InvocationID] * gl_in[i].gl_Position;
		gl_Layer = gl_InvocationID;
		EmitVertex();
	}
	EndPrimitive();
}
