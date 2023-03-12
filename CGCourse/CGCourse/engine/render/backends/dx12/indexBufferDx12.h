#pragma once""

#ifdef DX12_BACKEND

#include <d3dcommon.h>
#include <wrl/client.h>
#include "../interface/indexBufferInterface.h"
#include <d3d12.h>
#include <d3dcompiler.h>
#include "d3dUtil.h"
#include "../../gameRendererDx12.h"
namespace KUMA::RENDER {
	class IndexBufferDx12 : public IndexBufferInterface {
	public:
		int mSize = 0;
		IndexBufferDx12(const std::vector<unsigned>& indices) {
			GameRendererDx12::mApp->begin();

			auto sz = indices.size() * sizeof(int);
			ThrowIfFailed(D3DCreateBlob(sz, &IndexBufferCPU));
			CopyMemory(IndexBufferCPU->GetBufferPointer(), indices.data(), sz);

			IndexBufferByteSize = indices.size() * sizeof(int);
			mSize = indices.size();
			IndexBufferGPU = d3dUtil::CreateDefaultBuffer(indices.data(), sz, IndexBufferUploader);
			GameRendererDx12::mApp->end();
		}

		D3D12_INDEX_BUFFER_VIEW IndexBufferView() const {
			D3D12_INDEX_BUFFER_VIEW ibv;
			ibv.BufferLocation = IndexBufferGPU->GetGPUVirtualAddress();
			ibv.Format = DXGI_FORMAT_R32_UINT;
			ibv.SizeInBytes = IndexBufferByteSize;
			return ibv;
		}

		int IndexBufferByteSize = 0;

		Microsoft::WRL::ComPtr<ID3DBlob> IndexBufferCPU = nullptr;

		Microsoft::WRL::ComPtr<ID3D12Resource> IndexBufferGPU = nullptr;

		Microsoft::WRL::ComPtr<ID3D12Resource> IndexBufferUploader = nullptr;
		virtual ~IndexBufferDx12() = default;
		void bind(const ShaderInterface& shader) override {};
		int getIndexCount() override {
			return mSize;
		};
	};
}
#endif
