#version 330 core
in float v_Visibility;
in vec2 v_TexCoord;
in vec4 v_TintColor;
in float v_SunlightIntensity;

out vec4 o_Color;

uniform sampler2D u_Texture;
uniform vec4 u_FogColor;

void main()
{
	const float threshhold = 0.1;
	o_Color =  texelFetch(u_Texture, ivec2(v_TexCoord.xy), 0) * v_TintColor ; 

	if (o_Color.a < threshhold)
	{
		discard;
	}

	o_Color = mix(u_FogColor, o_Color, v_Visibility);
	o_Color = o_Color * vec4(v_SunlightIntensity, v_SunlightIntensity, v_SunlightIntensity, 1.0f) ; 
}