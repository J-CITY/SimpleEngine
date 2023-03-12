#version 460 core

out vec4 FragColor;

in vec2 TexCoords;

#include "../lib/engineUBO.glsl"

uniform sampler2D sssTex;
uniform sampler2D inputTex;

float rand(vec2 co)  {
    return fract(sin(dot(co.xy, vec2(12.9898, 78.233))) * 43758.5453);
}

void main() {
    float shadow = texture(sssTex, TexCoords).r;
    FragColor = vec4(texture(inputTex, TexCoords).rgb * shadow, 1);
}
