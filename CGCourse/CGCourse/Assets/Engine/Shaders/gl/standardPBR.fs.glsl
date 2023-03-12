#version 460 core

#include "../lib/engineUBO.glsl"
#include "../lib/engineLightSSBO.glsl"
#include "../lib/engineInData.glsl"

out vec4 FRAGMENT_COLOR;

uniform sampler2D   u_AlbedoMap;
uniform sampler2D   u_MetallicMap;
uniform sampler2D   u_RoughnessMap;
uniform sampler2D   u_AmbientOcclusionMap;
uniform sampler2D   u_NormalMap;
uniform sampler2D   u_HeightMap;


uniform vec4        u_Albedo                = vec4(1.0);
uniform vec2        u_TextureTiling         = vec2(1.0, 1.0);
uniform vec2        u_TextureOffset         = vec2(0.0, 0.0);
uniform bool        u_EnableNormalMapping   = false;
uniform float       u_HeightScale           = 0.0;
uniform float       u_Metallic              = 1.0;
uniform float       u_Roughness             = 1.0;

////ibl
//uniform samplerCube u_engine_irradianceMap;
//uniform samplerCube u_engine_prefilterMap;
//uniform sampler2D   u_engine_brdfLUT;
//uniform bool        u_UseIbl;
//
////dir light
//uniform sampler2D u_engine_ShadowMap;
//uniform vec3 u_engine_LightPos;
//uniform bool u_engine_UseDirShadow;
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

/* Global variables */
vec2 g_TexCoords;
vec3 g_Normal;
vec3 g_FragPos;
#include "../lib/pbr.glsl"
#include "../lib/shadows.glsl"

//#include "lib/fog.glsl"
//
//uniform FogParameters u_engine_FogParams;



vec2 ParallaxMapping(vec3 p_ViewDir) {
    const vec2 parallax = p_ViewDir.xy * u_HeightScale * texture(u_HeightMap, g_TexCoords).r;
    return g_TexCoords - vec2(parallax.x, 1.0 - parallax.y);
}

