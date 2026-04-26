#pragma once
#ifdef DX12_BACKEND
#include <d3d12.h>
#include <wrl/client.h>
#include "renderModule/backends/interface/uniformBufferInterface.h"

namespace IKIGAI::RENDER {
	class UniformBufferDx12 : public UniformBufferInterface {
	private:
		Microsoft::WRL::ComPtr<ID3D12Resource> mBuffer;
		D3D12_RESOURCE_STATES mState;
	public:
		UniformBufferDx12(void* data, size_t sz);
		template <class T>
		UniformBufferDx12(const T& data) : UniformBufferDx12((void*)&data, sizeof(T)) {}
		~UniformBufferDx12() override;
		void setData(const void* data, size_t sz) override;

		const Microsoft::WRL::ComPtr<ID3D12Resource>& getBuffer() const;
	};
}
#endif
