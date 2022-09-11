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

out vec4 FragColor;

in vec2 TexCoords;

layout (std140) uniform EngineUBO {
    mat4    ubo_Model;
    mat4    ubo_View;
    mat4    ubo_Projection;
    vec3    ubo_ViewPos;
    float   ubo_Time;
    vec2    ubo_ViewportSize;
};


uniform sampler2D albedoTex;
uniform sampler2D normalTex;
uniform sampler2D depthTex;

uniform sampler2D SSRTex;
uniform sampler2D HDRTex;
uniform sampler2D roughAO;

float rand(vec2 co)  {
    return fract(sin(dot(co.xy, vec2(12.9898, 78.233))) * 43758.5453);
}

void main() {
    vec3 albedo = texture(albedoTex, TexCoords).rgb;
    float metallicFactor = texture(albedoTex, TexCoords).a;
    float roughnessFactor = texture(roughAO, TexCoords).r;
    //FragmentInfo fragment = getFragmentInfo(TexCoords, albedoTex, normalTex, materialTex, depthTex, camera.invViewProjMatrix);
    
    float lod = 8.0 * roughnessFactor * roughnessFactor;

    vec3 F0 = mix(vec3(0.04), albedo, metallicFactor);
    
    float r = rand(TexCoords);
    vec2 texelSize = 1.0 / textureSize(SSRTex, 0);

    vec3 initialColor = texture(HDRTex, TexCoords).rgb;
    vec3 ssrReflection = texture(SSRTex, TexCoords + sqrt(lod) * texelSize * r, lod).rgb;
    vec3 totalColor = initialColor + F0 * ssrReflection;

    FragColor = vec4(totalColor, 1.0);
}
