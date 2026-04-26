#include "d3d12generatemips.h"
#ifdef DX12_BACKEND
#include "DirectXHelpers.h"
#include "PlatformHelpers.h"

using namespace DirectX;
using Microsoft::WRL::ComPtr;

namespace
{
#ifdef _GAMING_XBOX_SCARLETT
#include "XboxGamingScarlettGenerateMips_main.inc"
#elif defined(_GAMING_XBOX)
#include "XboxGamingXboxOneGenerateMips_main.inc"
#elif defined(_XBOX_ONE) && defined(_TITLE)
#include "XboxOneGenerateMips_main.inc"
#else
#include "GenerateMips_main.inc"
#endif

	bool FormatIsUAVCompatible(_In_ ID3D12Device* device, bool typedUAVLoadAdditionalFormats, DXGI_FORMAT format) noexcept
	{
		switch (format)
		{
		case DXGI_FORMAT_R32_FLOAT:
		case DXGI_FORMAT_R32_UINT:
		case DXGI_FORMAT_R32_SINT:
			// Unconditionally supported.
			return true;

		case DXGI_FORMAT_R32G32B32A32_FLOAT:
		case DXGI_FORMAT_R32G32B32A32_UINT:
		case DXGI_FORMAT_R32G32B32A32_SINT:
		case DXGI_FORMAT_R16G16B16A16_FLOAT:
		case DXGI_FORMAT_R16G16B16A16_UINT:
		case DXGI_FORMAT_R16G16B16A16_SINT:
		case DXGI_FORMAT_R8G8B8A8_UNORM:
		case DXGI_FORMAT_R8G8B8A8_UINT:
		case DXGI_FORMAT_R8G8B8A8_SINT:
		case DXGI_FORMAT_R16_FLOAT:
		case DXGI_FORMAT_R16_UINT:
		case DXGI_FORMAT_R16_SINT:
		case DXGI_FORMAT_R8_UNORM:
		case DXGI_FORMAT_R8_UINT:
		case DXGI_FORMAT_R8_SINT:
			// All these are supported if this optional feature is set.
			return typedUAVLoadAdditionalFormats;

		case DXGI_FORMAT_R16G16B16A16_UNORM:
		case DXGI_FORMAT_R16G16B16A16_SNORM:
		case DXGI_FORMAT_R32G32_FLOAT:
		case DXGI_FORMAT_R32G32_UINT:
		case DXGI_FORMAT_R32G32_SINT:
		case DXGI_FORMAT_R10G10B10A2_UNORM:
		case DXGI_FORMAT_R10G10B10A2_UINT:
		case DXGI_FORMAT_R11G11B10_FLOAT:
		case DXGI_FORMAT_R8G8B8A8_SNORM:
		case DXGI_FORMAT_R16G16_FLOAT:
		case DXGI_FORMAT_R16G16_UNORM:
		case DXGI_FORMAT_R16G16_UINT:
		case DXGI_FORMAT_R16G16_SNORM:
		case DXGI_FORMAT_R16G16_SINT:
		case DXGI_FORMAT_R8G8_UNORM:
		case DXGI_FORMAT_R8G8_UINT:
		case DXGI_FORMAT_R8G8_SNORM:
		case DXGI_FORMAT_R8G8_SINT:
		case DXGI_FORMAT_R16_UNORM:
		case DXGI_FORMAT_R16_SNORM:
		case DXGI_FORMAT_R8_SNORM:
		case DXGI_FORMAT_A8_UNORM:
		case DXGI_FORMAT_B5G6R5_UNORM:
		case DXGI_FORMAT_B5G5R5A1_UNORM:
		case DXGI_FORMAT_B4G4R4A4_UNORM:
			// Conditionally supported by specific devices.
			if (typedUAVLoadAdditionalFormats)
			{
				D3D12_FEATURE_DATA_FORMAT_SUPPORT formatSupport = { format, D3D12_FORMAT_SUPPORT1_NONE, D3D12_FORMAT_SUPPORT2_NONE };
				if (SUCCEEDED(device->CheckFeatureSupport(D3D12_FEATURE_FORMAT_SUPPORT, &formatSupport, sizeof(formatSupport))))
				{
					const DWORD mask = D3D12_FORMAT_SUPPORT2_UAV_TYPED_LOAD | D3D12_FORMAT_SUPPORT2_UAV_TYPED_STORE;
					return ((formatSupport.Support2 & mask) == mask);
				}
			}
			return false;

		default:
			return false;
		}
	}

