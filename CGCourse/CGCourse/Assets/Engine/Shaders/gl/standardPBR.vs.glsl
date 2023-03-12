#version 460 core

layout (location = 0) in vec3 geo_Pos;
layout (location = 1) in vec2 geo_TexCoords;
layout (location = 2) in vec3 geo_Normal;
layout (location = 3) in vec3 geo_Tangent;
layout (location = 4) in vec3 geo_Bitangent;
layout (location = 5) in ivec4 boneIds;
layout (location = 6) in vec4 weights;


#include "../lib/engineUBO.glsl"
#include "../lib/shadows.glsl"
#include "../lib/engineOutData.glsl"

const int MAX_BONES = 100;
const int MAX_BONE_INFLUENCE = 4;
uniform mat4 u_engine_FinalBonesMatrices[MAX_BONES];
uniform bool u_UseBone;

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
    vs_out.TBN = mat3 (
        normalize(vec3(engine_Model.model * vec4(geo_Tangent,   0.0))),
        normalize(vec3(engine_Model.model * vec4(geo_Bitangent, 0.0))),
        normalize(vec3(engine_Model.model * vec4(geo_Normal,    0.0)))
    );

    mat3 TBNi = transpose(vs_out.TBN);

    vec3 PrevFragPos;
    if (u_UseBone) {
        vs_out.FragPos          = vec3(engine_Model.model * totalPosition);
        PrevFragPos          = vec3(engine_TempData.previousModel  * vec4(totalPosition.xyz, 1.0));
        vs_out.EyeSpacePosition =  (engine_UBO.View * engine_Model.model) * totalPosition;
    }
    else {
        vs_out.FragPos          = vec3(engine_Model.model * vec4(geo_Pos, 1.0));
        PrevFragPos          = vec3(engine_TempData.previousModel  * vec4(geo_Pos, 1.0));
        vs_out.EyeSpacePosition =  (engine_UBO.View * engine_Model.model) * vec4(geo_Pos, 1.0);
    }
    vs_out.Normal           = normalize(mat3(transpose(inverse(engine_Model.model))) * geo_Normal);
    vs_out.TexCoords        = geo_TexCoords;
    vs_out.TangentViewPos   = TBNi * engine_UBO.ViewPos;
    vs_out.TangentFragPos   = TBNi * vs_out.FragPos;
    vs_out.FragPosDirLightSpace = engine_ShadowDataUBO.dirLightSpaceMatrix * vec4(vs_out.FragPos, 1.0);
    vs_out.FragPosSpotLightSpace = engine_ShadowDataUBO.spotLightSpaceMatrix * vec4(vs_out.FragPos, 1.0);
    
    gl_Position = engine_UBO.Projection * (engine_UBO.View * vec4(vs_out.FragPos, 1.0));
}
