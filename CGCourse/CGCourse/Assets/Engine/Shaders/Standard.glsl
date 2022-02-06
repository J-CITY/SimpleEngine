#shader vertex
#version 460 core

layout (location = 0) in vec3 geo_Pos;
layout (location = 1) in vec2 geo_TexCoords;
layout (location = 2) in vec3 geo_Normal;
layout (location = 3) in vec3 geo_Tangent;
layout (location = 4) in vec3 geo_Bitangent;

layout(location = 5) in ivec4 boneIds;
layout(location = 6) in vec4 weights;

const int MAX_BONES = 100;
const int MAX_BONE_INFLUENCE = 4;
uniform mat4 finalBonesMatrices[MAX_BONES];
uniform bool useBone;
uniform mat4 lightSpaceMatrix;

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
} vs_out;
int ub=1;

void main() {
    vec4 totalPosition = vec4(0.0f);
    if (useBone) {
        for(int i = 0; i < MAX_BONE_INFLUENCE; i++) {
            if(int(boneIds[i]) == -1) 
                continue;
            if(int(boneIds[i]) >=MAX_BONES) {
                totalPosition = vec4(geo_Pos,1.0f);
                break;
            }
            vec4 localPosition = (finalBonesMatrices[int(boneIds[i])]) * vec4(geo_Pos,1.0f);
            totalPosition += localPosition * weights[i];
        }
    }
    vs_out.TBN = mat3(
        normalize(vec3(ubo_Model * vec4(geo_Tangent,   0.0))),
        normalize(vec3(ubo_Model * vec4(geo_Bitangent, 0.0))),
        normalize(vec3(ubo_Model * vec4(geo_Normal,    0.0)))
    );

    mat3 TBNi = transpose(vs_out.TBN);

    if (useBone) {
        vs_out.FragPos          = vec3(ubo_Model * totalPosition);
    }
    else {
        vs_out.FragPos          = vec3(ubo_Model * vec4(geo_Pos, 1.0));
    }
    vs_out.Normal           = normalize(mat3(transpose(inverse(ubo_Model))) * geo_Normal);
    vs_out.TexCoords        = geo_TexCoords;
    vs_out.TangentViewPos   = TBNi * ubo_ViewPos;
    vs_out.TangentFragPos   = TBNi * vs_out.FragPos;
    vs_out.FragPosLightSpace = lightSpaceMatrix * vec4(vs_out.FragPos, 1.0);

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

uniform sampler2D shadowMap;
uniform vec3 lightPos;

/* Global variables */
vec3 g_Normal;
vec2 g_TexCoords;
vec3 g_ViewDir;
vec4 g_DiffuseTexel;
vec4 g_SpecularTexel;
vec4 g_HeightTexel;
vec4 g_NormalTexel;

out vec4 FRAGMENT_COLOR;



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


float ShadowCalculation(vec4 fragPosLightSpace)
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

void main()
{
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
    if (u_EnableNormalMapping)
    {
        g_Normal = texture(u_NormalMap, g_TexCoords).rgb;
        g_Normal = normalize(g_Normal * 2.0 - 1.0);
        g_Normal = normalize(fs_in.TBN * g_Normal);
    }
    else
    {
        g_Normal = normalize(fs_in.Normal);
    }
    vec3 lightSum = vec3(0.0);
    for (int i = 0; i < ssbo_Lights.length(); ++i)
    {
        switch(ssbo_Lights[i].type)
        {
            case 0: {
                lightSum += CalcPointLight(ssbo_Lights[i]);
                break;
            }
            case 1: {
                float shadow = ShadowCalculation(fs_in.FragPosLightSpace);
                lightSum += CalcDirectionalLight(ssbo_Lights[i], shadow);
                
                break;
            }
            case 2: lightSum += CalcSpotLight(ssbo_Lights[i]);          break;
            case 3: lightSum += CalcAmbientBoxLight(ssbo_Lights[i]);    break;
            case 4: lightSum += CalcAmbientSphereLight(ssbo_Lights[i]); break;
        }
    }
    FRAGMENT_COLOR = vec4(lightSum, g_DiffuseTexel.a);
}