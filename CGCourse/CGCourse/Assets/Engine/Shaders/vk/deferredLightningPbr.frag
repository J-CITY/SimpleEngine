#version 460 core

layout(location = 0) in vec2 TexCoords;

layout (set = 0, binding = 0) uniform EngineUBO {
    mat4    View;
    mat4    Projection;
    vec3    ViewPos;
    float   Time;
    vec2    ViewportSize;
} engine_UBO;

layout(set = 1, binding = 0) uniform sampler2D u_PositionMap;
layout(set = 1, binding = 1) uniform sampler2D u_NormalMap;
layout(set = 1, binding = 2) uniform sampler2D u_AlbedoSpecMap;
layout(set = 1, binding = 3) uniform sampler2D u_RoughAO;

//struct LightOGL {
//    float pos[3];
//    float forward[3];
//    float color[3];
//    int type;
//    float cutoff;
//    float outerCutoff;
//    float constant;
//    float linear;
//    float quadratic;
//    float intensity;
//};

struct LightOGL {
    vec3 pos;
    float cutoff;
    vec3 forward;
    float outerCutoff;
    vec3 color;
    float constant;
    int type;
    float linear;
    float quadratic;
    float intensity;
};

layout(std140, set = 2, binding = 0) readonly buffer ObjectBuffer {
    LightOGL l[];
} ssbo_Lights;

//layout (set = 2, binding = 0) uniform LightUBO {
//    LightOGL l[20];
//} ssbo_Lights;

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

//#include "../lib/pbr.glsl"
//vec3 toVec3(float inv[3]) {
//    return vec3(inv[0], inv[1], inv[2]);
//}

vec3 toVec3(vec3 inv) {
    return inv;
}

const float PI = 3.14159265359;

float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a      = roughness*roughness;
    float a2     = a*a;
    float NdotH  = max(dot(N, H), 0.0);
    float NdotH2 = NdotH*NdotH;
	
    float num   = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;
	
    return num / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r*r) / 8.0;

    float num   = NdotV;
    float denom = NdotV * (1.0 - k) + k;
	
    return num / denom;
}
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2  = GeometrySchlickGGX(NdotV, roughness);
    float ggx1  = GeometrySchlickGGX(NdotL, roughness);
	
    return ggx1 * ggx2;
}

vec3 fresnelSchlick(float cosTheta, vec3 F0) {
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}

bool PointInAABB(vec3 p_Point, vec3 p_AabbCenter, vec3 p_AabbHalfSize) {
    return
    (
        p_Point.x > p_AabbCenter.x - p_AabbHalfSize.x && p_Point.x < p_AabbCenter.x + p_AabbHalfSize.x &&
        p_Point.y > p_AabbCenter.y - p_AabbHalfSize.y && p_Point.y < p_AabbCenter.y + p_AabbHalfSize.y &&
        p_Point.z > p_AabbCenter.z - p_AabbHalfSize.z && p_Point.z < p_AabbCenter.z + p_AabbHalfSize.z
    );
}

float LuminosityFromAttenuation(LightOGL p_Light) {
    const vec3  lightPosition   = toVec3(p_Light.pos);
    const float constant        = p_Light.constant;
    const float linear          = p_Light.linear;
    const float quadratic       = p_Light.quadratic;

    const float distanceToLight = length(lightPosition - fs_in.FragPos);
    const float attenuation     = (constant + linear * distanceToLight + quadratic * (distanceToLight * distanceToLight));
    return 1.0 / attenuation;
}

vec3 CalcAmbientBoxLight(LightOGL p_Light, bool useAO, float ao) {
    const vec3  lightPosition   = toVec3(p_Light.pos);
    const vec3  lightColor      = toVec3(p_Light.color);
    float intensity       = p_Light.intensity;
    if (useAO) {
        intensity *= ao;
    }
    const vec3  size            = vec3(p_Light.constant, p_Light.linear, p_Light.quadratic);

    return PointInAABB(fs_in.FragPos, lightPosition, size) ? lightColor * intensity : vec3(0.0);
}

