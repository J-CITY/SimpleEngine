struct LightOGL {
    float3 pos;
    float cutoff;
    float3 forward;
    float outerCutoff;
    float3 color;
    float constant;
    int type;
    float _linear;
    float quadratic;
    float intensity;
};

struct VertexOut
{
	float4 PosH    : SV_POSITION;
	float2 TexC    : TEXCOORD0;
    float2 quadID : TEXCOORD1;
};

SamplerState gsamPointWrap : register(s0);
SamplerState gsamPointClamp : register(s1);
SamplerState gsamLinearWrap : register(s2);
SamplerState gsamLinearClamp : register(s3);
SamplerState gsamAnisotropicWrap : register(s4);
SamplerState gsamAnisotropicClamp : register(s5);

static const float2 gTexCoords[6] =
{
    float2(0.0f, 1.0f),
	float2(0.0f, 0.0f),
	float2(1.0f, 0.0f),
	float2(0.0f, 1.0f),
	float2(1.0f, 0.0f),
	float2(1.0f, 1.0f)
};


cbuffer engine_UBO : register(b0)
{
    float4x4    View;
    float4x4    Projection;
    float3    ViewPos;
    float   Time;
    float2    ViewportSize;
};

cbuffer ssbo_Lights : register(b1)
{
   LightOGL l[20];
};

Texture2D u_PositionMap : register(t0);
Texture2D u_NormalMap : register(t1);
Texture2D u_AlbedoSpecMap : register(t2);
Texture2D u_RoughAO : register(t3);

VertexOut VS(uint vid : SV_VertexID) {
    VertexOut vout = (VertexOut) 0.0f;
    int quadID = vid / 6;
    vid %= 6;
    vout.TexC = gTexCoords[vid];
    vout.PosH = float4(vout.TexC.x * 2 - 1, 1 - vout.TexC.y * 2, 0.0f, 1.0f);
    
    if (quadID >= 1) {
        vout.PosH.xy *= 0.25f;
        vout.PosH.x -= (0.75f - 0.5f * quadID);
        vout.PosH.y -= 0.75f;
    }
    
    vout.quadID.x = quadID;
    return vout;
}

float4 PS(VertexOut pin) : SV_Target
{
    float4 FRAGMENT_COLOR = float4(1.0f, 0.0f, 0.0f, 1.0f);

    float3 color = u_AlbedoSpecMap.Sample(gsamPointClamp, pin.TexC).xyz;

    FRAGMENT_COLOR = float4(color, 1.0f);
    return FRAGMENT_COLOR;
}
