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

uniform sampler2D u_Scene;
uniform bool u_UseHDR;
uniform float u_Exposure;
uniform float u_Gamma;

//god rays
//uniform vec3 sunPos;

layout (std140) uniform EngineUBO
{
    mat4    ubo_Model;
    mat4    ubo_View;
    mat4    ubo_Projection;
    vec3    ubo_ViewPos;
    float   ubo_Time;
};

//#include "./lib/fxaa.glsl"

//const float decay = 0.96815;
//const float density  = 0.926;
//const float weight  = 0.587;
//
//float near = 0.1; 
//float far  = 10.0; 
//float LinearizeDepth(float depth) 
//{
//    float z = depth * 2.0 - 1.0; // обратно к NDC 
//    return (2.0 * near * far) / (far + near - z * (far - near));	
//}
void main() {
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

    vec3 color = texture(u_Scene, TexCoords).rgb;
    if(u_UseHDR) {
        //vec3 result = color / (color + vec3(1.0));
		
        // Экспозиция
        vec3 result = vec3(1.0) - exp(-color * u_Exposure);
		
        // Заодно проведем гамма-коррекцию
        result = pow(result, vec3(1.0 / u_Gamma));
        FragColor = vec4(result, 1.0);
    }
    else {
        vec3 result = pow(color, vec3(1.0 / u_Gamma));
        FragColor = vec4(result, 1.0);
    }
}
