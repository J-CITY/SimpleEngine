#version 460 core

/* Uniforms (Tweakable from the material editor) */
layout(set = 1, binding = 0) uniform DataUBO {
    vec4        u_Albedo;
    float       u_Metallic;
    float       u_Roughness;
    vec2        u_TextureTiling;
    vec2        u_TextureOffset;
    vec3        u_Specular;
    float       u_Shininess;
    float       u_HeightScale;
    int        u_EnableNormalMapping;
} dataUBO;

layout(set = 2, binding = 0) uniform sampler2D   u_AlbedoMap;
layout(set = 2, binding = 1) uniform sampler2D   u_SpecularMap;
layout(set = 2, binding = 2) uniform sampler2D   u_NormalMap;
layout(set = 2, binding = 3) uniform sampler2D   u_HeightMap;
layout(set = 2, binding = 4) uniform sampler2D   u_MetallicMap;
layout(set = 2, binding = 5) uniform sampler2D   u_RoughnessMap;
layout(set = 2, binding = 6) uniform sampler2D   u_AmbientOcclusionMap;

/* Information passed from the fragment shader */
layout(location = 0) in VS_OUT {
    vec3        FragPos;
    vec3        Normal;
    vec2        TexCoords;
    mat3        TBN;
    flat vec3   TangentViewPos;
    vec3        TangentFragPos;

    vec4 FragPosDirLightSpace;
    vec4 FragPosSpotLightSpace;

    vec4 EyeSpacePosition;
} fs_in;

/* Global variables */
vec3 g_Normal;
vec2 g_TexCoords;
//vec3 g_ViewDir;
//vec4 g_DiffuseTexel;
//vec4 g_SpecularTexel;
//vec4 g_HeightTexel;
//vec4 g_NormalTexel;
bool engine_IsPBR = true;

//OUTS
layout (location = 0) out vec4 gPosition;
layout (location = 1) out vec4 gNormal;
layout (location = 2) out vec4 gAlbedoSpec;
layout (location = 3) out vec4 gRoughAO;

//vec2 ParallaxMapping(vec3 p_ViewDir) {
//    const vec2 parallax = p_ViewDir.xy * dataUBO.u_HeightScale * texture(u_HeightMap, g_TexCoords).r;
//    return g_TexCoords - vec2(parallax.x, 1.0 - parallax.y);
//}

void main() {
    //gPosition = vec4(texture(u_AlbedoMap, fs_in.TexCoords).rgb, 1);
    //gNormal = vec4(texture(u_AlbedoMap, fs_in.TexCoords).rgb,1);
    //gAlbedoSpec = vec4(texture(u_AlbedoMap, fs_in.TexCoords).rgb,1);
    //gRoughAO = vec4(texture(u_AlbedoMap, fs_in.TexCoords).rgb,1);
    //return;
    gPosition = vec4(fs_in.FragPos, 0);
    g_TexCoords = dataUBO.u_TextureOffset + vec2(mod(fs_in.TexCoords.x * dataUBO.u_TextureTiling.x, 1), mod(fs_in.TexCoords.y * dataUBO.u_TextureTiling.y, 1));
    if (dataUBO.u_HeightScale > 0.0) {
        //g_TexCoords = ParallaxMapping(normalize(fs_in.TangentViewPos - fs_in.TangentFragPos));
    }
    // Также храним нормали каждого фрагмента в g-буфере
    //if (dataUBO.u_EnableNormalMapping > 0) {
        g_Normal = texture(u_NormalMap, g_TexCoords).rgb;
        g_Normal = normalize(g_Normal * 2.0 - 1.0);
        g_Normal = normalize(fs_in.TBN * g_Normal);
    //}
    //else {
    //    g_Normal = normalize(fs_in.Normal);
    //}
    gNormal = vec4(g_Normal, 1);
    gRoughAO.b = abs(fs_in.EyeSpacePosition.z / fs_in.EyeSpacePosition.w);
    if (!engine_IsPBR) {
        gAlbedoSpec.rgb = (texture(u_AlbedoMap, g_TexCoords) * dataUBO.u_Albedo).rgb;
        gAlbedoSpec.a = (texture(u_SpecularMap, g_TexCoords) * vec4(dataUBO.u_Specular, 1.0)).r;
        //Shininess
        gRoughAO.r = dataUBO.u_Shininess;
    }
    else {
        vec4 albedoRGBA     = texture(u_AlbedoMap, g_TexCoords) * dataUBO.u_Albedo;
        gAlbedoSpec.rgb = pow(albedoRGBA.rgb, vec3(2.2));
        gAlbedoSpec.a = texture(u_MetallicMap, g_TexCoords).r * dataUBO.u_Metallic;
        gRoughAO.r = texture(u_RoughnessMap, g_TexCoords).r * dataUBO.u_Roughness;
        gRoughAO.g = texture(u_AmbientOcclusionMap, g_TexCoords).r;
    }
}
