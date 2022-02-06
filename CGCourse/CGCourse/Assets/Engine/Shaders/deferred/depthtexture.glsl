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
layout(location = 5)  in mat4 model;
layout(location = 9)  in mat3 normalMatrix;

uniform mat4 LightProjMatrix;
uniform float displacement;
uniform mat4 parentModel;
uniform mat3 parentNormal;
uniform vec2 uvMultipliers;
uniform sampler2D map_height;

out vec2 TexCoord;

void main()
{
    TexCoord = texCoord * uvMultipliers;

    vec4 modelPos = parentModel * model * position;
    vec3 normalObjectSpace = parentNormal * normalMatrix * normal;
    modelPos.xyz += normalObjectSpace * getDisplacement(TexCoord, uvMultipliers, map_height, displacement);
    gl_Position = LightProjMatrix * modelPos;
}

#shader fragment
#version 460 core

layout(early_fragment_tests) in;

in vec2 TexCoord;

void main()
{

}
