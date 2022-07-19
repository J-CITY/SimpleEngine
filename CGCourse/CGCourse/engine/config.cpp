#include "config.h"

#include <filesystem>

using namespace KUMA;

void Config::Init() {
	ROOT = std::filesystem::current_path().string() + "\\";
}

std::string Config::ROOT;
const std::string Config::ASSETS_PATH = "Assets/";
const int Config::BITS_PER_CHANNEL = 8;
const std::string Config::ENGINE_ASSETS_PATH  = "Assets\\Engine\\";
const std::string Config::USER_ASSETS_PATH = "Assets\\Game\\";

