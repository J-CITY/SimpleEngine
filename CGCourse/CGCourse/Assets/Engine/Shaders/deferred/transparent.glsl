#shader vertex
#version 460 core

float getDisplacement(vec2 texCoord, vec2 uvMultiplier, sampler2D heightMap, float displacementFactor)
{
    vec2 normTexCoord = texCoord / uvMultiplier;
    if (normTexCoord.x < 0.001f || normTexCoord.y < 0.001f ||
        normTexCoord.x > 0.999f || normTexCoord.y > 0.999f)
        return 0.0f;

    float heightTex = texture(heightMap, texCoord).r;
    return displacementFactor * heightTex;
}

vec2 applyParallaxMapping(vec2 texCoords, vec3 viewDirectionNormalSpace, sampler2D heightMap, float heightScale, out float occlusion)
{
    const float numLayers = 16.0;
    float layerDepth = 1.0 / numLayers;
    float currentLayerDepth = 0.0;
    vec2 P = viewDirectionNormalSpace.xy / viewDirectionNormalSpace.z * heightScale;
    vec2 deltaTexCoords = P / numLayers;

    vec2  currentTexCoords = texCoords;
    float currentDepthMapValue = texture2D(heightMap, currentTexCoords).r;

    while (currentLayerDepth < currentDepthMapValue && currentLayerDepth < 1.0)
    {
        currentTexCoords += deltaTexCoords;
        currentDepthMapValue = texture2D(heightMap, currentTexCoords).r;
        currentLayerDepth += layerDepth;
    }

    vec2 prevTexCoords = currentTexCoords + deltaTexCoords;

    float afterDepth = currentDepthMapValue - currentLayerDepth;
    float beforeDepth = texture(heightMap, prevTexCoords).r - currentLayerDepth + layerDepth;

    float weight = afterDepth / (afterDepth - beforeDepth);
    vec2 finalTexCoords = mix(currentTexCoords, prevTexCoords, weight);
    occlusion = currentLayerDepth == 0.0 ? 1.0 : currentLayerDepth;

    return finalTexCoords;
}

layout(location = 0)  in vec4 position;
layout(location = 1)  in vec2 texCoord;
layout(location = 2)  in vec3 normal;
layout(location = 3)  in vec3 tangent;
layout(location = 4)  in vec3 bitangent;
layout(location = 5)  in mat4 model;
layout(location = 9)  in mat3 normalMatrix;
layout(location = 12) in vec3 renderColor;

struct Camera
{
    vec3 position;
    mat4 viewProjMatrix;
    mat4 invViewProjMatrix;
};

uniform Camera camera;
uniform float displacement;
uniform mat4 parentModel;
uniform mat3 parentNormal;
uniform vec3 parentColor;
uniform vec2 uvMultipliers;
uniform sampler2D map_height;

out VSout
{
    vec2 TexCoord;
    vec3 Normal;
    vec3 RenderColor;
    mat3 TBN;
    vec3 Position;
} vsout;

