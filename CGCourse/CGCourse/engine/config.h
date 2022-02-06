
#pragma once

#include <filesystem>
#include <string>
namespace KUMA {
	struct Config {
		static void Init() {
			ROOT = std::filesystem::current_path().string() + "\\";
			
		}
		static std::string ROOT;
		const static std::string ASSETS_PATH;
		const static int BITS_PER_CHANNEL;

		const static std::string ENGINE_ASSETS_PATH;
		const static std::string USER_ASSETS_PATH;
	};

};
