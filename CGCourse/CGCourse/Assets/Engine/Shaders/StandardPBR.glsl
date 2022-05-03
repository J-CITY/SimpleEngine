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
} vs_out;

void main()
{
    vs_out.TBN = mat3
    (
        normalize(vec3(ubo_Model * vec4(geo_Tangent,   0.0))),
        normalize(vec3(ubo_Model * vec4(geo_Bitangent, 0.0))),
        normalize(vec3(ubo_Model * vec4(geo_Normal,    0.0)))
    );

    mat3 TBNi = transpose(vs_out.TBN);

    vs_out.FragPos          = vec3(ubo_Model * vec4(geo_Pos, 1.0));
    vs_out.Normal           = normalize(mat3(transpose(inverse(ubo_Model))) * geo_Normal);
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
} fs_in;

/* Light information sent by the engine */
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

layout(std430, binding = 0) buffer LightSSBO
{
    LightOGL ssbo_Lights[];
};

out vec4 FRAGMENT_COLOR;

uniform sampler2D   u_AlbedoMap;
uniform sampler2D   u_MetallicMap;
uniform sampler2D   u_RoughnessMap;
uniform sampler2D   u_AmbientOcclusionMap;
uniform sampler2D   u_NormalMap;
uniform vec4        u_Albedo                = vec4(1.0);
uniform vec2        u_TextureTiling         = vec2(1.0, 1.0);
uniform vec2        u_TextureOffset         = vec2(0.0, 0.0);
uniform bool        u_EnableNormalMapping   = false;
uniform float       u_HeightScale           = 0.0;
uniform float       u_Metallic              = 1.0;
uniform float       u_Roughness             = 1.0;

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

vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
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

vec3 toVec3(float inv[3]) {
    return vec3(inv[0], inv[1], inv[2]);
}

float LuminosityFromAttenuation(mat4 p_Light)
{
    const vec3  lightPosition   = toVec3(p_Light.pos);
    const float constant        = p_Light.constant;
    const float linear          = p_Light.linear;
    const float quadratic       = p_Light.quadratic;

    const float distanceToLight = length(lightPosition - fs_in.FragPos);
    const float attenuation     = (constant + linear * distanceToLight + quadratic * (distanceToLight * distanceToLight));
    return 1.0 / attenuation;
}

vec3 CalcAmbientBoxLight(mat4 p_Light)
{
    const vec3  lightPosition   = toVec3(p_Light.pos);
    const vec3  lightColor      = toVec3(p_Light.color);
    const float intensity       = p_Light.intensity;
    const vec3  size            = vec3(p_Light.constant, p_Light.linear, p_Light.quadratic);

    return PointInAABB(fs_in.FragPos, lightPosition, size) ? lightColor * intensity : vec3(0.0);
}

vec3 CalcAmbientSphereLight(mat4 p_Light)
{
    const vec3  lightPosition   = toVec3(p_Light.pos);
    const vec3  lightColor      = toVec3(p_Light.color);
    const float intensity       = p_Light.intensity;
    const float radius          = p_Light.constant;

    return distance(lightPosition, fs_in.FragPos) <= radius ? lightColor * intensity : vec3(0.0);
}

void main()
{
    vec2 texCoords = u_TextureOffset + vec2(mod(fs_in.TexCoords.x * u_TextureTiling.x, 1), mod(fs_in.TexCoords.y * u_TextureTiling.y, 1));

    vec4 albedoRGBA     = texture(u_AlbedoMap, texCoords) * u_Albedo;
    vec3 albedo         = pow(albedoRGBA.rgb, vec3(2.2));
    float metallic      = texture(u_MetallicMap, texCoords).r * u_Metallic;
    float roughness     = texture(u_RoughnessMap, texCoords).r * u_Roughness;
    float ao            = texture(u_AmbientOcclusionMap, texCoords).r;
    vec3 normal;

    if (u_EnableNormalMapping)
    {
        normal = texture(u_NormalMap, texCoords).rgb;
        normal = normalize(normal * 2.0 - 1.0);   
        normal = normalize(fs_in.TBN * normal);
    }
    else
    {
        normal = normalize(fs_in.Normal);
    }

    vec3 N = normalize(normal);
    vec3 V = normalize(ubo_ViewPos - fs_in.FragPos);

    vec3 F0 = vec3(0.04); 
    F0 = mix(F0, albedo, metallic);
	           
    // reflectance equation
    vec3 Lo = vec3(0.0);
    vec3 ambientSum = vec3(0.0);

    for (int i = 0; i < ssbo_Lights.length(); ++i) 
    {
        if (ssbo_Lights[i].type == 3) {
            ambientSum += CalcAmbientBoxLight(ssbo_Lights[i]);
        }
        else if (ssbo_Lights[i].type == 4) {
            ambientSum += CalcAmbientSphereLight(ssbo_Lights[i]);
        }
        else {
            // calculate per-light radiance
            vec3 L = ssbo_Lights[i].constant == 1 ? -toVec3(ssbo_Lights.forward) : normalize(toVec3(ssbo_Lights[i]pos) - fs_in.FragPos);
            vec3 H = normalize(V + L);
            float distance    = length(toVec3(ssbo_Lights[i].pos)- fs_in.FragPos);
            float lightCoeff = 0.0;

            switch(ssbo_Lights[i].type) {
                case 0:
                    lightCoeff = LuminosityFromAttenuation(ssbo_Lights[i]) * ssbo_Lights[i].intensity;
                    break;

                case 1:
                    lightCoeff = ssbo_Lights[i].intensity;
                    break;

                case 2:
                    const vec3  lightForward    = toVec3(ssbo_Lights[i]forward);
                    const float cutOff          = cos(radians(ssbo_Lights[i].cutoff));
                    const float outerCutOff     = cos(radians(ssbo_Lights[i].cutoff + ssbo_Lights[i].outerCutoff));

                    const vec3  lightDirection  = normalize(toVec3(ssbo_Lights[i].pos) - fs_in.FragPos);
                    const float luminosity      = LuminosityFromAttenuation(ssbo_Lights[i]);

                    /* Calculate the spot intensity */
                    const float theta           = dot(lightDirection, normalize(-lightForward)); 
                    const float epsilon         = cutOff - outerCutOff;
                    const float spotIntensity   = clamp((theta - outerCutOff) / epsilon, 0.0, 1.0);

                    lightCoeff = luminosity * spotIntensity * ssbo_Lights[i].intensity;
                    break;
            }

            vec3 radiance = toVec3(ssbo_Lights[i].color) * lightCoeff;        
            
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
            Lo += (kD * albedo / PI + specular) * radiance * NdotL; 
        }
    }

    vec3 ambient = ambientSum * albedo * ao;
    vec3 color = ambient + Lo;
	
    color = color / (color + vec3(1.0));
    color = pow(color, vec3(1.0/2.2));  
   
    FRAGMENT_COLOR = vec4(color, albedoRGBA.a);
}