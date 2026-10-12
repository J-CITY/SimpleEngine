#include "frameBufferDx12.h"
#ifdef DX12_BACKEND
#include "d3dUtil.h"
#include "driverDx12.h"
#include "textureDx12.h"
#include "d3dx12/DirectXHelpers.h"
#include "renderModule/backends/interface/resourceStruct.h"


IKIGAI::RENDER::FrameBufferDx12::FrameBufferDx12(const std::vector<std::shared_ptr<TextureInterface>>& textures, std::shared_ptr<TextureInterface> depth, unsigned mip, unsigned layer): mTextures(textures), mDepth(depth) {
	mMip=mip; mLayer=layer;
	if (textures.empty() && !depth) {
		throw std::runtime_error("Framebuffer needs a color or depth attachment");
	}
	static size_t ID = 0;
	mId = ID;
	ID++;
	auto sizeTexture = mTextures.empty() ? mDepth : mTextures.front();
	mWidth = std::max<size_t>(1,sizeTexture->getWidth() >> mMip);
	mHeight = std::max<size_t>(1,sizeTexture->getHeight() >> mMip);

	D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
	heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	heapDesc.NumDescriptors = std::max<size_t>(1, textures.size());
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
        auto resource=std::static_pointer_cast<TextureDx12>(texture)->getResource().Get();
        D3D12_RENDER_TARGET_VIEW_DESC view{}; view.Format=resource->GetDesc().Format;
        if(texture->getType()==TextureType::TEXTURE_CUBE || texture->getType()==TextureType::TEXTURE_2D_ARRAY) {
            view.ViewDimension=D3D12_RTV_DIMENSION_TEXTURE2DARRAY;view.Texture2DArray.MipSlice=mMip;view.Texture2DArray.FirstArraySlice=mLayer;view.Texture2DArray.ArraySize=1;
        } else { view.ViewDimension=D3D12_RTV_DIMENSION_TEXTURE2D;view.Texture2D.MipSlice=mMip; }
        d3dUtil::GetDriver()->getDevice()->CreateRenderTargetView(resource,&view,handler);
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
	dsv_desc.Format = depthTexture->getFormat() == PixelFormat::DEPTH32_FLOAT ? DXGI_FORMAT_D32_FLOAT : d3dUtil::GetDriver()->DefaultDepthFormat;
	dsv_desc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	d3dUtil::GetDriver()->getDevice() ->CreateDepthStencilView(depthTexture->getResource().Get(), &dsv_desc,
		mDepthHeap->GetCPUDescriptorHandleForHeapStart());
}
#endif
