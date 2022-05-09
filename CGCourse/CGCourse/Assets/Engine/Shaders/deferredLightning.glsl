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
layout(std430, binding = 0) buffer LightSSBO
{
    LightOGL ssbo_Lights[];
};

in vec2 TexCoords;

/* Uniforms (Tweakable from the material editor) */
uniform vec2        u_TextureTiling           = vec2(1.0, 1.0);
uniform vec2        u_TextureOffset           = vec2(0.0, 0.0);
uniform vec4        u_Diffuse                 = vec4(1.0, 1.0, 1.0, 1.0);
uniform vec3        u_Specular                = vec3(1.0, 1.0, 1.0);
uniform float       u_Shininess               = 100.0;
uniform float       u_HeightScale             = 0.0;
uniform bool        u_EnableNormalMapping     = false;

uniform sampler2D   u_PositionMap;
uniform sampler2D   u_NormalMap;
uniform sampler2D   u_DiffuseMap;
uniform sampler2D   u_SSAO;
uniform sampler2D   u_SpecularMap;
uniform sampler2D   u_HeightMap;
uniform sampler2D   u_MaskMap;

uniform samplerCube irradianceMap;
uniform samplerCube prefilterMap;
uniform sampler2D   brdfLUT;

/* Global variables */
vec3 g_Normal;
vec2 g_TexCoords;
vec3 g_ViewDir;
vec4 g_DiffuseTexel;
vec4 g_SpecularTexel;
vec4 g_HeightTexel;
vec4 g_NormalTexel;
vec3 g_FragPos;
float g_AmbientOcclusion;

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

    const float distanceToLight = length(lightPosition - g_FragPos);
    const float attenuation     = (constant + linear * distanceToLight + quadratic * (distanceToLight * distanceToLight));
    return 1.0 / attenuation;
}
vec3 CalcPointLight(LightOGL p_Light)
{
    const vec3 lightPosition  = vec3(p_Light.pos[0], p_Light.pos[1], p_Light.pos[2]);
    const vec3 lightColor     = vec3(p_Light.color[0], p_Light.color[1], p_Light.color[2]);
    const float intensity     = p_Light.intensity;

    const vec3  lightDirection  = normalize(lightPosition - g_FragPos);
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

    const vec3  lightDirection  = normalize(lightPosition - g_FragPos);
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

    return PointInAABB(g_FragPos, lightPosition, size) ? g_DiffuseTexel.rgb * lightColor * intensity : vec3(0.0);
}

vec3 CalcAmbientSphereLight(LightOGL p_Light)
{
    const vec3  lightPosition   = vec3(p_Light.pos[0], p_Light.pos[1], p_Light.pos[2]);
    const vec3  lightColor      = vec3(p_Light.color[0], p_Light.color[1], p_Light.color[2]);
    const float intensity       = p_Light.intensity * g_AmbientOcclusion; //for ssao
    const float radius          = p_Light.constant;

    return distance(lightPosition, g_FragPos) <= radius ? g_DiffuseTexel.rgb * lightColor * intensity : vec3(0.0);
}

vec3 gridSamplingDisk[20] = vec3[] (
   vec3(1, 1,  1), vec3( 1, -1,  1), vec3(-1, -1,  1), vec3(-1, 1,  1), 
   vec3(1, 1, -1), vec3( 1, -1, -1), vec3(-1, -1, -1), vec3(-1, 1, -1),
   vec3(1, 1,  0), vec3( 1, -1,  0), vec3(-1, -1,  0), vec3(-1, 1,  0),
   vec3(1, 0,  1), vec3(-1,  0,  1), vec3( 1,  0, -1), vec3(-1, 0, -1),
   vec3(0, 1,  1), vec3( 0, -1,  1), vec3( 0, -1, -1), vec3( 0, 1, -1)
);
/*
float ShadowCalculation(vec3 fragPos) {
    // Получаем вектор между положением фрагмента и положением источника света
    vec3 fragToLight = fragPos - lightPos;
    // Теперь получим текущую линейную глубину как длину между фрагментом и положением источника света
    float currentDepth = length(fragToLight);

    float shadow = 0.0;
    float bias = 0.15;
    int samples = 20;
    float viewDistance = length(viewPos - fragPos);
    float diskRadius = (1.0 + (viewDistance / far_plane)) / 25.0;
    for(int i = 0; i < samples; ++i)
    {
        float closestDepth = texture(depthMap, fragToLight + gridSamplingDisk[i] * diskRadius).r;
        closestDepth *= far_plane; 
        if(currentDepth - bias > closestDepth)
            shadow += 1.0;
    }
    shadow /= float(samples);
    return shadow;
}
*/
vec3 getNormalFromMap() {
    vec3 tangentNormal = texture(u_NormalMap, TexCoords).xyz * 2.0 - 1.0;

    vec3 Q1  = dFdx(g_FragPos);
    vec3 Q2  = dFdy(g_FragPos);
    vec2 st1 = dFdx(TexCoords);
    vec2 st2 = dFdy(TexCoords);

    vec3 N   = normalize(g_Normal);
    vec3 T  = normalize(Q1*st2.t - Q2*st1.t);
    vec3 B  = -normalize(cross(N, T));
    mat3 TBN = mat3(T, B, N);

    return normalize(TBN * tangentNormal);
}

