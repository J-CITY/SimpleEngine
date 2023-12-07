#include "textureDx12.h"
#ifdef DX12_BACKEND
#include "d3dx12.h"
#include "d3dUtil.h"
#include <stdexcept>

#include "driverDx12.h"

#define STB_IMAGE_IMPLEMENTATION
#include <functional>


using namespace IKIGAI;
using namespace IKIGAI::RENDER;
void OneTimeSubmit(ID3D12Device* device, const std::function<void(ID3D12GraphicsCommandList*)> func)
{
	D3D12_COMMAND_QUEUE_DESC queue_desc = {};
	queue_desc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	queue_desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	queue_desc.NodeMask = 1;

	ComPtr<ID3D12CommandQueue> cmd_queue;
	device->CreateCommandQueue(&queue_desc, IID_PPV_ARGS(cmd_queue.GetAddressOf()));

	ComPtr<ID3D12CommandAllocator> cmd_alloc;
	device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(cmd_alloc.GetAddressOf()));

	ComPtr<ID3D12GraphicsCommandList> cmd_list;
	device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, cmd_alloc.Get(), NULL,
		IID_PPV_ARGS(cmd_list.GetAddressOf()));

	ComPtr<ID3D12Fence> fence = NULL;
	device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(fence.GetAddressOf()));

	HANDLE event = CreateEvent(0, 0, 0, 0);

	func(cmd_list.Get());
	cmd_list->Close();
	cmd_queue->ExecuteCommandLists(1, CommandListCast(cmd_list.GetAddressOf()));
	cmd_queue->Signal(fence.Get(), 1);

	fence->SetEventOnCompletion(1, event);
	WaitForSingleObject(event, INFINITE);
}

#include "../../gameRendererDx12.h"
#include <coreModule/resourceManager/textureManager.h>
std::shared_ptr<TextureDx12> TextureDx12::Create(std::string path) {
    int texWidth, texHeight, texChannels;
    unsigned char* pixels = IKIGAI::RESOURCES::stbiLoad(path.c_str(), &texWidth, &texHeight, &texChannels, 4);
    if (!pixels) {
        throw std::runtime_error("failed to load texture image!");
    }

    auto texture = std::make_shared<TextureDx12>();

	const auto format = DXGI_FORMAT_R8G8B8A8_UNORM;

	auto prop = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
	auto desc = CD3DX12_RESOURCE_DESC::Tex2D(format, texWidth, texHeight);

	desc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;

	GameRendererDx12::mApp->mDriver->mDevice->CreateCommittedResource(&prop, D3D12_HEAP_FLAG_NONE, &desc,
		D3D12_RESOURCE_STATE_COMMON, NULL, IID_PPV_ARGS(texture->Resource.GetAddressOf()));

	//D3D12_DESCRIPTOR_HEAP_DESC heap_desc = {};
	//heap_desc.NumDescriptors = 1;
	//heap_desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	//heap_desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	//GameRendererDx12::mApp->mDriver->mDevice->CreateDescriptorHeap(&heap_desc, IID_PPV_ARGS(texture->UploadHeap.GetAddressOf()));

	D3D12_SHADER_RESOURCE_VIEW_DESC srv_desc = {};
	srv_desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srv_desc.Format = format;
	srv_desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srv_desc.Texture2D.MipLevels = 1;

	auto device = GameRendererDx12::mApp->mDriver->mDevice;
	auto& offset = GameRendererDx12::mApp->mDriver->mTesturesDescOffset;
	auto mCbvSrvDescriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	auto mRtvDescriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	auto cpuSrv = CD3DX12_CPU_DESCRIPTOR_HANDLE(
		GameRendererDx12::mApp->mDriver->mTexturesDescHeap->GetCPUDescriptorHandleForHeapStart()).Offset(offset, mCbvSrvDescriptorSize);;
	texture->mCpuSrv = cpuSrv;
	GameRendererDx12::mApp->mDriver->mDevice->CreateShaderResourceView(texture->Resource.Get(), &srv_desc, cpuSrv);

	auto gpuSrv = CD3DX12_GPU_DESCRIPTOR_HANDLE(
		GameRendererDx12::mApp->mDriver->mTexturesDescHeap->GetGPUDescriptorHandleForHeapStart()).Offset(offset, mCbvSrvDescriptorSize);;
	texture->mGpuSrv = gpuSrv;

	auto upload_size = GetRequiredIntermediateSize(texture->Resource.Get(), 0, 1);
	auto upload_desc = CD3DX12_RESOURCE_DESC::Buffer(upload_size);
	auto upload_prop = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);

	ComPtr<ID3D12Resource> upload_buffer = NULL;

	GameRendererDx12::mApp->mDriver->mDevice->CreateCommittedResource(&upload_prop, D3D12_HEAP_FLAG_NONE, &upload_desc,
		D3D12_RESOURCE_STATE_GENERIC_READ, NULL, IID_PPV_ARGS(upload_buffer.GetAddressOf()));

	D3D12_SUBRESOURCE_DATA subersource_data = {};
	subersource_data.pData = pixels;
	subersource_data.RowPitch = texWidth * texChannels;
	subersource_data.SlicePitch = texWidth * texHeight * texChannels;

	offset++;
	//auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(texture->Resource.Get(),
	//	D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_DEST);
	//
	//DriverDx12::GetInstance()->mCommandList->ResourceBarrier(1, &barrier);
	//
	//UpdateSubresources(DriverDx12::GetInstance()->mCommandList.Get(), texture->Resource.Get(), upload_buffer.Get(), 0, 0, 1, &subersource_data);
	//
	//barrier = CD3DX12_RESOURCE_BARRIER::Transition(texture->Resource.Get(),
	//	D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	//
	//DriverDx12::GetInstance()->mCommandList->ResourceBarrier(1, &barrier);

	OneTimeSubmit(GameRendererDx12::mApp->mDriver->mDevice.Get(), [&](ID3D12GraphicsCommandList* cmdlist) {
		auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(texture->Resource.Get(),
		D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_DEST);

		cmdlist->ResourceBarrier(1, &barrier);

		UpdateSubresources(cmdlist, texture->Resource.Get(), upload_buffer.Get(), 0, 0, 1, &subersource_data);

		barrier = CD3DX12_RESOURCE_BARRIER::Transition(texture->Resource.Get(),
		D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

		cmdlist->ResourceBarrier(1, &barrier);
	});
	
	//if (mipmap)
	//{
	//	generateMips();
	//}
	return texture;
}

