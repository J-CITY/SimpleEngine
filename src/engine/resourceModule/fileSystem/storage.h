#pragma once
#include <filesystem>
#include <span>
#include <string>
#include <vector>

namespace IKIGAI::RESOURCES::STORAGE {
inline std::string Utf8(const std::filesystem::path& path) {
	const auto value = path.generic_u8string();
	return {reinterpret_cast<const char*>(value.data()), value.size()};
}
// All paths are explicit native paths, never aliases that can change mid-save.
std::filesystem::path AppDataPath(const std::string& org, const std::string& app);
void WriteAtomic(const std::filesystem::path& path, std::span<const uint8_t> bytes);
void WriteTextAtomic(const std::filesystem::path& path, const std::string& text);
std::vector<uint8_t> Read(const std::filesystem::path& path, uint64_t limit = 64 * 1024 * 1024);
std::filesystem::path Child(const std::filesystem::path& root, const std::string& relative);
std::string Sha256(const std::filesystem::path& path);
std::string Sha256(std::span<const uint8_t> bytes);
}
