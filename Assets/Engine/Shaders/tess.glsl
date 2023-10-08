#shader vertex
#version 460 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTex;

out vec2 TexCoord;

void main()
{
    gl_Position = vec4(aPos, 1.0);
    TexCoord = aTex;
}


#shader tessControl
#version 460 core
layout (std140) uniform EngineUBO {
    mat4    ubo_Model;
    mat4    ubo_View;
    mat4    ubo_Projection;
    vec3    ubo_ViewPos;
    float   ubo_Time;
    vec2    ubo_ViewportSize;
};

layout(vertices=4) out;

in vec2 TexCoord[];
out vec2 TextureCoord[];

void main() {
    gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;
    TextureCoord[gl_InvocationID] = TexCoord[gl_InvocationID];

    if(gl_InvocationID == 0)
    {
        const int MIN_TESS_LEVEL = 4;
        const int MAX_TESS_LEVEL = 64;
        const float MIN_DISTANCE = 20;
        const float MAX_DISTANCE = 800;

        vec4 eyeSpacePos00 = ubo_View * ubo_Model * gl_in[0].gl_Position;
        vec4 eyeSpacePos01 = ubo_View * ubo_Model * gl_in[1].gl_Position;
        vec4 eyeSpacePos10 = ubo_View * ubo_Model * gl_in[2].gl_Position;
        vec4 eyeSpacePos11 = ubo_View * ubo_Model * gl_in[3].gl_Position;

        // "distance" from camera scaled between 0 and 1
        float distance00 = clamp( (abs(eyeSpacePos00.z) - MIN_DISTANCE) / (MAX_DISTANCE-MIN_DISTANCE), 0.0, 1.0 );
        float distance01 = clamp( (abs(eyeSpacePos01.z) - MIN_DISTANCE) / (MAX_DISTANCE-MIN_DISTANCE), 0.0, 1.0 );
        float distance10 = clamp( (abs(eyeSpacePos10.z) - MIN_DISTANCE) / (MAX_DISTANCE-MIN_DISTANCE), 0.0, 1.0 );
        float distance11 = clamp( (abs(eyeSpacePos11.z) - MIN_DISTANCE) / (MAX_DISTANCE-MIN_DISTANCE), 0.0, 1.0 );

        float tessLevel0 = mix( MAX_TESS_LEVEL, MIN_TESS_LEVEL, min(distance10, distance00) );
        float tessLevel1 = mix( MAX_TESS_LEVEL, MIN_TESS_LEVEL, min(distance00, distance01) );
        float tessLevel2 = mix( MAX_TESS_LEVEL, MIN_TESS_LEVEL, min(distance01, distance11) );
        float tessLevel3 = mix( MAX_TESS_LEVEL, MIN_TESS_LEVEL, min(distance11, distance10) );

        gl_TessLevelOuter[0] = tessLevel0;
        gl_TessLevelOuter[1] = tessLevel1;
        gl_TessLevelOuter[2] = tessLevel2;
        gl_TessLevelOuter[3] = tessLevel3;

        gl_TessLevelInner[0] = max(tessLevel1, tessLevel3);
        gl_TessLevelInner[1] = max(tessLevel0, tessLevel2);
    }
}

#shader tessEvaluation
#version 460 core

layout (std140) uniform EngineUBO {
    mat4    ubo_Model;
    mat4    ubo_View;
    mat4    ubo_Projection;
    vec3    ubo_ViewPos;
    float   ubo_Time;
    vec2    ubo_ViewportSize;
};
layout(quads, fractional_odd_spacing, ccw) in;

uniform sampler2D u_HeightMap;
in vec2 TextureCoord[];

out float Height;
out vec2 TexCoord;

void main() {
    float u = gl_TessCoord.x;
    float v = gl_TessCoord.y;

    vec2 t00 = TextureCoord[0];
    vec2 t01 = TextureCoord[1];
    vec2 t10 = TextureCoord[2];
    vec2 t11 = TextureCoord[3];

    vec2 t0 = (t01 - t00) * u + t00;
    vec2 t1 = (t11 - t10) * u + t10;
    vec2 texCoord = (t1 - t0) * v + t0;

    Height = texture(u_HeightMap, texCoord).y * 150.0-32;
    TexCoord = texCoord;

    vec4 p00 = gl_in[0].gl_Position;
    vec4 p01 = gl_in[1].gl_Position;
    vec4 p10 = gl_in[2].gl_Position;
    vec4 p11 = gl_in[3].gl_Position;

    vec4 uVec = p01 - p00;
    vec4 vVec = p10 - p00;
    vec4 normal = normalize( vec4(cross(vVec.xyz, uVec.xyz), 0) );

    vec4 p0 = (p01 - p00) * u + p00;
    vec4 p1 = (p11 - p10) * u + p10;
    vec4 p = (p1 - p0) * v + p0 + normal * Height;

    gl_Position = ubo_Projection * ubo_View * ubo_Model * p;
}

#shader fragment
#version 460 core

in float Height;
in vec2 TexCoord;

//
//void main() {
//    float h = (Height + 16)/64.0f;
//    FragColor = vec4(h, h, h, 1.0);
//}

/* Global information sent by the engine */
layout (std140) uniform EngineUBO {
    mat4    ubo_Model;
    mat4    ubo_View;
    mat4    ubo_Projection;
    vec3    ubo_ViewPos;
    float   ubo_Time;
    vec2    ubo_ViewportSize;
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
uniform int         u_LevelsCnt = 4;
const int MAX_LEVELS = 4;
uniform vec4       u_Levels;

out vec4 FRAGMENT_COLOR;

void main() {
    vec2 g_TexCoords = u_TextureOffset + vec2(mod(TexCoord.x * u_TextureTiling.x, 1), mod(TexCoord.y * u_TextureTiling.y, 1));

    float texturesCnt = u_LevelsCnt / 2 + 1;

	vec4 textureColor = vec4(0.0);
	bool isTextureColorSet = false;
	for(int i = 0; i < u_LevelsCnt && !isTextureColorSet; i++) {
		if(Height > u_Levels[i]) {
			continue;
		}
		int currentSamplerIndex = i / 2;
		vec2 texCoords = vec2(g_TexCoords.x / texturesCnt + currentSamplerIndex / texturesCnt, g_TexCoords.y);
		if(i % 2 == 0) {
			textureColor = texture(u_DiffuseMap, texCoords);
		}
		else {
			int nextSamplerIndex = currentSamplerIndex+1;
			vec2 texCoordsNext = vec2(g_TexCoords.x / texturesCnt + nextSamplerIndex / texturesCnt, g_TexCoords.y);
			vec4 textureColorCurrent = texture(u_DiffuseMap, texCoords);
			vec4 textureColorNext = texture(u_DiffuseMap, texCoordsNext);

			float levelDiff = u_Levels[i] - u_Levels[i-1];
			float factorNext = (Height - u_Levels[i-1]) / levelDiff;
			float factorCurrent = 1.0f - factorNext;
			
			textureColor = textureColorCurrent*factorCurrent + textureColorNext*factorNext;
		}

		isTextureColorSet = true;
	}

	if(!isTextureColorSet) {
		int lastSamplerIndex = u_LevelsCnt / 2;
		vec2 texCoords = vec2(g_TexCoords.x / texturesCnt + lastSamplerIndex / texturesCnt, g_TexCoords.y);
		textureColor = texture(u_DiffuseMap, texCoords);
	}

	vec4 objectColor = textureColor * u_Diffuse;
	
	//light calculation

	FRAGMENT_COLOR = objectColor;
}