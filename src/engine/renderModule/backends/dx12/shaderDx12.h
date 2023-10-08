#pragma once
#ifdef DX12_BACKEND
#include <combaseapi.h>
#include <d3d12.h>
#include <memory>
#include <wrl/client.h>

#include "driverDx12.h"
#include "hlslReflection.h"
#include "../interface/reflectionStructs.h"
#include "d3dUtil.h"

struct CD3DX12_ROOT_PARAMETER;
namespace IKIGAI::RENDER {
	class ShaderDx12 : public ShaderInterface {
	public:
		std::optional<SimpleHLSLReflection> mVertexReflection = std::nullopt;
		std::optional<SimpleHLSLReflection> mFragmentReflection = std::nullopt;

		Microsoft::WRL::ComPtr<ID3D12PipelineState> mPiplineState = nullptr;
		Microsoft::WRL::ComPtr<ID3D12RootSignature> mRootSignature = nullptr;
		std::vector<D3D12_INPUT_ELEMENT_DESC> mInputLayout;

		Microsoft::WRL::ComPtr<ID3D10Blob> mVertexShader = nullptr;
		Microsoft::WRL::ComPtr<ID3D10Blob> mFragmentShader = nullptr;

		Microsoft::WRL::ComPtr<ID3D12PipelineState> mPSO;

		bool m4xMsaaState = false;
		UINT m4xMsaaQuality = 0;

		DXGI_FORMAT mDepthStencilFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;

		static std::shared_ptr<ShaderDx12> Create(std::string vertexPath, std::string fragmentPath);
		void buildPSO();
		std::unordered_map<std::string, size_t> uniformNameToSlotId;

		void buildRootSignature();
		void bind() override {};
		void unbind() override {};
		int getId() override { return 0; };
		void setUniform(const UniformBufferInterface& uniform) override{};
		void setPushConstant(const PushConstantInterface& uniform) override{};

		std::unordered_map<std::string, IKIGAI::RENDER::UniformInform> uniformInfo;
		const std::unordered_map<std::string, IKIGAI::RENDER::UniformInform>& getUniformsInfo() const override
		{
			return  uniformInfo;
		};
	};
}
#endif