//void main() {
//    vec2 texCoords = u_TextureOffset + vec2(mod(fs_in.TexCoords.x * u_TextureTiling.x, 1), mod(fs_in.TexCoords.y * u_TextureTiling.y, 1));
//
//    /* Apply parallax mapping */
//    if (u_HeightScale > 0)
//        g_TexCoords = ParallaxMapping(normalize(fs_in.TangentViewPos - fs_in.TangentFragPos));
//
//    vec4 albedoRGBA     = texture(u_AlbedoMap, texCoords) * u_Albedo;
//    vec3 albedo         = pow(albedoRGBA.rgb, vec3(2.2));
//    float metallic      = texture(u_MetallicMap, texCoords).r * u_Metallic;
//    float roughness     = texture(u_RoughnessMap, texCoords).r * u_Roughness;
//    float ao            = texture(u_AmbientOcclusionMap, texCoords).r;
//    vec3 normal;
//
//    if (u_EnableNormalMapping)
//    {
//        normal = texture(u_NormalMap, texCoords).rgb;
//        normal = normalize(normal * 2.0 - 1.0);   
//        normal = normalize(fs_in.TBN * normal);
//    }
//    else
//    {
//        normal = normalize(fs_in.Normal);
//    }
//
//    vec3 N = normalize(normal);
//    vec3 V = normalize(ubo_ViewPos - fs_in.FragPos);
//
//    vec3 F0 = vec3(0.04); 
//    F0 = mix(F0, albedo, metallic);
//
//    ////ibl
//    //vec3 ambientIBL = vec3(0.0f);
//    //if (u_UseIbl) {
//    //    vec3 R = reflect(-V, N); 
//    //    vec3 irradiance = texture(u_engine_irradianceMap, N).rgb;
//    //    vec3 diffuse = irradiance * albedo;
//    //    vec3 F = fresnelSchlickRoughness(max(dot(N, V), 0.0), F0, roughness);
//    //    vec3 kS = F;
//    //    vec3 kD = 1.0 - kS;
//    //    kD *= 1.0 - metallic;	
//    //    // Производим выборки из префильтрованной карты LUT-текстуры BRDF и затем объединяем их вместе в соответствии с аппроксимацией разделенной суммы, чтобы получить зеркальную часть IBL
//    //    const float MAX_REFLECTION_LOD = 4.0;
//    //    vec3 prefilteredColor = textureLod(u_engine_prefilterMap, R,  roughness * MAX_REFLECTION_LOD).rgb;    
//    //    vec2 brdf  = texture(u_engine_brdfLUT, vec2(max(dot(N, V), 0.0), roughness)).rg;
//    //    vec3 specular = prefilteredColor * (F * brdf.x + brdf.y);
//    //    ambientIBL = (kD * diffuse + specular) * ao;
//    //}
//    ////ibl end
//
//	           
//    //// reflectance equation
//    //vec3 Lo = vec3(0.0);
//    //vec3 ambientSum = vec3(0.0);
//    //
//    //int pointShadowMapId = 0;
//    //for (int i = 0; i < ssbo_Lights.length(); ++i) 
//    //{
//    //    if (ssbo_Lights[i].type == 3) {
//    //        ambientSum += CalcAmbientBoxLight(ssbo_Lights[i], false, 0.0f);
//    //    }
//    //    else if (ssbo_Lights[i].type == 4) {
//    //        ambientSum += CalcAmbientSphereLight(ssbo_Lights[i], false, 0.0f);
//    //    }
//    //    else {
//    //        // calculate per-light radiance
//    //        vec3 L = ssbo_Lights[i].constant == 1 ? -toVec3(ssbo_Lights[i].forward) : normalize(toVec3(ssbo_Lights[i].pos) - fs_in.FragPos);
//    //        vec3 H = normalize(V + L);
//    //        float distance    = length(toVec3(ssbo_Lights[i].pos)- fs_in.FragPos);
//    //        float lightCoeff = 0.0;
//    //
//    //        float shadow = 0.0f;
//    //        switch(ssbo_Lights[i].type) {
//    //            case 0:
//    //                if (u_engine_UsePointShadow && pointShadowMapId < MAX_POINTS_LIGHT_SHADOW) {
//    //                    shadow = PointShadowCalculation(fs_in.FragPos, pointShadowMapId);
//    //                    pointShadowMapId++;
//    //                }
//    //                lightCoeff = LuminosityFromAttenuation(ssbo_Lights[i]) * ssbo_Lights[i].intensity;
//    //                break;
//    //
//    //            case 1:
//    //                if (u_engine_UseDirShadow) {
//    //                    shadow = DirShadowCalculation(fs_in.FragPosLightSpace);
//    //                }
//    //                lightCoeff = ssbo_Lights[i].intensity;
//    //                break;
//    //
//    //            case 2:
//    //                if (u_engine_UseSpotShadow) {
//    //                    shadow = SpotShadowCalculation(fs_in.FragPosSpotLightSpace);
//    //                }
//    //                const vec3  lightForward    = toVec3(ssbo_Lights[i].forward);
//    //                const float cutOff          = cos(radians(ssbo_Lights[i].cutoff));
//    //                const float outerCutOff     = cos(radians(ssbo_Lights[i].cutoff + ssbo_Lights[i].outerCutoff));
//    //
//    //                const vec3  lightDirection  = normalize(toVec3(ssbo_Lights[i].pos) - fs_in.FragPos);
//    //                const float luminosity      = LuminosityFromAttenuation(ssbo_Lights[i]);
//    //
//    //                /* Calculate the spot intensity */
//    //                const float theta           = dot(lightDirection, normalize(-lightForward)); 
//    //                const float epsilon         = cutOff - outerCutOff;
//    //                const float spotIntensity   = clamp((theta - outerCutOff) / epsilon, 0.0, 1.0);
//    //
//    //                lightCoeff = luminosity * spotIntensity * ssbo_Lights[i].intensity;
//    //                break;
//    //        }
//    //
//    //        vec3 radiance = toVec3(ssbo_Lights[i].color) * lightCoeff;        
//    //        
//    //        // cook-torrance brdf
//    //        float NDF = DistributionGGX(N, H, roughness);        
//    //        float G   = GeometrySmith(N, V, L, roughness);      
//    //        vec3 F    = fresnelSchlick(max(dot(H, V), 0.0), F0);       
//    //        
//    //        vec3 kS = F;
//    //        vec3 kD = vec3(1.0) - kS;
//    //        kD *= 1.0 - metallic;	  
//    //        
//    //        vec3 numerator    = NDF * G * F;
//    //        float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0);
//    //        vec3 specular     = numerator / max(denominator, 0.001);  
//    //            
//    //        // add to outgoing radiance Lo
//    //        float NdotL = max(dot(N, L), 0.0);
//    //        vec3 resLight = (kD * albedo / PI + specular) * radiance * NdotL;
//    //        resLight *= (1.0 - shadow);
//    //        Lo += resLight; 
//    //    }
//    //}
//
//    vec3 ambient = ambientSum * albedo * ao;
//    vec3 color = vec3(0.0f);
//    if (u_UseIbl) {
//        color = ambientIBL + Lo;
//    } else {
//        color = ambient + Lo;
//    }
//	
//    color = color / (color + vec3(1.0));
//    color = pow(color, vec3(1.0/2.2));  
//   
//    FRAGMENT_COLOR = vec4(color, albedoRGBA.a);
//
//    if (u_engine_FogParams.isEnabled) {
//        float fogCoordinate = abs(fs_in.EyeSpacePosition.z / fs_in.EyeSpacePosition.w);
//        FRAGMENT_COLOR = mix(FRAGMENT_COLOR, vec4(u_engine_FogParams.color, 1.0), getFogFactor(u_engine_FogParams, fogCoordinate));
//    }
//}

