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
uniform mat4 u_FinalBonesMatrices[MAX_BONES];
uniform bool u_UseBone;
uniform mat4 u_LightSpaceMatrix;

//fog
uniform vec4 fogScaleBias;

/* Global information sent by the engine */

layout (std140) uniform EngineUBO
{
    mat4    ubo_Model;
    mat4    ubo_View;
    mat4    ubo_Projection;
    vec3    ubo_ViewPos;
    float   ubo_Time;
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

    vec4 FragPosLightSpace;

    vec4 EyeSpacePosition;

    vec3 fogCrd;
} vs_out;
int ub=1;

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
            vec4 localPosition = (u_FinalBonesMatrices[int(boneIds[i])]) * vec4(geo_Pos,1.0f);
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
    vs_out.FragPosLightSpace = u_LightSpaceMatrix * vec4(vs_out.FragPos, 1.0);
    

    //fog
    vs_out.fogCrd = vs_out.FragPos * fogScaleBias.w + vec3(fogScaleBias.x, fogScaleBias.y * ubo_Time, fogScaleBias.z);

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

    vec4 EyeSpacePosition;

    vec3 fogCrd;
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
layout(std430, binding = 0) buffer LightSSBO
{
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
uniform sampler2D   u_MaskMap;


uniform vec4 fogScaleBias;
//uniform float fogStart;

uniform sampler2D shadowMap;
//uniform sampler2D spotShadowMap;
//uniform samplerCube pointShadowMap[4];
uniform vec3 lightPos;


uniform mat4 u_LightSpaceMatrix;

/* Global variables */
vec3 g_Normal;
vec2 g_TexCoords;
vec3 g_ViewDir;
vec4 g_DiffuseTexel;
vec4 g_SpecularTexel;
vec4 g_HeightTexel;
vec4 g_NormalTexel;

struct FogParameters {
	vec3 color;
	float linearStart;
	float linearEnd;
	float density;
	
	int equation;
	bool isEnabled;
};


//TODO: set as uniform
FogParameters fogParams;

out vec4 FRAGMENT_COLOR;

float getFogFactor(FogParameters params, float fogCoordinate) {
	float result = 0.0;
	if(params.equation == 0) {
		float fogLength = params.linearEnd - params.linearStart;
		result = (params.linearEnd - fogCoordinate) / fogLength;
	}
	else if(params.equation == 1) {
		result = exp(-params.density * fogCoordinate);
	}
	else if(params.equation == 2) {
		result = exp(-pow(params.density * fogCoordinate, 2.0));
	}
	result = 1.0 - clamp(result, 0.0, 1.0);
	return result;
}


bool PointInAABB(vec3 p_Point, vec3 p_AabbCenter, vec3 p_AabbHalfSize)
{
    return
    (
        p_Point.x > p_AabbCenter.x - p_AabbHalfSize.x && p_Point.x < p_AabbCenter.x + p_AabbHalfSize.x &&
        p_Point.y > p_AabbCenter.y - p_AabbHalfSize.y && p_Point.y < p_AabbCenter.y + p_AabbHalfSize.y &&
        p_Point.z > p_AabbCenter.z - p_AabbHalfSize.z && p_Point.z < p_AabbCenter.z + p_AabbHalfSize.z
    );
}

vec2 ParallaxMapping(vec3 p_ViewDir)
{
    const vec2 parallax = p_ViewDir.xy * u_HeightScale * texture(u_HeightMap, g_TexCoords).r;
    return g_TexCoords - vec2(parallax.x, 1.0 - parallax.y);
}

vec3 BlinnPhong(vec3 p_LightDir, vec3 p_LightColor, float p_Luminosity, float shadow)
{
    const vec3  halfwayDir          = normalize(p_LightDir + g_ViewDir);
    const float diffuseCoefficient  = max(dot(g_Normal, p_LightDir), 0.0);
    const float specularCoefficient = pow(max(dot(g_Normal, halfwayDir), 0.0), u_Shininess * 2.0);

    return (1.0-shadow) * (p_LightColor * g_DiffuseTexel.rgb * diffuseCoefficient * p_Luminosity + ((p_Luminosity > 0.0) ? (p_LightColor * g_SpecularTexel.rgb * specularCoefficient * p_Luminosity) : vec3(0.0)));
}


float LuminosityFromAttenuation(LightOGL p_Light)
{
    const vec3  lightPosition   = vec3(p_Light.pos[0], p_Light.pos[1], p_Light.pos[2]);
    const float constant        = p_Light.constant;
    const float linear          = p_Light.linear;
    const float quadratic       = p_Light.quadratic;

    const float distanceToLight = length(lightPosition - fs_in.FragPos);
    const float attenuation     = (constant + linear * distanceToLight + quadratic * (distanceToLight * distanceToLight));
    return 1.0 / attenuation;
}
vec3 CalcPointLight(LightOGL p_Light)
{
    const vec3 lightPosition  = vec3(p_Light.pos[0], p_Light.pos[1], p_Light.pos[2]);
    const vec3 lightColor     = vec3(p_Light.color[0], p_Light.color[1], p_Light.color[2]);
    const float intensity     = p_Light.intensity;

    const vec3  lightDirection  = normalize(lightPosition - fs_in.FragPos);
    const float luminosity      = LuminosityFromAttenuation(p_Light);

    return BlinnPhong(lightDirection, lightColor, intensity * luminosity, 0.0);
}

vec3 CalcDirectionalLight(LightOGL p_Light, float shadow)
{
    return BlinnPhong(-vec3(p_Light.forward[0], p_Light.forward[1], p_Light.forward[2]), 
        vec3(p_Light.color[0], p_Light.color[1], p_Light.color[2]), p_Light.intensity, shadow);
}

vec3 CalcSpotLight(LightOGL p_Light)
{
    const vec3  lightPosition   = vec3(p_Light.pos[0], p_Light.pos[1], p_Light.pos[2]);
    const vec3  lightForward    = vec3(p_Light.forward[0], p_Light.forward[1], p_Light.forward[2]);
    const vec3  lightColor      = vec3(p_Light.color[0], p_Light.color[1], p_Light.color[2]);
    const float intensity       = p_Light.intensity;
    const float cutOff          = cos(radians(p_Light.cutoff));
    const float outerCutOff     = cos(radians(p_Light.cutoff + p_Light.outerCutoff));

    const vec3  lightDirection  = normalize(lightPosition - fs_in.FragPos);
    const float luminosity      = LuminosityFromAttenuation(p_Light);

    /* Calculate the spot intensity */
    const float theta           = dot(lightDirection, normalize(-lightForward)); 
    const float epsilon         = cutOff - outerCutOff;
    const float spotIntensity   = clamp((theta - outerCutOff) / epsilon, 0.0, 1.0);
    
    return BlinnPhong(lightDirection, lightColor, intensity * spotIntensity * luminosity, 0.0);
}

vec3 CalcAmbientBoxLight(LightOGL p_Light)
{
    const vec3  lightPosition   = vec3(p_Light.pos[0], p_Light.pos[1], p_Light.pos[2]);
    const vec3  lightColor      = vec3(p_Light.color[0], p_Light.color[1], p_Light.color[2]);
    const float intensity       = p_Light.intensity;
    const vec3  size            = vec3(p_Light.constant, p_Light.linear, p_Light.quadratic);

    return PointInAABB(fs_in.FragPos, lightPosition, size) ? g_DiffuseTexel.rgb * lightColor * intensity : vec3(0.0);
}

vec3 CalcAmbientSphereLight(LightOGL p_Light)
{
    const vec3  lightPosition   = vec3(p_Light.pos[0], p_Light.pos[1], p_Light.pos[2]);
    const vec3  lightColor      = vec3(p_Light.color[0], p_Light.color[1], p_Light.color[2]);
    const float intensity       = p_Light.intensity;
    const float radius          = p_Light.constant;

    return distance(lightPosition, fs_in.FragPos) <= radius ? g_DiffuseTexel.rgb * lightColor * intensity : vec3(0.0);
}


float DirShadowCalculation(vec4 fragPosLightSpace)
{
    // Выполняем деление перспективы
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
	
    // Трансформируем в диапазон [0,1]
    projCoords = projCoords * 0.5 + 0.5;
	
    // Получаем наиболее близкое значение глубины исходя из перспективы глазами источника света (используя в диапазон [0,1] fragPosLight в качестве координат)
    float closestDepth = texture(shadowMap, projCoords.xy).r; 
	
    // Получаем глубину текущего фрагмента исходя из перспективы глазами источника света
    float currentDepth = projCoords.z;
	
    // Вычисляем смещение (на основе разрешения карты глубины и наклона)
    vec3 normal = normalize(fs_in.Normal);
    vec3 lightDir = normalize(lightPos - fs_in.FragPos);
    float bias = max(0.05 * (1.0 - dot(normal, lightDir)), 0.005);
	
    // Проверка нахождения текущего фрагмента в тени
    // float shadow = currentDepth - bias > closestDepth  ? 1.0 : 0.0;
	
    // PCF
    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
    for(int x = -1; x <= 1; ++x)
    {
        for(int y = -1; y <= 1; ++y)
        {
            float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r; 
            shadow += currentDepth - bias > pcfDepth  ? 1.0 : 0.0;        
        }    
    }
    shadow /= 9.0;
    
    // Оставляем значение тени на уровне 0.0 за границей дальней плоскости пирамиды видимости глазами источника света
    if(projCoords.z > 1.0)
        shadow = 0.0;
        
    return shadow;
}
float near = 0.1; 
float far  = 100.0;
float LinearizeDepth(float depth) 
{
    float z = depth * 2.0 - 1.0; // обратно к NDC 
    return (2.0 * near * far) / (far + near - z * (far - near));	
}
/*float SpotShadowCalculation(vec4 fragPosLightSpace)
{
    // Выполняем деление перспективы
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
	
    // Трансформируем в диапазон [0,1]
    projCoords = projCoords * 0.5 + 0.5;
	
    // Получаем наиболее близкое значение глубины исходя из перспективы глазами источника света (используя в диапазон [0,1] fragPosLight в качестве координат)
    float closestDepth = texture(spotShadowMap, projCoords.xy).r; 
	closestDepth = LinearizeDepth(closestDepth) / far; 
    // Получаем глубину текущего фрагмента исходя из перспективы глазами источника света
    float currentDepth = projCoords.z;
	
    // Вычисляем смещение (на основе разрешения карты глубины и наклона)
    vec3 normal = normalize(fs_in.Normal);
    vec3 lightDir = normalize(lightPos - fs_in.FragPos);
    float bias = max(0.05 * (1.0 - dot(normal, lightDir)), 0.005);
	
    // Проверка нахождения текущего фрагмента в тени
    // float shadow = currentDepth - bias > closestDepth  ? 1.0 : 0.0;
	
    // PCF
    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(spotShadowMap, 0);
    for(int x = -1; x <= 1; ++x)
    {
        for(int y = -1; y <= 1; ++y)
        {
            float pcfDepth = texture(spotShadowMap, projCoords.xy + vec2(x, y) * texelSize).r;
            pcfDepth = LinearizeDepth(pcfDepth) / far; 
            shadow += currentDepth - bias > pcfDepth  ? 1.0 : 0.0;        
        }    
    }
    shadow /= 9.0;
    
    // Оставляем значение тени на уровне 0.0 за границей дальней плоскости пирамиды видимости глазами источника света
    if(projCoords.z > 1.0)
        shadow = 0.0;
        
    return shadow;
}

vec3 gridSamplingDisk[20] = vec3[] (
   vec3(1, 1,  1), vec3( 1, -1,  1), vec3(-1, -1,  1), vec3(-1, 1,  1), 
   vec3(1, 1, -1), vec3( 1, -1, -1), vec3(-1, -1, -1), vec3(-1, 1, -1),
   vec3(1, 1,  0), vec3( 1, -1,  0), vec3(-1, -1,  0), vec3(-1, 1,  0),
   vec3(1, 0,  1), vec3(-1,  0,  1), vec3( 1,  0, -1), vec3(-1, 0, -1),
   vec3(0, 1,  1), vec3( 0, -1,  1), vec3( 0, -1, -1), vec3( 0, 1, -1)
);
int pointShadowMapId = 0;
float PointShadowCalculation(vec3 fragPos) {
    // Получаем вектор между положением фрагмента и положением источника света
    vec3 fragToLight = fragPos - lightPos;
    // Теперь получим текущую линейную глубину как длину между фрагментом и положением источника света
    float currentDepth = length(fragToLight);

    float shadow = 0.0;
    float bias = 0.15;
    int samples = 20;
    float viewDistance = length(ubo_ViewPos - fragPos);
    float diskRadius = (1.0 + (viewDistance / far)) / 25.0;
    for(int i = 0; i < samples; ++i) {
        float closestDepth = texture(pointShadowMap[pointShadowMapId], fragToLight + gridSamplingDisk[i] * diskRadius).r;
        closestDepth *= far; 
        if(currentDepth - bias > closestDepth)
            shadow += 1.0;
    }
    shadow /= float(samples);
    return shadow;
}*/
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

void main()
{
    fogParams.color = vec3(0.2);
    fogParams.density = 0.01;
    fogParams.equation = 2;
    fogParams.isEnabled = true;
	fogParams.linearStart=0.0;
	fogParams.linearEnd=0.0;

    //FRAGMENT_COLOR = texture(u_DiffuseMap, fs_in.TexCoords);
    //float depthValue = texture(shadowMap, fs_in.TexCoords).r;
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

    vec3 lightSum = vec3(0.0);
    for (int i = 0; i < ssbo_Lights.length(); ++i) {
        switch(ssbo_Lights[i].type) {
            case 0: {
                //float shadow = 1.0f;
                //if (pointShadowMapId < 4) {
                    //shadow = PointShadowCalculation(fs_in.FragPosLightSpace.xyz);
                //}
                lightSum += CalcPointLight(ssbo_Lights[i]);
                //pointShadowMapId++;
                break;
            }
            case 1: {
                float shadow = DirShadowCalculation(fs_in.FragPosLightSpace);
                lightSum += CalcDirectionalLight(ssbo_Lights[i], shadow);
                break;
            }
            case 2: {
                //float shadow = SpotShadowCalculation(fs_in.FragPosLightSpace);
                lightSum += CalcSpotLight(ssbo_Lights[i]);
                break;
            }
            case 3: lightSum += CalcAmbientBoxLight(ssbo_Lights[i]);    break;
            case 4: lightSum += CalcAmbientSphereLight(ssbo_Lights[i]); break;
        }
    }

    vec3 vol = Volumetric(fs_in.FragPos, ubo_ViewPos, u_LightSpaceMatrix, shadowMap, vec3(0), vec3(255), 0.05f, 0.0f);


    FRAGMENT_COLOR = vec4(lightSum * vol, g_DiffuseTexel.a);

    if(fogParams.isEnabled) {
      //volumetric

	  //vec2 _fogCoord = vec2(0,0);
      //_fogCoord.x = g_TexCoords.x * 0.015625 + ubo_Time / 10.0 * 0.015625;
      //_fogCoord.y = g_TexCoords.y;
      //
      //
      //vec4 f = texture(u_Noise, vec2(_fogCoord.x, _fogCoord.y));
      float fogCoordinate = abs(fs_in.EyeSpacePosition.z / fs_in.EyeSpacePosition.w);
      //FRAGMENT_COLOR = mix(FRAGMENT_COLOR, vec4(fogParams.color, 1.0)*f, getFogFactor(fogParams, fogCoordinate));
      
      FRAGMENT_COLOR = mix(FRAGMENT_COLOR, vec4(fogParams.color, 1.0), getFogFactor(fogParams, fogCoordinate));
    }
}