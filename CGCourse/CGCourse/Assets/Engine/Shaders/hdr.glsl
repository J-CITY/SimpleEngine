#shader vertex
#version 460 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoords;

out vec2 TexCoords;

void main() {
    TexCoords = aTexCoords;
    gl_Position = vec4(aPos, 1.0);
}

#shader fragment
#version 460 core

out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D hdrBuffer0;
uniform sampler2D hdrBuffer1;
uniform sampler2D hdrBuffer2;
uniform sampler2D hdrBuffer3;

uniform sampler2D bloomBlur;
uniform sampler2D   u_Noise;
uniform sampler2D   u_Depth;

uniform bool hdr;
uniform float exposure;

//god rays
uniform vec3 sunPos;

layout (std140) uniform EngineUBO
{
    mat4    ubo_Model;
    mat4    ubo_View;
    mat4    ubo_Projection;
    vec3    ubo_ViewPos;
    float   ubo_Time;
};

#include "./lib/fxaa.glsl"

const float decay = 0.96815;
const float density  = 0.926;
const float weight  = 0.587;
struct FogParameters {
	vec3 color;
	float linearStart;
	float linearEnd;
	float density;
	
	int equation;
	bool isEnabled;
};
FogParameters fogParams;
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
float near = 0.1; 
float far  = 10.0; 
float LinearizeDepth(float depth) 
{
    float z = depth * 2.0 - 1.0; // обратно к NDC 
    return (2.0 * near * far) / (far + near - z * (far - near));	
}
void main() {
 fogParams.color = vec3(0.2);
    fogParams.density = 0.01;
    fogParams.equation = 2;
    fogParams.isEnabled = true;
	fogParams.linearStart=0.0;
	fogParams.linearEnd=0.0;

    vec3 bloomColor = texture(bloomBlur, TexCoords).rgb;

    const float gamma = 2.2;
    vec3 hdrColor0 = texture(hdrBuffer0, TexCoords).rgb;
    vec3 hdrColor1 = texture(hdrBuffer1, TexCoords).rgb;
    vec3 hdrColor2 = texture(hdrBuffer2, TexCoords).rgb;
    vec3 hdrColor3 = texture(hdrBuffer3, TexCoords).rgb;

    //motion blur
    vec3 hdrColor = vec3(0.0);
    //hdrColor += hdrColor3 * 0.4;
    //hdrColor += hdrColor2 * 0.3;
    //hdrColor += hdrColor1 * 0.2;
    //hdrColor += hdrColor0 * 0.1;
    hdrColor = hdrColor3;

    //bloom
    //hdrColor += bloomColor;

    /*
    //god rays
    int NUM_SAMPLES = 80;
	vec2 tc = TexCoords.xy;
	vec2 deltatexCoord = (tc - (sunPos.xy*0.5 + 0.5));
	deltatexCoord *= 1.0/ float(NUM_SAMPLES);
	float illuminationDecay = 1.0f;

	vec4 godRayColor = texture(bloomBlur , tc.xy)*0.4;
	for(int i = 0 ; i< NUM_SAMPLES ; i++) {
		tc-= deltatexCoord;
		vec4 samp = texture(bloomBlur , tc )*0.4;
		samp *= illuminationDecay*weight;
		godRayColor += samp;
		illuminationDecay *= decay;
	}
	vec3 realColor = hdrColor;
	hdrColor = godRayColor.rgb * 0.3f + realColor;
    */
    /*
    int Samples = 128;
    float Intensity = 0.125;
    float Decay = 0.96875;
    vec2 _TexCoord = TexCoords.xy;
    vec2 Direction = vec2(0.5) - _TexCoord;
    Direction /= Samples;
    vec3 Color = texture2D(bloomBlur, _TexCoord).rgb;
    
    for(int Sample = 0; Sample < Samples; Sample++) {
        Color += texture2D(bloomBlur, _TexCoord).rgb * Intensity;
        Intensity *= Decay;
        _TexCoord += Direction;
    }
    hdrColor = Color.rgb * 0.3f + hdrColor;
    */

    hdrColor = fxaa(hdrBuffer3, TexCoords).rgb;

    if(hdr) {
        // vec3 result = hdrColor / (hdrColor + vec3(1.0));
		
        // Экспозиция
        vec3 result = vec3(1.0) - exp(-hdrColor * exposure);
		
        // Заодно проведем гамма-коррекцию      
        result = pow(result, vec3(1.0 / gamma));
        FragColor = vec4(result, 1.0);
    }
    else {
        vec3 result = pow(hdrColor, vec3(1.0 / gamma));
        FragColor = vec4(result, 1.0);
    }
}
