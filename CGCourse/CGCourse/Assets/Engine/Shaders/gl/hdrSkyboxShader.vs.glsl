#version 420 core
layout (location = 0) in vec3 aPos;

#include "../lib/engineUBO.glsl"

out vec3 WorldPos;

void main() {
    WorldPos = aPos;
	mat4 rotView = mat4(mat3(engine_UBO.View));
	vec4 clipPos = engine_UBO.Projection * rotView * vec4(WorldPos, 1.0);
	gl_Position = clipPos.xyww;

	//vec3 pos = aPos * 10000.0f;
    //gl_Position = engine_UBO.Projection * engine_UBO.View * vec4(pos, 1);
    ////vec4 WVP_Pos = gWVP * vec4(Position, 1.0);
    ////gl_Position = WVP_Pos.xyww;
    //WorldPos = aPos;
}