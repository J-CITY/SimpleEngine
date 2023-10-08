#shader vertex
#version 460 core

layout (location = 0) in vec3 geo_Pos;
layout (location = 1) in vec2 geo_TexCoords;
layout (location = 2) in vec3 geo_Normal;
layout (location = 3) in vec3 geo_Tangent;
layout (location = 4) in vec3 geo_Bitangent;

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
    vec3        NormalOffset;
} vs_out;

uniform float u_Amplitude       = 1.0;
uniform float u_Frequency       = 1.0;
uniform float u_Disparity       = 1.0;
uniform float u_NormalsScale    = 1.0;

float timeScale = 0.7;

void main()
{
    vs_out.TBN = mat3
    (
        normalize(vec3(ubo_Model * vec4(geo_Tangent,   0.0))),
        normalize(vec3(ubo_Model * vec4(geo_Bitangent, 0.0))),
        normalize(vec3(ubo_Model * vec4(geo_Normal,    0.0)))
    );

    mat3 TBNi = transpose(vs_out.TBN);

    vec3 offset = vec3(0, sin((ubo_Time * timeScale + geo_Pos.x * u_Disparity + ubo_Time * timeScale + geo_Pos.z * u_Disparity) * u_Frequency) * u_Amplitude * cos(ubo_Time * timeScale * 0.05 * geo_Pos.z), 0);
    vs_out.NormalOffset = vec3(offset.y, 0, offset.y) * 5.0;

    vs_out.FragPos          = vec3(ubo_Model * vec4(geo_Pos + offset, 1.0));
    vs_out.Normal           = normalize(mat3(transpose(inverse(ubo_Model))) * (geo_Normal));
    vs_out.TexCoords        = geo_TexCoords;
    vs_out.TangentViewPos   = TBNi * ubo_ViewPos;
    vs_out.TangentFragPos   = TBNi * vs_out.FragPos;

    gl_Position = ubo_Projection * ubo_View * vec4(vs_out.FragPos, 1.0);
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
    vec3        NormalOffset;
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

/* Global variables */
vec3 g_Normal;
vec2 g_TexCoords;
vec3 g_ViewDir;
vec4 g_DiffuseTexel;
vec4 g_SpecularTexel;
vec4 g_HeightTexel;
vec4 g_NormalTexel;

out vec4 FRAGMENT_COLOR;


float timeScale = 0.7;

vec3 UnPack(float p_Target)
{
    return vec3
    (
        float((uint(p_Target) >> 24) & 0xff)    * 0.003921568627451,
        float((uint(p_Target) >> 16) & 0xff)    * 0.003921568627451,
        float((uint(p_Target) >> 8) & 0xff)     * 0.003921568627451
    );
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
    return g_TexCoords - vec2(parallax.x, 1 - parallax.y);
}

vec3 Phong(vec3 p_LightDir, vec3 p_Diffuse, vec3 p_Specular, float p_Luminosity)
{
    const vec3  reflectDir          = reflect(-p_LightDir, g_Normal);
    const float diffuseCoefficient  = max(dot(g_Normal, p_LightDir), 0.0);
    const float specularCoefficient = pow(max(dot(g_ViewDir, reflectDir), 0.0), u_Shininess);

    return p_Diffuse * g_DiffuseTexel.rgb * diffuseCoefficient * p_Luminosity 
    + ((p_Luminosity > 0.0) ? (p_Specular * g_SpecularTexel.rgb * specularCoefficient * p_Luminosity) : vec3(0.0));
}

vec3 BlinnPhong(vec3 p_LightDir, vec3 p_LightColor, float p_Luminosity)
{
    const vec3  halfwayDir          = normalize(p_LightDir + g_ViewDir);
    const float diffuseCoefficient  = max(dot(g_Normal, p_LightDir), 0.0);
    const float specularCoefficient = pow(max(dot(g_Normal, halfwayDir), 0.0), u_Shininess * 2.0);

    return p_LightColor * g_DiffuseTexel.rgb * diffuseCoefficient * p_Luminosity 
    + ((p_Luminosity > 0.0) ? (p_LightColor * g_SpecularTexel.rgb * specularCoefficient * p_Luminosity) : vec3(0.0));
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
    /* Extract light information from light mat4 */
    const vec3 lightPosition  = vec3(p_Light.pos[0], p_Light.pos[1], p_Light.pos[2]);
    const vec3 lightColor     = vec3(p_Light.color[0], p_Light.color[1], p_Light.color[2]);
    const float intensity     = p_Light.intensity;

    const vec3  lightDirection  = normalize(lightPosition - fs_in.FragPos);
    const float luminosity      = LuminosityFromAttenuation(p_Light);

    return BlinnPhong(lightDirection, lightColor, intensity * luminosity);
}

vec3 CalcDirectionalLight(LightOGL p_Light)
{
    return BlinnPhong(-vec3(p_Light.forward[0], p_Light.forward[1], p_Light.forward[2]), 
        vec3(p_Light.color[0], p_Light.color[1], p_Light.color[2]), p_Light.intensity);
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
    
    return BlinnPhong(lightDirection, lightColor, intensity * spotIntensity * luminosity);
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

void main()
{
    g_ViewDir   = normalize(ubo_ViewPos - fs_in.FragPos);
    vec2 offset = vec2(ubo_Time * timeScale, ubo_Time * timeScale) * 0.1f;
    g_TexCoords = offset + vec2(mod(fs_in.TexCoords.x * u_TextureTiling.x, 1), mod(fs_in.TexCoords.y * u_TextureTiling.y, 1));

    /* Apply parallax mapping */
    if (u_HeightScale != 0)
        g_TexCoords = ParallaxMapping(normalize(fs_in.TangentViewPos - fs_in.TangentFragPos));

    /* Apply color mask */
    if (texture(u_MaskMap, g_TexCoords).r == 0.0)
        discard;

    g_DiffuseTexel     = textureLod(u_DiffuseMap,  g_TexCoords, 2)        * u_Diffuse;
    g_SpecularTexel    = textureLod(u_SpecularMap, g_TexCoords, 2)        * vec4(u_Specular, 1.0);

    if (u_EnableNormalMapping)
    {
        g_Normal = textureLod(u_NormalMap, g_TexCoords, 2).rgb + fs_in.NormalOffset;
        g_Normal = normalize(g_Normal * 2.0 - 1.0);   
        g_Normal = normalize(fs_in.TBN * g_Normal);
    }
    else
    {
        g_Normal = normalize(fs_in.Normal);
    }

    vec3 lightSum = vec3(0.0);

    for (uint i = 0; i < ssbo_Lights.length(); ++i)
    {
        if      (ssbo_Lights[i].type == 0.0)    lightSum += CalcPointLight          (ssbo_Lights[i]);
        else if (ssbo_Lights[i].type == 1.0)    lightSum += CalcDirectionalLight    (ssbo_Lights[i]);
        else if (ssbo_Lights[i].type == 2.0)    lightSum += CalcSpotLight           (ssbo_Lights[i]);
        else if (ssbo_Lights[i].type == 3.0)    lightSum += CalcAmbientBoxLight     (ssbo_Lights[i]);
        else if (ssbo_Lights[i].type == 4.0)    lightSum += CalcAmbientSphereLight  (ssbo_Lights[i]);
    }

    FRAGMENT_COLOR = vec4(lightSum, g_DiffuseTexel.a);
}