vec3 CalcAmbientSphereLight(LightOGL p_Light, bool useAO, float ao) {
    const vec3  lightPosition   = toVec3(p_Light.pos);
    const vec3  lightColor      = toVec3(p_Light.color);
    float intensity       = p_Light.intensity;
    if (useAO) {
        intensity *= ao;
    }
    const float radius          = p_Light.constant;

    return distance(lightPosition, fs_in.FragPos) <= radius ? lightColor * intensity : vec3(0.0);
}

vec3 fresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness) {
    return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(1.0 - cosTheta, 5.0);
}
//end

//uniform 
bool engine_UseSSAO = false;
//layout(binding = 13) uniform sampler2D engine_SSAOMap;

//uniform FogParameters u_engine_FogParams;

layout (location = 0) out vec4 FRAGMENT_COLOR;

void main() {
    FRAGMENT_COLOR = vec4(1.0f);
    g_TexCoords         = TexCoords;
    g_FragPos           = texture(u_PositionMap, TexCoords).rgb;
    g_ViewDir           = normalize(engine_UBO.ViewPos - g_FragPos);
    g_DiffuseTexel      = texture(u_AlbedoSpecMap,  TexCoords).rgb;
    float metallic      = texture(u_AlbedoSpecMap, TexCoords).a;
    float roughness     = texture(u_RoughAO, TexCoords).r;
    float ao            = texture(u_RoughAO, TexCoords).g;
    g_Normal            = texture(u_NormalMap, TexCoords).rgb;
    //if (engine_UseSSAO) {
    //    g_AmbientOcclusion  = texture(engine_SSAOMap, TexCoords).r;
    //}
    float EyeSpacePosition = texture(u_RoughAO, TexCoords).b;
    fs_in.FragPos = g_FragPos;
    fs_in.Normal = g_Normal;
    vec3 albedo = g_DiffuseTexel;
    vec3 N = normalize(g_Normal);
    vec3 V = normalize(engine_UBO.ViewPos - fs_in.FragPos);

    //vec4 FragPosSpotLightSpace = engine_ShadowUBO.spotLightSpaceMatrix * vec4(fs_in.FragPos, 1.0);
    //vec4 FragPosDirLightSpace = engine_ShadowUBO.dirLightSpaceMatrix * vec4(fs_in.FragPos, 1.0);

    vec3 F0 = vec3(0.04); 
    F0 = mix(F0, albedo, metallic);

    ////ibl
    //vec3 ambientIBL = vec3(0.0f);
    //if (u_UseIbl) {
    //    vec3 R = reflect(-V, N); 
    //    vec3 irradiance = texture(u_engine_irradianceMap, N).rgb;
    //    vec3 diffuse = irradiance * albedo;
    //    vec3 F = fresnelSchlickRoughness(max(dot(N, V), 0.0), F0, roughness);
    //    vec3 kS = F;
    //    vec3 kD = 1.0 - kS;
    //    kD *= 1.0 - metallic;	
    //    // Производим выборки из префильтрованной карты LUT-текстуры BRDF и затем объединяем их вместе в соответствии с аппроксимацией разделенной суммы, чтобы получить зеркальную часть IBL
    //    const float MAX_REFLECTION_LOD = 4.0;
    //    vec3 prefilteredColor = textureLod(u_engine_prefilterMap, R,  roughness * MAX_REFLECTION_LOD).rgb;    
    //    vec2 brdf  = texture(u_engine_brdfLUT, vec2(max(dot(N, V), 0.0), roughness)).rg;
    //    vec3 specular = prefilteredColor * (F * brdf.x + brdf.y);
    //    ambientIBL = (kD * diffuse + specular) * ao;
    //}
    ////ibl end
    
    // reflectance equation
    vec3 Lo = vec3(0.0);
    vec3 ambientSum = vec3(0.0);
    
    //if (ssbo_Lights.l[1].type == 4) {
    //    FRAGMENT_COLOR = vec4(g_DiffuseTexel, 1.0f);
    //    return;
    //}

    int pointShadowMapId = 0;
    for (int i = 0; i < ssbo_Lights.l.length(); ++i)  {
        if (ssbo_Lights.l[i].type == -1) {
            break;
        }
        if (ssbo_Lights.l[i].type == 3) {
            ambientSum += CalcAmbientBoxLight(ssbo_Lights.l[i], engine_UseSSAO, g_AmbientOcclusion);
        }
        else if (ssbo_Lights.l[i].type == 4) {
            ambientSum += CalcAmbientSphereLight(ssbo_Lights.l[i], engine_UseSSAO, g_AmbientOcclusion);
        }
        else {
            // calculate per-light radiance
            vec3 L = ssbo_Lights.l[i].constant == 1 ? -toVec3(ssbo_Lights.l[i].forward) : normalize(toVec3(ssbo_Lights.l[i].pos) - fs_in.FragPos);
            vec3 H = normalize(V + L);
            float distance    = length(toVec3(ssbo_Lights.l[i].pos)- fs_in.FragPos);
            float lightCoeff = 0.0;

            float shadow = 0.0f;
            switch(ssbo_Lights.l[i].type) {
                case 0:
                    //if (u_engine_UsePointShadow && pointShadowMapId < MAX_POINTS_LIGHT_SHADOW) {
                    //    shadow = PointShadowCalculation(fs_in.FragPos, pointShadowMapId);
                    //    pointShadowMapId++;
                    //}
                    lightCoeff = LuminosityFromAttenuation(ssbo_Lights.l[i]) * ssbo_Lights.l[i].intensity;
                    break;

                case 1:
                    //if (engine_ShadowUBO.useDirLightShadow) {
                    //    shadow = DirShadowCalculation(FragPosDirLightSpace);
                    //}
                    lightCoeff = ssbo_Lights.l[i].intensity;
                    break;

                case 2:
                    //if (u_engine_UseSpotShadow) {
                    //    shadow = SpotShadowCalculation(FragPosSpotLightSpace);
                    //}
                    const vec3  lightForward    = toVec3(ssbo_Lights.l[i].forward);
                    const float cutOff          = cos(radians(ssbo_Lights.l[i].cutoff));
                    const float outerCutOff     = cos(radians(ssbo_Lights.l[i].cutoff + ssbo_Lights.l[i].outerCutoff));

                    const vec3  lightDirection  = normalize(toVec3(ssbo_Lights.l[i].pos) - fs_in.FragPos);
                    const float luminosity      = LuminosityFromAttenuation(ssbo_Lights.l[i]);

                    const float theta           = dot(lightDirection, normalize(-lightForward)); 
                    const float epsilon         = cutOff - outerCutOff;
                    const float spotIntensity   = clamp((theta - outerCutOff) / epsilon, 0.0, 1.0);

                    lightCoeff = luminosity * spotIntensity * ssbo_Lights.l[i].intensity;
                    break;
            }

            vec3 radiance = toVec3(ssbo_Lights.l[i].color) * lightCoeff;
            
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
    //if (u_UseIbl) {
    //    if (u_UseSSAO) {
    //        ambientIBL *= g_AmbientOcclusion;
    //    }
    //    color = ambientIBL + Lo;
    //} else {
        color = ambient + Lo;
    //}

    color = color / (color + vec3(1.0));
    color = pow(color, vec3(1.0/2.2));  

    FRAGMENT_COLOR = vec4(color, 1.0f);
    //if(u_engine_FogParams.isEnabled) {
    //  float fogCoordinate = EyeSpacePosition;
    //  FRAGMENT_COLOR = mix(FRAGMENT_COLOR, vec4(u_engine_FogParams.color, 1.0), getFogFactor(u_engine_FogParams, fogCoordinate));
    //}
}