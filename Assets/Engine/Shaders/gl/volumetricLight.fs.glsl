#version 460 core

in vec2 TexCoords;
out vec4 OutColor;


uniform sampler2D cameraOutput;
uniform sampler2D depthMap;

#include "../lib/engineUBO.glsl"
#include "../lib/shadowData.glsl"
//layout (std140, binding = 0) uniform EngineUBO {
//    mat4    View;
//    mat4    Projection;
//    vec3    ViewPos;
//    float   Time;
//    vec2    ViewportSize;
//    float    FPS;
//    int      FrameCount;
//} engine_UBO;
//
//struct PushModel {
//    mat4 model;
//};
//uniform PushModel engine_Model;

int DirLightCascadeMapCount = 4;
struct DirLight
{
    //mat4 transform[DirLightCascadeMapCount];
	mat4 transform;
    vec4 color;
    vec3 direction;
};

uniform DirLight light;
uniform sampler2DArray lightDepthMap;


uniform float maxSteps;
uniform float sampleStep;
uniform float stepIncrement;
uniform float maxDistance;
uniform float asymmetry;


uniform int voutmat;

uniform mat4 lightProj;

#define PI 3.1415926535f

int layer = -1;

float getTextureLodLevel(vec2 uv)
{
    vec2 dxVtc = dFdx(uv);
    vec2 dyVtc = dFdy(uv);
    float deltaMax2 = max(dot(dxVtc, dxVtc), dot(dyVtc, dyVtc));
    return 0.5 * log2(deltaMax2);
}

float sampleShadowMap(sampler2DArray depthMap, vec2 coords, float lod, float compare)
{
    return step(compare, texture(depthMap, vec3(coords, layer), lod).r);
}

//sample only once without extra interpolation
float godRayShadowFactor2D(vec3 coords, sampler2DArray depthMap, float bias)
{
	float compare = coords.z - bias;

	const float lod = getTextureLodLevel(coords.xy);
	vec2 texelSize = textureSize(depthMap, 0).xy;
	vec2 texelSizeInv = 1.0 / texelSize;
	vec2 pixelPos = coords.xy * texelSize + vec2(0.5);
	vec2 fracPart = fract(pixelPos);
	vec2 startTexel = (pixelPos - fracPart) * texelSizeInv;
	float res = sampleShadowMap(depthMap, startTexel + vec2(0.0, 0.0) * texelSizeInv, lod, compare);
	return pow(clamp(res, 0.0, 1.0), 8.0);
}

float godRayShadowSampler(vec3 position, DirLight light, sampler2DArray shadowMap)
{
	const float bias = 0.002;
    const vec2 textureSplitSize = vec2(1.01, 0.99);
	int index = 0;
	float fIndex = float(index);
	vec4 textureLimitsXY = vec4(vec2(fIndex, fIndex + 1.f) * textureSplitSize, 0.001, 0.999);
	//vec4 fragLightSpace = light.transform * vec4(position, 1.0);
	//vec4 fragLightSpace = dirMatrices[layer]  * vec4(position, 1.0);
	vec4 fragLightSpace = lightProj  * vec4(position, 1.0);
	vec3 projectedPosition = fragLightSpace.xyz / fragLightSpace.w;
	if (projectedPosition.x > textureLimitsXY[1] || projectedPosition.x < textureLimitsXY[0] ||
		projectedPosition.y > textureLimitsXY[3] || projectedPosition.y < textureLimitsXY[2] )
	{
		return 0.0;
	}
	return godRayShadowFactor2D(projectedPosition, shadowMap, bias);
}

float phaseHG(float cosTheta)
{
	const float inv4PI = 1.0 / (4.0 * PI);
	float denom = 1 + asymmetry * asymmetry + 2 * asymmetry * cosTheta;
	return inv4PI * (1 - asymmetry * asymmetry) / (denom * sqrt(denom));
}

float noise(vec2 p) 
{
    return fract(sin(dot(p, vec2(123.45, 875.43))) * 5432.3);
}

