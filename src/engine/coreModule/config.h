#pragma once

#include <filesystem>
#include <string>
#include <utilsModule/stringUtils.h>

namespace IKIGAI {
	struct Config {
		const inline static std::string ROOT = UTILS::ReplaceSubstrings(std::filesystem::current_path().string(), "\\", "/") + "/";
		constexpr static auto ASSETS_PATH = "../Assets/";
		constexpr static auto ENGINE_ASSETS_PATH = "../Assets/Engine/";
		constexpr static auto USER_ASSETS_PATH = "../Assets/Game/";
	};
};
