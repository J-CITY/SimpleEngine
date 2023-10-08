#version 460 core

layout (std140, binding = 0) uniform EngineUBO {
    mat4    View;
    mat4    Projection;
    vec3    ViewPos;
    float   Time;
    vec2    ViewportSize;
} engine_UBO;

struct PushModel {
    mat4 model;
};
uniform PushModel engine_Model;

/* Information passed from the fragment shader */
in VS_OUT
{
    vec3        FragPos;
    vec3        Normal;
    vec2        TexCoords;
    mat3        TBN;
    flat vec3   TangentViewPos;
    vec3        TangentFragPos;
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

layout(std430, binding = 0) buffer LightSSBO {
    LightOGL ssbo_Lights[];
};

/* Uniforms (Tweakable from the material editor) */
//uniform vec2        u_TextureTiling           = vec2(1.0, 1.0);
//uniform vec2        u_TextureOffset           = vec2(0.0, 0.0);
//uniform vec4        u_Diffuse                 = vec4(1.0, 1.0, 1.0, 1.0);
//uniform vec3        u_Specular                = vec3(1.0, 1.0, 1.0);
//uniform float       u_Shininess               = 100.0;
//uniform float       u_HeightScale             = 0.0;
//uniform bool        u_EnableNormalMapping     = false;


layout(std140, binding = 1) uniform DataUBO {
    vec2        u_TextureTiling;
    vec2        u_TextureOffset;
    vec4        u_Diffuse;
    vec3        u_Specular;
    float       u_Shininess;
    float       u_HeightScale;
    bool        u_EnableNormalMapping;
} data_UBO;


uniform sampler2D   u_DiffuseMap;
uniform sampler2D   u_SpecularMap;
uniform sampler2D   u_NormalMap;
uniform sampler2D   u_HeightMap;


/* Global variables */
vec3 g_Normal;
vec2 g_TexCoords;
vec3 g_ViewDir;
vec4 g_DiffuseTexel;
vec4 g_SpecularTexel;
vec4 g_HeightTexel;
vec4 g_NormalTexel;

//#include "lib/blinnPhongLight.glsl"
bool PointInAABB(vec3 p_Point, vec3 p_AabbCenter, vec3 p_AabbHalfSize) {
    return
    (
        p_Point.x > p_AabbCenter.x - p_AabbHalfSize.x && p_Point.x < p_AabbCenter.x + p_AabbHalfSize.x &&
        p_Point.y > p_AabbCenter.y - p_AabbHalfSize.y && p_Point.y < p_AabbCenter.y + p_AabbHalfSize.y &&
        p_Point.z > p_AabbCenter.z - p_AabbHalfSize.z && p_Point.z < p_AabbCenter.z + p_AabbHalfSize.z
    );
}

vec3 BlinnPhong(vec3 p_LightDir, vec3 p_LightColor, float p_Luminosity, float shadow)
{
    const vec3  halfwayDir          = normalize(p_LightDir + g_ViewDir);
    const float diffuseCoefficient  = max(dot(g_Normal, p_LightDir), 0.0);
    const float specularCoefficient = pow(max(dot(g_Normal, halfwayDir), 0.0), data_UBO.u_Shininess * 2.0);

    return (1.0-shadow) * (p_LightColor * g_DiffuseTexel.rgb * diffuseCoefficient * p_Luminosity + 
        ((p_Luminosity > 0.0) ? (p_LightColor * g_SpecularTexel.rgb * specularCoefficient * p_Luminosity) : vec3(0.0)));
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
vec3 CalcPointLight(LightOGL p_Light, float shadow)
{
    const vec3 lightPosition  = vec3(p_Light.pos[0], p_Light.pos[1], p_Light.pos[2]);
    const vec3 lightColor     = vec3(p_Light.color[0], p_Light.color[1], p_Light.color[2]);
    const float intensity     = p_Light.intensity;

    const vec3  lightDirection  = normalize(lightPosition - fs_in.FragPos);
    const float luminosity      = LuminosityFromAttenuation(p_Light);

    return BlinnPhong(lightDirection, lightColor, intensity * luminosity, shadow);
}

vec3 CalcDirectionalLight(LightOGL p_Light, float shadow)
{
    return BlinnPhong(-vec3(p_Light.forward[0], p_Light.forward[1], p_Light.forward[2]), 
        vec3(p_Light.color[0], p_Light.color[1], p_Light.color[2]), p_Light.intensity, shadow);
}

vec3 CalcSpotLight(LightOGL p_Light, float shadow)
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
    
    return BlinnPhong(lightDirection, lightColor, intensity * spotIntensity * luminosity, shadow);
}

vec3 CalcAmbientBoxLight(LightOGL p_Light, bool useAO, float ao) {
    const vec3  lightPosition   = vec3(p_Light.pos[0], p_Light.pos[1], p_Light.pos[2]);
    const vec3  lightColor      = vec3(p_Light.color[0], p_Light.color[1], p_Light.color[2]);
    float intensity             = p_Light.intensity;
    if (useAO) {
        intensity *= ao;
    }
    const vec3  size            = vec3(p_Light.constant, p_Light.linear, p_Light.quadratic);

    return PointInAABB(fs_in.FragPos, lightPosition, size) ? g_DiffuseTexel.rgb * lightColor * intensity : vec3(0.0);
}

vec3 CalcAmbientSphereLight(LightOGL p_Light, bool useAO, float ao) {
    const vec3  lightPosition   = vec3(p_Light.pos[0], p_Light.pos[1], p_Light.pos[2]);
    const vec3  lightColor      = vec3(p_Light.color[0], p_Light.color[1], p_Light.color[2]);
    float intensity       = p_Light.intensity;
    if (useAO) {
        intensity *= ao;
    }
    const float radius          = p_Light.constant;

    return distance(lightPosition, fs_in.FragPos) <= radius ? g_DiffuseTexel.rgb * lightColor * intensity : vec3(0.0);
}
//end #include "lib/blinnPhongLight.glsl"

out vec4 FRAGMENT_COLOR;

vec2 ParallaxMapping(vec3 p_ViewDir) {
    const vec2 parallax = p_ViewDir.xy * data_UBO.u_HeightScale * texture(u_HeightMap, g_TexCoords).r;
    return g_TexCoords - vec2(parallax.x, 1.0 - parallax.y);
}

void main() {
    g_TexCoords = data_UBO.u_TextureOffset + vec2(mod(fs_in.TexCoords.x * data_UBO.u_TextureTiling.x, 1), mod(fs_in.TexCoords.y * data_UBO.u_TextureTiling.y, 1));

    /* Apply parallax mapping */
    if (data_UBO.u_HeightScale > 0)
        g_TexCoords = ParallaxMapping(normalize(fs_in.TangentViewPos - fs_in.TangentFragPos));

    /* Apply color mask */
    g_ViewDir           = normalize(engine_UBO.ViewPos - fs_in.FragPos);
    g_DiffuseTexel      = texture(u_DiffuseMap,  g_TexCoords) * data_UBO.u_Diffuse;
    g_SpecularTexel     = texture(u_SpecularMap, g_TexCoords) * vec4(data_UBO.u_Specular, 1.0);
    if (data_UBO.u_EnableNormalMapping) {
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
                lightSum += CalcPointLight(ssbo_Lights[i], shadow);
                break;
            }
            case 1: {
                float shadow = 0.0f;
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
}
