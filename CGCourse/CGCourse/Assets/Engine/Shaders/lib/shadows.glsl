

float DirShadowCalculation(vec4 fragPosLightSpace)
{
    // Выполняем деление перспективы
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
	
    // Трансформируем в диапазон [0,1]
    projCoords = projCoords * 0.5 + 0.5;
	
    // Получаем наиболее близкое значение глубины исходя из перспективы глазами источника света (используя в диапазон [0,1] fragPosLight в качестве координат)
    float closestDepth = texture(u_engine_ShadowMap, projCoords.xy).r; 
	
    // Получаем глубину текущего фрагмента исходя из перспективы глазами источника света
    float currentDepth = projCoords.z;
	
    // Вычисляем смещение (на основе разрешения карты глубины и наклона)
    vec3 normal = normalize(fs_in.Normal);
    vec3 lightDir = normalize(u_engine_LightPos - fs_in.FragPos);
    float bias = max(0.05 * (1.0 - dot(normal, lightDir)), 0.005);
	
    // Проверка нахождения текущего фрагмента в тени
    // float shadow = currentDepth - bias > closestDepth  ? 1.0 : 0.0;
	
    // PCF
    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(u_engine_ShadowMap, 0);
    for(int x = -1; x <= 1; ++x)
    {
        for(int y = -1; y <= 1; ++y)
        {
            float pcfDepth = texture(u_engine_ShadowMap, projCoords.xy + vec2(x, y) * texelSize).r; 
            shadow += currentDepth - bias > pcfDepth  ? 1.0 : 0.0;        
        }    
    }
    shadow /= 9.0;
    
    // Оставляем значение тени на уровне 0.0 за границей дальней плоскости пирамиды видимости глазами источника света
    if(projCoords.z > 1.0)
        shadow = 0.0;
        
    return shadow;
}

float LinearizeDepth(float depth, float near, float far) 
{
    float z = depth * 2.0 - 1.0; // обратно к NDC 
    return (2.0 * near * far) / (far + near - z * (far - near));	
}

float SpotShadowCalculation(vec4 fragPosLightSpace) {
    // Выполняем деление перспективы
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
	
    // Трансформируем в диапазон [0,1]
    projCoords = projCoords * 0.5 + 0.5;
	
    // Получаем наиболее близкое значение глубины исходя из перспективы глазами источника света (используя в диапазон [0,1] fragPosLight в качестве координат)
    float closestDepth = texture(u_engine_SpotLightShadowMap, projCoords.xy).r; 
	closestDepth = LinearizeDepth(closestDepth, u_engine_SpotNearPlane, u_engine_SpotFarPlane) / u_engine_SpotFarPlane; 
    // Получаем глубину текущего фрагмента исходя из перспективы глазами источника света
    float currentDepth = projCoords.z;
	
    // Вычисляем смещение (на основе разрешения карты глубины и наклона)
    vec3 normal = normalize(fs_in.Normal);
    vec3 lightDir = normalize(u_engine_SpotLightPos - fs_in.FragPos);
    float bias = max(0.05 * (1.0 - dot(normal, lightDir)), 0.005);
	
    // Проверка нахождения текущего фрагмента в тени
    // float shadow = currentDepth - bias > closestDepth  ? 1.0 : 0.0;
	
    // PCF
    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(u_engine_SpotLightShadowMap, 0);
    for(int x = -1; x <= 1; ++x) {
        for(int y = -1; y <= 1; ++y) {
            float pcfDepth = texture(u_engine_SpotLightShadowMap, projCoords.xy + vec2(x, y) * texelSize).r;
            pcfDepth = LinearizeDepth(closestDepth, u_engine_SpotNearPlane, u_engine_SpotFarPlane) / u_engine_SpotFarPlane;
            shadow += currentDepth - bias > pcfDepth  ? 1.0 : 0.0;
        }    
    }
    shadow /= 9.0;
    
    // Оставляем значение тени на уровне 0.0 за границей дальней плоскости пирамиды видимости глазами источника света
    if(projCoords.z > 1.0) {
        shadow = 0.0;
    }
    return shadow;
}

vec3 gridSamplingDisk[20] = vec3[] (
   vec3(1, 1,  1), vec3( 1, -1,  1), vec3(-1, -1,  1), vec3(-1, 1,  1), 
   vec3(1, 1, -1), vec3( 1, -1, -1), vec3(-1, -1, -1), vec3(-1, 1, -1),
   vec3(1, 1,  0), vec3( 1, -1,  0), vec3(-1, -1,  0), vec3(-1, 1,  0),
   vec3(1, 0,  1), vec3(-1,  0,  1), vec3( 1,  0, -1), vec3(-1, 0, -1),
   vec3(0, 1,  1), vec3( 0, -1,  1), vec3( 0, -1, -1), vec3( 0, 1, -1)
);

float PointShadowCalculation(vec3 fragPos, int pointId) {
    // Получаем вектор между положением фрагмента и положением источника света
    vec3 fragToLight = fragPos - u_engine_PointLightsPos[pointId];
    // Теперь получим текущую линейную глубину как длину между фрагментом и положением источника света
    float currentDepth = length(fragToLight);

    float shadow = 0.0;
    float bias = 0.15;
    int samples = 20;
    float viewDistance = length(ubo_ViewPos - fragPos);
    float diskRadius = (1.0 + (viewDistance / u_engine_FarPlane)) / 25.0;
    for (int i = 0; i < samples; ++i) {
        float closestDepth = texture(u_engine_PointLightsCubeMap[pointId], fragToLight + gridSamplingDisk[i] * diskRadius).r;
        closestDepth *= u_engine_FarPlane; 
        if (currentDepth - bias > closestDepth) {
            shadow += 1.0;
        }
    }
    shadow /= float(samples);

    //float closestDepth = texture(u_engine_PointLightsCubeMap[pointId], fragToLight).r;
    //closestDepth *= u_engine_FarPlane;
    
    // Проводим проверку на нахождение в тени
    //bias = 0.05; 
    //shadow = currentDepth - bias > closestDepth ? 1.0 : 0.0;
    
    //FRAGMENT_COLOR = vec4(vec3(closestDepth / u_engine_FarPlane), 1.0);
    
    return shadow;
}
