#pragma once

#ifdef DX12_BACKEND
#include "d3dUtil.h"
#include "../../gameRendererDx12.h"
namespace IKIGAI::RENDER
{
    template<typename T>
    class UploadBuffer
    {
    public:
        UploadBuffer(UINT elementCount, bool isConstantBuffer) : mIsConstantBuffer(isConstantBuffer)
        {
            mElementByteSize = sizeof(T);

            if (isConstantBuffer)
                mElementByteSize = d3dUtil::CalcConstantBufferByteSize(sizeof(T));

            ThrowIfFailed(GameRendererDx12::mApp->mDriver->mDevice->CreateCommittedResource(
                &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
                D3D12_HEAP_FLAG_NONE,
                &CD3DX12_RESOURCE_DESC::Buffer(mElementByteSize * elementCount),
                D3D12_RESOURCE_STATE_GENERIC_READ,
                nullptr,
                IID_PPV_ARGS(&mUploadBuffer)));

            ThrowIfFailed(mUploadBuffer->Map(0, nullptr, reinterpret_cast<void**>(&mMappedData)));
        }

        UploadBuffer(const UploadBuffer& rhs) = delete;
        UploadBuffer& operator=(const UploadBuffer& rhs) = delete;

        ~UploadBuffer()
        {
            if (mUploadBuffer != nullptr)
                mUploadBuffer->Unmap(0, nullptr);

            mMappedData = nullptr;
        }

        ID3D12Resource* Resource()const
        {
            return mUploadBuffer.Get();
        }

        void CopyData(int elementIndex, const T& data)
        {
            memcpy(&mMappedData[elementIndex * mElementByteSize], &data, sizeof(T));
        }

    private:
        ComPtr<ID3D12Resource> mUploadBuffer;
        BYTE* mMappedData = nullptr;

        UINT mElementByteSize = 0;
        bool mIsConstantBuffer = false;
    };
}
#endif