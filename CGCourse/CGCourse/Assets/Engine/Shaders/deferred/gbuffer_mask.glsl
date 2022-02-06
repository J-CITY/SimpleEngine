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

in VSout
{
    vec2 TexCoord;
    vec3 Normal;
    vec3 RenderColor;
    mat3 TBN;
    vec3 Position;
} fsin;

layout(location = 0) out vec4 OutAlbedo;
layout(location = 1) out vec4 OutNormal;
layout(location = 2) out vec4 OutMaterial;

struct Material
{
    float emmisive;
    float roughness;
    float metallic;
    float transparency;
};

struct Camera
{
    vec3 position;
    mat4 viewProjMatrix;
    mat4 invViewProjMatrix;
};

uniform sampler2D map_albedo;
uniform sampler2D map_roughness;
uniform sampler2D map_metallic;
uniform sampler2D map_emmisive;
uniform sampler2D map_normal;
uniform sampler2D map_occlusion;
uniform sampler2D map_height;
uniform Material material;
uniform vec2 uvMultipliers;
uniform float displacement;
uniform float gamma;
uniform Camera camera;

vec3 calcNormal(vec2 texcoord, mat3 TBN, sampler2D normalMap)
{
    vec3 normal;
    normal.xy = texture(normalMap, texcoord).rg;
    normal.xy = 2.0 * normal.xy - 1.0;
    normal.z = sqrt(1.0 - dot(normal.xy, normal.xy));
    return TBN * normal;
}

void main()
{
    vec2 TexCoord = uvMultipliers * fsin.TexCoord;
    vec3 viewDirection = fsin.Position - camera.position;
    float parallaxOcclusion = 1.0;
    //TexCoord = applyParallaxMapping(TexCoord, fsin.TBN * viewDirection, map_height, displacement, parallaxOcclusion);

    vec4 albedoAlphaTex = texture(map_albedo, TexCoord).rgba;
    float alphaCutoff = 1.0 - material.transparency;
    if (albedoAlphaTex.a <= alphaCutoff) discard; // mask fragments with opacity less than cutoff

    vec3 normal = calcNormal(TexCoord, fsin.TBN, map_normal);

    vec3 albedoTex = albedoAlphaTex.rgb;
    float occlusion = texture(map_occlusion, TexCoord).r;
    float emmisiveTex = texture(map_emmisive, TexCoord).r;
    float metallicTex = texture(map_metallic, TexCoord).r;
    float roughnessTex = texture(map_roughness, TexCoord).r;

    float emmisive = material.emmisive * emmisiveTex;
    float roughness = material.roughness * roughnessTex;
    float metallic = material.metallic * metallicTex;

    vec3 albedo = pow(fsin.RenderColor * albedoTex, vec3(gamma));

    OutAlbedo = vec4(fsin.RenderColor * albedo, emmisive / (emmisive + 1.0f));
    OutNormal = vec4(0.5f * normal + 0.5f, 1.0f);
    OutMaterial = vec4(parallaxOcclusion * occlusion, roughness, metallic, 1.0f);
}
