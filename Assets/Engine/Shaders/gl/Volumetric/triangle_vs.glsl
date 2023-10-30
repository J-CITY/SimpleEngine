#version 460 core
// ------------------------------------------------------------------
// OUTPUT VARIABLES -------------------------------------------------
// ------------------------------------------------------------------
layout (location = 0) in vec3 aPos;
out vec2 FS_IN_TexCoord;
// ------------------------------------------------------------------
// MAIN -------------------------------------------------------------
// ------------------------------------------------------------------
#include "../../lib/engineUBO.glsl"
void main()
{
    FS_IN_TexCoord = vec2((gl_VertexID << 1) & 2, gl_VertexID & 2);
    gl_Position    = vec4(FS_IN_TexCoord * 2.0f - 1.0f, 1.0f, 1.0f);

    //FS_IN_TexCoord = aPos.xy;
	//mat4 rotView = mat4(mat3(engine_UBO.View));
	//vec4 clipPos = engine_UBO.Projection * rotView * vec4(aPos, 1.0);
	//gl_Position = clipPos.xyww;
}

// ------------------------------------------------------------------
