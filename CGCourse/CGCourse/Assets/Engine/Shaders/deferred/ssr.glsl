#shader vertex
#version 460 core

layout(location = 0) in vec4 position;

out vec2 TexCoord;

void main()
{
    gl_Position = position;
    TexCoord = position.xy * 0.5 + vec2(0.5);
}

#shader fragment
#version 460 core
vec3 reconstructWorldPosition(float depth, vec2 texcoord, mat4 invViewProjMatrix)
{
    vec4 normPosition = vec4(2.0f * texcoord - vec2(1.0f), depth, 1.0f);
    vec4 worldPosition = invViewProjMatrix * normPosition;
    worldPosition /= worldPosition.w;
    return worldPosition.xyz;
}

float getTextureLodLevel(vec2 uv)
{
    vec2 dxVtc = dFdx(uv);
    vec2 dyVtc = dFdy(uv);
    float deltaMax2 = max(dot(dxVtc, dxVtc), dot(dyVtc, dyVtc));
    return 0.5 * log2(deltaMax2);
}

float sampleShadowMap(sampler2D depthMap, vec2 coords, float lod, float compare)
{
    return step(compare, texture(depthMap, coords, lod).r);
}

float calcShadowFactor2D(vec3 coords, sampler2D depthMap, vec4 textureLimitsXY, float bias)
{
    if (coords.x > textureLimitsXY[1] || coords.x < textureLimitsXY[0]) return 1.0;
    if (coords.y > textureLimitsXY[3] || coords.y < textureLimitsXY[2]) return 1.0;
    if (coords.z > 1.0 - bias || coords.z < bias) return 1.0; // do not handle corner cases, assume no shadows
    float compare = coords.z - bias;
    
    const float lod = getTextureLodLevel(coords.xy);
    vec2 texelSize = textureSize(depthMap, 0);
    vec2 texelSizeInv = 1.0 / texelSize;
    vec2 pixelPos = coords.xy * texelSize + vec2(0.5);
    vec2 fracPart = fract(pixelPos);
    vec2 startTexel = (pixelPos - fracPart) * texelSizeInv;

    vec3 mixY[3];

    mixY[0][0] = sampleShadowMap(depthMap, startTexel + vec2( 1.0,  1.0) * texelSizeInv, lod, compare);
    mixY[0][1] = sampleShadowMap(depthMap, startTexel + vec2( 0.0,  1.0) * texelSizeInv, lod, compare);
    mixY[0][2] = sampleShadowMap(depthMap, startTexel + vec2(-1.0,  1.0) * texelSizeInv, lod, compare);
    mixY[1][0] = sampleShadowMap(depthMap, startTexel + vec2( 1.0,  0.0) * texelSizeInv, lod, compare);
    mixY[1][1] = sampleShadowMap(depthMap, startTexel + vec2( 0.0,  0.0) * texelSizeInv, lod, compare);
    mixY[1][2] = sampleShadowMap(depthMap, startTexel + vec2(-1.0,  0.0) * texelSizeInv, lod, compare);
    mixY[2][0] = sampleShadowMap(depthMap, startTexel + vec2( 1.0, -1.0) * texelSizeInv, lod, compare);
    mixY[2][1] = sampleShadowMap(depthMap, startTexel + vec2( 0.0, -1.0) * texelSizeInv, lod, compare);
    mixY[2][2] = sampleShadowMap(depthMap, startTexel + vec2(-1.0, -1.0) * texelSizeInv, lod, compare);

    vec3 samplesY = mix(mixY[1], mixY[0], fracPart.y) + mix(mixY[2], mixY[1], fracPart.y);
    float s = mix(samplesY[1], samplesY[0], fracPart.x) + mix(samplesY[2], samplesY[1], fracPart.x);
    //s = sampleShadowMap(depthMap, coords.xy, lod, compare);
    s = pow(clamp(s, 0.0, 1.0), 8.0);
    return s;
}

const int POINT_LIGHT_SAMPLES = 20;
vec3 sampleOffsetDirections[POINT_LIGHT_SAMPLES] = vec3[]
(
    vec3(1, 1,  1), vec3( 1, -1,  1), vec3(-1, -1,  1), vec3(-1, 1,  1),
    vec3(1, 1, -1), vec3( 1, -1, -1), vec3(-1, -1, -1), vec3(-1, 1, -1),
    vec3(1, 1,  0), vec3( 1, -1,  0), vec3(-1, -1,  0), vec3(-1, 1,  0),
    vec3(1, 0,  1), vec3(-1,  0,  1), vec3( 1,  0, -1), vec3(-1, 0, -1),
    vec3(0, 1,  1), vec3( 0, -1,  1), vec3( 0, -1, -1), vec3( 0, 1, -1)
);

float CalcShadowFactor3D(vec3 fragToLightRay, vec3 viewDist, float zfar, float bias, samplerCube depthMap)
{
    float invZfar = 1.0f / zfar;
    float currentDepth = length(fragToLightRay);
    currentDepth = (currentDepth - bias) * invZfar;
    float diskRadius = (1.0f + invZfar) * 0.04f;
    float shadowFactor = 0.0f;

    for (int i = 0; i < POINT_LIGHT_SAMPLES; i++)
    {
        float closestDepth = textureLod(depthMap, sampleOffsetDirections[i] * diskRadius - fragToLightRay, 0).r;
        shadowFactor += (currentDepth > closestDepth) ? 0.0f : 1.0f;
    }
    shadowFactor /= float(POINT_LIGHT_SAMPLES);
    return shadowFactor;
}

