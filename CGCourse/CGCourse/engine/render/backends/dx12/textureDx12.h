#pragma once
#ifdef DX12_BACKEND
#include <memory>
#include <string>
#include <wrl/client.h>

#include <d3d12.h>
#include "d3dx12.h"
#include "../interface/textureInterface.h"

namespace KUMA::RENDER {
	class TextureDx12: public TextureInterface {
	public:
		std::string Name;

		std::string Filename;

		Microsoft::WRL::ComPtr<ID3D12Resource> Resource = nullptr;

		//For render target
		UINT mSrvDescSize;
		UINT mRtvDescSize;
		CD3DX12_CPU_DESCRIPTOR_HANDLE mCpuSrv;
		CD3DX12_GPU_DESCRIPTOR_HANDLE mGpuSrv;
		//CD3DX12_CPU_DESCRIPTOR_HANDLE mCpuRtv;
		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> mRtvHeap = nullptr;

		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> UploadHeap = nullptr;

		static std::shared_ptr<TextureDx12> Create(std::string path);
		static std::shared_ptr<TextureDx12> TextureDx12::CreateForAttach(size_t width, size_t height);
		size_t mWidth = 0;
		size_t mHeight = 0;

		void buildDescriptorHeaps();

		DXGI_FORMAT mColorFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
		DXGI_FORMAT mNormalPosFormat = DXGI_FORMAT_R16G16B16A16_FLOAT;

		float ClearColor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
		CD3DX12_CLEAR_VALUE optClear = { mNormalPosFormat, ClearColor };


		static void BuildDescriptors(std::shared_ptr<TextureDx12> texture);
		static void BuildResource(std::shared_ptr<TextureDx12> texture);
	};
}
#endif