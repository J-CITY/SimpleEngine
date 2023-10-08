#shader vertex
#version 460 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoords;

out vec2 TexCoords;

void main() {
    TexCoords = aTexCoords;
    gl_Position = vec4(aPos, 1.0);
}

#shader fragment
#version 460 core

#include "./lib/fxaa.glsl"

in vec2 TexCoords;
out vec4 OutColor;

uniform sampler2D u_Scene;
uniform bool u_UseFXAA;

void main() {
    vec4 color = texture(u_Scene, TexCoords);

    float avg = (color.r + color.g + color.b) / 3.0;
    float rd = avg - color.r;
    float gd = avg - color.g;
    float bd = avg - color.b;
    
    color.r = color.r + rd;
    color.g = color.g + gd;
    color.b = color.b + bd;

    OutColor = color;
}