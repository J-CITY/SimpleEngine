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

//dir light
uniform mat4 u_engine_LightSpaceMatrix;

//spot light
uniform mat4 u_engine_SpotLightSpaceMatrix;

//fog
//uniform vec4 fogScaleBias;

/* Global information sent by the engine */

layout (std140) uniform EngineUBO {
    mat4    ubo_Model;
    mat4    ubo_View;
    mat4    ubo_Projection;
    vec3    ubo_ViewPos;
    float   ubo_Time;
    vec2    ubo_ViewportSize;
};

/* Information passed to the fragment shader */
out VS_OUT {
    vec3        FragPos;
    vec3        Normal;
    vec2        TexCoords;
    mat3        TBN;
    flat vec3   TangentViewPos;
    vec3        TangentFragPos;

    vec4 FragPosLightSpace;
    vec4 FragPosSpotLightSpace;

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
    vs_out.FragPosLightSpace = u_engine_LightSpaceMatrix * vec4(vs_out.FragPos, 1.0);
    vs_out.FragPosSpotLightSpace = u_engine_SpotLightSpaceMatrix * vec4(vs_out.FragPos, 1.0);
    

    //fog
    //vs_out.fogCrd = vs_out.FragPos * fogScaleBias.w + vec3(fogScaleBias.x, fogScaleBias.y * ubo_Time, fogScaleBias.z);

    //mat4 viewModel = ubo_View * ubo_Model;
    gl_Position = ubo_Projection * (ubo_View * vec4(vs_out.FragPos, 1.0));
    //gl_Position = ubo_Projection * viewModel * totalPosition;
    
    //gl_Position = vec4(geo_Pos, 1.0);
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

    vec4 FragPosLightSpace;
    vec4 FragPosSpotLightSpace;

    vec4 EyeSpacePosition;
} fs_in;

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


//uniform vec4 fogScaleBias;
//uniform float fogStart;

//dir light
uniform sampler2D u_engine_ShadowMap;
uniform vec3 u_engine_LightPos;
uniform bool u_engine_UseDirShadow;

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

uniform mat4 u_engine_LightSpaceMatrix;

/* Global variables */
vec3 g_Normal;
vec2 g_TexCoords;
vec3 g_ViewDir;
vec4 g_DiffuseTexel;
vec4 g_SpecularTexel;
vec4 g_HeightTexel;
vec4 g_NormalTexel;

#include "lib/blinnPhongLight.glsl"
#include "lib/fog.glsl"
#include "lib/shadows.glsl"

uniform FogParameters u_engine_FogParams;

out vec4 FRAGMENT_COLOR;

vec2 ParallaxMapping(vec3 p_ViewDir) {
    const vec2 parallax = p_ViewDir.xy * u_HeightScale * texture(u_HeightMap, g_TexCoords).r;
    return g_TexCoords - vec2(parallax.x, 1.0 - parallax.y);
}


#define PI 3.1415926535897932384626433832795
const mat4 shadowBiasMatrix = mat4( 
	0.5, 0.0, 0.0, 0.0,
	0.0, 0.5, 0.0, 0.0,
	0.0, 0.0, 1.0, 0.0,
	0.5, 0.5, 0.0, 1.0 );
const mat4 volumetricDither = mat4(
  0.0, 0.5, 0.125, 0.625,
  0.75, 0.22, 0.875, 0.375,
  0.1875, 0.6875, 0.0625, 0.5625,
  0.9375, 0.4375, 0.8125, 0.3125 );

