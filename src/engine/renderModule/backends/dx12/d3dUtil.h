#pragma once
#ifdef DX12_BACKEND
#include "renderModule/backends/interface/reflectionStructs.h"
#include <windows.h>
#include <wrl.h>
#include <dxgi1_4.h>
#include <d3d12.h>
#include <D3Dcompiler.h>
#include <DirectXMath.h>
#include <string>
#include <cstdint>
#include <fstream>
#include <functional>

class DxException {
public:
    DxException() = default;
    DxException(HRESULT hr, const std::wstring& functionName, const std::wstring& filename, int lineNumber);

    std::wstring ToString()const;

    HRESULT ErrorCode = S_OK;
    std::wstring FunctionName;
    std::wstring Filename;
    int LineNumber = -1;
};

inline std::wstring AnsiToWString(const std::string& str) {
    WCHAR buffer[512];
    MultiByteToWideChar(CP_ACP, 0, str.c_str(), -1, buffer, 512);
    return std::wstring(buffer);
}

namespace IKIGAI::RENDER
{
	struct ShaderReflection;
	class DriverDx12;

	class d3dUtil {
    public:
        static Microsoft::WRL::ComPtr<ID3DBlob> CompileShader(ShaderReflection& reflection, const std::wstring& filename, const D3D_SHADER_MACRO* defines, const std::wstring& entrypoint, const std::wstring& target, ShaderType shaderType);

        static void OneTimeSubmit(std::function<void(ID3D12GraphicsCommandList*)> func);
        static Microsoft::WRL::ComPtr<ID3D12Resource> CreateBuffer(uint64_t size);

        static DriverDx12* mDriver;
        static DriverDx12* GetDriver();
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