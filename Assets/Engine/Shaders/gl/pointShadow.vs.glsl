#version 460 core

layout (location = 0) in vec3 geo_Pos;
layout (location = 1) in vec2 geo_TexCoords;
layout (location = 2) in vec3 geo_Normal;
layout (location = 3) in vec3 geo_Tangent;
layout (location = 4) in vec3 geo_Bitangent;
layout(location = 5) in ivec4 boneIds;
layout(location = 6) in vec4 weights;

layout (std140, binding = 0) uniform EngineUBO {
    mat4    View;
    mat4    Projection;
    vec3    ViewPos;
    float   Time;
    vec2    ViewportSize;
} engine_UBO;

struct PushModel {
    mat4 model;
};
uniform PushModel engine_Model;

const int MAX_BONES = 100;
const int MAX_BONE_INFLUENCE = 4;

uniform mat4 u_engine_FinalBonesMatrices[MAX_BONES];
uniform bool u_UseBone = false;

void main() {
    vec4 totalPosition = vec4(0.0f);
    if (u_UseBone) {
        for(int i = 0; i < MAX_BONE_INFLUENCE; i++) {
            if(int(boneIds[i]) == -1) 
                continue;
            if(int(boneIds[i]) >=MAX_BONES) {
                totalPosition = vec4(geo_Pos,1.0f);
                break;
            }
            vec4 localPosition = (u_engine_FinalBonesMatrices[int(boneIds[i])]) * vec4(geo_Pos,1.0f);
            totalPosition += localPosition * weights[i];
        }
    }

    if (u_UseBone) {
        gl_Position = engine_Model.model * totalPosition;
    }
    else {
        gl_Position = engine_Model.model * vec4(geo_Pos, 1.0);
    }
}