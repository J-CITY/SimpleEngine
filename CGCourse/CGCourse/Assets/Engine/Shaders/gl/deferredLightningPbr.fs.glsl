#version 460 core

#include "../lib/engineUBO.glsl"
#include "../lib/engineLightSSBO.glsl"

in vec2 TexCoords;

uniform sampler2D   u_PositionMap;
uniform sampler2D   u_NormalMap;
uniform sampler2D   u_AlbedoSpecMap;
uniform sampler2D   u_RoughAO;
////ssao
//uniform sampler2D   u_engine_SSAO;
//uniform bool        u_UseSSAO;
////ibl
//layout(binding = 14)
uniform samplerCube u_engine_irradianceMap;
//layout(binding = 15)
uniform samplerCube u_engine_prefilterMap;
//layout(binding = 16)
uniform sampler2D   u_engine_brdfLUT;
uniform bool        u_UseIbl;

////dir light
//uniform sampler2D u_engine_ShadowMap;
//uniform vec3 u_engine_LightPos;
//uniform bool u_engine_UseDirShadow;
//uniform mat4 u_engine_LightSpaceMatrix;
//
////point lights
//const int MAX_POINTS_LIGHT_SHADOW = 4;
//uniform int u_engine_PointLightsSize;
//uniform vec3 u_engine_PointLightsPos[MAX_POINTS_LIGHT_SHADOW];
//uniform float u_engine_FarPlane;
//uniform samplerCube u_engine_PointLightsCubeMap[MAX_POINTS_LIGHT_SHADOW];
//uniform bool u_engine_UsePointShadow;
//
////spot light
//uniform sampler2D u_engine_SpotLightShadowMap;
//uniform float u_engine_SpotFarPlane;
//uniform float u_engine_SpotNearPlane;
//uniform vec3 u_engine_SpotLightPos;
//uniform bool u_engine_UseSpotShadow;
//uniform mat4 u_engine_SpotLightSpaceMatrix;

struct DATA {
    vec3        FragPos;
    vec3        Normal;
};
DATA fs_in;

/* Global variables */
vec3 g_Normal;
vec2 g_TexCoords;
vec3 g_ViewDir;
vec3 g_DiffuseTexel;
vec3 g_FragPos;
float g_AmbientOcclusion;

#include "../lib/pbr.glsl"

//#include "lib/fog.glsl"
#include "../lib/shadows.glsl"

uniform bool engine_UseSSAO = true;
//layout(binding = 13)
uniform sampler2D engine_SSAOMap;

//uniform FogParameters u_engine_FogParams;

out vec4 FRAGMENT_COLOR;

