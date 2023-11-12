#version 460 core
layout (location = 0) in vec3 geo_Pos;
layout (location = 1) in vec2 geo_TexCoords;
layout (location = 2) in vec3 geo_Normal;
layout (location = 3) in vec3 geo_Tangent;
layout (location = 4) in vec3 geo_Bitangent;
layout (location = 5) in ivec4 boneIds;
layout (location = 6) in vec4 weights;

#include "../lib/engineUBO.glsl"

uniform mat4 prj;

void main() {
    gl_Position = prj * engine_Model.model * vec4(geo_Pos, 1.0);
}
