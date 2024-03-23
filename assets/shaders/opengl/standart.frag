#version 120

//in
varying vec2 out_TexCoords;
varying mat3 out_TBN;
varying vec3 out_FragPos;
varying vec3 out_Normal;
varying vec3 out_TangentViewPos;
varying vec3 out_TangentFragPos;
varying vec3 out_ViewPos;

//uniforms
uniform vec2 u_TextureTiling;
uniform vec2 u_TextureOffset;
uniform vec4 u_Diffuse;
uniform vec3 u_Specular;
uniform float u_Shininess;
uniform float u_HeightScale;
uniform bool u_EnableNormalMapping;
uniform sampler2D u_DiffuseMap;
uniform sampler2D u_SpecularMap;
uniform sampler2D u_NormalMap;
uniform sampler2D u_HeightMap;

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
    float radius;
};

const int MAX_LIGHTS = 10;
uniform LightOGL engine_Lights[10];
uniform int engine_LightCount;

//Global variables
vec3 g_Normal;
vec3 g_FragPos;
vec2 g_TexCoords;
vec3 g_ViewDir;
vec4 g_DiffuseTexel;
vec4 g_SpecularTexel;
float g_Shininess;

vec2 ParallaxMapping(vec3 p_ViewDir) {
    vec2 parallax = p_ViewDir.xy * u_HeightScale * texture2D(u_HeightMap, g_TexCoords).r;
    return g_TexCoords - vec2(parallax.x, 1.0 - parallax.y);
}

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
    vec3  halfwayDir          = normalize(p_LightDir + g_ViewDir);
    float diffuseCoefficient  = max(dot(g_Normal, p_LightDir), 0.0);
    float specularCoefficient = pow(max(dot(g_Normal, halfwayDir), 0.0), g_Shininess * 2.0);

    return (1.0-shadow) * (p_LightColor * g_DiffuseTexel.rgb * diffuseCoefficient * p_Luminosity + 
        ((p_Luminosity > 0.0) ? (p_LightColor * g_SpecularTexel.rgb * specularCoefficient * p_Luminosity) : vec3(0.0)));
}

float LuminosityFromAttenuation(LightOGL p_Light)
{
    vec3  lightPosition   = vec3(p_Light.pos[0], p_Light.pos[1], p_Light.pos[2]);
    float constant        = p_Light.constant;
    float linear          = p_Light.linear;
    float quadratic       = p_Light.quadratic;

    float distanceToLight = length(lightPosition - out_FragPos);
    float attenuation     = (constant + linear * distanceToLight + quadratic * (distanceToLight * distanceToLight));
    return 1.0 / attenuation;
}
vec3 CalcPointLight(LightOGL p_Light, float shadow)
{
    vec3 lightPosition  = vec3(p_Light.pos[0], p_Light.pos[1], p_Light.pos[2]);
    vec3 lightColor     = vec3(p_Light.color[0], p_Light.color[1], p_Light.color[2]);
    float intensity     = p_Light.intensity;

    vec3  lightDirection  = normalize(lightPosition - out_FragPos);
    float luminosity      = LuminosityFromAttenuation(p_Light);

    return BlinnPhong(lightDirection, lightColor, intensity * luminosity, shadow);
}

vec3 CalcDirectionalLight(LightOGL p_Light, float shadow)
{
    return BlinnPhong(-vec3(p_Light.forward[0], p_Light.forward[1], p_Light.forward[2]), 
        vec3(p_Light.color[0], p_Light.color[1], p_Light.color[2]), p_Light.intensity, shadow);
}

