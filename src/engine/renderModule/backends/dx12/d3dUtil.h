//***************************************************************************************
// d3dUtil.h by Frank Luna (C) 2015 All Rights Reserved.
//
// General helper code.
//***************************************************************************************

//fxc "color.hlsl" / Od / Zi / T vs_5_0 / E "VS" / Fo "color_vs.cso" / Fc "color_vs.asm"

#pragma once
#ifdef DX12_BACKEND
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <wrl.h>
#include <dxgi1_4.h>
#include <d3d12.h>
#include <D3Dcompiler.h>
#include <DirectXMath.h>
#include <DirectXPackedVector.h>
#include <DirectXColors.h>
#include <DirectXCollision.h>
#include <string>
#include <memory>
#include <algorithm>
#include <vector>
#include <array>
#include <unordered_map>
#include <cstdint>
#include <fstream>
#include <sstream>
#include <cassert>
#include "d3dx12.h"
#include "MathHelper.h"
extern const int gNumFrameResources;
namespace IKIGAI::RENDER
{

    using Microsoft::WRL::ComPtr;

    inline void d3dSetDebugName(IDXGIObject* obj, const char* name)
    {
        if (obj)
        {
            obj->SetPrivateData(WKPDID_D3DDebugObjectName, lstrlenA(name), name);
        }
    }
    inline void d3dSetDebugName(ID3D12Device* obj, const char* name)
    {
        if (obj)
        {
            obj->SetPrivateData(WKPDID_D3DDebugObjectName, lstrlenA(name), name);
        }
    }
    inline void d3dSetDebugName(ID3D12DeviceChild* obj, const char* name)
    {
        if (obj)
        {
            obj->SetPrivateData(WKPDID_D3DDebugObjectName, lstrlenA(name), name);
        }
    }

    inline std::wstring AnsiToWString(const std::string& str)
    {
        WCHAR buffer[512];
        MultiByteToWideChar(CP_ACP, 0, str.c_str(), -1, buffer, 512);
        return std::wstring(buffer);
    }

    /*
    #if defined(_DEBUG)
        #ifndef Assert
        #define Assert(x, description)                                  \
        {                                                               \
            static bool ignoreAssert = false;                           \
            if(!ignoreAssert && !(x))                                   \
            {                                                           \
                Debug::AssertResult result = Debug::ShowAssertDialog(   \
                (L#x), description, AnsiToWString(__FILE__), __LINE__); \
            if(result == Debug::AssertIgnore)                           \
            {                                                           \
                ignoreAssert = true;                                    \
            }                                                           \
                        else if(result == Debug::AssertBreak)           \
            {                                                           \
                __debugbreak();                                         \
            }                                                           \
            }                                                           \
        }
        #endif
    #else
        #ifndef Assert
        #define Assert(x, description)
        #endif
    #endif
        */

    class d3dUtil
    {
    public:

        static bool IsKeyDown(int vkeyCode);

        static std::string ToString(HRESULT hr);

        static UINT CalcConstantBufferByteSize(UINT byteSize)
        {
            // 常量缓冲区的大小必须是硬件最小分配空间的整数倍(最小空间通常是256B)
            // 为此需要凑齐256的整数倍
            // 将输入值的byteSize加上255, 再屏蔽求和结果的低2字节(即结果中小于256的部分)
            // 以此来实现凑齐256倍数
            // 例如 byteSize = 300.
            // (300 + 255) & ~255
            // 555 & ~255
            // 0x022B & ~0x00ff
            // 0x022B & 0xff00
            // 0x0200
            // 512
            return (byteSize + 255) & ~255;
        }

        static ComPtr<ID3DBlob> LoadBinary(const std::wstring& filename);
        static ComPtr<ID3D12Resource> CreateDefaultBuffer(const void* initData, UINT64 byteSize, ComPtr<ID3D12Resource>& uploadBuffer);
        static ComPtr<ID3D12Resource> CreateDefaultBuffer(ID3D12Device* device,
            ID3D12GraphicsCommandList* cmdList,
            const void* initData, UINT64 byteSize,
            ComPtr<ID3D12Resource>& uploadBuffer);

        static ComPtr<ID3DBlob> CompileShader(const std::wstring& filename,
            const D3D_SHADER_MACRO* defines,
            const std::string& entrypoint,
            const std::string& target);
    };

    class DxException
    {
    public:
        DxException() = default;
        DxException(HRESULT hr, const std::wstring& functionName, const std::wstring& filename, int lineNumber);

        std::wstring ToString()const;

        HRESULT ErrorCode = S_OK;
        std::wstring FunctionName;
        std::wstring Filename;
        int LineNumber = -1;
    };

    // 利用SubmeshGeometry来定义MeshGermetry中存储的单个几何体
    // 此结构体适用于将多个几何体存储于一个顶点缓冲(即全局顶点缓冲)和一个索引缓冲(即全局索引缓冲)
    // 他提供了单个几何体在两个全局缓冲区中的偏移量(即起始位置)
    // 以此来实现图6.3的技术(P186)
    struct SubmeshGeometry
    {
        UINT IndexCount = 0;
        UINT StartIndexLocation = 0;
        INT BaseVertexLocation = 0;

