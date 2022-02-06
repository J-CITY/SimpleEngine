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

uniform float displacement;
uniform vec2 uvMultipliers;
uniform mat4 parentModel;
uniform mat3 parentNormal;
uniform sampler2D map_height;

out vec2 VertexTexCoord;

void main()
{
    gl_Position = parentModel * vec4(position.xyz, 1.0);
    return;

    VertexTexCoord = texCoord * uvMultipliers;

    vec4 modelPos = parentModel * model * position;
    vec3 normalObjectSpace = parentNormal * normalMatrix * normal;
    modelPos.xyz += normalObjectSpace * getDisplacement(uvMultipliers * texCoord, uvMultipliers, map_height, displacement);
    gl_Position = modelPos;
}

#shader fragment
#version 460 core

in vec4 FragPos;
in vec2 TexCoord;

uniform vec3 lightPos;
uniform float zFar;
uniform sampler2D map_albedo;

void main()
{
    float alpha = texture2D(map_albedo, TexCoord).a;
    //if (alpha < 0.5)
    //    discard;

    float lightDistance = length(FragPos.xyz - lightPos);
    lightDistance = lightDistance / zFar;
    gl_FragDepth = lightDistance;
}

#shader geometry
#version 460 core

layout(triangles) in;
layout(triangle_strip, max_vertices = 18) out;

in vec2 VertexTexCoord[];

out vec4 FragPos;
out vec2 TexCoord;

uniform mat4 LightProjMatrix[6];

void emitFace(mat4 lightMatrix)
{
    for (int triangleIndex = 0; triangleIndex < 3; triangleIndex++)
    {
        FragPos = gl_in[triangleIndex].gl_Position;
        gl_Position = lightMatrix * FragPos;
        EmitVertex();
    }
    EndPrimitive();
}

void main()
{
    TexCoord = VertexTexCoord[0];
    // gl_Layer must be assigned to a constant to work on most devices
    gl_Layer = 0;
    emitFace(LightProjMatrix[0]);

    gl_Layer = 1;
    emitFace(LightProjMatrix[1]);

    gl_Layer = 2;
    emitFace(LightProjMatrix[2]);

    gl_Layer = 3;
    emitFace(LightProjMatrix[3]);

    gl_Layer = 4;
    emitFace(LightProjMatrix[4]);

    gl_Layer = 5;
    emitFace(LightProjMatrix[5]);
}