std::shared_ptr<TextureDx12> TextureDx12::CreateForAttach(size_t width, size_t height) {
	auto texture = std::make_shared<TextureDx12>();
	texture->mHeight = height;
	texture->mWidth = width;
	
	BuildResource(texture);
	BuildDescriptors(texture);


	//OneTimeSubmit(DriverDx12::GetInstance()->device.Get(), [&](ID3D12GraphicsCommandList* cmdlist) {
	//	auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(texture->Resource.Get(),
	//	D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_DEST);
	//
	//	cmdlist->ResourceBarrier(1, &barrier);
	//
	//	//UpdateSubresources(cmdlist, texture->Resource.Get(), upload_buffer.Get(), 0, 0, 1, &subersource_data);
	//
	//	barrier = CD3DX12_RESOURCE_BARRIER::Transition(texture->Resource.Get(),
	//		D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	//
	//	cmdlist->ResourceBarrier(1, &barrier);
	//});

	return texture;
}

void TextureDx12::BuildResource(std::shared_ptr<TextureDx12> texture) {
	auto device = GameRendererDx12::mApp->mDriver->mDevice;

	D3D12_RESOURCE_DESC texDesc;
	ZeroMemory(&texDesc, sizeof(D3D12_RESOURCE_DESC));
	texDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	texDesc.Alignment = 0;
	texDesc.Width = texture->mWidth;
	texDesc.Height = texture->mHeight;
	texDesc.DepthOrArraySize = 1;
	texDesc.MipLevels = 1;
	texDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;
	texDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	texDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
	
	auto prop = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
	ThrowIfFailed(device->CreateCommittedResource(
		&prop,
		D3D12_HEAP_FLAG_NONE,
		&texDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		&texture->optClear,
		IID_PPV_ARGS(&texture->Resource)));
}

void TextureDx12::BuildDescriptors(std::shared_ptr<TextureDx12> texture) {
	auto device = GameRendererDx12::mApp->mDriver->mDevice;

	
	//D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc = {};
	//srvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	//srvHeapDesc.NumDescriptors = 1;
	//srvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	//ThrowIfFailed(device->CreateDescriptorHeap(&srvHeapDesc, IID_PPV_ARGS(&texture->UploadHeap)));

	auto& offset = GameRendererDx12::mApp->mDriver->mTesturesDescOffset;
	auto mCbvSrvDescriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	auto mRtvDescriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	auto cpuSrv = CD3DX12_CPU_DESCRIPTOR_HANDLE(
		GameRendererDx12::mApp->mDriver->mTexturesDescHeap->GetCPUDescriptorHandleForHeapStart()).Offset(offset, mCbvSrvDescriptorSize);
	auto gpuSrv = CD3DX12_GPU_DESCRIPTOR_HANDLE(
		GameRendererDx12::mApp->mDriver->mTexturesDescHeap->GetGPUDescriptorHandleForHeapStart()).Offset(offset, mCbvSrvDescriptorSize);
	//auto cpuRtv = CD3DX12_CPU_DESCRIPTOR_HANDLE(
	//	texture->mRtvHeap->GetCPUDescriptorHandleForHeapStart()).Offset(0, mRtvDescriptorSize);


	texture->mCpuSrv = cpuSrv;
	texture->mGpuSrv = gpuSrv;
	//texture->mCpuRtv = cpuRtv;
	texture->mSrvDescSize = mCbvSrvDescriptorSize;
	texture->mRtvDescSize = mRtvDescriptorSize;
	////////////////////////////////////////////////////


	auto _cpuSrv = texture->mCpuSrv;
	//auto _cpuRtv = texture->mCpuRtv;
	
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Format = texture->mNormalPosFormat;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = 1;
	device->CreateShaderResourceView(texture->Resource.Get(), &srvDesc, _cpuSrv);
	
	offset++;
	//D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
	//rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
	//rtvDesc.Format = texture->mNormalPosFormat;
	//rtvDesc.Texture2D.MipSlice = 0;
	//rtvDesc.Texture2D.PlaneSlice = 0;
	//device->CreateRenderTargetView(texture->Resource.Get(), &rtvDesc, _cpuRtv);

}
#endif
