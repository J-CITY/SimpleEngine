#version 460 core

out vec4 FragColor;

in vec2 TexCoords;

#include "../lib/engineUBO.glsl"


uniform sampler2D albedoTex;
//uniform sampler2D normalTex;
//uniform sampler2D depthTex;

uniform sampler2D SSRTex;
uniform sampler2D HDRTex;
uniform sampler2D roughAO;
uniform sampler2D inputTex;

float rand(vec2 co)  {
    return fract(sin(dot(co.xy, vec2(12.9898, 78.233))) * 43758.5453);
}

void main() {
    vec3 albedo = texture(albedoTex, TexCoords).rgb;
    //TODO: uncoment (this support only for PBR)
    float metallicFactor = 0.3;//texture(albedoTex, TexCoords).a;
    float roughnessFactor = 0.3;//texture(roughAO, TexCoords).r;
    //FragmentInfo fragment = getFragmentInfo(TexCoords, albedoTex, normalTex, materialTex, depthTex, camera.invViewProjMatrix);
    
    float lod = 8.0 * roughnessFactor * roughnessFactor;

    vec3 F0 = mix(vec3(0.04), albedo, metallicFactor);
    
    float r = rand(TexCoords);
    vec2 texelSize = 1.0 / textureSize(SSRTex, 0);

    vec3 initialColor = texture(inputTex, TexCoords).rgb;
    vec3 ssrReflection = texture(SSRTex, TexCoords + sqrt(lod) * texelSize * r, lod).rgb;
    vec3 totalColor = initialColor + F0 * ssrReflection;

    FragColor = vec4(totalColor, 1.0);
    //FragColor = vec4(texture(SSRTex, TexCoords).rgb, 1);
}
