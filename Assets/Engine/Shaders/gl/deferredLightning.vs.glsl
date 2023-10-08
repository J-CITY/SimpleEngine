#version 460 core

layout (location = 0) in vec3 geo_Pos;
layout (location = 1) in vec2 geo_TexCoords;

out vec2 TexCoords;

void main() {
    TexCoords = geo_TexCoords;
    gl_Position = vec4(geo_Pos, 1.0);
}