vec3 fresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness) {
    return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(1.0 - cosTheta, 5.0);
}   

void main()
{
    vec3 FragPos = texture(u_PositionMap, TexCoords).rgb;
    vec3 Normal = texture(u_NormalMap, TexCoords).rgb;
    vec3 Diffuse = texture(u_DiffuseMap, TexCoords).rgb;
    float Specular = texture(u_DiffuseMap, TexCoords).a;

    fogParams.color = vec3(0.2);
    fogParams.density = 0.01;
    fogParams.equation = 2;
    fogParams.isEnabled = true;
	fogParams.linearStart=0.0;
	fogParams.linearEnd=0.0;

    g_TexCoords = TexCoords;
    g_FragPos = FragPos;
    g_ViewDir           = normalize(ubo_ViewPos - FragPos);
    g_DiffuseTexel      = texture(u_DiffuseMap,  TexCoords) * u_Diffuse;
    g_SpecularTexel     = texture(u_DiffuseMap, TexCoords).a * vec4(u_Specular, 1.0);
    g_Normal            = texture(u_NormalMap, TexCoords).rgb;
    g_AmbientOcclusion = texture(u_SSAO, TexCoords).r;

    //ibl
    vec3 N = getNormalFromMap();
    vec3 V = normalize(ubo_ViewPos - g_FragPos);
    vec3 R = reflect(-V, N); 
    vec3 irradiance = texture(irradianceMap, N).rgb;
    vec3 diffuse = irradiance * g_DiffuseTexel.xyz;
    float roughness = 1.0; //take from tex pbr
    float metallic = 1.0; //take from tex pbr
    float ao = 1.0; //take from tex pbr
    vec3 F0 = vec3(0.04); 
    F0 = mix(F0, g_DiffuseTexel.xyz, metallic);
    vec3 F = fresnelSchlickRoughness(max(dot(N, V), 0.0), F0, roughness);
    vec3 kS = F;
    vec3 kD = 1.0 - kS;
    kD *= 1.0 - metallic;	
    // Производим выборки из префильтрованной карты LUT-текстуры BRDF и затем объединяем их вместе в соответствии с аппроксимацией разделенной суммы, чтобы получить зеркальную часть IBL
    const float MAX_REFLECTION_LOD = 4.0;
    vec3 prefilteredColor = textureLod(prefilterMap, R,  roughness * MAX_REFLECTION_LOD).rgb;    
    vec2 brdf  = texture(brdfLUT, vec2(max(dot(N, V), 0.0), roughness)).rg;
    vec3 specular = prefilteredColor * (F * brdf.x + brdf.y);
    vec3 ambient = (kD * diffuse + specular) * ao;
    //ibl end

    vec3 lightSum = vec3(0.0);
    for (int i = 0; i < ssbo_Lights.length(); ++i) {
        switch(ssbo_Lights[i].type) {
            case 0: {
                lightSum += CalcPointLight(ssbo_Lights[i]);
                break;
            }
            case 1: {
                //float shadow = DirShadowCalculation(fs_in.FragPosLightSpace);
                lightSum += CalcDirectionalLight(ssbo_Lights[i], 1.0f);
                
                break;
            }
            case 2: lightSum += CalcSpotLight(ssbo_Lights[i]);          break;
            case 3: lightSum += CalcAmbientBoxLight(ssbo_Lights[i]);    break;
            case 4: lightSum += CalcAmbientSphereLight(ssbo_Lights[i]); break;
        }
    }

    //ibl
    lightSum += ambient;


    FRAGMENT_COLOR = vec4(lightSum, g_DiffuseTexel.a);

    //if(fogParams.isEnabled) {
    //  float fogCoordinate = abs(fs_in.EyeSpacePosition.z / fs_in.EyeSpacePosition.w);
    //  FRAGMENT_COLOR = mix(FRAGMENT_COLOR, vec4(fogParams.color, 1.0), getFogFactor(fogParams, fogCoordinate));
    //}
}