void main() {
    g_TexCoords         = TexCoords;
    g_FragPos           = texture(u_PositionMap, TexCoords).rgb;
    g_ViewDir           = normalize(engine_UBO.ViewPos - g_FragPos);
    g_DiffuseTexel      = texture(u_AlbedoSpecMap,  TexCoords).rgb;
    float metallic      = texture(u_AlbedoSpecMap, TexCoords).a;
    float roughness     = texture(u_RoughAO, TexCoords).r;
    float ao            = texture(u_RoughAO, TexCoords).g;
    g_Normal            = texture(u_NormalMap, TexCoords).rgb;
    if (engine_UseSSAO) {
        g_AmbientOcclusion  = texture(engine_SSAOMap, TexCoords).r;
    }
    float EyeSpacePosition = texture(u_RoughAO, TexCoords).b;
    fs_in.FragPos = g_FragPos;
    fs_in.Normal = g_Normal;
    vec3 albedo = g_DiffuseTexel;
    vec3 N = normalize(g_Normal);
    vec3 V = normalize(engine_UBO.ViewPos - fs_in.FragPos);

    vec4 FragPosSpotLightSpace = engine_ShadowUBO.spotLightSpaceMatrix * vec4(fs_in.FragPos, 1.0);
    vec4 FragPosDirLightSpace = engine_ShadowUBO.dirLightSpaceMatrix * vec4(fs_in.FragPos, 1.0);

    vec3 F0 = vec3(0.04); 
    F0 = mix(F0, albedo, metallic);

    ////ibl
    vec3 ambientIBL = vec3(0.0f);
    if (u_UseIbl) {
        vec3 R = reflect(-V, N); 
        vec3 irradiance = texture(u_engine_irradianceMap, N).rgb;
        vec3 diffuse = irradiance * albedo;
        vec3 F = fresnelSchlickRoughness(max(dot(N, V), 0.0), F0, roughness);
        vec3 kS = F;
        vec3 kD = 1.0 - kS;
        kD *= 1.0 - metallic;	
        // Производим выборки из префильтрованной карты LUT-текстуры BRDF и затем объединяем их вместе в соответствии с аппроксимацией разделенной суммы, чтобы получить зеркальную часть IBL
        const float MAX_REFLECTION_LOD = 4.0;
        vec3 prefilteredColor = textureLod(u_engine_prefilterMap, R,  roughness * MAX_REFLECTION_LOD).rgb;    
        vec2 brdf  = texture(u_engine_brdfLUT, vec2(max(dot(N, V), 0.0), roughness)).rg;
        vec3 specular = prefilteredColor * (F * brdf.x + brdf.y);
        ambientIBL = (kD * diffuse + specular) * ao;
    }
    ////ibl end
    
    // reflectance equation
    vec3 Lo = vec3(0.0);
    vec3 ambientSum = vec3(0.0);
    
    int pointShadowMapId = 0;
    for (int i = 0; i < ssbo_Lights.length(); ++i)  {
        if (ssbo_Lights[i].type == 3) {
            ambientSum += CalcAmbientBoxLight(ssbo_Lights[i], engine_UseSSAO, g_AmbientOcclusion);
        }
        else if (ssbo_Lights[i].type == 4) {
            ambientSum += CalcAmbientSphereLight(ssbo_Lights[i], engine_UseSSAO, g_AmbientOcclusion);
        }
        else {
            // calculate per-light radiance
            vec3 L = ssbo_Lights[i].constant == 1 ? -toVec3(ssbo_Lights[i].forward) : normalize(toVec3(ssbo_Lights[i].pos) - fs_in.FragPos);
            vec3 H = normalize(V + L);
            float distance    = length(toVec3(ssbo_Lights[i].pos)- fs_in.FragPos);
            float lightCoeff = 0.0;

            float shadow = 0.0f;
            switch(ssbo_Lights[i].type) {
                case 0:
                    //if (u_engine_UsePointShadow && pointShadowMapId < MAX_POINTS_LIGHT_SHADOW) {
                    //    shadow = PointShadowCalculation(fs_in.FragPos, pointShadowMapId);
                    //    pointShadowMapId++;
                    //}
                    lightCoeff = LuminosityFromAttenuation(ssbo_Lights[i]) * ssbo_Lights[i].intensity;
                    break;

                case 1:
                    if (engine_ShadowUBO.useDirLightShadow) {
                        shadow = DirShadowCalculation(FragPosDirLightSpace);
                    }
                    lightCoeff = ssbo_Lights[i].intensity;
                    break;

                case 2:
                    //if (u_engine_UseSpotShadow) {
                    //    shadow = SpotShadowCalculation(FragPosSpotLightSpace);
                    //}
                    const vec3  lightForward    = toVec3(ssbo_Lights[i].forward);
                    const float cutOff          = cos(radians(ssbo_Lights[i].cutoff));
                    const float outerCutOff     = cos(radians(ssbo_Lights[i].cutoff + ssbo_Lights[i].outerCutoff));

                    const vec3  lightDirection  = normalize(toVec3(ssbo_Lights[i].pos) - fs_in.FragPos);
                    const float luminosity      = LuminosityFromAttenuation(ssbo_Lights[i]);

                    /* Calculate the spot intensity */
                    const float theta           = dot(lightDirection, normalize(-lightForward)); 
                    const float epsilon         = cutOff - outerCutOff;
                    const float spotIntensity   = clamp((theta - outerCutOff) / epsilon, 0.0, 1.0);

                    lightCoeff = luminosity * spotIntensity * ssbo_Lights[i].intensity;
                    break;
            }

            vec3 radiance = toVec3(ssbo_Lights[i].color) * lightCoeff;
            
            // cook-torrance brdf
            float NDF = DistributionGGX(N, H, roughness);
            float G   = GeometrySmith(N, V, L, roughness);
            vec3 F    = fresnelSchlick(max(dot(H, V), 0.0), F0);
            
            vec3 kS = F;
            vec3 kD = vec3(1.0) - kS;
            kD *= 1.0 - metallic;
            
            vec3 numerator    = NDF * G * F;
            float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0);
            vec3 specular     = numerator / max(denominator, 0.001);  
                
            // add to outgoing radiance Lo
            float NdotL = max(dot(N, L), 0.0);
            vec3 resLight = (kD * albedo / PI + specular) * radiance * NdotL;
            resLight *= (1.0 - shadow);
            Lo += resLight; 
        }
    }

    vec3 ambient = ambientSum * albedo * ao;
    vec3 color = vec3(0.0f);
    if (u_UseIbl) {
        if (engine_UseSSAO) {
            ambientIBL *= g_AmbientOcclusion;
        }
        color = ambientIBL + Lo;
    } else {
        color = ambient + Lo;
    }

    color = color / (color + vec3(1.0));
    color = pow(color, vec3(1.0/2.2));  

    FRAGMENT_COLOR = vec4(color, 1.0f);
    //if(u_engine_FogParams.isEnabled) {
    //  float fogCoordinate = EyeSpacePosition;
    //  FRAGMENT_COLOR = mix(FRAGMENT_COLOR, vec4(u_engine_FogParams.color, 1.0), getFogFactor(u_engine_FogParams, fogCoordinate));
    //}
}