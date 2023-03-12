#pragma once

#ifdef DX12_BACKEND
#include <array>
#include <d3d12.h>
#include <wrl/client.h>
#include "../interface/driverInterface.h"
#include "d3dx12.h"

#if defined(DEBUG) || defined(_DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif

#include "d3dUtil.h"

namespace KUMA::RENDER {
	class DriverDx12 : public KUMA::RENDER::DriverInterface {
	public:
		bool mUseDepth = false;

		UINT mCbvSrvDescriptorSize = 0;
		static const int mSwapChainBufferCount = 2;

		inline static DriverDx12* instance = nullptr;

		int mCurrFrameResourceIndex = 0;

		static std::array<const CD3DX12_STATIC_SAMPLER_DESC, 6> GetStaticSamplers()
		{
			// Applications usually only need a handful of samplers.  So just define them all up front
			// and keep them available as part of the root signature.  

			const CD3DX12_STATIC_SAMPLER_DESC pointWrap(
				0, // shaderRegister
				D3D12_FILTER_MIN_MAG_MIP_POINT, // filter
				D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressU
				D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressV
				D3D12_TEXTURE_ADDRESS_MODE_WRAP); // addressW

			const CD3DX12_STATIC_SAMPLER_DESC pointClamp(
				1, // shaderRegister
				D3D12_FILTER_MIN_MAG_MIP_POINT, // filter
				D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressU
				D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressV
				D3D12_TEXTURE_ADDRESS_MODE_CLAMP); // addressW

			const CD3DX12_STATIC_SAMPLER_DESC linearWrap(
				2, // shaderRegister
				D3D12_FILTER_MIN_MAG_MIP_LINEAR, // filter
				D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressU
				D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressV
				D3D12_TEXTURE_ADDRESS_MODE_WRAP); // addressW

			const CD3DX12_STATIC_SAMPLER_DESC linearClamp(
				3, // shaderRegister
				D3D12_FILTER_MIN_MAG_MIP_LINEAR, // filter
				D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressU
				D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressV
				D3D12_TEXTURE_ADDRESS_MODE_CLAMP); // addressW

			const CD3DX12_STATIC_SAMPLER_DESC anisotropicWrap(
				4, // shaderRegister
				D3D12_FILTER_ANISOTROPIC, // filter
				D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressU
				D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressV
				D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressW
				0.0f,                             // mipLODBias
				8);                               // maxAnisotropy

			const CD3DX12_STATIC_SAMPLER_DESC anisotropicClamp(
				5, // shaderRegister
				D3D12_FILTER_ANISOTROPIC, // filter
				D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressU
				D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressV
				D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressW
				0.0f,                              // mipLODBias
				8);                                // maxAnisotropy

			return {
				pointWrap, pointClamp,
				linearWrap, linearClamp,
				anisotropicWrap, anisotropicClamp };
		}

		///////////////////////////

		int init() override;
		void deinit();
		void createRtvAndDsvDescriptorHeaps();
		void onResize();
		void set4xMsaaState(bool value);
		bool get4xMsaaState() const;
		void switchMSAA();
		bool initDirect3D();
		void flushCommandQueue();
		void createSwapChain();
		D3D12_CPU_DESCRIPTOR_HANDLE currentBackBufferView() const;
		D3D12_CPU_DESCRIPTOR_HANDLE depthStencilView() const;
		void logAdapters();
		void logAdapterOutputs(IDXGIAdapter* adapter);
		void logOutputDisplayModes(IDXGIOutput* output, DXGI_FORMAT format);
		ID3D12Resource* currentBackBuffer() const;
		void createCommandObjects();
		void begin() override {};
		void end() override {};
		void setViewport(const ShaderInterface& shader, float x, float y, float w, float h) override {};
		void setScissor(const ShaderInterface& shader, int x, int y, unsigned w, unsigned h) override {};
		void drawIndexed(std::shared_ptr<ShaderInterface> shader, size_t indexCount) override {};
		void draw(std::shared_ptr<ShaderInterface> shader, size_t vertexCount) override {};
		void submit() override {};
		void cleanup() override {};

		Microsoft::WRL::ComPtr<IDXGIFactory4> mDxgiFactory;
		Microsoft::WRL::ComPtr<IDXGISwapChain> mSwapChain;
		Microsoft::WRL::ComPtr<ID3D12Device> mDevice;

		Microsoft::WRL::ComPtr<ID3D12CommandQueue> mCommandQueue;
		Microsoft::WRL::ComPtr<ID3D12CommandAllocator> mDirectCmdListAlloc;
		Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> mCommandList;

		static const int SwapChainBufferCount = 2;
		int mCurrBackBuffer = 0;
		Microsoft::WRL::ComPtr<ID3D12Resource> mSwapChainBuffer[SwapChainBufferCount];
		Microsoft::WRL::ComPtr<ID3D12Resource> mDepthStencilBuffer;

		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> mRtvHeap;
		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> mDsvHeap;

		Microsoft::WRL::ComPtr<ID3D12Fence> mFence;

		bool      m4xMsaaState = false;
		UINT      m4xMsaaQuality = 0;

		UINT64 mCurrentFence = 0;

		D3D_DRIVER_TYPE md3dDriverType = D3D_DRIVER_TYPE_HARDWARE;
		DXGI_FORMAT mBackBufferFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
		DXGI_FORMAT mDepthStencilFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;

		D3D12_VIEWPORT mScreenViewport;
		D3D12_RECT mScissorRect;

		UINT mRtvDescriptorSize = 0;
		UINT mDsvDescriptorSize = 0;
		UINT mCbvSrvUavDescriptorSize = 0;

	};

}

#endif