	bool FormatIsBGR(DXGI_FORMAT format) noexcept
	{
		switch (format)
		{
		case DXGI_FORMAT_B8G8R8A8_UNORM:
		case DXGI_FORMAT_B8G8R8X8_UNORM:
		case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB:
		case DXGI_FORMAT_B8G8R8X8_UNORM_SRGB:
			return true;
		default:
			return false;
		}
	}

	bool FormatIsSRGB(DXGI_FORMAT format) noexcept
	{
		switch (format)
		{
		case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
		case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB:
		case DXGI_FORMAT_B8G8R8X8_UNORM_SRGB:
			return true;
		default:
			return false;
		}
	}

	DXGI_FORMAT ConvertSRVtoResourceFormat(DXGI_FORMAT format) noexcept
	{
		switch (format)
		{
		case DXGI_FORMAT_R32G32B32A32_FLOAT:
		case DXGI_FORMAT_R32G32B32A32_UINT:
		case DXGI_FORMAT_R32G32B32A32_SINT:
			return DXGI_FORMAT_R32G32B32A32_TYPELESS;

		case DXGI_FORMAT_R16G16B16A16_FLOAT:
		case DXGI_FORMAT_R16G16B16A16_UNORM:
		case DXGI_FORMAT_R16G16B16A16_UINT:
		case DXGI_FORMAT_R16G16B16A16_SNORM:
		case DXGI_FORMAT_R16G16B16A16_SINT:
			return DXGI_FORMAT_R16G16B16A16_TYPELESS;

		case DXGI_FORMAT_R32G32_FLOAT:
		case DXGI_FORMAT_R32G32_UINT:
		case DXGI_FORMAT_R32G32_SINT:
			return DXGI_FORMAT_R32G32_TYPELESS;

		case DXGI_FORMAT_R10G10B10A2_UNORM:
		case DXGI_FORMAT_R10G10B10A2_UINT:
			return DXGI_FORMAT_R10G10B10A2_TYPELESS;

		case DXGI_FORMAT_R8G8B8A8_UNORM:
		case DXGI_FORMAT_R8G8B8A8_UINT:
		case DXGI_FORMAT_R8G8B8A8_SNORM:
		case DXGI_FORMAT_R8G8B8A8_SINT:
			return DXGI_FORMAT_R8G8B8A8_TYPELESS;

		case DXGI_FORMAT_R16G16_FLOAT:
		case DXGI_FORMAT_R16G16_UNORM:
		case DXGI_FORMAT_R16G16_UINT:
		case DXGI_FORMAT_R16G16_SNORM:
		case DXGI_FORMAT_R16G16_SINT:
			return DXGI_FORMAT_R16G16_TYPELESS;

		case DXGI_FORMAT_R32_FLOAT:
		case DXGI_FORMAT_R32_UINT:
		case DXGI_FORMAT_R32_SINT:
			return DXGI_FORMAT_R32_TYPELESS;

		case DXGI_FORMAT_R8G8_UNORM:
		case DXGI_FORMAT_R8G8_UINT:
		case DXGI_FORMAT_R8G8_SNORM:
		case DXGI_FORMAT_R8G8_SINT:
			return DXGI_FORMAT_R8G8_TYPELESS;

		case DXGI_FORMAT_R16_FLOAT:
		case DXGI_FORMAT_R16_UNORM:
		case DXGI_FORMAT_R16_UINT:
		case DXGI_FORMAT_R16_SNORM:
		case DXGI_FORMAT_R16_SINT:
			return DXGI_FORMAT_R16_TYPELESS;

		case DXGI_FORMAT_R8_UNORM:
		case DXGI_FORMAT_R8_UINT:
		case DXGI_FORMAT_R8_SNORM:
		case DXGI_FORMAT_R8_SINT:
			return DXGI_FORMAT_R8_TYPELESS;

		default:
			return format;
		}
	}

	class GenerateMipsResources
	{
	public:
		enum RootParameterIndex
		{
			Constants,
			SourceTexture,
			TargetTexture,
			RootParameterCount
		};

#pragma pack(push, 4)
		struct ConstantData
		{
			XMFLOAT2 InvOutTexelSize;
			uint32_t SrcMipIndex;
		};
#pragma pack(pop)