        // 包围盒
        // 后续章节中会使用
        DirectX::BoundingBox Bounds;
    };

    struct MeshGeometry
    {
        // 指定几何体集合的名称
        std::string Name;

        // 系统内存的副本. 由于顶点和索引可以是泛型格式, 所以用Blob来表示
        // 用户使用时再转换为合适的格式(float, int等)
        ComPtr<ID3DBlob> VertexBufferCPU = nullptr;
        ComPtr<ID3DBlob> IndexBufferCPU = nullptr;

        ComPtr<ID3D12Resource> VertexBufferGPU = nullptr;
        ComPtr<ID3D12Resource> IndexBufferGPU = nullptr;

        ComPtr<ID3D12Resource> VertexBufferUploader = nullptr;
        ComPtr<ID3D12Resource> IndexBufferUploader = nullptr;

        // 与缓冲区相关数据
        UINT VertexByteStride = 0;
        UINT VertexBufferByteSize = 0;
        DXGI_FORMAT IndexFormat = DXGI_FORMAT_R16_UINT;
        UINT IndexBufferByteSize = 0;

        // 一个MeshGeometry对象可以存储一组顶点/索引缓冲区中多个几何体
        // 用散列表来储存单个几何体的起始位置
        std::unordered_map<std::string, SubmeshGeometry> DrawArgs;

        D3D12_VERTEX_BUFFER_VIEW VertexBufferView()const
        {
            D3D12_VERTEX_BUFFER_VIEW vbv;
            vbv.BufferLocation = VertexBufferGPU->GetGPUVirtualAddress();
            vbv.StrideInBytes = VertexByteStride;
            vbv.SizeInBytes = VertexBufferByteSize;

            return vbv;
        }

        D3D12_INDEX_BUFFER_VIEW IndexBufferView()const
        {
            D3D12_INDEX_BUFFER_VIEW ibv;
            ibv.BufferLocation = IndexBufferGPU->GetGPUVirtualAddress();
            ibv.Format = IndexFormat;
            ibv.SizeInBytes = IndexBufferByteSize;

            return ibv;
        }

        // 顶点上传完成后可释放内存
        void DisposeUploaders()
        {
            VertexBufferUploader = nullptr;
            IndexBufferUploader = nullptr;
        }
    };

    //struct Light
    //{
    //    DirectX::XMFLOAT3 Strength = { 0.5f, 0.5f, 0.5f };  // 光源颜色
    //    float FalloffStart = 1.0f;                          // 供点光源/聚光灯使用的衰减开始距离
    //    DirectX::XMFLOAT3 Direction = { 0.0f, -1.0f, 0.0f };// 供方向光/聚光灯使用的光源方向
    //    float FalloffEnd = 10.0f;                           // 供点光源/聚光灯使用的衰减结束距离
    //    DirectX::XMFLOAT3 Position = { 0.0f, 0.0f, 0.0f };  // 供点光源/聚光灯使用的光源位置
    //    float SpotPower = 64.0f;                            // 供聚光灯使用的光强衰减幂
    //};

#define MaxLights 16

    struct MaterialConstants
    {
        DirectX::XMFLOAT4 DiffuseAlbedo = { 1.0f, 1.0f, 1.0f, 1.0f };
        DirectX::XMFLOAT3 FresnelR0 = { 0.01f, 0.01f, 0.01f };
        float Roughness = 0.25f;

        // 在贴图那一章会用到
        DirectX::XMFLOAT4X4 MatTransform = MathHelper::Identity4x4();
    };


    // 表示材质的结构体
    struct Material
    {
        // 材质的名称
        std::string Name;

        // 本材质的常量缓冲区索引
        //int MatCBIndex = -1;

        // 漫反射纹理在SRV堆中的索引
        //int DiffuseSrvHeapIndex = -1;

        // 法线贴图在SRV堆的索引
        //int NormalSrvHeapIndex = -1;

        // 已更新标志标识本材质已有改动, 因此也需要更新常量区.
        // 每个帧资源 FrameResource 都有一个材质常量缓冲区, 所以需要对每个帧资源都进行更新.
        // 因此, 修改某个材质时, 应设置NumFramesDirty = gNumFrameResources 以使每个帧资源都更新
        int NumFramesDirty = gNumFrameResources;

        // 用于着色的材质常量缓冲区数据
        DirectX::XMFLOAT4 DiffuseAlbedo = { 1.0f, 1.0f, 1.0f, 1.0f };  //漫反射
        DirectX::XMFLOAT3 FresnelR0 = { 0.01f, 0.01f, 0.01f };  //材质属性R0
        float Roughness = .25f;  //粗糙度
        DirectX::XMFLOAT4X4 MatTransform = MathHelper::Identity4x4();
        float metallic = 0.0f;
    };
}
#ifndef ThrowIfFailed
#define ThrowIfFailed(x)                                              \
{                                                                     \
    HRESULT hr__ = (x);                                               \
    std::wstring wfn = AnsiToWString(__FILE__);                       \
    if(FAILED(hr__)) { throw DxException(hr__, L#x, wfn, __LINE__); } \
}
#endif

#ifndef ReleaseCom
#define ReleaseCom(x) { if(x){ x->Release(); x = 0; } }
#endif

#endif