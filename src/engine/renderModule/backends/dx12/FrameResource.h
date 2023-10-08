#pragma once
#ifdef DX12_BACKEND
#include "d3dUtil.h"
#include "MathHelper.h"
#include "UploadBuffer.h"
#include "../../light.h"

namespace IKIGAI::RENDER
{

    //struct ObjectConstants
    //{
    //    DirectX::XMFLOAT4X4 World = MathHelper::Identity4x4();
    //    DirectX::XMFLOAT4X4 TexTransform = MathHelper::Identity4x4();
    //    //UINT     MaterialIndex;
    //    //UINT     ObjPad0;
    //    //UINT     ObjPad1;
    //    //UINT     ObjPad2;
    //};
    //
    //struct PassConstants
    //{
    //    DirectX::XMFLOAT4X4 View = MathHelper::Identity4x4();
    //    DirectX::XMFLOAT4X4 InvView = MathHelper::Identity4x4();
    //    DirectX::XMFLOAT4X4 Proj = MathHelper::Identity4x4();
    //    DirectX::XMFLOAT4X4 InvProj = MathHelper::Identity4x4();
    //    DirectX::XMFLOAT4X4 ViewProj = MathHelper::Identity4x4();
    //    DirectX::XMFLOAT4X4 InvViewProj = MathHelper::Identity4x4();
    //    DirectX::XMFLOAT3 EyePosW = { 0.0f, 0.0f, 0.0f };
    //    float cbPerObjectPad1 = 0.0f;
    //    DirectX::XMFLOAT2 RenderTargetSize = { 0.0f, 0.0f };
    //    DirectX::XMFLOAT2 InvRenderTargetSize = { 0.0f, 0.0f };
    //    float NearZ = 0.0f;
    //    float FarZ = 0.0f;
    //    float TotalTime = 0.0f;
    //    float DeltaTime = 0.0f;
    //
    //    DirectX::XMFLOAT4 AmbientLight = { 0.0f, 0.0f, 0.0f, 1.0f };
    //
    //    // Indices [0, NUM_DIR_LIGHTS) are directional lights;
    //    // indices [NUM_DIR_LIGHTS, NUM_DIR_LIGHTS+NUM_POINT_LIGHTS) are point lights;
    //    // indices [NUM_DIR_LIGHTS+NUM_POINT_LIGHTS, NUM_DIR_LIGHTS+NUM_POINT_LIGHT+NUM_SPOT_LIGHTS)
    //    // are spot lights for a maximum of MaxLights per object.
    //    LightOGL Lights[MaxLights];
    //};
    //
    //struct MaterialData
    //{
    //    DirectX::XMFLOAT4 DiffuseAlbedo = { 1.0f, 1.0f, 1.0f, 1.0f };
    //    DirectX::XMFLOAT3 FresnelR0 = { 0.01f, 0.01f, 0.01f };
    //    float Roughness = 0.5f;
    //
    //    // Used in texture mapping.
    //    DirectX::XMFLOAT4X4 MatTransform = MathHelper::Identity4x4();
    //
    //    //UINT DiffuseMapIndex = 0;
    //    //UINT NormalMapIndex = 0;
    //    //UINT MaterialPad1;
    //    //UINT MaterialPad2;
    //};

    // Stores the resources needed for the CPU to build the command lists
    // for a frame. 
    struct FrameResource
    {
    public:
        FrameResource();
        FrameResource(const FrameResource& rhs) = delete;
        FrameResource& operator=(const FrameResource& rhs) = delete;
        ~FrameResource();

        Microsoft::WRL::ComPtr<ID3D12CommandAllocator> CmdListAlloc;

        //std::unique_ptr<UploadBuffer<PassConstants>> PassCB = nullptr;
        //std::unique_ptr<UploadBuffer<ObjectConstants>> ObjectCB = nullptr;
        //std::unique_ptr<UploadBuffer<MaterialData>> MaterialBuffer = nullptr;

        UINT64 Fence = 0;
    };

}

#endif