float calcShadowFactor2D(vec3 coords, sampler2DArray depthMap, vec4 textureLimitsXY, float bias)
{
    //if (coords.x < textureLimitsXY[0] || coords.x > textureLimitsXY[1]) return -1.0;
   // if (coords.y < textureLimitsXY[2] || coords.y > textureLimitsXY[3]) return -1.0;
    if (coords.z > 1.0 - bias || coords.z < bias) return -1.0;
    float compare = coords.z - bias;
    
    const float lod = getTextureLodLevel(coords.xy);
    vec2 texelSize = textureSize(depthMap, 0).xy;
    vec2 texelSizeInv = 1.0 / texelSize;
    vec2 pixelPos = coords.xy * texelSize + vec2(0.5);
    vec2 fracPart = fract(pixelPos);
    vec2 startTexel = (pixelPos - fracPart) * texelSizeInv;

    float s = 0.0;

    s += sampleShadowMap(depthMap, startTexel + vec2( 1.0,  1.0) * texelSizeInv, lod, compare);
    s += sampleShadowMap(depthMap, startTexel + vec2( 0.0,  1.0) * texelSizeInv, lod, compare);
    s += sampleShadowMap(depthMap, startTexel + vec2(-1.0,  1.0) * texelSizeInv, lod, compare);
    s += sampleShadowMap(depthMap, startTexel + vec2( 1.0,  0.0) * texelSizeInv, lod, compare);
    s += sampleShadowMap(depthMap, startTexel + vec2( 0.0,  0.0) * texelSizeInv, lod, compare);
    s += sampleShadowMap(depthMap, startTexel + vec2(-1.0,  0.0) * texelSizeInv, lod, compare);
    s += sampleShadowMap(depthMap, startTexel + vec2( 1.0, -1.0) * texelSizeInv, lod, compare);
    s += sampleShadowMap(depthMap, startTexel + vec2( 0.0, -1.0) * texelSizeInv, lod, compare);
    s += sampleShadowMap(depthMap, startTexel + vec2(-1.0, -1.0) * texelSizeInv, lod, compare);

    return s / 9.0;
}
float calcShadowFactorCascade(vec4 position, DirLight light, sampler2DArray shadowMap) {
    const vec2 TEXTURE_BIAS = vec2(0.001, 0.001);
    const float invCascadeCount = 1.0 / DirLightCascadeMapCount;
    vec3 depthBias = vec3(0.0005, 0.0003, 0.0002);
    for (int i = 0; i < DirLightCascadeMapCount; i++) {
		layer = i;
        vec2 textureLimitsX = vec2(i, i + 1) * invCascadeCount + vec2(TEXTURE_BIAS.x, -TEXTURE_BIAS.x);
        vec4 textureLimitsXY = vec4(textureLimitsX, TEXTURE_BIAS.y, 1.0 - TEXTURE_BIAS.y);
        vec4 fragLightSpace = dirMatrices[layer] * position;
        vec3 projectedPosition = fragLightSpace.xyz / fragLightSpace.w;
        float s = calcShadowFactor2D(projectedPosition, shadowMap, textureLimitsXY, depthBias[i]);
        if (s != -1.0) return s;
    }
    return 1.0;
}

vec3 g_Normal = vec3(0.0f);

float ShadowCalculationWithCascade(vec3 fragPosWorldSpace) {
    // select cascade layer
    vec4 fragPosViewSpace = engine_UBO.View * vec4(fragPosWorldSpace, 1.0);
    float depthValue = abs(fragPosViewSpace.z);

    int layer = -1;
    for (int i = 0; i < engine_ShadowUBO.dirCascadeCount; ++i) {
        if (depthValue < dirCascadePlaneDistances[i])
        {
            layer = i;
            break;
        }
    }
    if (layer == -1) {
        layer = engine_ShadowUBO.dirCascadeCount;
    }

    vec4 fragPosLightSpace = dirMatrices[layer] * vec4(fragPosWorldSpace, 1.0);
    // perform perspective divide
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    // transform to [0,1] range
    projCoords = projCoords * 0.5 + 0.5;

    // get depth of current fragment from light's perspective
    float currentDepth = projCoords.z;

    // keep the shadow at 0.0 when outside the far_plane region of the light's frustum.
    if (currentDepth > 1.0) {
        return 0.0;
    }
    // calculate bias (based on depth map resolution and slope)
    vec3 normal = normalize(g_Normal);
    float bias = max(0.05 * (1.0 - dot(normal, engine_ShadowUBO.dirLightDir)), 0.005);
    const float biasModifier = 0.5f;
    if (layer == engine_ShadowUBO.dirCascadeCount) {
        bias *= 1 / (engine_ShadowUBO.dirFarPlane * biasModifier);
    }
    else {
        bias *= 1 / (dirCascadePlaneDistances[layer] * biasModifier);
    }

    //float shadow = PCSSCascade(vec3(projCoords.xy, layer), bias, currentDepth);

    // PCF
    float shadow = 0.0;
    vec2 texelSize = 1.0 / vec2(textureSize(lightDepthMap, 0).xy);
    for(int x = -1; x <= 1; ++x) {
        for(int y = -1; y <= 1; ++y) {
            float pcfDepth = texture(lightDepthMap, vec3(projCoords.xy + vec2(x, y) * texelSize, layer)).r;
            shadow += (currentDepth - bias) > pcfDepth ? 1.0 : 0.0;
        }    
    }
    shadow /= 9.0;
    return shadow;
}

