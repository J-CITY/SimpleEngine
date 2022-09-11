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

const float PI = 3.14159265359;

layout (std140) uniform EngineUBO {
    mat4    ubo_Model;
    mat4    ubo_View;
    mat4    ubo_Projection;
    vec3    ubo_ViewPos;
    float   ubo_Time;
    vec2    ubo_ViewportSize;
};

uniform sampler2D albedoTex;//remove
uniform sampler2D normalTex;//remove
uniform sampler2D depthTex;


uniform sampler2D inputTex;
uniform int raySteps = 3;
uniform float intensity = 2.5;
uniform float distance = 50.0;

mat4 viewProjMatrix;

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

float rand(vec2 co)
{
    return fract(sin(dot(co.xy, vec2(12.9898, 78.233))) * 43758.5453);
}

float getLOD(vec2 offset)
{
    float v = clamp(pow(dot(offset, offset), 0.1), 0.0, 1.0);
    float lod = 10.0 * v;
    return lod;
}

float calculateShadowRayCast(vec3 startPosition, vec3 endPosition, vec2 startUV)
{
    float rayDistance = length(endPosition - startPosition);
    vec3 rayDirection = normalize(endPosition - startPosition);

    float distancePerStep = rayDistance / raySteps;
    for (int i = 1; i < raySteps; i++)
    {
        float currentDistance = i * distancePerStep;
        vec3 currentPosition = startPosition + rayDirection * currentDistance;

        vec4 projectedPosition = worldToFragSpace(currentPosition, viewProjMatrix);
        vec2 currentUV = projectedPosition.xy;

        float lod = getLOD(currentUV - startUV);
        float projectedDepth = 1.0 / projectedPosition.z;

        float currentDepth = 1.0 / textureLod(depthTex, currentUV, lod).r;
        if (projectedDepth > currentDepth + 0.1)
        {
            return float(i - 1) / raySteps;
        }
    }

    return 1.0;
}

void main() {
    viewProjMatrix = ubo_Projection * ubo_View;
    mat4 invViewProjMatrix = inverse(viewProjMatrix);
    //FragmentInfo fragment = getFragmentInfo(TexCoords, albedoTex, normalTex, materialTex, depthTex, invViewProjMatrix);

    vec3 position = texture(depthTex, TexCoords).rgb;
    vec3 viewDirection = normalize(ubo_ViewPos - position);

    float r = rand(TexCoords);
    vec2 invSize = 1.0 / textureSize(inputTex, 0);
    const int SAMPLES = 4;
    vec3 accum = vec3(0.0);
    for (int i = 0; i < SAMPLES; i++)
    {
        float sampleDistance = exp(i - SAMPLES);
        float phi = ((i + r * SAMPLES) * 2.0 * PI) / SAMPLES;
        vec2 uv = sampleDistance * vec2(cos(phi), sin(phi));

        float lod = getLOD(uv);
        vec3 lightColor = textureLod(inputTex, TexCoords + uv, lod).rgb;
        float sampleDepth = textureLod(depthTex, TexCoords + uv, lod).r;
        vec3 lightPosition = reconstructWorldPosition(sampleDepth, TexCoords + uv, invViewProjMatrix);
        vec3 lightDirection = lightPosition - position;

        float currentDistance = length(lightDirection);
        float distanceAttenuation = clamp(1.0f - pow(currentDistance / distance, 4.0f), 0.0, 1.0);
        distanceAttenuation = isinf(currentDistance) ? 0.0 : distanceAttenuation;

        float shadowFactor = calculateShadowRayCast(position, lightPosition, TexCoords);

        accum += lightColor * shadowFactor * distanceAttenuation;
    }

    FragColor = vec4(accum * intensity, 1.0);
}
