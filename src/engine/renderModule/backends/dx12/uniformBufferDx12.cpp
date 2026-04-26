#include "uniformBufferDx12.h"

#ifdef DX12_BACKEND
#include "d3dUtil.h"
#include "driverDx12.h"
#include "d3dx12/d3dx12.h"
#include "d3dx12/DirectXHelpers.h"

IKIGAI::RENDER::UniformBufferDx12::UniformBufferDx12(void* data, size_t sz) : UniformBufferInterface(DirectX::AlignUp((int)sz, D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT)) {
	mBuffer = d3dUtil::CreateBuffer(mSizeByte);

	mState = D3D12_RESOURCE_STATE_COMMON;
	d3dUtil::OneTimeSubmit([&](ID3D12GraphicsCommandList* cmdlist) {
		DirectX::TransitionResource(cmdlist, mBuffer.Get(), mState, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
	});
	if (data) {
		UniformBufferDx12::setData(data, sz);
	}
}

IKIGAI::RENDER::UniformBufferDx12::~UniformBufferDx12() {
	d3dUtil::GetDriver()->destroyDeferred(mBuffer);
}

void IKIGAI::RENDER::UniformBufferDx12::setData(const void* data, size_t sz) {
	if (sz > mSizeByte) {
		mSizeByte = DirectX::AlignUp((int)sz, D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT);
		d3dUtil::GetDriver()->destroyDeferred(mBuffer);
		mBuffer = d3dUtil::CreateBuffer(mSizeByte);
	}

	auto buffer = d3dUtil::CreateBuffer(sz);
	void* mapBuffer = nullptr;
	buffer->Map(0, nullptr, &mapBuffer);
	memcpy(mapBuffer, data, sz);
	buffer->Unmap(0, nullptr);

	auto barrier = DirectX::ScopedBarrier(d3dUtil::GetDriver()->getCommandList().Get(), {
		CD3DX12_RESOURCE_BARRIER::Transition(mBuffer.Get(), mState, D3D12_RESOURCE_STATE_COPY_DEST)
	});
	d3dUtil::GetDriver()->getCommandList()->CopyBufferRegion(mBuffer.Get(), 0, buffer.Get(), 0, sz);
	d3dUtil::GetDriver()->destroyDeferred(buffer);
}

const Microsoft::WRL::ComPtr<ID3D12Resource>& IKIGAI::RENDER::UniformBufferDx12::getBuffer() const {
	return mBuffer;
}
#endif

