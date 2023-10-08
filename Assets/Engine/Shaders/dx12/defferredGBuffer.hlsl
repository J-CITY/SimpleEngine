struct VertexIn
{
    float3 geo_Pos : POSITION;
    float2 geo_TexCoords : TEXCOORD;
    float3 geo_Normal : NORMAL;
    float3 geo_Tangent : TANGENT;
    float3 geo_Bitangent : POSITIONT;
    float4 boneIds : COLOR;
    float4 weights : BINORMAL;
};

//struct VertexOut
//{
//    float3        Pos : POSITION0;
//    float3        FragPos : POSITION1;
//    float3        Normal : NORMAL;
//    float2        TexCoords : TEXCOORD;
//    float3x3      TBN : POSITION2;
//    float3        TangentViewPos : TANGENT0;
//    float3        TangentFragPos : TANGENT1;
//
//    //float4 FragPosDirLightSpace : POSITION;
//    //float4 FragPosSpotLightSpace : POSITION;
//    float4 EyeSpacePosition : SV_POSITION0;
//};

struct VertexOut
{
	float4 PosH    : SV_POSITION;
    float3 PosW    : POSITION;
    float3 NormalW : NORMAL;
	float3 TangentW : TANGENT;
	float2 TexC    : TEXCOORD;
};

struct PixelOut
{
    float4 gPosition : SV_Target0;
    float4 gNormal : SV_Target1;
    float4 gAlbedoSpec : SV_Target2;
    float4 gRoughAO : SV_Target3;
};

cbuffer engine_UBO : register(b0) {
    float4x4    View;
    float4x4    Projection;
    float3    ViewPos;
    float   Time;
    float2    ViewportSize;
    float FPS;
    int FrameCount;
};

cbuffer pushModel : register(b1) {
	float4x4 model;
};

cbuffer dataUBO : register(b2) {
    float4        u_Albedo;
    float       u_Metallic;
    float       u_Roughness;
    float2        u_TextureTiling;
    float2        u_TextureOffset;
    float3        u_Specular;
    float       u_Shininess;
    float       u_HeightScale;
    int        u_EnableNormalMapping;
};

Texture2D u_AlbedoMap : register(t0);
Texture2D u_NormalMap : register(t1);
Texture2D u_MetallicMap : register(t2);
Texture2D u_RoughnessMap : register(t3);
Texture2D u_AmbientOcclusionMap : register(t4);


SamplerState gsamPointWrap        : register(s0);
SamplerState gsamPointClamp       : register(s1);
SamplerState gsamLinearWrap       : register(s2);
SamplerState gsamLinearClamp      : register(s3);
SamplerState gsamAnisotropicWrap  : register(s4);
SamplerState gsamAnisotropicClamp : register(s5);

//vec2 ParallaxMapping(vec3 p_ViewDir) {
//    const vec2 parallax = p_ViewDir.xy * u_HeightScale * texture(u_HeightMap, g_TexCoords).r;
//    return g_TexCoords - vec2(parallax.x, 1.0 - parallax.y);
//}

