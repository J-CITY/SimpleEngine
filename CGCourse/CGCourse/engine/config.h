#pragma once

#include <filesystem>
#include <string>

namespace KUMA {
	struct Config {
		const inline static std::string ROOT = std::filesystem::current_path().string() + "/";;
		const inline static std::string ASSETS_PATH = "Assets/";
		const inline static std::string ENGINE_ASSETS_PATH = "Assets/Engine/";
		const inline static std::string USER_ASSETS_PATH = "Assets/Game/";
	};
};