float Shadow(const mat4 shadowMapViewProjectionMatrix, const vec3 position, const sampler2D shadowMap, const float shadowBias) {
  vec4 shadowCoord = shadowBiasMatrix * shadowMapViewProjectionMatrix * vec4(position, 1.0);
  shadowCoord /= shadowCoord.w;
  
  if (texture(shadowMap, shadowCoord.xy).r < shadowCoord.z - shadowBias)
  {
    return 0.0;
  }
  
  return 1.0;
}
vec3 Volumetric(const vec3 position, const vec3 eyePosition, const mat4 shadowMapViewProjectionMatrix, const sampler2D shadowMap, const vec3 lightDirection, const vec3 lightColor, const float lightIntensity, const float shadowBias) {
  const int VOLUMETRIC_STEPS = 10;
  const float VOLUMETRIC_SCATTERING = 0.2;
  const float VOLUMETRIC_INTENSITY = 5.0;
  const vec3 rayVector = position - eyePosition;
  const float rayLength = length(rayVector);
  const vec3 rayDirection = rayVector / rayLength;
  const float stepLength = rayLength / VOLUMETRIC_STEPS;
  const vec3 step = rayDirection * stepLength;
  const ivec2 screenUV = ivec2(gl_FragCoord.xy);
  const float ditherValue = volumetricDither[screenUV.x % 4][screenUV.y % 4];
  
  vec3 currentPosition = eyePosition + step * ditherValue;
  float volumetric = 0.0;
  
  for (int i = 0; i < VOLUMETRIC_STEPS; ++i)
  {
    if (Shadow(shadowMapViewProjectionMatrix, currentPosition, shadowMap, shadowBias) > 0.5)
    {
      float scattering = 1.0 - VOLUMETRIC_SCATTERING * VOLUMETRIC_SCATTERING;
      scattering /= 4.0 * PI * pow(1.0 + VOLUMETRIC_SCATTERING * VOLUMETRIC_SCATTERING - (2.0 * VOLUMETRIC_SCATTERING) * dot(rayDirection, lightDirection), 1.5);
      volumetric += scattering;
    }
    
    currentPosition += step;
  }
  
  return (volumetric / VOLUMETRIC_STEPS) * lightIntensity * VOLUMETRIC_INTENSITY * lightColor;
}

void main() {
    //FRAGMENT_COLOR = texture(u_DiffuseMap, fs_in.TexCoords);
    //float depthValue = texture(u_engine_ShadowMap, fs_in.TexCoords).r;
    //FRAGMENT_COLOR = vec4(vec3(depthValue), 1.0);
    //return;
    g_TexCoords = u_TextureOffset + vec2(mod(fs_in.TexCoords.x * u_TextureTiling.x, 1), mod(fs_in.TexCoords.y * u_TextureTiling.y, 1));

    /* Apply parallax mapping */
    if (u_HeightScale > 0)
        g_TexCoords = ParallaxMapping(normalize(fs_in.TangentViewPos - fs_in.TangentFragPos));

    /* Apply color mask */
    
    g_ViewDir           = normalize(ubo_ViewPos - fs_in.FragPos);
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

    int pointShadowMapId = 0;
    vec3 lightSum = vec3(0.0);
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
                    shadow = DirShadowCalculation(fs_in.FragPosLightSpace);
                }
                lightSum += CalcDirectionalLight(ssbo_Lights[i], shadow);
                break;
            }
            case 2: {
                float shadow = 0.0f;
                if (u_engine_UseSpotShadow) {
                    shadow = SpotShadowCalculation(fs_in.FragPosSpotLightSpace);
                }
                lightSum += CalcSpotLight(ssbo_Lights[i], shadow);
                break;
            }
            case 3: lightSum += CalcAmbientBoxLight(ssbo_Lights[i], false, 0.0f);    break;
            case 4: lightSum += CalcAmbientSphereLight(ssbo_Lights[i], false, 0.0f); break;
        }
    }
    //vec3 vol = Volumetric(fs_in.FragPos, ubo_ViewPos, u_engine_LightSpaceMatrix, u_engine_ShadowMap, vec3(0), vec3(255), 0.05f, 0.0f);


    FRAGMENT_COLOR = vec4(lightSum /** vol*/, g_DiffuseTexel.a);

    if (u_engine_FogParams.isEnabled) {
	    float fogCoordinate = abs(fs_in.EyeSpacePosition.z / fs_in.EyeSpacePosition.w);
        FRAGMENT_COLOR = mix(FRAGMENT_COLOR, vec4(u_engine_FogParams.color, 1.0), getFogFactor(u_engine_FogParams, fogCoordinate));
    }
    FRAGMENT_COLOR =  vec4(1);
}
