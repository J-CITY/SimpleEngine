#shader vertex
#version 460 core

layout (location = 0) in vec3 geo_Pos;
layout (location = 1) in vec2 geo_TexCoords;

/* Global information sent by the engine */

layout (std140) uniform EngineUBO
{
    mat4    ubo_Model;
    mat4    ubo_View;
    mat4    ubo_Projection;
    vec3    ubo_ViewPos;
    float   ubo_Time;
};

out vec2 TexCoords;

void main() {
    TexCoords = geo_TexCoords;
    gl_Position = vec4(geo_Pos, 1.0);
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
};

struct LightOGL {
	float pos[3];
	float forward[3];
	float color[3];
	int type;
	float cutoff;
	float outerCutoff;
	float constant;
	float linear;
	float quadratic;
	float intensity;
};

/* Light information sent by the engine */
layout(std430, binding = 0) buffer LightSSBO {
    LightOGL ssbo_Lights[];
};

in vec2 TexCoords;

uniform sampler2D   u_PositionMap;
uniform sampler2D   u_NormalMap;
uniform sampler2D   u_AlbedoSpecMap;
uniform sampler2D   u_RoughAO;
//ssao
uniform sampler2D   u_engine_SSAO;
uniform bool        u_UseSSAO;

//dir light
uniform sampler2D u_engine_ShadowMap;
uniform vec3 u_engine_LightPos;
uniform bool u_engine_UseDirShadow;
uniform mat4 u_engine_LightSpaceMatrix;

//point lights
const int MAX_POINTS_LIGHT_SHADOW = 4;
uniform int u_engine_PointLightsSize;
uniform vec3 u_engine_PointLightsPos[MAX_POINTS_LIGHT_SHADOW];
uniform float u_engine_FarPlane;
uniform samplerCube u_engine_PointLightsCubeMap[MAX_POINTS_LIGHT_SHADOW];
uniform bool u_engine_UsePointShadow;

//spot light
uniform sampler2D u_engine_SpotLightShadowMap;
uniform float u_engine_SpotFarPlane;
uniform float u_engine_SpotNearPlane;
uniform vec3 u_engine_SpotLightPos;
uniform bool u_engine_UseSpotShadow;
uniform mat4 u_engine_SpotLightSpaceMatrix;

uniform float       u_Shininess               = 100.0;

struct DATA
{
    vec3        FragPos;
    vec3        Normal;
};
DATA fs_in;
/* Global variables */
vec3 g_Normal;
vec2 g_TexCoords;
vec3 g_ViewDir;
vec4 g_DiffuseTexel;
vec4 g_SpecularTexel;
vec3 g_FragPos;
float g_AmbientOcclusion;

#include "lib/blinnPhongLight.glsl"
#include "lib/fog.glsl"
#include "lib/shadows.glsl"

uniform FogParameters u_engine_FogParams;



out vec4 FRAGMENT_COLOR;

void main()
{
    //FRAGMENT_COLOR = vec4(texture(u_RoughAO,  TexCoords).rgb, 1.0f);;
    //FRAGMENT_COLOR = vec4(1.0f,0.0f,0.0f, 1.0f);
    //return;
    g_TexCoords = TexCoords;
    g_FragPos           = texture(u_PositionMap, TexCoords).rgb;
    fs_in.FragPos = g_FragPos;
    g_ViewDir           = normalize(ubo_ViewPos - g_FragPos);
    g_DiffuseTexel      = vec4(texture(u_AlbedoSpecMap,  TexCoords).rgb, 1.0f);
    g_SpecularTexel     = vec4(texture(u_AlbedoSpecMap, TexCoords).a);
    g_Normal            = texture(u_NormalMap, TexCoords).rgb;
    fs_in.Normal = g_Normal;
    if (u_UseSSAO) {
        g_AmbientOcclusion  = texture(u_engine_SSAO, TexCoords).r;
    }
    float EyeSpacePosition = texture(u_RoughAO, TexCoords).b;
    //FRAGMENT_COLOR = vec4(EyeSpacePosition,EyeSpacePosition,EyeSpacePosition, 1.0f);
    //return;
    vec4 FragPosSpotLightSpace = u_engine_SpotLightSpaceMatrix * vec4(fs_in.FragPos, 1.0);
    vec4 FragPosLightSpace = u_engine_LightSpaceMatrix * vec4(fs_in.FragPos, 1.0);

    vec3 lightSum = vec3(0.0);
    int pointShadowMapId = 0;
    for (int i = 0; i < ssbo_Lights.length(); ++i) {
        switch(ssbo_Lights[i].type) {
            case 0: {
                float shadow = 0.0f;
                if (u_engine_UsePointShadow && pointShadowMapId < MAX_POINTS_LIGHT_SHADOW) {
                    shadow = PointShadowCalculation(fs_in.FragPos, pointShadowMapId);
                    pointShadowMapId++;
                }
                lightSum += CalcPointLight(ssbo_Lights[i], shadow);
                break;
            }
            case 1: {
                float shadow = 0.0f;
                if (u_engine_UseDirShadow) {
                    shadow = DirShadowCalculation(FragPosLightSpace);
                }
                lightSum += CalcDirectionalLight(ssbo_Lights[i], shadow);
                
                break;
            }
            case 2: {
                float shadow = 0.0f;
                if (u_engine_UseSpotShadow) {
                    shadow = SpotShadowCalculation(FragPosSpotLightSpace);
                }
                lightSum += CalcSpotLight(ssbo_Lights[i], shadow);
                break;
            }
            case 3: lightSum += CalcAmbientBoxLight(ssbo_Lights[i], u_UseSSAO, g_AmbientOcclusion);    break;
            case 4: lightSum += CalcAmbientSphereLight(ssbo_Lights[i], u_UseSSAO, g_AmbientOcclusion); break;
        }
    }

    FRAGMENT_COLOR = vec4(lightSum, 1.0f);
    if(u_engine_FogParams.isEnabled) {
      float fogCoordinate = EyeSpacePosition;
      FRAGMENT_COLOR = mix(FRAGMENT_COLOR, vec4(u_engine_FogParams.color, 1.0), getFogFactor(u_engine_FogParams, fogCoordinate));
    }
}