		static constexpr uint32_t Num32BitConstants = static_cast<uint32_t>(sizeof(ConstantData) / sizeof(uint32_t));
		static constexpr uint32_t ThreadGroupSize = 8;

		ComPtr<ID3D12RootSignature> rootSignature;
		ComPtr<ID3D12PipelineState> generateMipsPSO;

		GenerateMipsResources(
			_In_ ID3D12Device* device)
		{
			rootSignature = CreateGenMipsRootSignature(device);
			generateMipsPSO = CreateGenMipsPipelineState(device, rootSignature.Get(), GenerateMips_main, sizeof(GenerateMips_main));
		}

	private:
		static ComPtr<ID3D12RootSignature> CreateGenMipsRootSignature(
			_In_ ID3D12Device* device)
		{
			constexpr D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags =
				D3D12_ROOT_SIGNATURE_FLAG_DENY_VERTEX_SHADER_ROOT_ACCESS
				| D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS
				| D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS
				| D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS
				| D3D12_ROOT_SIGNATURE_FLAG_DENY_PIXEL_SHADER_ROOT_ACCESS;

			const CD3DX12_STATIC_SAMPLER_DESC sampler(
				0, // register
				D3D12_FILTER_MIN_MAG_LINEAR_MIP_POINT,
				D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
				D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
				D3D12_TEXTURE_ADDRESS_MODE_CLAMP);

			const CD3DX12_DESCRIPTOR_RANGE sourceDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);
			const CD3DX12_DESCRIPTOR_RANGE targetDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0);

			CD3DX12_ROOT_PARAMETER rootParameters[RootParameterIndex::RootParameterCount] = {};
			rootParameters[RootParameterIndex::Constants].InitAsConstants(Num32BitConstants, 0);
			rootParameters[RootParameterIndex::SourceTexture].InitAsDescriptorTable(1, &sourceDescriptorRange);
			rootParameters[RootParameterIndex::TargetTexture].InitAsDescriptorTable(1, &targetDescriptorRange);

			CD3DX12_ROOT_SIGNATURE_DESC rsigDesc;
			rsigDesc.Init(static_cast<UINT>(std::size(rootParameters)), rootParameters, 1, &sampler, rootSignatureFlags);

			ComPtr<ID3D12RootSignature> rootSignature;
			ThrowIfFailed(CreateRootSignature(device, &rsigDesc, rootSignature.ReleaseAndGetAddressOf()));

			SetDebugObjectName(rootSignature.Get(), L"GenerateMips RootSignature");

			return rootSignature;
		}

		static ComPtr<ID3D12PipelineState> CreateGenMipsPipelineState(
			_In_ ID3D12Device* device,
			_In_ ID3D12RootSignature* rootSignature,
			_In_reads_(bytecodeSize) const uint8_t* bytecode,
			_In_ size_t bytecodeSize)
		{
			D3D12_COMPUTE_PIPELINE_STATE_DESC desc = {};
			desc.CS.BytecodeLength = bytecodeSize;
			desc.CS.pShaderBytecode = bytecode;
			desc.pRootSignature = rootSignature;

			ComPtr<ID3D12PipelineState> pso;
			ThrowIfFailed(device->CreateComputePipelineState(&desc, IID_PPV_ARGS(pso.GetAddressOf())));

			SetDebugObjectName(pso.Get(), L"GenerateMips PSO");

			return pso;
		}
	};
} // anonymous namespace

