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

uniform sampler2D albedoTex;
uniform sampler2D normalTex;
uniform sampler2D materialTex;
uniform sampler2D depthTex;

struct Camera
{
    mat4 invViewProjMatrix;
    mat4 viewProjMatrix;
    vec3 position;
};
uniform Camera camera;

uniform sampler2D noiseTex;
uniform int sampleCount;
uniform float radius;

const int MAX_SAMPLES = 32;
vec3 kernel[MAX_SAMPLES] = vec3[]
(
   vec3(0.2150, 0.2006, 0.1975   ),
   vec3(0.3214, -0.2144, 0.5133  ),
   vec3(- 0.5527, -0.1417, 0.5493),
   vec3(- 0.4857, 0.2709, 0.2328 ),
   vec3(0.0322, 0.0530, 0.0635   ),
   vec3(-0.0049, 0.0029, 0.0016  ),
   vec3(0.0193, -0.0850, 0.0371  ),
   vec3(0.0273, -0.0143, 0.0257  ),
   vec3(- 0.0246, 0.0094, 0.0103 ),
   vec3(0.0086, -0.0321, 0.0190  ),
   vec3(- 0.0808, 0.0783, 0.0451 ),
   vec3(0.0209, -0.0224, 0.0854  ),
   vec3(- 0.0036, 0.0002, 0.0058 ),
   vec3(- 0.0142, 0.0322, 0.0247 ),
   vec3(0.0314, -0.0193, 0.0291  ),
   vec3(- 0.0002, 0.0001, 0.0002 ),
   vec3(0.1338, 0.0206, 0.1988   ),
   vec3(- 0.1816, -0.0292, 0.0617),
   vec3(- 0.0882, -0.1226, 0.1161),
   vec3(- 0.0368, 0.0016, 0.1073 ),
   vec3(- 0.0065, -0.1033, 0.2065),
   vec3(0.0140, 0.2051, 0.2840   ),
   vec3(0.0587, 0.0604, 0.1500   ),
   vec3(0.1547, -0.1773, 0.0162  ),
   vec3(0.1020, -0.0572, 0.0582  ),
   vec3(- 0.0274, -0.0251, 0.0336),
   vec3(0.0167, -0.0195, 0.0104  ),
   vec3(- 0.0900, -0.4750, 0.2303),
   vec3(0.2311, -0.1284, 0.0191  ),
   vec3(- 0.0383, -0.0705, 0.0705),
   vec3(0.1737, 0.1539, 0.0114   ),
   vec3(0.0623, 0.1574, 0.0438   )
);

float random(vec2 co)
{
    return fract(sin(dot(co, vec2(12.9898, 78.233))) * 43758.5453);
}

mat3 computeTBN(vec3 normal)
{
    vec2 r = vec2(random(TexCoord.xy), random(TexCoord.yx));
    vec3 randomVec = normalize(vec3(2.0 * r - 1.0, 0.0));

    vec3 tangent = cross(randomVec, normal);
    vec3 bitangent = cross(normal, tangent);
    return mat3(tangent, bitangent, normal);
}

void main()
{
    FragmentInfo fragment = getFragmentInfo(TexCoord, albedoTex, normalTex, materialTex, depthTex, camera.invViewProjMatrix);
    mat3 TBN = computeTBN(fragment.normal);
    
    const float sampleDepth = 1.0 / fragment.depth;
    int samples = min(sampleCount, MAX_SAMPLES);
    float totalOcclusion = 0.0;
    for (int i = 0; i < samples; i++)
    {
        vec3 kernelWorldSpace = TBN * kernel[i];
        vec3 sampleVec = fragment.position + kernelWorldSpace * radius;

        vec4 frag = worldToFragSpace(sampleVec, camera.viewProjMatrix);
        float currentDepth = 1.0 / texture(depthTex, frag.xy).r;

        float depthDiff = abs(sampleDepth - currentDepth);
        float bias = 0.01 * sampleDepth;
        float rangeCheck = clamp(1.0 - 25.0 * depthDiff / sampleDepth, 0.0, 1.0);
        float offscreenFading = 1.0 - 4.0 * dot(TexCoord - 0.5, TexCoord - 0.5);
        offscreenFading *= offscreenFading;

        float occlusion = (sampleDepth > currentDepth + bias ? 1.0 : 0.0) * rangeCheck * offscreenFading;

        totalOcclusion += occlusion;
    }
    totalOcclusion = totalOcclusion / float(samples);
    
    OutColor = vec4(totalOcclusion, 0.0, 0.0, 1.0);
}
