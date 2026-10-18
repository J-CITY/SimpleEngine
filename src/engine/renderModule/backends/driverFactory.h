#pragma once
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include "interface/driverInterface.h"

namespace IKIGAI::WINDOW {
	class Window;
}

namespace IKIGAI::RENDER {
void SetPresentationSmokeFocus(bool);
void FocusPresentationForSmoke(void* nativeWindow);
	const char* BackendToString(RenderSettings::Backend backend);
	std::optional<RenderSettings::Backend> ParseBackendName(std::string_view name);
	bool IsBackendCompiled(RenderSettings::Backend backend);
	void ValidateBackendAvailable(RenderSettings::Backend backend);
	void SetCliRenderBackendOverride(std::string_view name);
	void SetCliRenderPipelineOverride(std::string_view path);
    void SetCliUpscalerOverride(std::string_view name);
    void SetCliUpscalerQualityOverride(std::string_view name);
    void SetCliFrameGenerationOverride(std::string_view name);
    void SetCliFrameGenerationMultiplier(unsigned multiplier);
	bool ApplyCliRenderBackendOverride(RenderSettings& settings);
	// A shared asset config may target another platform. Only fall back when
	// the binary contains exactly one backend and no explicit CLI selection.
	void ResolveRenderBackend(RenderSettings& settings);

	std::unique_ptr<DriverInterface> CreateRenderDriver(RenderSettings::Backend backend, WINDOW::Window& window);
}