void GenerateMips_UnorderedAccessPath(ID3D12Device* device, ID3D12Resource* resource, ID3D12GraphicsCommandList* cmdlist, GenerateMipsResources& genMipsResources, std::vector<ComPtr<ID3D12DeviceChild>>& trackedObjects)
{
#if defined(_MSC_VER) || !defined(_WIN32)
	const auto desc = resource->GetDesc();
#else
	D3D12_RESOURCE_DESC tmpDesc;
	const auto& desc = *resource->GetDesc(&tmpDesc);
#endif
	assert(!FormatIsBGR(desc.Format) && !FormatIsSRGB(desc.Format));

	const CD3DX12_HEAP_PROPERTIES defaultHeapProperties(D3D12_HEAP_TYPE_DEFAULT);
	const D3D12_RESOURCE_STATES originalState = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;

	// Create a staging resource if we have to
	ComPtr<ID3D12Resource> staging;
	if ((desc.Flags & D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS) == 0)
	{
		D3D12_RESOURCE_DESC stagingDesc = desc;
		stagingDesc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
		stagingDesc.Format = ConvertSRVtoResourceFormat(desc.Format);

		ThrowIfFailed(device->CreateCommittedResource(
			&defaultHeapProperties,
			D3D12_HEAP_FLAG_NONE,
			&stagingDesc,
			D3D12_RESOURCE_STATE_COPY_DEST,
			nullptr,
			IID_PPV_ARGS(staging.GetAddressOf())));

		SetDebugObjectName(staging.Get(), L"GenerateMips Staging");

		// Copy the top mip of resource to staging
		TransitionResource(cmdlist, resource, originalState, D3D12_RESOURCE_STATE_COPY_SOURCE);

		const CD3DX12_TEXTURE_COPY_LOCATION src(resource, 0);
		const CD3DX12_TEXTURE_COPY_LOCATION dst(staging.Get(), 0);
		cmdlist->CopyTextureRegion(&dst, 0, 0, 0, &src, nullptr);

		TransitionResource(cmdlist, staging.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
	}
	else
	{
		// Resource is already a UAV so we can do this in-place
		staging = resource;

		TransitionResource(cmdlist, staging.Get(), originalState, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
	}

	// Create a descriptor heap that holds our resource descriptors
	ComPtr<ID3D12DescriptorHeap> descriptorHeap;
	D3D12_DESCRIPTOR_HEAP_DESC descriptorHeapDesc = {};
	descriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	descriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	descriptorHeapDesc.NumDescriptors = desc.MipLevels;
	device->CreateDescriptorHeap(&descriptorHeapDesc, IID_PPV_ARGS(descriptorHeap.GetAddressOf()));

	SetDebugObjectName(descriptorHeap.Get(), L"ResourceUploadBatch");

	auto const descriptorSize = static_cast<int>(device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV));

	// Create the top-level SRV
#if defined(_MSC_VER) || !defined(_WIN32)
	CD3DX12_CPU_DESCRIPTOR_HANDLE handleIt(descriptorHeap->GetCPUDescriptorHandleForHeapStart());
#else
	CD3DX12_CPU_DESCRIPTOR_HANDLE handleIt;
	std::ignore = descriptorHeap->GetCPUDescriptorHandleForHeapStart(&handleIt);
#endif
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format = desc.Format;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = desc.MipLevels;

	device->CreateShaderResourceView(staging.Get(), &srvDesc, handleIt);

	// Create the UAVs for the tail
	for (uint16_t mip = 1; mip < desc.MipLevels; ++mip)
	{
		D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
		uavDesc.Format = desc.Format;
		uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
		uavDesc.Texture2D.MipSlice = mip;

		handleIt.Offset(descriptorSize);
		device->CreateUnorderedAccessView(staging.Get(), nullptr, &uavDesc, handleIt);
	}

	// Set up UAV barrier (used in loop)
	D3D12_RESOURCE_BARRIER barrierUAV = {};
	barrierUAV.Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
	barrierUAV.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	barrierUAV.UAV.pResource = staging.Get();

	// Barrier for transitioning the subresources to UAVs
	D3D12_RESOURCE_BARRIER srv2uavDesc = {};
	srv2uavDesc.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	srv2uavDesc.Transition.pResource = staging.Get();
	srv2uavDesc.Transition.Subresource = 0;
	srv2uavDesc.Transition.StateBefore = D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
	srv2uavDesc.Transition.StateAfter = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;

	// Barrier for transitioning the subresources to SRVs
	D3D12_RESOURCE_BARRIER uav2srvDesc = {};
	uav2srvDesc.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	uav2srvDesc.Transition.pResource = staging.Get();
	uav2srvDesc.Transition.Subresource = 0;
	uav2srvDesc.Transition.StateBefore = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
	uav2srvDesc.Transition.StateAfter = D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;

	// based on format, select srgb or not
	ComPtr<ID3D12PipelineState> pso = genMipsResources.generateMipsPSO;

	// Set up state
	cmdlist->SetComputeRootSignature(genMipsResources.rootSignature.Get());
	cmdlist->SetPipelineState(pso.Get());
	cmdlist->SetDescriptorHeaps(1, descriptorHeap.GetAddressOf());

#if defined(_MSC_VER) || !defined(_WIN32)
	D3D12_GPU_DESCRIPTOR_HANDLE handle(descriptorHeap->GetGPUDescriptorHandleForHeapStart());
#else
	D3D12_GPU_DESCRIPTOR_HANDLE handle;
	std::ignore = descriptorHeap->GetGPUDescriptorHandleForHeapStart(&handle);
#endif
	cmdlist->SetComputeRootDescriptorTable(GenerateMipsResources::SourceTexture, handle);

	// Get the descriptor handle -- uavH will increment over each loop
	CD3DX12_GPU_DESCRIPTOR_HANDLE uavH(handle, descriptorSize); // offset by 1 descriptor

	// Process each mip
	auto mipWidth = static_cast<uint32_t>(desc.Width);
	uint32_t mipHeight = desc.Height;
	for (uint32_t mip = 1; mip < desc.MipLevels; ++mip)
	{
		mipWidth = std::max<uint32_t>(1, mipWidth >> 1);
		mipHeight = std::max<uint32_t>(1, mipHeight >> 1);

		// Transition the mip to a UAV
		srv2uavDesc.Transition.Subresource = mip;
		cmdlist->ResourceBarrier(1, &srv2uavDesc);

		// Bind the mip subresources
		cmdlist->SetComputeRootDescriptorTable(GenerateMipsResources::TargetTexture, uavH);

		// Set constants
		GenerateMipsResources::ConstantData constants;
		constants.SrcMipIndex = mip - 1;
		constants.InvOutTexelSize = XMFLOAT2(1 / float(mipWidth), 1 / float(mipHeight));
		cmdlist->SetComputeRoot32BitConstants(
			GenerateMipsResources::Constants,
			GenerateMipsResources::Num32BitConstants,
			&constants,
			0);

		// Process this mip
		cmdlist->Dispatch(
			(mipWidth + GenerateMipsResources::ThreadGroupSize - 1) / GenerateMipsResources::ThreadGroupSize,
			(mipHeight + GenerateMipsResources::ThreadGroupSize - 1) / GenerateMipsResources::ThreadGroupSize,
			1);

		cmdlist->ResourceBarrier(1, &barrierUAV);

		// Transition the mip to an SRV
		uav2srvDesc.Transition.Subresource = mip;
		cmdlist->ResourceBarrier(1, &uav2srvDesc);

		// Offset the descriptor heap handles
		uavH.Offset(descriptorSize);
	}

	// If the staging resource is NOT the same as the resource, we need to copy everything back
	if (staging.Get() != resource)
	{
		// Transition the resources ready for copy
		D3D12_RESOURCE_BARRIER barrier[2] = {};
		barrier[0].Type = barrier[1].Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		barrier[0].Transition.Subresource = barrier[1].Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
		barrier[0].Transition.pResource = staging.Get();
		barrier[0].Transition.StateBefore = D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
		barrier[0].Transition.StateAfter = D3D12_RESOURCE_STATE_COPY_SOURCE;

		barrier[1].Transition.pResource = resource;
		barrier[1].Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_SOURCE;
		barrier[1].Transition.StateAfter = D3D12_RESOURCE_STATE_COPY_DEST;

		cmdlist->ResourceBarrier(2, barrier);

		// Copy the entire resource back
		cmdlist->CopyResource(resource, staging.Get());

		// Transition the target resource back to pixel shader resource
		TransitionResource(cmdlist, resource, D3D12_RESOURCE_STATE_COPY_DEST, originalState);

		trackedObjects.push_back(staging);
	}
	else
	{
		TransitionResource(cmdlist, staging.Get(), D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, originalState);
	}

	// Add our temporary objects to the deferred deletion queue
	trackedObjects.push_back(genMipsResources.rootSignature);
	trackedObjects.push_back(pso);
	trackedObjects.push_back(resource);
	trackedObjects.push_back(descriptorHeap);
};

void GenerateMips_TexturePathBGR(ID3D12Device* device, ID3D12Resource* resource, ID3D12GraphicsCommandList* cmdlist, GenerateMipsResources& genMipsResources, std::vector<ComPtr<ID3D12DeviceChild>>& trackedObjects)
{
#if defined(_MSC_VER) || !defined(_WIN32)
	const auto resourceDesc = resource->GetDesc();
#else
	D3D12_RESOURCE_DESC tmpDesc;
	const auto& resourceDesc = *resource->GetDesc(&tmpDesc);
#endif
	assert(FormatIsBGR(resourceDesc.Format));

	// Create a resource with the same description with RGB and with UAV flags
	auto copyDesc = resourceDesc;
	copyDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	copyDesc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
#if !defined(_GAMING_XBOX) && !(defined(_XBOX_ONE) && defined(_TITLE))
	copyDesc.Layout = D3D12_TEXTURE_LAYOUT_64KB_STANDARD_SWIZZLE;
#endif

	D3D12_HEAP_DESC heapDesc = {};
#if defined(_MSC_VER) || !defined(_WIN32)
	auto const allocInfo = device->GetResourceAllocationInfo(0, 1, &copyDesc);
#else
	D3D12_RESOURCE_ALLOCATION_INFO allocInfo;
	std::ignore = mDevice->GetResourceAllocationInfo(&allocInfo, 0, 1, &copyDesc);
#endif
	heapDesc.SizeInBytes = allocInfo.SizeInBytes;
	heapDesc.Flags = D3D12_HEAP_FLAG_ALLOW_ONLY_NON_RT_DS_TEXTURES;
	heapDesc.Properties.Type = D3D12_HEAP_TYPE_DEFAULT;

	ComPtr<ID3D12Heap> heap;
	ThrowIfFailed(device->CreateHeap(&heapDesc, IID_PPV_ARGS(heap.GetAddressOf())));

	SetDebugObjectName(heap.Get(), L"ResourceUploadBatch");

	ComPtr<ID3D12Resource> resourceCopy;
	ThrowIfFailed(device->CreatePlacedResource(
		heap.Get(),
		0,
		&copyDesc,
		D3D12_RESOURCE_STATE_COPY_DEST,
		nullptr,
		IID_PPV_ARGS(resourceCopy.GetAddressOf())));

	SetDebugObjectName(resourceCopy.Get(), L"GenerateMips Resource Copy");

	// Create a BGRA alias
	auto aliasDesc = resourceDesc;
	aliasDesc.Format = (resourceDesc.Format == DXGI_FORMAT_B8G8R8X8_UNORM || resourceDesc.Format == DXGI_FORMAT_B8G8R8X8_UNORM_SRGB) ? DXGI_FORMAT_B8G8R8X8_UNORM : DXGI_FORMAT_B8G8R8A8_UNORM;
	aliasDesc.Layout = copyDesc.Layout;
	aliasDesc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;

	ComPtr<ID3D12Resource> aliasCopy;
	ThrowIfFailed(device->CreatePlacedResource(
		heap.Get(),
		0,
		&aliasDesc,
		D3D12_RESOURCE_STATE_COPY_DEST,
		nullptr,
		IID_PPV_ARGS(aliasCopy.GetAddressOf())));

	SetDebugObjectName(aliasCopy.Get(), L"GenerateMips BGR Alias Copy");

	const D3D12_RESOURCE_STATES originalState = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;

	// Copy the top mip of the resource data BGR to RGB
	D3D12_RESOURCE_BARRIER aliasBarrier[3] = {};
	aliasBarrier[0].Type = D3D12_RESOURCE_BARRIER_TYPE_ALIASING;
	aliasBarrier[0].Aliasing.pResourceAfter = aliasCopy.Get();

	aliasBarrier[1].Type = aliasBarrier[2].Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	aliasBarrier[1].Transition.Subresource = aliasBarrier[2].Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	aliasBarrier[1].Transition.pResource = resource;
	aliasBarrier[1].Transition.StateBefore = originalState;
	aliasBarrier[1].Transition.StateAfter = D3D12_RESOURCE_STATE_COPY_SOURCE;

	cmdlist->ResourceBarrier(2, aliasBarrier);

	const CD3DX12_TEXTURE_COPY_LOCATION src(resource, 0);
	const CD3DX12_TEXTURE_COPY_LOCATION dst(aliasCopy.Get(), 0);
	cmdlist->CopyTextureRegion(&dst, 0, 0, 0, &src, nullptr);

	// Generate the mips
	aliasBarrier[0].Aliasing.pResourceBefore = aliasCopy.Get();
	aliasBarrier[0].Aliasing.pResourceAfter = resourceCopy.Get();

	aliasBarrier[1].Transition.pResource = resourceCopy.Get();
	aliasBarrier[1].Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
	aliasBarrier[1].Transition.StateAfter = originalState;

	cmdlist->ResourceBarrier(2, aliasBarrier);
	GenerateMips_UnorderedAccessPath(device, resourceCopy.Get(), cmdlist, genMipsResources, trackedObjects);

	// Direct copy back RGB to BGR
	aliasBarrier[0].Aliasing.pResourceBefore = resourceCopy.Get();
	aliasBarrier[0].Aliasing.pResourceAfter = aliasCopy.Get();

	aliasBarrier[1].Transition.pResource = aliasCopy.Get();
	aliasBarrier[1].Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
	aliasBarrier[1].Transition.StateAfter = D3D12_RESOURCE_STATE_COPY_SOURCE;

	aliasBarrier[2].Transition.pResource = resource;
	aliasBarrier[2].Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_SOURCE;
	aliasBarrier[2].Transition.StateAfter = D3D12_RESOURCE_STATE_COPY_DEST;

	cmdlist->ResourceBarrier(3, aliasBarrier);

	// Copy the entire resource back
	cmdlist->CopyResource(resource, aliasCopy.Get());
	TransitionResource(cmdlist, resource, D3D12_RESOURCE_STATE_COPY_DEST, originalState);

	// Track these object lifetimes on the GPU
	trackedObjects.push_back(heap);
	trackedObjects.push_back(resourceCopy);
	trackedObjects.push_back(aliasCopy);
	trackedObjects.push_back(resource);
};

void GenerateMips_TexturePath(ID3D12Device* device, ID3D12Resource* resource, ID3D12GraphicsCommandList* cmdlist, GenerateMipsResources& genMipsResources, std::vector<ComPtr<ID3D12DeviceChild>>& trackedObjects)
{
#if defined(_MSC_VER) || !defined(_WIN32)
	const auto resourceDesc = resource->GetDesc();
#else
	D3D12_RESOURCE_DESC tmpDesc;
	const auto& resourceDesc = *resource->GetDesc(&tmpDesc);
#endif
	assert(!FormatIsBGR(resourceDesc.Format) || FormatIsSRGB(resourceDesc.Format));

	auto copyDesc = resourceDesc;
	copyDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	copyDesc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;

	const CD3DX12_HEAP_PROPERTIES heapProperties(D3D12_HEAP_TYPE_DEFAULT);

	// Create a resource with the same description, but without SRGB, and with UAV flags
	ComPtr<ID3D12Resource> resourceCopy;
	ThrowIfFailed(device->CreateCommittedResource(
		&heapProperties,
		D3D12_HEAP_FLAG_NONE,
		&copyDesc,
		D3D12_RESOURCE_STATE_COPY_DEST,
		nullptr,
		IID_PPV_ARGS(resourceCopy.GetAddressOf())));

	SetDebugObjectName(resourceCopy.Get(), L"GenerateMips Resource Copy");

	const D3D12_RESOURCE_STATES originalState = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;

	// Copy the top mip of resource data
	TransitionResource(cmdlist, resource, originalState, D3D12_RESOURCE_STATE_COPY_SOURCE);

	const CD3DX12_TEXTURE_COPY_LOCATION src(resource, 0);
	const CD3DX12_TEXTURE_COPY_LOCATION dst(resourceCopy.Get(), 0);
	cmdlist->CopyTextureRegion(&dst, 0, 0, 0, &src, nullptr);

	TransitionResource(cmdlist, resourceCopy.Get(), D3D12_RESOURCE_STATE_COPY_DEST, originalState);

	// Generate the mips
	GenerateMips_UnorderedAccessPath(device, resourceCopy.Get(), cmdlist, genMipsResources, trackedObjects);

	// Direct copy back
	D3D12_RESOURCE_BARRIER barrier[2] = {};
	barrier[0].Type = barrier[1].Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier[0].Transition.Subresource = barrier[1].Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	barrier[0].Transition.pResource = resourceCopy.Get();
	barrier[0].Transition.StateBefore = originalState;
	barrier[0].Transition.StateAfter = D3D12_RESOURCE_STATE_COPY_SOURCE;

	barrier[1].Transition.pResource = resource;
	barrier[1].Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_SOURCE;
	barrier[1].Transition.StateAfter = D3D12_RESOURCE_STATE_COPY_DEST;

	cmdlist->ResourceBarrier(2, barrier);

	// Copy the entire resource back
	cmdlist->CopyResource(resource, resourceCopy.Get());

	TransitionResource(cmdlist, resource, D3D12_RESOURCE_STATE_COPY_DEST, originalState);

	// Track these object lifetimes on the GPU
	trackedObjects.push_back(resourceCopy);
	trackedObjects.push_back(resource);
};

void GenerateMips(ID3D12Device* device, ID3D12Resource* resource, ID3D12GraphicsCommandList* cmdlist, std::vector<ComPtr<ID3D12DeviceChild>>& trackedObjects, bool typedUAVLoadAdditionalFormats, bool standardSwizzle64KBSupported)
{
	if (resource == nullptr)
	{
		throw std::invalid_argument("Nullptr passed to GenerateMips");
	}

#if defined(_MSC_VER) || !defined(_WIN32)
	const auto desc = resource->GetDesc();
#else
	D3D12_RESOURCE_DESC tmpDesc;
	const auto& desc = *resource->GetDesc(&tmpDesc);
#endif

	if (desc.MipLevels == 1)
	{
		// Nothing to do
		return;
	}
	if (desc.MipLevels == 0)
	{
		throw std::runtime_error("GenerateMips: texture has no mips");
	}
	if (desc.Dimension != D3D12_RESOURCE_DIMENSION_TEXTURE2D)
	{
		throw std::runtime_error("GenerateMips only supports Texture2D resources");
	}
	if (desc.DepthOrArraySize != 1)
	{
		throw std::runtime_error("GenerateMips only supports 2D textures of array size 1");
	}

	const bool uavCompat = FormatIsUAVCompatible(device, typedUAVLoadAdditionalFormats, desc.Format);

	if (!uavCompat && !FormatIsSRGB(desc.Format) && !FormatIsBGR(desc.Format))
	{
		throw std::runtime_error("GenerateMips doesn't support this texture format on this device");
	}

	auto genMipsResources = GenerateMipsResources(device);

	// If the texture's format doesn't support UAVs we'll have to copy it to a texture that does first.
	// This is true of BGRA or sRGB textures, for example.
	if (uavCompat)
	{
		GenerateMips_UnorderedAccessPath(device, resource, cmdlist, genMipsResources, trackedObjects);
	}
	else if (!typedUAVLoadAdditionalFormats)
	{
		throw std::runtime_error("GenerateMips needs TypedUAVLoadAdditionalFormats device support for sRGB/BGR");
	}
	else if (FormatIsBGR(desc.Format))
	{
#if !defined(_GAMING_XBOX) && !(defined(_XBOX_ONE) && defined(_TITLE))
		if (!standardSwizzle64KBSupported)
		{
			throw std::runtime_error("GenerateMips needs StandardSwizzle64KBSupported device support for BGR");
		}
#endif

		GenerateMips_TexturePathBGR(device, resource, cmdlist, genMipsResources, trackedObjects);
	}
	else
	{
		GenerateMips_TexturePath(device, resource, cmdlist, genMipsResources, trackedObjects);
	}
};

void D3D12GenerateMips(ID3D12Device* device, ID3D12GraphicsCommandList* cmdlist, ID3D12Resource* texture,
	std::vector<ComPtr<ID3D12DeviceChild>>& staging_objects)
{
	auto typedUAVLoadAdditionalFormats = false;
	auto standardSwizzle64KBSupported = false;
	
	D3D12_FEATURE_DATA_D3D12_OPTIONS options = {};
	if (SUCCEEDED(device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS, &options, sizeof(options))))
	{
		typedUAVLoadAdditionalFormats = options.TypedUAVLoadAdditionalFormats != 0;
		standardSwizzle64KBSupported = options.StandardSwizzle64KBSupported != 0;
	}

	GenerateMips(device, texture, cmdlist, staging_objects, typedUAVLoadAdditionalFormats, standardSwizzle64KBSupported);
}
#endif
