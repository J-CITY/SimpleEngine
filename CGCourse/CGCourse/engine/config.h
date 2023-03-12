#pragma once

#include <filesystem>
#include <string>

namespace KUMA {
	struct Config {
        static void ReplaceSubstringsInPlace(std::string& subject, const std::string& search, const std::string& replace) {
            size_t pos = 0;
            while ((pos = subject.find(search, pos)) != std::string::npos) {
                subject.replace(pos, search.length(), replace);
                pos += replace.length();
            }
        }

        static std::string ReplaceSubstrings(const std::string& _subject, const std::string& search, const std::string& replace) {
            auto subject = _subject;
            size_t pos = 0;
            while ((pos = subject.find(search, pos)) != std::string::npos) {
                subject.replace(pos, search.length(), replace);
                pos += replace.length();
            }
            return subject;
        }


		const inline static std::string ROOT = ReplaceSubstrings(std::filesystem::current_path().string(), "\\", "/") + "/";
		constexpr static auto ASSETS_PATH = "Assets/";
		constexpr static auto ENGINE_ASSETS_PATH = "Assets/Engine/";
		constexpr static auto USER_ASSETS_PATH = "Assets/Game/";
	};
};