float4x4 inverse(float4x4 m) {
    float n11 = m[0][0], n12 = m[1][0], n13 = m[2][0], n14 = m[3][0];
    float n21 = m[0][1], n22 = m[1][1], n23 = m[2][1], n24 = m[3][1];
    float n31 = m[0][2], n32 = m[1][2], n33 = m[2][2], n34 = m[3][2];
    float n41 = m[0][3], n42 = m[1][3], n43 = m[2][3], n44 = m[3][3];

    float t11 = n23 * n34 * n42 - n24 * n33 * n42 + n24 * n32 * n43 - n22 * n34 * n43 - n23 * n32 * n44 + n22 * n33 * n44;
    float t12 = n14 * n33 * n42 - n13 * n34 * n42 - n14 * n32 * n43 + n12 * n34 * n43 + n13 * n32 * n44 - n12 * n33 * n44;
    float t13 = n13 * n24 * n42 - n14 * n23 * n42 + n14 * n22 * n43 - n12 * n24 * n43 - n13 * n22 * n44 + n12 * n23 * n44;
    float t14 = n14 * n23 * n32 - n13 * n24 * n32 - n14 * n22 * n33 + n12 * n24 * n33 + n13 * n22 * n34 - n12 * n23 * n34;

    float det = n11 * t11 + n21 * t12 + n31 * t13 + n41 * t14;
    float idet = 1.0f / det;

    float4x4 ret;

    ret[0][0] = t11 * idet;
    ret[0][1] = (n24 * n33 * n41 - n23 * n34 * n41 - n24 * n31 * n43 + n21 * n34 * n43 + n23 * n31 * n44 - n21 * n33 * n44) * idet;
    ret[0][2] = (n22 * n34 * n41 - n24 * n32 * n41 + n24 * n31 * n42 - n21 * n34 * n42 - n22 * n31 * n44 + n21 * n32 * n44) * idet;
    ret[0][3] = (n23 * n32 * n41 - n22 * n33 * n41 - n23 * n31 * n42 + n21 * n33 * n42 + n22 * n31 * n43 - n21 * n32 * n43) * idet;

    ret[1][0] = t12 * idet;
    ret[1][1] = (n13 * n34 * n41 - n14 * n33 * n41 + n14 * n31 * n43 - n11 * n34 * n43 - n13 * n31 * n44 + n11 * n33 * n44) * idet;
    ret[1][2] = (n14 * n32 * n41 - n12 * n34 * n41 - n14 * n31 * n42 + n11 * n34 * n42 + n12 * n31 * n44 - n11 * n32 * n44) * idet;
    ret[1][3] = (n12 * n33 * n41 - n13 * n32 * n41 + n13 * n31 * n42 - n11 * n33 * n42 - n12 * n31 * n43 + n11 * n32 * n43) * idet;

    ret[2][0] = t13 * idet;
    ret[2][1] = (n14 * n23 * n41 - n13 * n24 * n41 - n14 * n21 * n43 + n11 * n24 * n43 + n13 * n21 * n44 - n11 * n23 * n44) * idet;
    ret[2][2] = (n12 * n24 * n41 - n14 * n22 * n41 + n14 * n21 * n42 - n11 * n24 * n42 - n12 * n21 * n44 + n11 * n22 * n44) * idet;
    ret[2][3] = (n13 * n22 * n41 - n12 * n23 * n41 - n13 * n21 * n42 + n11 * n23 * n42 + n12 * n21 * n43 - n11 * n22 * n43) * idet;

    ret[3][0] = t14 * idet;
    ret[3][1] = (n13 * n24 * n31 - n14 * n23 * n31 + n14 * n21 * n33 - n11 * n24 * n33 - n13 * n21 * n34 + n11 * n23 * n34) * idet;
    ret[3][2] = (n14 * n22 * n31 - n12 * n24 * n31 - n14 * n21 * n32 + n11 * n24 * n32 + n12 * n21 * n34 - n11 * n22 * n34) * idet;
    ret[3][3] = (n12 * n23 * n31 - n13 * n22 * n31 + n13 * n21 * n32 - n11 * n23 * n32 - n12 * n21 * n33 + n11 * n22 * n33) * idet;

    return ret;
}

VertexOut VS(VertexIn vin) {
    //VertexOut vs_out = (VertexOut)0.0f;
    //
    //float4 totalPosition = float4(0.0f, 0.0f, 0.0f, 0.0f);
    //vs_out.TBN = float3x3(
    //    normalize((float3) mul(float4(vin.geo_Tangent,   0.0), model)),
    //    normalize((float3) mul(float4(vin.geo_Bitangent, 0.0), model)),
    //    normalize((float3) mul(float4(vin.geo_Normal,    0.0), model))
    //);
//
    //float3x3 TBNi = transpose(vs_out.TBN);
    ////if (u_UseBone) {
    ////    vs_out.FragPos          = vec3(ubo_Model * totalPosition);
    ////    vs_out.EyeSpacePosition =  (ubo_View * ubo_Model) * totalPosition;
    ////}
    ////else {
    //    vs_out.FragPos          = (float3) mul(float4(vin.geo_Pos, 1.0), model);
    //    vs_out.EyeSpacePosition =  mul(float4(vin.geo_Pos, 1.0), (View * model));
    ////}
    //vs_out.Normal           = normalize(mul(vin.geo_Normal, (float3x3)transpose(inverse(model))));
    //vs_out.TexCoords        = vin.geo_TexCoords;
    //vs_out.TangentViewPos   = mul(ViewPos, TBNi);
    //vs_out.TangentFragPos   = mul(vs_out.FragPos, TBNi);
    //
    //vs_out.Pos = mul(mul(float4(vs_out.FragPos, 1.0), View), Projection);
    //return vs_out;

    VertexOut vout = (VertexOut)0.0f;
    
    float4 posW = mul(float4(vin.geo_Pos, 1.0f), model);
    vout.PosW = posW.xyz;
    float4 v = mul(posW, View);
    vout.PosH = mul(v, Projection);

    vout.NormalW = mul(vin.geo_Normal, (float3x3) model);
	
    vout.TangentW = mul(vin.geo_Tangent, (float3x3) model);
    
    float4 texC = float4(vin.geo_TexCoords, 0.0f, 1.0f);
    vout.TexC = mul(texC, model).xy;

    return vout;
}

