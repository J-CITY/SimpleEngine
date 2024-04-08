#include "shaderInterface.h"
#include <utilsModule/stringUtils.h>

#include "driverInterface.h"

std::string IKIGAI::RENDER::ShaderInterface::constructRealPath(const std::string& path) {
	if (IKIGAI::UTILS::IsFindInString(path, "/gl/") || IKIGAI::UTILS::IsFindInString(path, "/gui/") || //TODO: remove it
		IKIGAI::UTILS::IsFindInString(path, "/opengl/") || IKIGAI::UTILS::IsFindInString(path, "/vulkan/") || IKIGAI::UTILS::IsFindInString(path, "/dx12/")) {
		return path;
	}

#ifdef OPENGL_BACKEND
	if (RENDER::DriverInterface::settings.backend == RENDER::RenderSettings::Backend::OPENGL) {
		return IKIGAI::UTILS::ReplaceSubstring(path, "shaders/", "shaders/opengl/");
	}
#endif
#ifdef VULKAN_BACKEND
	if (RENDER::DriverInterface::settings.backend == RENDER::RenderSettings::Backend::VULKAN) {
		return IKIGAI::UTILS::ReplaceSubstring(path, "shaders/", "shaders/vulkan/");
	}
#endif
#ifdef DX12_BACKEND
	if (RENDER::DriverInterface::settings.backend == RENDER::RenderSettings::Backend::DIRECTX12) {
		return IKIGAI::UTILS::ReplaceSubstring(path, "shaders/", "shaders/dx12/");
	}
#endif
}
