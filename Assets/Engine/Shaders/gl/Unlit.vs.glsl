#version 460 core

layout (location = 0) in vec3 geo_Pos;
layout (location = 1) in vec2 geo_TexCoords;
layout (location = 2) in vec3 geo_Normal;

#include "../lib/engineUBO.glsl"

out VS_OUT
{
    vec2 TexCoords;
} vs_out;

void main()
{
    vs_out.TexCoords = geo_TexCoords;

    gl_Position = engine_UBO.Projection * engine_UBO.View * engine_Model.model * vec4(geo_Pos, 1.0);
}
