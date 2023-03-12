#pragma once
#ifdef DX12_BACKEND
#include <d3dcommon.h>
#include <wrl/client.h>
#include "../interface/vertexBufferInterface.h"

#include "../../gameRendererDx12.h"

namespace KUMA::RENDER {
	template<class T>
	class VertexBufferDx12 : public VertexBufferInterface
	{
	public:
		int mSize = 0;
		VertexBufferDx12(const std::vector<T>& vertices) {

			GameRendererDx12::mApp->begin();
			auto sz = vertices.size() * sizeof(T);
			ThrowIfFailed(D3DCreateBlob(sz, &VertexBufferCPU));
			CopyMemory(VertexBufferCPU->GetBufferPointer(), vertices.data(), sz);

			VertexByteStride = sizeof(T);
			VertexBufferByteSize = vertices.size() * sizeof(T);
			mSize = vertices.size();

			
			VertexBufferGPU = d3dUtil::CreateDefaultBuffer(GameRendererDx12::mApp->mDriver->mDevice.Get(),
				GameRendererDx12::mApp->mDriver->mCommandList.Get(), vertices.data(), sz, VertexBufferUploader);
			GameRendererDx12::mApp->end();
		}

		D3D12_VERTEX_BUFFER_VIEW VertexBufferView()const {
			D3D12_VERTEX_BUFFER_VIEW vbv;
			vbv.BufferLocation = VertexBufferGPU->GetGPUVirtualAddress();
			vbv.StrideInBytes = VertexByteStride;
			vbv.SizeInBytes = VertexBufferByteSize;
			return vbv;
		}

		int VertexByteStride = 0;
		int VertexBufferByteSize = 0;

		Microsoft::WRL::ComPtr<ID3DBlob> VertexBufferCPU = nullptr;

		Microsoft::WRL::ComPtr<ID3D12Resource> VertexBufferGPU = nullptr;

		Microsoft::WRL::ComPtr<ID3D12Resource> VertexBufferUploader = nullptr;

		virtual ~VertexBufferDx12() = default;
		void bind(const ShaderInterface& shader) override {};
		int getVertexCount() override
		{
			return mSize;
		};
	};
}
#endif
