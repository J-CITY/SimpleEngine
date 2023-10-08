#pragma once
#ifdef DX12_BACKEND
#include "d3dUtil.h"
#include "driverDx12.h"
#include "textureDx12.h"
#include "../../gameRendererDx12.h"
using Microsoft::WRL::ComPtr;

namespace IKIGAI::RENDER {

	enum class BufferType
	{
		pos = 0,
		normal,
		color,
		rough
	};

	class GBuffer
	{
		std::vector<std::shared_ptr<TextureDx12>> mTextures;
	public:
		GBuffer(std::vector<std::shared_ptr<TextureDx12>> textures);

		GBuffer(const GBuffer& rhs) = delete;
		GBuffer& operator=(const GBuffer& rhs) = delete;
		~GBuffer() = default;

		UINT Width()const { return mWidth; }
		UINT Height()const { return mHeight; }

		CD3DX12_CPU_DESCRIPTOR_HANDLE Rtv(BufferType t)const
		{
			auto cpuRtv = mCpuRtv;
			return cpuRtv.Offset((int)t, mRtvDescSize);
		}

		D3D12_VIEWPORT Viewport()const { return mViewport; }
		D3D12_RECT ScissorRect()const { return mScissorRect; }


		void OnResize(UINT newWidth, UINT newHeight);

		FLOAT* clear(/*BufferType type*/) {
			//if (type == BufferType::color)
			//{
			//	optClear.Format = mColorFormat;
			//	return optClear.Color;
			//}
			optClear.Format = mNormalPosFormat;
			return optClear.Color;
		}

	private:
		//void BuildDescriptors();
		//void BuildResource();

	private:

		//ID3D12Device* md3dDevice = nullptr;

		D3D12_VIEWPORT mViewport;
		D3D12_RECT mScissorRect;

		UINT mWidth = 0;
		UINT mHeight = 0;
		DXGI_FORMAT mColorFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
		DXGI_FORMAT mNormalPosFormat = DXGI_FORMAT_R16G16B16A16_FLOAT;

		float ClearColor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
		CD3DX12_CLEAR_VALUE optClear = { mNormalPosFormat, ClearColor };
		FLOAT* c = optClear.Color;

		//UINT mSrvDescSize;
		UINT mRtvDescSize;
		//CD3DX12_CPU_DESCRIPTOR_HANDLE mCpuSrv;
		//CD3DX12_GPU_DESCRIPTOR_HANDLE mGpuSrv;
		CD3DX12_CPU_DESCRIPTOR_HANDLE mCpuRtv;

		//ComPtr<ID3D12Resource> mPositionMap = nullptr;
		//ComPtr<ID3D12Resource> mNormalMap = nullptr;
		//ComPtr<ID3D12Resource> mDiffuseColorMap = nullptr;
	public:
		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> mRtvHeap;
		//ComPtr<ID3D12DescriptorHeap> mSrvDescriptorHeap = nullptr;
	};


	GBuffer::GBuffer(std::vector<std::shared_ptr<TextureDx12>> textures) : mTextures(textures) {
		mWidth = textures[0]->mWidth;
		mHeight = textures[0]->mHeight;

		mViewport = { 0.0f, 0.0f, (float)mWidth, (float)mHeight, 0.0f, 1.0f };
		mScissorRect = { 0, 0, (int)mWidth, (int)mHeight };

		auto device = GameRendererDx12::mApp->mDriver->mDevice;


		D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc;
		rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
		rtvHeapDesc.NumDescriptors = textures.size();
		rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		rtvHeapDesc.NodeMask = 0;
		ThrowIfFailed(device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(mRtvHeap.GetAddressOf())));

		mRtvDescSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

		mCpuRtv = CD3DX12_CPU_DESCRIPTOR_HANDLE(
			mRtvHeap->GetCPUDescriptorHandleForHeapStart()).Offset(0, mRtvDescSize);


		D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
		rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;

		rtvDesc.Texture2D.MipSlice = 0;
		rtvDesc.Texture2D.PlaneSlice = 0;
		auto cpuRtv = mCpuRtv;
		for (auto& texture : textures) {
			rtvDesc.Format = texture->mNormalPosFormat;
			device->CreateRenderTargetView(texture->Resource.Get(), &rtvDesc, cpuRtv);
			cpuRtv.Offset(1, mRtvDescSize);
		}


		//BuildResource();
	}
	void GBuffer::OnResize(UINT newWidth, UINT newHeight)
	{
		//mNormalMap.Reset();
		//mPositionMap.Reset();
		//mDiffuseColorMap.Reset();
		//
		////BuildDescriptors();
		//
		//if (mWidth != newWidth || mHeight != newHeight)
		//{
		//	mWidth = newWidth;
		//	mHeight = newHeight;
		//
		//	// We render to ambient map at half the resolution.
		//	mViewport.TopLeftX = 0.0f;
		//	mViewport.TopLeftY = 0.0f;
		//	mViewport.Width = mWidth;
		//	mViewport.Height = mHeight;
		//	mViewport.MinDepth = 0.0f;
		//	mViewport.MaxDepth = 1.0f;
		//
		//	mScissorRect = { 0, 0, (int)mWidth, (int)mHeight };
		//
		//	BuildResource();
		//}
	}
}

#endif