#shader vertex
#version 460 core

layout (location = 0) in vec3 geo_Pos;
layout (location = 1) in vec2 geo_TexCoords;
layout (location = 2) in vec3 geo_Normal;
layout (location = 3) in vec3 geo_Tangent;
layout (location = 4) in vec3 geo_Bitangent;

layout(location = 5) in ivec4 boneIds;
layout(location = 6) in vec4 weights;

layout (std140) uniform EngineUBO
{
    mat4    ubo_Model;
    mat4    ubo_View;
    mat4    ubo_Projection;
    vec3    ubo_ViewPos;
    float   ubo_Time;
};

const int MAX_BONES = 100;
const int MAX_BONE_INFLUENCE = 4;
uniform mat4 finalBonesMatrices[MAX_BONES];
uniform bool useBone;
uniform mat4 lightSpaceMatrix;
uniform mat4 model;

void main()
{
    vec4 totalPosition = vec4(0.0f);
    if (useBone) {
        for(int i = 0; i < MAX_BONE_INFLUENCE; i++) {
            if(int(boneIds[i]) == -1) 
                continue;
            if(int(boneIds[i]) >=MAX_BONES) {
                totalPosition = vec4(geo_Pos,1.0f);
                break;
            }
            vec4 localPosition = (finalBonesMatrices[int(boneIds[i])]) * vec4(geo_Pos,1.0f);
            totalPosition += localPosition * weights[i];
        }
    }

    if (useBone) {
        gl_Position = lightSpaceMatrix * model * totalPosition;
    }
    else {
        gl_Position = lightSpaceMatrix * model * vec4(geo_Pos, 1.0);
    }
}

#shader fragment
#version 460 core

void main()
{             
    // gl_FragDepth = gl_FragCoord.z;
}