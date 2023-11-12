#version 460 core

out vec4 FragColor;

in vec2 TexCoords;

#include "../lib/engineUBO.glsl"

uniform sampler2D inputTex;
uniform sampler2D SSGITex;
uniform sampler2D albedoTex;

void main() {
    vec3 originalColor = texture(inputTex, TexCoords, 0).rgb;
    vec3 ssgiColor = texture(SSGITex, TexCoords, 0).rgb;

    vec3 albedo = texture(albedoTex, TexCoords).rgb;
    //TODO: uncoment (this support only for PBR)
    float metallic = 0.3;//texture(albedoTex, TexCoords).a;
    vec3 color = mix(albedo, originalColor, metallic * 0.5);

    FragColor = vec4(originalColor + color * ssgiColor, 1.0);
}
