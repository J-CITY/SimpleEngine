#pragma once
#include <filesystem>
#include <iostream>
#include <windows.h>

namespace IKIGAI::RENDER {
inline std::filesystem::path UpscalerExecutableDirectory() {
	wchar_t path[32768] { };
	const auto size = GetModuleFileNameW(nullptr, path, 32768);
	if (!size || size >= 32768) {
		throw UnsupportedUpscaler("Cannot resolve SDK runtime directory");
	}
	return std::filesystem::path(path).parent_path();
}

class UpscalerModule {
	HMODULE mModule = nullptr;

public:
	explicit UpscalerModule(const wchar_t* name) {
		const auto path = UpscalerExecutableDirectory() / name;
		mModule = LoadLibraryExW(path.c_str(), nullptr, LOAD_LIBRARY_SEARCH_DLL_LOAD_DIR | LOAD_LIBRARY_SEARCH_DEFAULT_DIRS);
		if (!mModule) {
			throw UnsupportedUpscaler("Upscaler runtime unavailable: " + path.string() + " (" + std::to_string(GetLastError()) + ")");
		}
	}

	~UpscalerModule() {
		if (mModule) {
			FreeLibrary(mModule);
		}
	}

	template <class T> T function(const char* name) const {
		auto result = reinterpret_cast<T>(GetProcAddress(mModule, name));
		if (!result) {
			throw UnsupportedUpscaler(std::string("SDK entry point missing: ") + name);
		}
		return result;
	}
};

inline void ValidateUpscalerTextures(const UpscaleFrameInputs& f) {
	ValidateUpscaleInputs(f);
	for (auto texture : { f.Color, f.Depth, f.MotionVectors }) {
		if (texture->getWidth() != f.RenderSize.x || texture->getHeight() != f.RenderSize.y) {
			throw std::runtime_error("UPSCALE input dimensions disagree with render size");
		}
	}
	if (f.Output->getWidth() != f.DisplaySize.x || f.Output->getHeight() != f.DisplaySize.y) {
		throw std::runtime_error("UPSCALE output dimensions disagree with display size");
	}
	if (f.Color->getFormat() != PixelFormat::RGBA_FLOAT || f.Output->getFormat() != PixelFormat::RGBA_FLOAT
		|| f.Depth->getFormat() != PixelFormat::DEPTH32_FLOAT || f.MotionVectors->getFormat() != PixelFormat::RG_FLOAT) {
		throw std::runtime_error("UPSCALE requires RGBA_FLOAT color/output, DEPTH32_FLOAT depth, RG_FLOAT motion");
	}
}
}
