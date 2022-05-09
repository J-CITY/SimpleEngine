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
    if (u_UseFXAA) {
        OutColor = vec4(fxaa(u_Scene, TexCoords).rgb, 1.0f);
    }
    else {
        OutColor = texture(u_Scene, TexCoords);
    }
}