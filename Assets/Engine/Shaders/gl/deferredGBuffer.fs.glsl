#version 460 core
#include "../lib/engineUBO.glsl"
#include "../lib/engineInData.glsl"

uniform bool engine_IsPBR = false;


/* Uniforms (Tweakable from the material editor) */

uniform vec4        u_Albedo                  = vec4(1.0, 1.0, 1.0, 1.0);;
uniform float       u_Metallic                = 1.0;
uniform float       u_Roughness               = 1.0;
uniform vec2        u_TextureTiling           = vec2(1.0, 1.0);
uniform vec2        u_TextureOffset           = vec2(0.0, 0.0);
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

/* Global variables */
vec3 g_Normal;
vec2 g_TexCoords;
//vec3 g_ViewDir;
//vec4 g_DiffuseTexel;
//vec4 g_SpecularTexel;
//vec4 g_HeightTexel;
//vec4 g_NormalTexel;

//OUTS
layout (location = 0) out vec4 gPosition;
layout (location = 1) out vec4 gNormal;
layout (location = 2) out vec4 gAlbedoSpec;
layout (location = 3) out vec4 gRoughAO;

vec2 ParallaxMapping(vec3 p_ViewDir) {
    const vec2 parallax = p_ViewDir.xy * u_HeightScale * texture(u_HeightMap, g_TexCoords).r;
    return g_TexCoords - vec2(parallax.x, 1.0 - parallax.y);
}

void main() {
    if (texture(u_AlbedoMap, g_TexCoords).a > 0.5) {
        gPosition = vec4(fs_in.FragPos, 0);
    }

    g_TexCoords = u_TextureOffset + vec2(mod(fs_in.TexCoords.x * u_TextureTiling.x, 1), mod(fs_in.TexCoords.y * u_TextureTiling.y, 1));

    if (u_HeightScale > 0.0) {
        //g_TexCoords = ParallaxMapping(normalize(fs_in.TangentViewPos - fs_in.TangentFragPos));
    }
    // Также храним нормали каждого фрагмента в g-буфере
    if (u_EnableNormalMapping) {
        g_Normal = texture(u_NormalMap, g_TexCoords).rgb;
        g_Normal = normalize(g_Normal * 2.0 - 1.0);
        g_Normal = normalize(fs_in.TBN * g_Normal);
    }
    else {
        g_Normal = normalize(fs_in.Normal);
    }
    gNormal = vec4(g_Normal, 1);
    gRoughAO.b = abs(fs_in.EyeSpacePosition.z / fs_in.EyeSpacePosition.w);
    if (!engine_IsPBR) {
        gAlbedoSpec.rgb = (texture(u_AlbedoMap, g_TexCoords) * u_Albedo).rgb;
        gAlbedoSpec.a = (texture(u_SpecularMap, g_TexCoords) * vec4(u_Specular, 1.0)).r;
        //Shininess
        gRoughAO.r = u_Shininess;
    }
    else {
        vec4 albedoRGBA     = texture(u_AlbedoMap, g_TexCoords) * u_Albedo;
        gAlbedoSpec.rgb = pow(albedoRGBA.rgb, vec3(2.2));
        gAlbedoSpec.a = texture(u_MetallicMap, g_TexCoords).r * u_Metallic;
        gRoughAO.r = texture(u_RoughnessMap, g_TexCoords).r * u_Roughness;
        gRoughAO.g = texture(u_AmbientOcclusionMap, g_TexCoords).r;
    }
}
