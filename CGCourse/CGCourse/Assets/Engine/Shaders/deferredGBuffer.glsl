#shader vertex
#version 460 core

layout (location = 0) in vec3 geo_Pos;
layout (location = 1) in vec2 geo_TexCoords;
layout (location = 2) in vec3 geo_Normal;
layout (location = 3) in vec3 geo_Tangent;
layout (location = 4) in vec3 geo_Bitangent;
layout (location = 5) in ivec4 boneIds;
layout (location = 6) in vec4 weights;

const int MAX_BONES = 100;
const int MAX_BONE_INFLUENCE = 4;
uniform mat4 u_engine_FinalBonesMatrices[MAX_BONES];
uniform bool u_UseBone;
uniform mat4 u_engine_LightSpaceMatrix;

/* Global information sent by the engine */

layout (std140) uniform EngineUBO
{
    mat4    ubo_Model;
    mat4    ubo_View;
    mat4    ubo_Projection;
    vec3    ubo_ViewPos;
    float   ubo_Time;
    vec2    ubo_ViewportSize;
};

/* Information passed to the fragment shader */
out VS_OUT
{
    vec3        FragPos;
    vec3        Normal;
    vec2        TexCoords;
    mat3        TBN;
    flat vec3   TangentViewPos;
    vec3        TangentFragPos;

    vec4 EyeSpacePosition;
} vs_out;

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
    vs_out.TBN = mat3(
        normalize(vec3(ubo_Model * vec4(geo_Tangent,   0.0))),
        normalize(vec3(ubo_Model * vec4(geo_Bitangent, 0.0))),
        normalize(vec3(ubo_Model * vec4(geo_Normal,    0.0)))
    );

    mat3 TBNi = transpose(vs_out.TBN);

    if (u_UseBone) {
        vs_out.FragPos          = vec3(ubo_Model * totalPosition);
        vs_out.EyeSpacePosition =  (ubo_View * ubo_Model) * totalPosition;
    }
    else {
        vs_out.FragPos          = vec3(ubo_Model * vec4(geo_Pos, 1.0));
        vs_out.EyeSpacePosition =  (ubo_View * ubo_Model) * vec4(geo_Pos, 1.0);
    }
    vs_out.Normal           = normalize(mat3(transpose(inverse(ubo_Model))) * geo_Normal);
    vs_out.TexCoords        = geo_TexCoords;
    vs_out.TangentViewPos   = TBNi * ubo_ViewPos;
    vs_out.TangentFragPos   = TBNi * vs_out.FragPos;
    
    gl_Position = ubo_Projection * (ubo_View * vec4(vs_out.FragPos, 1.0));
}

#shader fragment
#version 460 core

/* Global information sent by the engine */
layout (std140) uniform EngineUBO
{
    mat4    ubo_Model;
    mat4    ubo_View;
    mat4    ubo_Projection;
    vec3    ubo_ViewPos;
    float   ubo_Time;
    vec2    ubo_ViewportSize;
};

/* Information passed from the fragment shader */
in VS_OUT
{
    vec3        FragPos;
    vec3        Normal;
    vec2        TexCoords;
    mat3        TBN;
    flat vec3   TangentViewPos;
    vec3        TangentFragPos;
    vec4 EyeSpacePosition;
} fs_in;


/* Uniforms (Tweakable from the material editor) */
uniform vec2        u_TextureTiling           = vec2(1.0, 1.0);
uniform vec2        u_TextureOffset           = vec2(0.0, 0.0);
uniform vec4        u_Albedo                  = vec4(1.0, 1.0, 1.0, 1.0);
uniform vec3        u_Specular                = vec3(1.0, 1.0, 1.0);
uniform float       u_Shininess               = 100.0;
uniform float       u_HeightScale             = 0.0;
uniform bool        u_EnableNormalMapping     = false;
uniform sampler2D   u_AlbedoMap;
uniform sampler2D   u_SpecularMap;
uniform sampler2D   u_NormalMap;
uniform sampler2D   u_HeightMap;
uniform sampler2D   u_MetallicMap;
uniform sampler2D   u_RoughnessMap;
uniform sampler2D   u_AmbientOcclusionMap;

uniform float       u_Metallic              = 1.0;
uniform float       u_Roughness             = 1.0;

/* Global variables */
vec3 g_Normal;
vec2 g_TexCoords;
vec3 g_ViewDir;
vec4 g_DiffuseTexel;
vec4 g_SpecularTexel;
vec4 g_HeightTexel;
vec4 g_NormalTexel;

//OUTS
layout (location = 0) out vec3 gPosition;
layout (location = 1) out vec3 gNormal;
layout (location = 2) out vec4 gAlbedoSpec;
layout (location = 3) out vec4 gRoughAO;

uniform bool u_engine_IsPBR = false;

vec2 ParallaxMapping(vec3 p_ViewDir) {
    const vec2 parallax = p_ViewDir.xy * u_HeightScale * texture(u_HeightMap, g_TexCoords).r;
    return g_TexCoords - vec2(parallax.x, 1.0 - parallax.y);
}

void main() {
    gPosition = fs_in.FragPos;

    vec2 g_TexCoords = u_TextureOffset + vec2(mod(fs_in.TexCoords.x * u_TextureTiling.x, 1), mod(fs_in.TexCoords.y * u_TextureTiling.y, 1));

    if (u_HeightScale > 0)
        g_TexCoords = ParallaxMapping(normalize(fs_in.TangentViewPos - fs_in.TangentFragPos));

    // Также храним нормали каждого фрагмента в g-буфере
    if (u_EnableNormalMapping) {
        g_Normal = texture(u_NormalMap, g_TexCoords).rgb;
        g_Normal = normalize(g_Normal * 2.0 - 1.0);
        g_Normal = normalize(fs_in.TBN * g_Normal);
    }
    else {
      g_Normal = normalize(fs_in.Normal);
    }
    gNormal = normalize(g_Normal);
    gRoughAO.b = abs(fs_in.EyeSpacePosition.z / fs_in.EyeSpacePosition.w);
    if (!u_engine_IsPBR) {
        gAlbedoSpec.rgb = (texture(u_AlbedoMap, g_TexCoords) * u_Albedo).rgb;
        gAlbedoSpec.a = (texture(u_SpecularMap, g_TexCoords) * vec4(u_Specular, 1.0)).r;
    }
    else {
        vec4 albedoRGBA     = texture(u_AlbedoMap, g_TexCoords) * u_Albedo;
        gAlbedoSpec.rgb = pow(albedoRGBA.rgb, vec3(2.2));
        gAlbedoSpec.a = texture(u_MetallicMap, g_TexCoords).r * u_Metallic;
        gRoughAO.r = texture(u_RoughnessMap, g_TexCoords).r * u_Roughness;
        gRoughAO.g = texture(u_AmbientOcclusionMap, g_TexCoords).r;
    }
}
