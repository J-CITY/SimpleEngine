#shader vertex
#version 460 core

layout (location = 0) in vec3 geo_Pos;
layout (location = 1) in vec2 geo_TexCoords;
layout (location = 2) in vec3 geo_Normal;
layout (location = 3) in vec3 geo_Tangent;
layout (location = 4) in vec3 geo_Bitangent;
layout (location = 5) in ivec4 boneIds;
layout (location = 6) in vec4 weights;


uniform mat4 u_LightSpaceMatrix;

/* Global information sent by the engine */

layout (std140) uniform EngineUBO {
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
    float       Height;
} vs_out;
int ub=1;

void main() {

    vs_out.TBN = mat3(
        normalize(vec3(ubo_Model * vec4(geo_Tangent,   0.0))),
        normalize(vec3(ubo_Model * vec4(geo_Bitangent, 0.0))),
        normalize(vec3(ubo_Model * vec4(geo_Normal,    0.0)))
    );

    mat3 TBNi = transpose(vs_out.TBN);

    vs_out.FragPos           = vec3(ubo_Model * vec4(geo_Pos.x, geo_Pos.y * 100.0f, geo_Pos.z, 1.0));
    vs_out.Normal            = normalize(mat3(transpose(inverse(ubo_Model))) * geo_Normal);
    vs_out.TexCoords         = geo_TexCoords;
    vs_out.TangentViewPos    = TBNi * ubo_ViewPos;
    vs_out.TangentFragPos    = TBNi * vs_out.FragPos;
    vs_out.Height            = geo_Pos.y * 1000;

    gl_Position = ubo_Projection * (ubo_View * vec4(vs_out.FragPos, 1.0));
}

#shader fragment
#version 460 core

/* Global information sent by the engine */
layout (std140) uniform EngineUBO {
    mat4    ubo_Model;
    mat4    ubo_View;
    mat4    ubo_Projection;
    vec3    ubo_ViewPos;
    float   ubo_Time;
};

/* Information passed from the fragment shader */
in VS_OUT {
    vec3        FragPos;
    vec3        Normal;
    vec2        TexCoords;
    mat3        TBN;
    flat vec3   TangentViewPos;
    vec3        TangentFragPos;
    float       Height;
} fs_in;


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

/* Global variables */
vec3 g_Normal;
vec2 g_TexCoords;
vec3 g_ViewDir;
vec4 g_DiffuseTexel;
vec4 g_SpecularTexel;
vec4 g_HeightTexel;
vec4 g_NormalTexel;


out vec4 FRAGMENT_COLOR;

void main() {
    g_TexCoords = u_TextureOffset + vec2(mod(fs_in.TexCoords.x * u_TextureTiling.x, 1), mod(fs_in.TexCoords.y * u_TextureTiling.y, 1));

    /* Apply parallax mapping */
    //if (u_HeightScale > 0)
    //    g_TexCoords = ParallaxMapping(normalize(fs_in.TangentViewPos - fs_in.TangentFragPos));

    /* Apply color mask */
    
    //g_ViewDir           = normalize(ubo_ViewPos - fs_in.FragPos);
    //g_DiffuseTexel      = texture(u_DiffuseMap,  g_TexCoords) * u_Diffuse;
    //g_SpecularTexel     = texture(u_SpecularMap, g_TexCoords) * vec4(u_Specular, 1.0);
    //if (u_EnableNormalMapping) {
    //    g_Normal = texture(u_NormalMap, g_TexCoords).rgb;
    //    g_Normal = normalize(g_Normal * 2.0 - 1.0);
    //    g_Normal = normalize(fs_in.TBN * g_Normal);
    //}
    //else {
    //    g_Normal = normalize(fs_in.Normal);
    //}
    
    float texturesCnt = u_LevelsCnt / 2 + 1;

	vec4 textureColor = vec4(0.0);
	bool isTextureColorSet = false;
	for(int i = 0; i < u_LevelsCnt && !isTextureColorSet; i++) {
		if(fs_in.Height > u_Levels[i]) {
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
			float factorNext = (fs_in.Height - u_Levels[i-1]) / levelDiff;
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