vec3 reconstructWorldPosition(float depth, vec2 texcoord, mat4 invViewProjMatrix) {
    vec4 normPosition = vec4(2.0f * texcoord - vec2(1.0f), depth, 1.0f);
    vec4 worldPosition = invViewProjMatrix * normPosition;
    worldPosition /= worldPosition.w;
    return worldPosition.xyz;
}

void main() {
	//vec3 _pos = texture(depthMap, TexCoords).xyz;

    mat4 invViewProjMatrix = inverse(engine_UBO.Projection * engine_UBO.View);
    vec3 _pos = reconstructWorldPosition(texture(depthMap, TexCoords).z, TexCoords, invViewProjMatrix);

	vec3 camera2Frag = (_pos - engine_UBO.ViewPos);
	float fragDistance = length(camera2Frag);
	vec3 fragDirection = normalize(camera2Frag);
	vec3 currentColor = texture(cameraOutput, TexCoords).rgb;

	//vec4 fragPosViewSpace = engine_UBO.View * vec4(_pos, 1.0);
    //float depthValue = abs(fragPosViewSpace.z);
    //for (int i = 0; i < engine_ShadowUBO.dirCascadeCount; ++i) {
    //    if (depthValue < dirCascadePlaneDistances[i]) {
    //        layer = i;
    //        break;
    //    }
    //}
    //if (layer == -1) {
    //    layer = engine_ShadowUBO.dirCascadeCount;
    //}
    layer = DirLightCascadeMapCount-1;

	
	
	float randomness = noise(TexCoords) * .6;//producing blur to decrease sampling rate
	//for (int lightIndex = 0; lightIndex < 4; lightIndex++) {
	//layer=lightIndex;
		float illum = 0.0f;
		float i = 0.0f;
		float stp = sampleStep;
		vec3 pos = engine_UBO.ViewPos;
		for (; i < maxSteps; i++) {
			pos = engine_UBO.ViewPos + stp * fragDirection * (i + randomness);
			if (fragDistance <= distance(pos, engine_UBO.ViewPos))
				break;
			stp *= stepIncrement;

			//float shadowFactor = ShadowCalculationWithCascade(pos);
			float shadowFactor = godRayShadowSampler(pos, light, lightDepthMap);

			illum += clamp(shadowFactor, 0.0f, 1.0f);
		}
		illum /= i;
		float distanceTraveled = length(engine_UBO.ViewPos - pos);
		illum *= distanceTraveled / maxDistance;
		illum = clamp(illum, 0.0, 0.6);
		
		float cosTheta = dot(fragDirection, light.direction);
		float scattering = phaseHG(cosTheta);
		scattering = max(scattering, 0.0001);

		if (voutmat == 0) {
			currentColor = mix(currentColor, light.color.rgb * scattering, illum);
		}
		else {
			currentColor = light.color.rgb * scattering;
		}
	//}
	


	//vec3 rayColor = vec3(1.f);
	//
	//vec3 samplePosition = engine_UBO.ViewPos;
    //float illum = 0.0f;
    //for (float i = 0.0f; i < maxSteps; i++) 
	//{
	//	samplePosition+=sampleStep*fragDirection;
	//	float shadowFactor = calcShadowFactorCascade(
    //        vec4(samplePosition,1.0), 
    //        light,
    //        lightDepthMap);
    //    illum += smoothstep(0.0f, 1.0f, shadowFactor);
	//}
    //illum /= maxSteps;
    //currentColor = mix(currentColor, rayColor, pow(illum, 0.7f));

    OutColor = vec4(currentColor,1.f);

}
