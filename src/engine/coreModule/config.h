#pragma once

#include <string>

namespace IKIGAI {
	struct Config {
		inline static std::string ROOT;
        inline static std::string CONTENT_RELEASE;
        inline static std::string STORAGE_ROOT;
        inline static std::string PROFILE_ID = "default";
        inline static std::string ORGANIZATION = "Ikigai";
        inline static std::string APPLICATION = "IkigaiEngine";
		inline const static std::string ASSETS_PATH = "assets/";
		inline const static std::string ENGINE_ASSETS_PATH = ASSETS_PATH + "engine/";
		inline const static std::string USER_ASSETS_PATH = ASSETS_PATH + "game/";
	};
};
