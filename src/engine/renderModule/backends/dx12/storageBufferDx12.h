#pragma once
#include "d3dx12/d3dx12.h"

#ifdef DX12_BACKEND
#include "renderModule/backends/interface/storageBufferInterface.h"
#include <d3d12.h>
#include <vector>
#include <wrl/client.h>

namespace IKIGAI::RENDER {
	class StorageBufferDx12 : public StorageBufferInterface {
		D3D12_CPU_DESCRIPTOR_HANDLE mSrvHeapBegin{};
		D3D12_CPU_DESCRIPTOR_HANDLE mUavHeapBegin{};

		CD3DX12_GPU_DESCRIPTOR_HANDLE mGpuSrvDescriptorHandle;
		CD3DX12_GPU_DESCRIPTOR_HANDLE mGpuUavDescriptorHandle;
		//Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>	mSrvHeap;
		//Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>	mUavHeap;
		Microsoft::WRL::ComPtr<ID3D12Resource> mBuffer;
		D3D12_RESOURCE_STATES mState;
		void init();
	public:
		StorageBufferDx12(void* data, size_t sz, size_t stride);
		template <class T>
		StorageBufferDx12(const std::vector<T>& vertices) : StorageBufferDx12((void*)vertices.data(), vertices.size(), sizeof(T)) {}
		~StorageBufferDx12() override;
		void bind() override {};
		void unbind() override {};
		void setData(const void* data, size_t sz, size_t stride) override;

		CD3DX12_GPU_DESCRIPTOR_HANDLE getSRVHandler();
		CD3DX12_GPU_DESCRIPTOR_HANDLE getUAVHandler();
		const Microsoft::WRL::ComPtr<ID3D12Resource>& getBuffer() const;
	};

}
#endif
