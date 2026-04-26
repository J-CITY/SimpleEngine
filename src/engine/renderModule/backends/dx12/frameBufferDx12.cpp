#include "frameBufferDx12.h"
#ifdef DX12_BACKEND
#include "d3dUtil.h"
#include "driverDx12.h"
#include "textureDx12.h"
#include "d3dx12/DirectXHelpers.h"
#include "renderModule/backends/interface/resourceStruct.h"


IKIGAI::RENDER::FrameBufferDx12::FrameBufferDx12(const std::vector<std::shared_ptr<TextureInterface>>& textures, std::shared_ptr<TextureInterface> depth): mTextures(textures), mDepth(depth) {
	if (textures.empty()) {
		throw;
	}
	static size_t ID = 0;
	mId = ID;
	ID++;
	mWidth = mTextures[0]->getWidth();
	mHeight = mTextures[0]->getHeight();

	D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
	heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	heapDesc.NumDescriptors = textures.size();
	d3dUtil::GetDriver()->getDevice()->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(mTexHeap.GetAddressOf()));

	CD3DX12_CPU_DESCRIPTOR_HANDLE handler{};
	handler = mTexHeap->GetCPUDescriptorHandleForHeapStart();
	create(handler);
}

const std::vector<std::shared_ptr<IKIGAI::RENDER::TextureInterface>>& IKIGAI::RENDER::FrameBufferDx12::getTextures() const {
	return mTextures;
}

const std::shared_ptr<IKIGAI::RENDER::TextureInterface>& IKIGAI::RENDER::FrameBufferDx12::getDepth() const {
	return mDepth;
}

const Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>& IKIGAI::RENDER::FrameBufferDx12::getTexHeap() const {
	return mTexHeap;
}

const Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>& IKIGAI::RENDER::FrameBufferDx12::getDepthHeap() const {
	return mDepthHeap;
}

void IKIGAI::RENDER::FrameBufferDx12::create(CD3DX12_CPU_DESCRIPTOR_HANDLE handler) {
	const auto incSize = d3dUtil::GetDriver()->getDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	for (auto& texture : mTextures) {
		DirectX::CreateRenderTargetView(d3dUtil::GetDriver()->getDevice().Get(), 
			std::static_pointer_cast<TextureDx12>(texture)->getResource().Get(), handler);
		handler.Offset(1, incSize);
	}

	//depth
	if (!mDepth) {
		TextureResource desc;
		desc.texType = TextureType::DEPTH;
		desc.pixelType = PixelFormat::DEPTH_24_UNORM_STENCIL_8_UINT;
		desc.useMipmap = false;
		desc.width = mWidth;
		desc.height = mHeight;
		mDepth = std::make_shared<TextureDx12>(desc, std::vector<void*>{});
	}
	auto depthTexture = std::static_pointer_cast<TextureDx12>(mDepth);

	D3D12_DESCRIPTOR_HEAP_DESC dsv_heap_desc = {};
	dsv_heap_desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	dsv_heap_desc.NumDescriptors = 1;
	d3dUtil::GetDriver()->getDevice()->CreateDescriptorHeap(&dsv_heap_desc, IID_PPV_ARGS(mDepthHeap.GetAddressOf()));

	D3D12_DEPTH_STENCIL_VIEW_DESC dsv_desc = {};
	dsv_desc.Format = d3dUtil::GetDriver()->DefaultDepthFormat;
	dsv_desc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	d3dUtil::GetDriver()->getDevice() ->CreateDepthStencilView(depthTexture->getResource().Get(), &dsv_desc,
		mDepthHeap->GetCPUDescriptorHandleForHeapStart());
}
#endif
