#include "shaderDx12.h"

#ifdef DX12_BACKEND
#include "../../gameRendererDx12.h"
#include <utilsModule/loader.h>
using namespace IKIGAI::RENDER;


std::shared_ptr<ShaderDx12> ShaderDx12::Create(std::string vertexPath, std::string fragmentPath) {
	auto shader = std::make_shared<ShaderDx12>();

	vertexPath = IKIGAI::UTILS::getRealPath(vertexPath);
	fragmentPath = IKIGAI::UTILS::getRealPath(fragmentPath);
	shader->mVertexShader = d3dUtil::CompileShader(std::wstring(vertexPath.begin(), vertexPath.end()), nullptr, "VS", "vs_5_1");;
	shader->mFragmentShader = d3dUtil::CompileShader(std::wstring(fragmentPath.begin(), fragmentPath.end()), nullptr, "PS", "ps_5_1");


	shader->mVertexReflection = SimpleHLSLReflection(vertexPath, "VS", SimpleHLSLReflection::ShaderType::VERTEX);
	shader->mFragmentReflection = SimpleHLSLReflection(vertexPath, "PS", SimpleHLSLReflection::ShaderType::FRAGMENT);

	std::unordered_map<SimpleHLSLReflection::VarType, DXGI_FORMAT> varTypeToDxtype = {
		{SimpleHLSLReflection::VarType::VEC4, DXGI_FORMAT_R32G32B32A32_FLOAT},
		{SimpleHLSLReflection::VarType::VEC3, DXGI_FORMAT_R32G32B32_FLOAT},
		{SimpleHLSLReflection::VarType::VEC2, DXGI_FORMAT_R32G32_FLOAT},
	};

	std::unordered_map<SimpleHLSLReflection::VarType, unsigned> varTypeToSize = {
		{SimpleHLSLReflection::VarType::VEC4, 16},
		{SimpleHLSLReflection::VarType::VEC3, 12},
		{SimpleHLSLReflection::VarType::VEC2, 8},
	};

	if (!shader->mVertexReflection->vertexDescriptor->name.empty()) {
		unsigned int offset = 0;
		for (auto& e : shader->mVertexReflection->vertexDescriptor->members) {
			shader->mInputLayout.push_back({ e.semantic.c_str(), 0, varTypeToDxtype[e.type], 0, offset, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 });
			offset += varTypeToSize[e.type];
		}
	}
	shader->buildRootSignature();
	shader->buildPSO();

	return shader;
}

void ShaderDx12::buildPSO() {
	D3D12_GRAPHICS_PIPELINE_STATE_DESC opaquePsoDesc;
	ZeroMemory(&opaquePsoDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));

	opaquePsoDesc.pRootSignature = mRootSignature.Get();
	opaquePsoDesc.VS = {
				reinterpret_cast<BYTE*>(mVertexShader->GetBufferPointer()),
				mVertexShader->GetBufferSize()
	};
	opaquePsoDesc.PS = {
		reinterpret_cast<BYTE*>(mFragmentShader->GetBufferPointer()),
		mFragmentShader->GetBufferSize()
	};
	if (mInputLayout.size() > 0) {
		opaquePsoDesc.InputLayout = { mInputLayout.data(),(UINT)mInputLayout.size() };
	}
	if (GameRendererDx12::mApp->mDriver->mUseDepth) {
		opaquePsoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	}
	else {
		opaquePsoDesc.DepthStencilState.DepthEnable = false;
		opaquePsoDesc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
		opaquePsoDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_ALWAYS;
	}
	opaquePsoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	opaquePsoDesc.SampleMask = UINT_MAX;
	opaquePsoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	opaquePsoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	opaquePsoDesc.NumRenderTargets = mFragmentReflection->colorDescriptor->members.empty() ? 1 : mFragmentReflection->colorDescriptor->members.size();
	for (int i = 0; i < opaquePsoDesc.NumRenderTargets; i++) {
		opaquePsoDesc.RTVFormats[i] = DXGI_FORMAT_R16G16B16A16_FLOAT;
	}
	opaquePsoDesc.SampleDesc.Count = m4xMsaaState ? 4 : 1;
	opaquePsoDesc.SampleDesc.Quality = m4xMsaaState ? (m4xMsaaQuality - 1) : 0;
	ThrowIfFailed(GameRendererDx12::mApp->mDriver->mDevice->CreateGraphicsPipelineState(&opaquePsoDesc, IID_PPV_ARGS(&mPSO)));
}


void ShaderDx12::buildRootSignature() {
	std::vector<CD3DX12_DESCRIPTOR_RANGE> texTables;
	int texSize = 0;
	for (auto& [name, uniform] : mVertexReflection->uniformsDescriptors) {
		if (uniform.type == SimpleHLSLReflection::UniformType::TEXTURE_2D ||
			uniform.type == SimpleHLSLReflection::UniformType::TEXTURE_CUBE) {
			texSize++;
		}
	}
	texTables.reserve(texSize);

	std::vector<CD3DX12_ROOT_PARAMETER> slotRootParameters;

	for (auto& [name, uniform] : mVertexReflection->uniformsDescriptors) {
		CD3DX12_ROOT_PARAMETER elem;
		if (uniform.type == SimpleHLSLReflection::UniformType::UNIFORM_BUFFER) {
			uniformNameToSlotId[name] = slotRootParameters.size();
			elem.InitAsConstantBufferView(uniform.bind);
			slotRootParameters.push_back(elem);
		}
		else if (uniform.type == SimpleHLSLReflection::UniformType::STRUCTURED_BUFFER) {
			uniformNameToSlotId[name] = slotRootParameters.size();
			elem.InitAsShaderResourceView(uniform.bind, uniform.space);
			slotRootParameters.push_back(elem);
		}
		else if (uniform.type == SimpleHLSLReflection::UniformType::TEXTURE_2D ||
			uniform.type == SimpleHLSLReflection::UniformType::TEXTURE_CUBE) {
			uniformNameToSlotId[name] = slotRootParameters.size();
			CD3DX12_DESCRIPTOR_RANGE texTable;
			texTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, uniform.bind, 0);
			texTables.push_back(texTable);
			elem.InitAsDescriptorTable(1, &texTables.back(), D3D12_SHADER_VISIBILITY_PIXEL);
			slotRootParameters.push_back(elem);
		}
	}

	auto staticSamplers = DriverDx12::GetStaticSamplers();

	CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(slotRootParameters.size(), slotRootParameters.data(),
		(UINT)staticSamplers.size(), staticSamplers.data(),
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS);

	ComPtr<ID3DBlob> serializedRootSig = nullptr;
	ComPtr<ID3DBlob> errorBlob = nullptr;
	HRESULT hr = D3D12SerializeRootSignature(&rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1,
		serializedRootSig.GetAddressOf(), errorBlob.GetAddressOf());

	if (errorBlob != nullptr) {
		::OutputDebugStringA((char*)errorBlob->GetBufferPointer());
	}
	ThrowIfFailed(hr);

	ThrowIfFailed(GameRendererDx12::mApp->mDriver->mDevice->CreateRootSignature(
		0,
		serializedRootSig->GetBufferPointer(),
		serializedRootSig->GetBufferSize(),
		IID_PPV_ARGS(mRootSignature.GetAddressOf())));

}

#endif
