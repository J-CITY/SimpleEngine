#pragma once
#include <cstdint>
#include <algorithm>
#include <map>
#include <stdexcept>
#include <vector>

namespace IKIGAI::RENDER {
	// Texture slots and sampler slots have different limits. Compact samplers
	// per stage rather than using Vulkan texture bindings (which can exceed 15).
	inline std::map<uint32_t, uint32_t> MetalSamplerBindings(std::vector<uint32_t> bindings) {
		std::sort(bindings.begin(), bindings.end());
		bindings.erase(std::unique(bindings.begin(), bindings.end()), bindings.end());
		if (bindings.size() > 16) throw std::runtime_error("Metal stage exceeds 16 sampler slots");
		std::map<uint32_t, uint32_t> result;
		for (uint32_t i = 0; i < bindings.size(); ++i) result[bindings[i]] = i;
		return result;
	}
	// Buffer 0 is the vertex-fetch buffer. SPIR-V descriptor bindings are shifted
	// so they do not alias it. Indices from 20 upward are reserved for
	// tessellation kernels (same slots SPIRV-Cross uses) and push constants.
	inline constexpr uint32_t kMetalVertexBufferIndex = 0;
	inline constexpr uint32_t kMetalResourceBufferBase = 1;
	inline constexpr uint32_t kMetalUserBufferLimit = 20;
	inline constexpr uint32_t kMetalShaderPatchInputBufferIndex = 20;
	inline constexpr uint32_t kMetalShaderIndexBufferIndex = 21;
	inline constexpr uint32_t kMetalShaderInputBufferIndex = 22;
	inline constexpr uint32_t kMetalTessFactorBufferIndex = 26;
	inline constexpr uint32_t kMetalPatchOutputBufferIndex = 27;
	inline constexpr uint32_t kMetalShaderOutputBufferIndex = 28;
	inline constexpr uint32_t kMetalBufferSizeBufferIndex = 25;
	inline constexpr uint32_t kMetalIndirectParamsBufferIndex = 29;
	inline constexpr uint32_t kMetalPushConstantBufferIndex = 30;

	inline uint32_t MetalBufferIndexForBinding(uint32_t binding) {
		const uint32_t index = binding + kMetalResourceBufferBase;
		if (index >= kMetalUserBufferLimit) {
			return kMetalUserBufferLimit - 1;
		}
		return index;
	}
}
