#pragma once

#include <filesystem>
#include <string>

namespace KUMA {
	struct Config {
		const inline static std::string ROOT = std::filesystem::current_path().string() + "/";
		constexpr static auto ASSETS_PATH = "Assets/";
		constexpr static auto ENGINE_ASSETS_PATH = "Assets/Engine/";
		constexpr static auto USER_ASSETS_PATH = "Assets/Game/";
	};
};
