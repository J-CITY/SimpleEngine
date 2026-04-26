#pragma once

#include "pch.h"
#ifdef DX12_BACKEND
void D3D12GenerateMips(ID3D12Device* device, ID3D12GraphicsCommandList* cmdlist, ID3D12Resource* texture,
	std::vector<Microsoft::WRL::ComPtr<ID3D12DeviceChild>>& staging_objects);
#endif