float3 NormalSampleToWorldSpace(float3 normalMapSample, float3 unitNormalW, float3 tangentW) {
    float3 normalT = 2.0f * normalMapSample - 1.0f;
    float3 N = unitNormalW;
    float3 T = normalize(tangentW - dot(tangentW, N) * N);
    float3 B = cross(N, T);

    float3x3 TBN = float3x3(T, B, N);
    float3 bumpedNormalW = mul(normalT, TBN);

    return bumpedNormalW;
}

PixelOut PS(VertexOut pin)
{
    //PixelOut outp = (PixelOut)0.0f;;
    //outp.gPosition = float4(pin.FragPos, 0);
    //float2 g_TexCoords = u_TextureOffset + float2(fmod(pin.TexCoords.x * u_TextureTiling.x, 1), fmod(pin.TexCoords.y * u_TextureTiling.y, 1));
    //if (u_HeightScale > 0.0) {
    //    //g_TexCoords = ParallaxMapping(normalize(fs_in.TangentViewPos - fs_in.TangentFragPos));
    //}
	//float3 g_Normal = u_NormalMap.Sample(gsamAnisotropicWrap, g_TexCoords).rgb;
    //g_Normal = normalize(g_Normal * 2.0 - 1.0);
    //g_Normal = normalize(mul(g_Normal, pin.TBN));
    //outp.gNormal = float4(g_Normal, 1);
    //outp.gRoughAO.b = abs(pin.EyeSpacePosition.z / pin.EyeSpacePosition.w);
    //float4 albedoRGBA     = u_AlbedoMap.Sample(gsamAnisotropicWrap, g_TexCoords) * u_Albedo;
    //outp.gAlbedoSpec.rgb = pow(albedoRGBA.rgb, float3(2.2, 2.2, 2.2));
    //outp.gAlbedoSpec.a = u_MetallicMap.Sample(gsamAnisotropicWrap, g_TexCoords).r * u_Metallic;
    //outp.gRoughAO.r = u_RoughnessMap.Sample(gsamAnisotropicWrap, g_TexCoords).r * u_Roughness;
    //outp.gRoughAO.g = u_AmbientOcclusionMap.Sample(gsamAnisotropicWrap, g_TexCoords).r;
    //return outp;

    pin.NormalW = normalize(pin.NormalW);

    float4 normalSample = u_NormalMap.Sample(gsamAnisotropicWrap, pin.TexC);
    float3 bumpedNormalW = NormalSampleToWorldSpace(normalSample.xyz, pin.NormalW, pin.TangentW);
   
    float4 diffuseAlbedo = u_Albedo * 
    u_AlbedoMap.Sample(gsamAnisotropicWrap, pin.TexC);
    
    
    const float shininess = (1.0f - u_Roughness) * normalSample.a;
    
    PixelOut pout;
    pout.gPosition = float4(pin.PosW, 0);
    pout.gNormal = float4(bumpedNormalW, shininess);
    

    //pout.gRoughAO.b = abs(pin.EyeSpacePosition.z / pin.EyeSpacePosition.w);
    float4 albedoRGBA = u_AlbedoMap.Sample(gsamAnisotropicWrap, pin.TexC);
    pout.gAlbedoSpec.rgb = albedoRGBA.rgb;// pow(albedoRGBA.rgb, float3(2.2, 2.2, 2.2));
    pout.gAlbedoSpec.a = u_MetallicMap.Sample(gsamAnisotropicWrap, pin.TexC).r * u_Metallic;
    pout.gRoughAO.r = u_RoughnessMap.Sample(gsamAnisotropicWrap, pin.TexC).r * u_Roughness;
    pout.gRoughAO.g = u_AmbientOcclusionMap.Sample(gsamAnisotropicWrap, pin.TexC).r;
    
    //float3 toEyeW = normalize(gEyePosW - pin.PosW);
    //float3 ref = reflect(-toEyeW, bumpedNormalW);
    //float4 reflectColor = float4(1.0, 1.0, 1.0, 1.0); //gCubeMap.Sample(gsamAnisotropicWrap, ref);
    //float3 fresnelFactor = SchlickFresnel(matData.FresnelR0, bumpedNormalW, ref);
    //pout.color = float4(diffuseAlbedo.xyz + shininess * fresnelFactor * reflectColor.xyz, 1.0f);
    
    return pout;
}


