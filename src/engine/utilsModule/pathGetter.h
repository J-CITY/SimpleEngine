#pragma once
#include <string>
#include <coreModule/config.h>
#include <filesystem>
#ifdef __ANDROID__
#include <resourceModule/androidStorage.h>
#endif

namespace IKIGAI::UTILS {
	//TODO: android support
	static std::string GetRealPath(const std::string& p_path) {
#ifdef __ANDROID__
		std::string filePath;
		if (gHelperObject->ExtractAssetReturnFilename(p_path, filePath, true)) {
			return filePath;
		}
		if (gHelperObject->ExtractAssetReturnFilename(Config::ENGINE_ASSETS_PATH + p_path, filePath, true)) {
			return filePath;
		}
		if (gHelperObject->ExtractAssetReturnFilename(Config::USER_ASSETS_PATH + p_path, filePath, true)) {
			return filePath;
		}
		//TODO: throw or assert here
		return "";
#endif
		if (std::filesystem::exists(p_path)) {
			return p_path;
		}
		if (std::filesystem::exists(Config::ENGINE_ASSETS_PATH + p_path)) {
			return Config::ENGINE_ASSETS_PATH + p_path;
		}
		if (std::filesystem::exists(Config::USER_ASSETS_PATH + p_path)) {
			return Config::USER_ASSETS_PATH + p_path;
		}
		//TODO: throw or assert here
		return "";
	}
}
