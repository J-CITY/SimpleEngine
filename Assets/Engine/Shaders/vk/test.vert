#version 460 core

layout (location = 0) in vec3 geo_Pos;
layout (location = 1) in vec2 geo_TexCoords;
layout (location = 2) in vec3 geo_Normal;
layout (location = 3) in vec3 geo_Tangent;
layout (location = 4) in vec3 geo_Bitangent;
layout (location = 5) in ivec4 boneIds;
layout (location = 6) in vec4 weights;

layout (set = 0, binding = 0) uniform EngineUBO {
    mat4    View;
    mat4    Projection;
    vec3    ViewPos;
    float   Time;
    vec2    ViewportSize;
} engine_UBO;

layout(push_constant) uniform PushModel {
	mat4 model;
} pushModel;

/* Information passed to the fragment shader */
layout(location = 0) out VS_OUT {
    vec3        FragPos;
    vec3        Normal;
    vec2        TexCoords;
    mat3        TBN;
    flat vec3   TangentViewPos;
    vec3        TangentFragPos;

    vec4 FragPosDirLightSpace;
    vec4 FragPosSpotLightSpace;
    
    vec4 EyeSpacePosition;
} vs_out;


void main() {
    vec4 totalPosition = vec4(0.0f);
    vs_out.TBN = mat3(
        normalize(vec3(pushModel.model * vec4(geo_Tangent,   0.0))),
        normalize(vec3(pushModel.model * vec4(geo_Bitangent, 0.0))),
        normalize(vec3(pushModel.model * vec4(geo_Normal,    0.0)))
    );

    mat3 TBNi = transpose(vs_out.TBN);
    vs_out.FragPos          = vec3(pushModel.model * vec4(geo_Pos, 1.0));
    vs_out.EyeSpacePosition =  (engine_UBO.View * pushModel.model) * vec4(geo_Pos, 1.0);
    vs_out.Normal           = normalize(mat3(transpose(inverse(pushModel.model))) * geo_Normal);
    vs_out.TexCoords        = geo_TexCoords;
    vs_out.TangentViewPos   = TBNi * engine_UBO.ViewPos;
    vs_out.TangentFragPos   = TBNi * vs_out.FragPos;
    
    gl_Position = engine_UBO.Projection * (engine_UBO.View * vec4(vs_out.FragPos, 1.0));
}