vec3 calcReflectionColor(samplerCube reflectionMap, mat3 reflectionMapTransform, vec3 viewDir, vec3 normal, float lod)
{
    vec3 I = -viewDir;
    vec3 reflectionRay = reflect(I, normal);
    reflectionRay = dot(viewDir, normal) < 0.0 ? -reflectionRay : reflectionRay;
    reflectionRay = reflectionMapTransform * reflectionRay;

    float defaultLod = getTextureLodLevel(reflectionRay.xy);
    vec3 color = textureLod(reflectionMap, reflectionRay, max(lod, defaultLod)).rgb;
    return color;
}

vec3 calcReflectionColor(samplerCube reflectionMap, mat3 reflectionMapTransform, vec3 viewDir, vec3 normal)
{
    return calcReflectionColor(reflectionMap, reflectionMapTransform, viewDir, normal, 0.0);
}

vec4 worldToFragSpace(vec3 v, mat4 viewProj)
{
    vec4 proj = viewProj * vec4(v, 1.0f);
    proj.xyz /= proj.w;
    proj.xy = proj.xy * 0.5f + vec2(0.5f);
    return proj;
}

struct FragmentInfo
{
    vec3 albedo;
    float ambientOcclusion;
    float emmisionFactor;
    float roughnessFactor;
    float metallicFactor;
    float depth;
    vec3 normal;
    vec3 position;
};

struct EnvironmentInfo
{
    samplerCube skybox;
    samplerCube irradiance;
    mat3 skyboxRotation;
    float intensity;
    sampler2D envBRDFLUT;
};

FragmentInfo getFragmentInfo(vec2 texCoord, sampler2D albedoTexture, sampler2D normalTexture, sampler2D materialTexture, sampler2D depthTexture, mat4 invViewProjMatrix)
{
    FragmentInfo fragment;

    fragment.normal = normalize(texture(normalTexture, texCoord).rgb - vec3(0.5f));
    vec4 albedo = texture(albedoTexture, texCoord).rgba;
    vec4 material = texture(materialTexture, texCoord).rgba;
    fragment.depth = texture(depthTexture, texCoord).r;

    fragment.albedo = albedo.rgb;
    fragment.emmisionFactor = albedo.a / (1.0f - albedo.a);
    fragment.ambientOcclusion = material.r;
    fragment.roughnessFactor = material.g;
    fragment.metallicFactor = material.b;

    fragment.position = reconstructWorldPosition(fragment.depth, texCoord, invViewProjMatrix);

    return fragment;
}

in vec2 TexCoord;
out vec4 OutColor;

struct Camera
{
    vec3 position;
    mat4 viewProjMatrix;
    mat4 invViewProjMatrix;
};


uniform sampler2D albedoTex;
uniform sampler2D normalTex;
uniform sampler2D materialTex;
uniform sampler2D depthTex;
uniform sampler2D HDRTex;

uniform Camera camera;
uniform EnvironmentInfo environment;

uniform int   steps;
uniform float thickness;
uniform float startDistance;

void main()
{
    FragmentInfo fragment = getFragmentInfo(TexCoord, albedoTex, normalTex, materialTex, depthTex, camera.invViewProjMatrix);

    vec3 viewDistance = camera.position - fragment.position;
    vec3 viewDirection = normalize(viewDistance);

    vec3 pivot = normalize(reflect(-viewDirection, fragment.normal));
    vec3 startPos = fragment.position + (pivot * 0.0001);

    float currentLength = min(length(viewDistance) / steps, startDistance);
    float bestDepth = 10000.0;
    vec2 bestUV = vec2(0.0);

    for (int i = 0; i < steps; i++)
    {
        vec3 currentPosition = startPos + pivot * currentLength;
        vec4 projectedPosition = worldToFragSpace(currentPosition, camera.viewProjMatrix);
        vec2 currentUV = projectedPosition.xy;
        float projectedDepth = projectedPosition.z;

        if (currentUV.x > 1.0 || currentUV.y > 1.0 ||
            currentUV.x < 0.0 || currentUV.y < 0.0) break;

        float currentFragDepth = texture(depthTex, currentUV).r;
        float depthDiff = abs(1.0 / projectedDepth - 1.0 / currentFragDepth);
        if (depthDiff < bestDepth)
        {
            bestUV = currentUV;
            bestDepth = depthDiff;
            if (depthDiff < thickness)
                break;
        }
        else
        {
            vec3 newPosition = reconstructWorldPosition(currentFragDepth, currentUV, camera.invViewProjMatrix);
            currentLength = length(startPos - newPosition);
        }
    }
    
    vec3 reflection = bestUV != vec2(0.0) ? texture(HDRTex, bestUV).rgb : vec3(0.0);
    OutColor = vec4(reflection, 1.0);
}
