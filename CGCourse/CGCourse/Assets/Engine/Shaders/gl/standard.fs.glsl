#version 460 core

#include "../lib/engineUBO.glsl"
#include "../lib/engineInData.glsl"
#include "../lib/engineLightSSBO.glsl"


/* Uniforms (Tweakable from the material editor) */
uniform vec2        u_TextureTiling           = vec2(1.0, 1.0);
uniform vec2        u_TextureOffset           = vec2(0.0, 0.0);
uniform vec4        u_Diffuse                 = vec4(1.0, 1.0, 1.0, 1.0);
uniform vec3        u_Specular                = vec3(1.0, 1.0, 1.0);
uniform float       u_Shininess               = 100.0;
uniform float       u_HeightScale             = 0.0;
uniform bool        u_EnableNormalMapping     = false;


uniform sampler2D   u_DiffuseMap;
uniform sampler2D   u_SpecularMap;
uniform sampler2D   u_NormalMap;
uniform sampler2D   u_HeightMap;

//Shadow Dir
//uniform sampler2DArray engine_dirShadowMap;
//uniform sampler2DArray engine_dirBakedShadowMap;
//uniform sampler2D engine_dirBakedShadowMap;



/* Global variables */
vec3 g_Normal;
vec3 g_FragPos;
vec2 g_TexCoords;
vec3 g_ViewDir;
vec4 g_DiffuseTexel;
vec4 g_SpecularTexel;
vec4 g_HeightTexel;
vec4 g_NormalTexel;
float g_Shininess;

#include "../lib/shadows.glsl"
#include "../lib/blinnPhongLight.glsl"

out vec4 FRAGMENT_COLOR;

vec2 ParallaxMapping(vec3 p_ViewDir) {
    const vec2 parallax = p_ViewDir.xy * u_HeightScale * texture(u_HeightMap, g_TexCoords).r;
    return g_TexCoords - vec2(parallax.x, 1.0 - parallax.y);
}

void main() {
    g_TexCoords = u_TextureOffset + vec2(mod(fs_in.TexCoords.x * u_TextureTiling.x, 1), mod(fs_in.TexCoords.y * u_TextureTiling.y, 1));

    /* Apply parallax mapping */
    if (u_HeightScale > 0)
        g_TexCoords = ParallaxMapping(normalize(fs_in.TangentViewPos - fs_in.TangentFragPos));

    /* Apply color mask */
    g_ViewDir           = normalize(engine_UBO.ViewPos - fs_in.FragPos);
    g_DiffuseTexel      = texture(u_DiffuseMap,  g_TexCoords) * u_Diffuse;
    g_SpecularTexel     = texture(u_SpecularMap, g_TexCoords) * vec4(u_Specular, 1.0);
    if (u_EnableNormalMapping) {
        g_Normal = texture(u_NormalMap, g_TexCoords).rgb;
        g_Normal = normalize(g_Normal * 2.0 - 1.0);
        g_Normal = normalize(fs_in.TBN * g_Normal);
    }
    else {
        g_Normal = normalize(fs_in.Normal);
    }

    g_Shininess = u_Shininess;

    g_FragPos = fs_in.FragPos;

    vec3 a = vec3(100, 0, 0);

    int pointShadowMapId = 0;
    vec3 lightSum = vec3(0.0);
    for (int i = 0; i < ssbo_Lights.length(); ++i) {
        switch(ssbo_Lights[i].type) {
            case 0: {
                float shadow = 0.0f;
                lightSum += CalcPointLight(ssbo_Lights[i], shadow);
                break;
            }
            case 1: {
                float shadow = 0.0f;
                if (engine_ShadowUBO.useDirLightShadow) {
                    shadow = DirShadowCalculation(fs_in.FragPosDirLightSpace);
                    //shadow = ShadowCalculationWithCascade(fs_in.FragPos);
                }
                lightSum += CalcDirectionalLight(ssbo_Lights[i], shadow);
                break;
            }
            case 2: {
                float shadow = 0.0f;
                lightSum += CalcSpotLight(ssbo_Lights[i], shadow);
                break;
            }
            case 3: lightSum += CalcAmbientBoxLight(ssbo_Lights[i], false, 0.0f);    break;
            case 4: lightSum += CalcAmbientSphereLight(ssbo_Lights[i], false, 0.0f); break;
        }
    }
    FRAGMENT_COLOR = vec4(lightSum /** vol*/, g_DiffuseTexel.a);
    //FRAGMENT_COLOR = vec4(texture(engine_dirShadowMap, vec3(g_TexCoords, 0)).rrr, 1);
}
