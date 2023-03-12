#include "FrameResource.h"
#ifdef DX12_BACKEND
#include "driverDx12.h"
#include "../../gameRendererDx12.h"
using namespace KUMA::RENDER;

FrameResource::FrameResource()
{
    ThrowIfFailed(GameRendererDx12::mApp->mDriver->mDevice->CreateCommandAllocator(
        D3D12_COMMAND_LIST_TYPE_DIRECT,
		IID_PPV_ARGS(CmdListAlloc.GetAddressOf())));

    //PassCB = std::make_unique<UploadBuffer<PassConstants>>(1, true);
	//MaterialBuffer = std::make_unique<UploadBuffer<MaterialData>>(1, false);
    //ObjectCB = std::make_unique<UploadBuffer<ObjectConstants>>(1, true);
}

FrameResource::~FrameResource()
{

}

#endif