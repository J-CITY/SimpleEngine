#shader vertex
#version 460 core

layout(location = 0)  in vec4 position;
layout(location = 5)  in mat4 transform;
layout(location = 9)  in vec4 sphereParameters;
layout(location = 10) in vec4 colorParameters;

out PointLightInfo
{
    vec3 position;
    float radius;
    vec4 color;
} pointLight;

struct Camera
{
    vec3 position;
    mat4 invViewProjMatrix;
    mat4 viewProjMatrix;
};

uniform Camera camera;

void main()
{
    vec4 position = camera.viewProjMatrix * transform * position;
    gl_Position = position;

    pointLight.position = sphereParameters.xyz;
    pointLight.radius = sphereParameters.w;
    pointLight.color = colorParameters;
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

// https://m.habr.com/ru/post/326852/

#define PI 3.1415926535f

float GGXPartialGeometry(float NV, float roughness2)
{
    return NV / mix(NV, 1.0, roughness2);
}

float GGXDistribution(float NH, float roughness)
{
    float roughness2 = roughness * roughness;
    float alpha2 = roughness2 * roughness2;
    float distr = (NH * NH) * (alpha2 - 1.0f) + 1.0f;
    float distr2 = distr * distr;
    float totalDistr = alpha2 / (PI * distr2);
    return totalDistr;
}

float GGXSmith(float NV, float NL, float roughness)
{
    float d = roughness * 0.125 + 0.125;
    float roughness2 = roughness * d + d;
    return GGXPartialGeometry(NV, roughness2)* GGXPartialGeometry(NL, roughness2);
}

vec3 fresnelSchlick(vec3 F0, float HV)
{
    vec3 fresnel = F0 + (1.0 - F0) * pow(2.0, (-5.55473 * HV - 6.98316) * HV);
    return fresnel;
}

vec3 fresnelSchlickRoughness(vec3 F0, float HV, float roughness)
{
    vec3 fresnel = F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(2.0, (-5.55473 * HV - 6.98316) * HV);
    return fresnel;
}

mat3 computeSampleTransform(vec3 normal)
{
    vec3 up = abs(normal.z) < 0.999f ? vec3(0.0f, 0.0f, 1.0f) : vec3(1.0f, 0.0f, 0.0f);

    mat3 w;
    w[0] = normalize(cross(normal, up));
    w[1] = cross(normal, w[0]);
    w[2] = normal;
    return w;
}

vec3 GGXImportanceSample(vec2 Xi, float roughness, vec3 normal, mat3 transform)
{
    float alpha = roughness * roughness;
    float phi = 2.0 * PI * Xi.x;
    float cosTheta = sqrt((1.0 - Xi.y) / (1.0 + (alpha * alpha - 1.0f) * Xi.y));
    float sinTheta = sqrt(1.0 - cosTheta * cosTheta);
    vec3 H = vec3(sinTheta * cos(phi), sinTheta * sin(phi), cosTheta);
    return transform * H;
}

float computeA(samplerCube tex, int sampleCount)
{
    vec2 sizes = textureSize(tex, 0);
    float A = 0.5f * log2(sizes.x * sizes.y / float(sampleCount));
    return A;
}

float computeLOD(float A, float pdf, vec3 lightDirection)
{
    float du = 2.0f * 1.2f * abs(lightDirection.y + 1.0f);
    return max(0.0f, A - 0.5f * log2(pdf * du * du) + 1.0f);
}

float radicalInverseVDC(uint bits)
{
    bits = (bits << 16u) | (bits >> 16u);
    bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
    bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
    bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
    bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
    return float(bits) * 2.3283064365386963e-10; // / 0x100000000
}

vec2 sampleHammersley(uint i, float invSampleCount)
{
    return vec2(i * invSampleCount, radicalInverseVDC(i));
}

void GGXCookTorranceSampled(vec3 normal, vec3 lightDirection, vec3 viewDirection, float roughness, float metallic, vec3 albedo, 
    out vec3 specular, out vec3 diffuse)
{
    vec3 H = normalize(viewDirection + lightDirection);
    float LV = dot(lightDirection, viewDirection);
    float NV = dot(normal, viewDirection);
    float NL = dot(normal, lightDirection);
    float NH = dot(normal, H);
    float HV = dot(H, viewDirection);

    if (NV < 0.0 || NL < 0.0)
    {
        specular = vec3(0.0);
        diffuse = vec3(0.0);
        return;
    }

    vec3 F0 = mix(vec3(0.04), albedo, metallic);

    float G = GGXSmith(NV, NL, roughness);
    float D = GGXDistribution(NH, roughness);
    vec3 F = fresnelSchlick(F0, HV);
    
    specular = D * F * G / (4.0 * NL * NV);
    specular = clamp(specular, vec3(0.0), vec3(1.0));

    float s = max(LV, 0.0) - NL * NV;
    float t = mix(1.0, max(NL, NV), step(0.0, s));
    float d = 1.0 - metallic;

    float sigma2 = roughness * roughness;
    float A = 1.0 + sigma2 * (d / (sigma2 + 0.13) + 0.5 / (sigma2 + 0.33));
    float B = 0.45 * sigma2 / (sigma2 + 0.09);

    diffuse = albedo * NL * (1.0 - F) * (A + B * s / t) / PI;
}

vec3 calculateLighting(FragmentInfo fragment, vec3 viewDirection, vec3 lightDirection, vec3 lightColor, float ambientFactor, float shadowFactor)
{
    float roughness = clamp(fragment.roughnessFactor, 0.05, 0.95);
    float metallic = clamp(fragment.metallicFactor, 0.05, 0.95);

    vec3 specularColor = vec3(0.0);
    vec3 diffuseColor = vec3(0.0);
    GGXCookTorranceSampled(fragment.normal, normalize(lightDirection), viewDirection, roughness, metallic, fragment.albedo, 
        specularColor, diffuseColor);

    vec3 ambientColor = diffuseColor * ambientFactor;

    float NL = clamp(dot(fragment.normal, lightDirection), 0.0, 1.0);

    float shadowCoef = NL * shadowFactor;

    vec3 totalColor = (ambientColor + (diffuseColor + specularColor) * shadowCoef) * lightColor;
    return totalColor * fragment.ambientOcclusion;
}


out vec4 OutColor;

uniform sampler2D albedoTex;
uniform sampler2D normalTex;
uniform sampler2D materialTex;
uniform sampler2D depthTex;

in PointLightInfo
{
    vec3 position;
    float radius;
    vec4 color;
} pointLight;

struct PointLight
{
    vec3 position;
    float radius;
    vec4 color;
};

struct Camera
{
    vec3 position;
    mat4 invViewProjMatrix;
    mat4 viewProjMatrix;
};

uniform samplerCube lightDepthMap;
uniform bool castsShadows;
uniform Camera camera;
uniform int pcfDistance;
uniform vec2 viewportSize;

vec3 calcColorUnderPointLight(FragmentInfo fragment, PointLight light, vec3 viewDirection, samplerCube map_shadow, bool computeShadow)
{
    vec3 lightPath = light.position - fragment.position;
    float lightDistance = length(lightPath);

    float shadowFactor = 1.0f;
    if (computeShadow) { shadowFactor = CalcShadowFactor3D(lightPath, viewDirection, light.radius, 0.15f, map_shadow); }
    
    float attenuation = clamp(1.0f - pow(lightDistance / light.radius, 4.0f), 0.0, 1.0);
    float intensity = attenuation * attenuation / (lightDistance * lightDistance + 1.0f);
    intensity = isnan(lightDistance) || light.radius < lightDistance ? 0.0f : intensity;

    return calculateLighting(fragment, viewDirection, lightPath, intensity * light.color.rgb, light.color.a, shadowFactor);
}

void main()
{
    vec2 TexCoord = gl_FragCoord.xy / viewportSize;
    FragmentInfo fragment = getFragmentInfo(TexCoord, albedoTex, normalTex, materialTex, depthTex, camera.invViewProjMatrix);

    float fragDistance = length(camera.position - fragment.position);
    vec3 viewDirection = normalize(camera.position - fragment.position);
    
    PointLight light;
    light.position = pointLight.position;
    light.radius = pointLight.radius;
    light.color = pointLight.color;

    vec3 totalColor = calcColorUnderPointLight(fragment, light, viewDirection, lightDepthMap, castsShadows);

    OutColor = vec4(totalColor, 1.0f);
}