
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
    const float specularCoefficient = pow(max(dot(g_Normal, halfwayDir), 0.0), g_Shininess * 2.0);

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
    if (distance(lightPosition, fs_in.FragPos) <= p_Light.radius) {
        return vec3(0);
    }
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
    if (distance(lightPosition, fs_in.FragPos) <= p_Light.radius) {
        return vec3(0);
    }
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