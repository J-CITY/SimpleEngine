// Include common HLSL code.
#include "Common.hlsl"

struct VertexIn
{
	float3 PosL    : POSITION;
	float3 NormalL : NORMAL;
	float2 TexC    : TEXCOORD;
};

struct VertexOut
{
	float4 PosH : SV_POSITION;
    float3 PosL : POSITION;
};

struct PixelOut
{
    float4 position : SV_Target0;
    float4 normal : SV_Target1;
    float4 color : SV_Target2;
};
 
VertexOut VS(VertexIn vin)
{
	VertexOut vout;

	// Use local vertex position as cubemap lookup vector.
	vout.PosL = vin.PosL;
	
	// Transform to world space.
	float4 posW = mul(float4(vin.PosL, 1.0f), gWorld);

	// Always center sky about camera.
	posW.xyz += gEyePosW;

	// Set z = w so that z/w = 1 (i.e., skydome always on far plane).
	vout.PosH = mul(posW, gViewProj).xyww;
	
	return vout;
}

const static float4 nullColor = { 0.0f, 0.0f, 0.0f, 0.0f };

PixelOut PS(VertexOut pin)
{
    PixelOut pout;
    pout.position = nullColor;
    pout.normal = nullColor;
	pout.color = gCubeMap.Sample(gsamLinearWrap, pin.PosL);
	
    return pout;
}

