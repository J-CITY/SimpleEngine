#pragma once

#include <string>

namespace IKIGAI {
	struct Config {
		inline static std::string ROOT;
		inline const static std::string ASSETS_PATH = "assets/";
		inline const static std::string ENGINE_ASSETS_PATH = ASSETS_PATH + "engine/";
		inline const static std::string USER_ASSETS_PATH = ASSETS_PATH + "game/";
	};
};
