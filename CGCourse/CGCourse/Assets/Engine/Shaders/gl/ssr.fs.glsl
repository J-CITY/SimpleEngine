#version 460 core

out vec4 FragColor;

in vec2 TexCoords;

#include "../lib/engineUBO.glsl"

uniform sampler2D normalTex;
uniform sampler2D depthTex;
uniform sampler2D HDRTex;

uniform int   steps = 10;
uniform float thickness = 0.5;
uniform float startDistance = 2.0;

vec4 worldToFragSpace(vec3 v, mat4 viewProj) {
    vec4 proj = viewProj * vec4(v, 1.0f);
    proj.xyz /= proj.w;
    proj.xy = proj.xy * 0.5f + vec2(0.5f);
    return proj;
}

vec3 reconstructWorldPosition(float depth, vec2 texcoord, mat4 invViewProjMatrix) {
    vec4 normPosition = vec4(2.0f * texcoord - vec2(1.0f), depth, 1.0f);
    vec4 worldPosition = invViewProjMatrix * normPosition;
    worldPosition /= worldPosition.w;
    return worldPosition.xyz;
}

void main() {
    mat4 viewProjMatrix = engine_UBO.Projection * engine_UBO.View;
    mat4 invViewProjMatrix = inverse(viewProjMatrix);

    vec3 position = texture(depthTex, TexCoords).rgb;
    vec3 normal = texture(normalTex, TexCoords).rgb;

    vec3 viewDistance = engine_UBO.ViewPos - position;
    vec3 viewDirection = normalize(viewDistance);

    vec3 pivot = normalize(reflect(-viewDirection, normal));
    vec3 startPos = position + (pivot * 0.0001);

    float currentLength = min(length(viewDistance) / steps, startDistance);
    float bestDepth = 10000.0;
    vec2 bestUV = vec2(0.0);

    for (int i = 0; i < steps; i++) {
        vec3 currentPosition = startPos + pivot * currentLength;
        vec4 projectedPosition = worldToFragSpace(currentPosition, viewProjMatrix);
        vec2 currentUV = projectedPosition.xy;
        float projectedDepth = projectedPosition.z;

        if (currentUV.x > 1.0 || currentUV.y > 1.0 ||
            currentUV.x < 0.0 || currentUV.y < 0.0) {
            break;
        }
        float currentFragDepth = texture(depthTex, currentUV).r;
        float depthDiff = abs(1.0 / projectedDepth - 1.0 / currentFragDepth);
        if (depthDiff < bestDepth) {
            bestUV = currentUV;
            bestDepth = depthDiff;
            if (depthDiff < thickness)
                break;
        }
        else {
            vec3 newPosition = reconstructWorldPosition(currentFragDepth, currentUV, invViewProjMatrix);
            currentLength = length(startPos - newPosition);
        }
    }
    
    vec3 reflection = bestUV != vec2(0.0) ? texture(HDRTex, bestUV).rgb : vec3(0.0);
    FragColor = vec4(reflection, 1.0);
}