void main()
{
    vec4 modelPos = parentModel * model * position;
    mat3 normalSpaceMatrix = parentNormal * normalMatrix;

    vec3 T = normalize(vec3(normalSpaceMatrix * tangent));
    vec3 B = normalize(vec3(normalSpaceMatrix * bitangent));
    vec3 N = normalize(vec3(normalSpaceMatrix * normal));

    vsout.TBN = mat3(T, B, N);
    vsout.Normal = N;
    vsout.RenderColor = parentColor * renderColor;

    float displacementFactor = getDisplacement(uvMultipliers * texCoord, uvMultipliers, map_height, displacement);

    modelPos.xyz += vsout.Normal * displacementFactor;
    vsout.Position = modelPos.xyz;

    vec3 viewDirection = camera.position - vsout.Position;
    vsout.TexCoord = texCoord;

    gl_Position = camera.viewProjMatrix * modelPos;
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

vec3 calculateIBL(FragmentInfo fragment, vec3 viewDirection, EnvironmentInfo environment, float gamma)
{
    float roughness = clamp(fragment.roughnessFactor, 0.05, 0.95);
    float metallic = clamp(fragment.metallicFactor, 0.05, 0.95);

    float NV = clamp(dot(fragment.normal, viewDirection), 0.0, 0.999);
    
    float lod = log2(textureSize(environment.skybox, 0).x * roughness * roughness);
    vec3 F0 = mix(vec3(0.04f), fragment.albedo, metallic);
    vec3 F = fresnelSchlickRoughness(F0, NV, roughness);
    
    vec3 prefilteredColor = calcReflectionColor(environment.skybox, environment.skyboxRotation, viewDirection, fragment.normal, lod);
    prefilteredColor = pow(prefilteredColor, vec3(gamma));
    vec2 envBRDF = texture2D(environment.envBRDFLUT, vec2(NV, 1.0 - roughness)).rg;
    vec3 specularColor = prefilteredColor * (F * envBRDF.x + envBRDF.y);

    vec3 irradianceColor = calcReflectionColor(environment.irradiance, environment.skyboxRotation, viewDirection, fragment.normal);
    irradianceColor = pow(irradianceColor, vec3(gamma));
    
    float diffuseCoef = 1.0f - metallic;
    vec3 diffuseColor = fragment.albedo * (irradianceColor - irradianceColor * envBRDF.y) * diffuseCoef;
    vec3 iblColor = (diffuseColor + specularColor) * environment.intensity;

    return fragment.emmisionFactor * fragment.albedo + iblColor * fragment.ambientOcclusion;
}

const int DirLightCascadeMapCount = 3;
const int MaxDirLightCount = 4;

struct DirLight
{
    mat4 transform[DirLightCascadeMapCount];
    vec4 color;
    vec3 direction;
};

float calcShadowFactorCascade(vec4 position, DirLight light, sampler2D shadowMap)
{
    vec3 projectedPositions[DirLightCascadeMapCount];
    float shadowFactors[DirLightCascadeMapCount + 1];
    shadowFactors[DirLightCascadeMapCount] = 1.0;

    const vec2 textureSplitSize = vec2(1.01, 0.99) / DirLightCascadeMapCount;
    for (int i = 0; i < DirLightCascadeMapCount; i++)
    {
        vec4 textureLimitsXY = vec4(vec2(i, i + 1) * textureSplitSize, 0.001, 0.999);
        vec4 fragLightSpace = light.transform[i] * position;
        projectedPositions[i] = fragLightSpace.xyz / fragLightSpace.w;
        shadowFactors[i] = calcShadowFactor2D(projectedPositions[i], shadowMap, textureLimitsXY, 0.002);
    }

    float totalFactor = 1.0f;
    bool cascadeFound = false;
    for (int i = 0; i < DirLightCascadeMapCount; i++)
    {
        vec3 pos = projectedPositions[i];
        vec3 normCoords = abs(2.0 * pos - 1.0);
        normCoords = clamp(10.0 * normCoords - 9.0, 0.0, 1.0);
        float mixCoef = max(max(normCoords.x, normCoords.y), normCoords.z);
        float currentFactor = mix(shadowFactors[i], shadowFactors[i + 1], mixCoef);

        currentFactor = min(currentFactor, 1.0f);
        totalFactor = totalFactor * currentFactor;
    }

    return totalFactor;
}

out vec4 OutColor;

in VSout
{
    vec2 TexCoord;
    vec3 Normal;
    vec3 RenderColor;
    mat3 TBN;
    vec3 Position;
} fsin;

struct Material
{
    float emmisive;
    float roughness;
    float metallic;
    float transparency;
};

uniform sampler2D map_albedo;
uniform sampler2D map_metallic;
uniform sampler2D map_roughness;
uniform sampler2D map_emmisive;
uniform sampler2D map_normal;
uniform sampler2D map_transparency;
uniform sampler2D map_occlusion;
uniform Material material;
uniform vec2 uvMultipliers;
uniform float gamma;

struct Camera
{
    vec3 position;
    mat4 invProjMatrix;
    mat4 invViewMatrix;
    mat4 viewProjMatrix;
};

uniform int lightCount;
uniform vec3 viewportPosition;
uniform sampler2D envBRDFLUT;

uniform EnvironmentInfo environment;

const int MaxLightCount = 4;
uniform DirLight lights[MaxLightCount];
uniform sampler2D lightDepthMaps[MaxLightCount];

vec3 calcNormal(vec2 texcoord, mat3 TBN, sampler2D normalMap)
{
    vec3 normal = texture(normalMap, texcoord).rgb;
    normal = normalize(normal * 2.0f - 1.0f);
    return TBN * normal;
}

void main()
{
    vec2 TexCoord = uvMultipliers * fsin.TexCoord;
    vec4 albedoAlphaTex = texture(map_albedo, TexCoord).rgba;

    FragmentInfo fragment;
    fragment.albedo = pow(fsin.RenderColor * albedoAlphaTex.rgb, vec3(gamma));
    fragment.ambientOcclusion = texture(map_occlusion, TexCoord).r;
    fragment.roughnessFactor = material.roughness * texture(map_roughness, TexCoord).r;
    fragment.metallicFactor = material.metallic * texture(map_metallic, TexCoord).r;
    fragment.emmisionFactor = material.emmisive * texture(map_emmisive, TexCoord).r;
    fragment.depth = gl_FragCoord.z;
    fragment.normal = calcNormal(TexCoord, fsin.TBN, map_normal);
    fragment.position = fsin.Position;
    
    float transparency = material.transparency * albedoAlphaTex.a;
    vec3 viewDirection = normalize(viewportPosition - fragment.position);
    
    vec3 IBLColor = calculateIBL(fragment, viewDirection, environment, gamma);

    vec3 totalColor = IBLColor;
    for (int i = 0; i < lightCount; i++)
    {
        vec4 pos = vec4(fragment.position, 1.0f);
        float shadowFactor = calcShadowFactorCascade(pos, lights[i], lightDepthMaps[i]);
        totalColor += calculateLighting(fragment, viewDirection, lights[i].direction, lights[i].color.rgb, lights[i].color.a, shadowFactor);
    }

    OutColor = vec4(totalColor, transparency);
}