void main() {
    vec2 texCoords = u_TextureOffset + vec2(mod(fs_in.TexCoords.x * u_TextureTiling.x, 1), mod(fs_in.TexCoords.y * u_TextureTiling.y, 1));

    vec4 albedoRGBA     = texture(u_AlbedoMap, texCoords) * u_Albedo;
    vec3 albedo         = pow(albedoRGBA.rgb, vec3(2.2));
    float metallic      = texture(u_MetallicMap, texCoords).r * u_Metallic;
    float roughness     = texture(u_RoughnessMap, texCoords).r * u_Roughness;
    float ao            = texture(u_AmbientOcclusionMap, texCoords).r;
    vec3 normal;

    if (u_EnableNormalMapping) {
        normal = texture(u_NormalMap, texCoords).rgb;
        normal = normalize(normal * 2.0 - 1.0);   
        normal = normalize(fs_in.TBN * normal);
    }
    else {
        normal = normalize(fs_in.Normal);
    }

    vec3 N = normalize(normal);
    g_Normal = N;
    g_FragPos = fs_in.FragPos;
    vec3 V = normalize(engine_UBO.ViewPos - fs_in.FragPos);

    vec3 F0 = vec3(0.04); 
    F0 = mix(F0, albedo, metallic);
	           
    // reflectance equation
    vec3 Lo = vec3(0.0);
    vec3 ambientSum = vec3(0.0);

    for (int i = 0; i < ssbo_Lights.length(); ++i)  {
        if (int(ssbo_Lights[i][3][0]) == 3) {
            ambientSum += CalcAmbientBoxLight(ssbo_Lights[i]);
        }
        else if (int(ssbo_Lights[i][3][0]) == 4) {
            ambientSum += CalcAmbientSphereLight(ssbo_Lights[i]);
        }
        else {
            // calculate per-light radiance
            vec3 L = int(ssbo_Lights[i][3][0]) == 1 ? -ssbo_Lights[i][1].rgb : normalize(ssbo_Lights[i][0].rgb - fs_in.FragPos);
            vec3 H = normalize(V + L);
            float distance    = length(ssbo_Lights[i][0].rgb - fs_in.FragPos);
            float lightCoeff = 0.0;

            float shadow = 0.0f;
            switch(int(ssbo_Lights[i][3][0])) {
                case 0:
                    lightCoeff = LuminosityFromAttenuation(ssbo_Lights[i]) * ssbo_Lights[i][3][3];
                    break;

                case 1:
                    if (engine_ShadowUBO.useDirLightShadow) {
                        shadow = DirShadowCalculation(fs_in.FragPosDirLightSpace);
                    }
                    lightCoeff = ssbo_Lights[i][3][3];
                    break;

                case 2:
                    const vec3  lightForward    = ssbo_Lights[i][1].rgb;
                    const float cutOff          = cos(radians(ssbo_Lights[i][3][1]));
                    const float outerCutOff     = cos(radians(ssbo_Lights[i][3][1] + ssbo_Lights[i][3][2]));

                    const vec3  lightDirection  = normalize(ssbo_Lights[i][0].rgb - fs_in.FragPos);
                    const float luminosity      = LuminosityFromAttenuation(ssbo_Lights[i]);

                    /* Calculate the spot intensity */
                    const float theta           = dot(lightDirection, normalize(-lightForward)); 
                    const float epsilon         = cutOff - outerCutOff;
                    const float spotIntensity   = clamp((theta - outerCutOff) / epsilon, 0.0, 1.0);

                    lightCoeff = luminosity * spotIntensity * ssbo_Lights[i][3][3];
                    break;
            }

            vec3 radiance = UnPack(ssbo_Lights[i][2][0]) * lightCoeff;
            
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
            //Lo += (kD * albedo / PI + specular) * radiance * NdotL;
            vec3 resLight = (kD * albedo / PI + specular) * radiance * NdotL;
            resLight *= (1.0 - shadow);
            Lo += resLight; 
        }
    }

    vec3 ambient = ambientSum * albedo * ao;
    vec3 color = ambient + Lo;

    color = color / (color + vec3(1.0));
    color = pow(color, vec3(1.0/2.2));  
   
    FRAGMENT_COLOR = vec4(color, albedoRGBA.a);
}