vec3 CalcSpotLight(LightOGL p_Light, float shadow)
{
    vec3  lightPosition   = vec3(p_Light.pos[0], p_Light.pos[1], p_Light.pos[2]);
    vec3  lightForward    = vec3(p_Light.forward[0], p_Light.forward[1], p_Light.forward[2]);
    vec3  lightColor      = vec3(p_Light.color[0], p_Light.color[1], p_Light.color[2]);
    float intensity       = p_Light.intensity;
    float cutOff          = cos(radians(p_Light.cutoff));
    float outerCutOff     = cos(radians(p_Light.cutoff + p_Light.outerCutoff));

    vec3  lightDirection  = normalize(lightPosition - out_FragPos);
    float luminosity      = LuminosityFromAttenuation(p_Light);

    float theta           = dot(lightDirection, normalize(-lightForward)); 
    float epsilon         = cutOff - outerCutOff;
    float spotIntensity   = clamp((theta - outerCutOff) / epsilon, 0.0, 1.0);
    
    return BlinnPhong(lightDirection, lightColor, intensity * spotIntensity * luminosity, shadow);
}

vec3 CalcAmbientBoxLight(LightOGL p_Light, bool useAO, float ao) {
    vec3  lightPosition   = vec3(p_Light.pos[0], p_Light.pos[1], p_Light.pos[2]);
    vec3  lightColor      = vec3(p_Light.color[0], p_Light.color[1], p_Light.color[2]);
    float intensity             = p_Light.intensity;
    if (useAO) {
        intensity *= ao;
    }
    vec3  size            = vec3(p_Light.constant, p_Light.linear, p_Light.quadratic);

    return PointInAABB(out_FragPos, lightPosition, size) ? g_DiffuseTexel.rgb * lightColor * intensity : vec3(0.0);
}

vec3 CalcAmbientSphereLight(LightOGL p_Light, bool useAO, float ao) {
    vec3  lightPosition   = vec3(p_Light.pos[0], p_Light.pos[1], p_Light.pos[2]);
    vec3  lightColor      = vec3(p_Light.color[0], p_Light.color[1], p_Light.color[2]);
    float intensity       = p_Light.intensity;
    if (useAO) {
        intensity *= ao;
    }
    float radius          = p_Light.constant;

    return distance(lightPosition, out_FragPos) <= radius ? g_DiffuseTexel.rgb * lightColor * intensity : vec3(0.0);
}

void main() {
    g_TexCoords = out_TexCoords;//u_TextureOffset + vec2(mod(out_TexCoords.x * u_TextureTiling.x, 1), mod(out_TexCoords.y * u_TextureTiling.y, 1));

    if (u_HeightScale > 0.0)
        g_TexCoords = ParallaxMapping(normalize(out_TangentViewPos - out_TangentFragPos));

    g_ViewDir           = normalize(out_ViewPos - out_FragPos);
    g_DiffuseTexel      = texture2D(u_DiffuseMap,  g_TexCoords) * u_Diffuse;
    g_SpecularTexel     = texture2D(u_SpecularMap, g_TexCoords) * vec4(u_Specular, 1.0);
    if (u_EnableNormalMapping) {
        g_Normal = texture2D(u_NormalMap, g_TexCoords).rgb;
        g_Normal = normalize(g_Normal * 2.0 - 1.0);
        g_Normal = normalize(out_TBN * g_Normal);
    }
    else {
        g_Normal = normalize(out_Normal);
    }

    g_Shininess = u_Shininess;
    g_FragPos = out_FragPos;

    vec3 lightSum = vec3(0.0);
    for (int i = 0; i < MAX_LIGHTS; ++i) {
        if (i >= engine_LightCount) {
            break;
        }
        if (engine_Lights[i].type == 0) {
            lightSum += CalcPointLight(engine_Lights[i], 0.0);
        }
        else if (engine_Lights[i].type == 1) {
            lightSum += CalcDirectionalLight(engine_Lights[i], 0.0);
        }
        else if (engine_Lights[i].type == 2) {
            lightSum += CalcSpotLight(engine_Lights[i], 0.0);
        }
        else if (engine_Lights[i].type == 3) {
            lightSum += CalcAmbientBoxLight(engine_Lights[i], false, 0.0);
        }
        else if (engine_Lights[i].type == 4) {
            lightSum += CalcAmbientSphereLight(engine_Lights[i], false, 0.0);
        }
    }
    gl_FragColor = vec4(lightSum, g_DiffuseTexel.a);
    //gl_FragColor = texture2D(u_DiffuseMap, out_TexCoords);
}
