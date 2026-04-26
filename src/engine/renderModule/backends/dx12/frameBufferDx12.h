#pragma once
#ifdef DX12_BACKEND
#include <d3d12.h>
#include <wrl/client.h>
#include <memory>
#include <vector>
#include "renderModule/backends/interface/frameBufferInterface.h"


struct CD3DX12_CPU_DESCRIPTOR_HANDLE;

namespace IKIGAI::RENDER {
	class TextureDx12;
	class TextureInterface;

	class FrameBufferDx12 : public FrameBufferInterface {
	private:
		std::vector<std::shared_ptr<TextureInterface>> mTextures;
		std::shared_ptr<TextureInterface> mDepth;

		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> mTexHeap;
		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> mDepthHeap;

		void create(CD3DX12_CPU_DESCRIPTOR_HANDLE handler);

	public:
		FrameBufferDx12(const std::vector<std::shared_ptr<TextureInterface>>& textures, std::shared_ptr<TextureInterface> depth=nullptr);
		const std::vector<std::shared_ptr<TextureInterface>>& getTextures() const override;
		const std::shared_ptr<TextureInterface>& getDepth() const override;

		const Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>& getTexHeap() const;
		const Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>& getDepthHeap() const;
	};
}
#endif

