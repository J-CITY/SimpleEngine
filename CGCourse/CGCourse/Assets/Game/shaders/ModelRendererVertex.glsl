#shader vertex
#version 330 core
layout (location = 0) in ivec3 a_Position;
layout (location = 1) in ivec2 a_TexCoords;
layout (location = 2) in uint a_LightingLevel;
layout (location = 3) in uint a_BlockFaceLightLevel;

uniform int u_RenderDistance;
uniform vec4 u_AmbientLight;
uniform mat4 u_ViewProjection;
uniform mat4 u_ViewMatrix;
uniform vec2 u_ChunkCoordinates;
uniform float u_SunPositionY;
uniform float u_Time;

// To transform the position
uniform int u_ChunkX;
uniform int u_ChunkZ;
uniform int u_CHUNK_SIZE_X;
uniform int u_CHUNK_SIZE_Z;

out float v_Visibility;
out vec2 v_TexCoord;
out vec4 v_TintColor;
out float v_SunlightIntensity;

const float fog_density = 0.01f;
float fog_gradient = float(u_RenderDistance + 1.0f);

void main()
{
	vec3 pos = vec3(a_Position.x + (u_ChunkX * u_CHUNK_SIZE_X), a_Position.y, a_Position.z + (u_ChunkZ * u_CHUNK_SIZE_Z));;

    pos.x += sin((u_Time + pos.z + pos.y) * 1.8f) / 15.0f;
    pos.z -= cos((u_Time + pos.x + pos.y) * 1.8f) / 15.0f;

	// Calculate fog
	vec4 relative_camera_pos = u_ViewMatrix * vec4(pos, 1.0f);
	float fog_distance = length(relative_camera_pos);
	v_Visibility = exp(-pow((fog_distance * fog_density), fog_gradient));
	v_Visibility = clamp(v_Visibility, 0.0f, 1.0f);

	float lighting_level = a_LightingLevel ;	
	lighting_level /= 2;
	lighting_level /= 10;

	v_SunlightIntensity = max(u_SunPositionY / 1000.0f, 0.6f);

	if (lighting_level < 0.2)
	{
		float face_light = float(a_BlockFaceLightLevel);
		face_light /= 10;
		v_TintColor =  u_AmbientLight;
		v_TintColor = v_TintColor * vec4(face_light, face_light, face_light, 1.0f);
	}

	else 
	{
		if (lighting_level < u_AmbientLight.x)
		{
			lighting_level = u_AmbientLight.x;
		}

		v_TintColor = vec4(lighting_level, lighting_level, lighting_level, 1.0f);

		if (v_SunlightIntensity < 1.0f)
		{
			v_SunlightIntensity = 1.0f;
		}
	}

	gl_Position = u_ViewProjection * vec4(pos, 1.0f);
	v_TexCoord = vec2(a_TexCoords.xy) ;
}

#shader fragment
#version 330 core
in float v_Visibility;
in vec2 v_TexCoord;
in vec4 v_TintColor;
in float v_SunlightIntensity;

out vec4 FRAGMENT_COLOR;

uniform sampler2D u_Texture;
uniform vec4 u_FogColor;

void main()
{
    const float threshhold = 0.01;
	FRAGMENT_COLOR = texelFetch(u_Texture, ivec2(v_TexCoord.xy), 0) * v_TintColor ; 

	if (FRAGMENT_COLOR.a < threshhold)
	{
		discard;
	}

	FRAGMENT_COLOR = mix(u_FogColor, FRAGMENT_COLOR, v_Visibility);
	FRAGMENT_COLOR = FRAGMENT_COLOR * vec4(v_SunlightIntensity, v_SunlightIntensity, v_SunlightIntensity, 1.0f);

	//FRAGMENT_COLOR = vec4(1.0, 0.0 ,0.0 ,1.0);
}