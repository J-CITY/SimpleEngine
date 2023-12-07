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
            // �����������Ĵ�С������Ӳ����С����ռ��������(��С�ռ�ͨ����256B)
            // Ϊ����Ҫ����256��������
            // ������ֵ��byteSize����255, ��������ͽ���ĵ�2�ֽ�(�������С��256�Ĳ���)
            // �Դ���ʵ�ִ���256����
            // ���� byteSize = 300.
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

    // ����SubmeshGeometry������MeshGermetry�д洢�ĵ���������
    // �˽ṹ�������ڽ����������洢��һ�����㻺��(��ȫ�ֶ��㻺��)��һ����������(��ȫ����������)
    // ���ṩ�˵���������������ȫ�ֻ������е�ƫ����(����ʼλ��)
    // �Դ���ʵ��ͼ6.3�ļ���(P186)
    struct SubmeshGeometry
    {
        UINT IndexCount = 0;
        UINT StartIndexLocation = 0;
        INT BaseVertexLocation = 0;

        // ��Χ��
        // �����½��л�ʹ��
        DirectX::BoundingBox Bounds;
    };

    struct MeshGeometry
    {
        // ָ�������弯�ϵ�����
        std::string Name;

        // ϵͳ�ڴ�ĸ���. ���ڶ�������������Ƿ��͸�ʽ, ������Blob����ʾ
        // �û�ʹ��ʱ��ת��Ϊ���ʵĸ�ʽ(float, int��)
        ComPtr<ID3DBlob> VertexBufferCPU = nullptr;
        ComPtr<ID3DBlob> IndexBufferCPU = nullptr;

        ComPtr<ID3D12Resource> VertexBufferGPU = nullptr;
        ComPtr<ID3D12Resource> IndexBufferGPU = nullptr;

        ComPtr<ID3D12Resource> VertexBufferUploader = nullptr;
        ComPtr<ID3D12Resource> IndexBufferUploader = nullptr;

        // �뻺�����������
        UINT VertexByteStride = 0;
        UINT VertexBufferByteSize = 0;
        DXGI_FORMAT IndexFormat = DXGI_FORMAT_R16_UINT;
        UINT IndexBufferByteSize = 0;

        // һ��MeshGeometry������Դ洢һ�鶥��/�����������ж��������
        // ��ɢ�б������浥�����������ʼλ��
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

        // �����ϴ���ɺ���ͷ��ڴ�
        void DisposeUploaders()
        {
            VertexBufferUploader = nullptr;
            IndexBufferUploader = nullptr;
        }
    };

    //struct Light
    //{
    //    DirectX::XMFLOAT3 Strength = { 0.5f, 0.5f, 0.5f };  // ��Դ��ɫ
    //    float FalloffStart = 1.0f;                          // �����Դ/�۹��ʹ�õ�˥����ʼ����
    //    DirectX::XMFLOAT3 Direction = { 0.0f, -1.0f, 0.0f };// �������/�۹��ʹ�õĹ�Դ����
    //    float FalloffEnd = 10.0f;                           // �����Դ/�۹��ʹ�õ�˥����������
    //    DirectX::XMFLOAT3 Position = { 0.0f, 0.0f, 0.0f };  // �����Դ/�۹��ʹ�õĹ�Դλ��
    //    float SpotPower = 64.0f;                            // ���۹��ʹ�õĹ�ǿ˥����
    //};

#define MaxLights 16

    struct MaterialConstants
    {
        DirectX::XMFLOAT4 DiffuseAlbedo = { 1.0f, 1.0f, 1.0f, 1.0f };
        DirectX::XMFLOAT3 FresnelR0 = { 0.01f, 0.01f, 0.01f };
        float Roughness = 0.25f;

        // ����ͼ��һ�»��õ�
        DirectX::XMFLOAT4X4 MatTransform = MathHelper::Identity4x4();
    };


    // ��ʾ���ʵĽṹ��
    struct Material
    {
        // ���ʵ�����
        std::string Name;

        // �����ʵĳ�������������
        //int MatCBIndex = -1;

        // ������������SRV���е�����
        //int DiffuseSrvHeapIndex = -1;

        // ������ͼ��SRV�ѵ�����
        //int NormalSrvHeapIndex = -1;

        // �Ѹ��±�־��ʶ���������иĶ�, ���Ҳ��Ҫ���³�����.
        // ÿ��֡��Դ FrameResource ����һ�����ʳ���������, ������Ҫ��ÿ��֡��Դ�����и���.
        // ���, �޸�ĳ������ʱ, Ӧ����NumFramesDirty = gNumFrameResources ��ʹÿ��֡��Դ������
        int NumFramesDirty = gNumFrameResources;

        // ������ɫ�Ĳ��ʳ�������������
        DirectX::XMFLOAT4 DiffuseAlbedo = { 1.0f, 1.0f, 1.0f, 1.0f };  //������
        DirectX::XMFLOAT3 FresnelR0 = { 0.01f, 0.01f, 0.01f };  //��������R0
        float Roughness = .25f;  //�ֲڶ�
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