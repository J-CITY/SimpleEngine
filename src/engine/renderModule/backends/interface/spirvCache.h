#pragma once
#include "resourceModule/fileSystem/storage.h"
#include <array>
#include <atomic>
#include <chrono>
#include <cstring>
#include <fstream>
#include <string_view>

namespace IKIGAI::RENDER::SHADER_CACHE {
inline std::string Hash(std::string_view text) {
	return RESOURCES::STORAGE::Sha256({reinterpret_cast<const uint8_t*>(text.data()), text.size()});
}
inline std::string Key(std::string_view compiler, uint32_t stage, uint32_t target, std::string_view source, std::string_view preamble) {
	std::string input = "ikigai-spirv-v1:" + std::to_string(stage) + ":" + std::to_string(target);
	for (const auto part : {compiler, source, preamble}) {
		input += ':' + std::to_string(part.size()) + ':';
		input.append(part);
	}
	return Hash(input);
}
inline constexpr char Magic[] = "IKSPV01";
inline constexpr size_t HeaderSize = sizeof(Magic) + 64;
inline constexpr size_t MaxSize = 16 * 1024 * 1024;

inline std::vector<uint32_t> Read(const std::filesystem::path& path) {
	std::ifstream input(path, std::ios::binary | std::ios::ate);
	if (!input) return {};
	const std::streamoff size = input.tellg();
	const auto invalid = [&]() {
		input.close();
		std::error_code error;
		std::filesystem::remove(path, error);
		return std::vector<uint32_t>{};
	};
	if (size < static_cast<std::streamoff>(HeaderSize + 5 * sizeof(uint32_t))
		|| size > static_cast<std::streamoff>(MaxSize) || (size - HeaderSize) % sizeof(uint32_t)) return invalid();
	input.seekg(0);
	std::array<char, HeaderSize> header{};
	std::vector<uint32_t> words((static_cast<size_t>(size) - HeaderSize) / sizeof(uint32_t));
	input.read(header.data(), header.size());
	input.read(reinterpret_cast<char*>(words.data()), words.size() * sizeof(uint32_t));
	const std::string_view bytes(reinterpret_cast<const char*>(words.data()), words.size() * sizeof(uint32_t));
	if (!input || std::memcmp(header.data(), Magic, sizeof(Magic)) || words.front() != 0x07230203
		|| Hash(bytes) != std::string_view(header.data() + sizeof(Magic), 64)) return invalid();
	return words;
}

// A cache failure must never prevent shader compilation. Readers only see
// complete files; concurrent writers can keep the first complete result.
inline void Write(const std::filesystem::path& path, const std::vector<uint32_t>& words) {
	if (words.size() < 5 || words.size() * sizeof(uint32_t) + HeaderSize > MaxSize) return;
	std::error_code error;
	std::filesystem::create_directories(path.parent_path(), error);
	if (error) return;
	static std::atomic<uint64_t> sequence{0};
	auto temporary = path;
	temporary += ".tmp-" + std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()) + "-" + std::to_string(sequence++);
	{
		std::ofstream output(temporary, std::ios::binary | std::ios::trunc);
		const std::string_view bytes(reinterpret_cast<const char*>(words.data()), words.size() * sizeof(uint32_t));
		const auto hash = Hash(bytes);
		output.write(Magic, sizeof(Magic));
		output.write(hash.data(), hash.size());
		output.write(bytes.data(), bytes.size());
		output.close();
		if (output) std::filesystem::rename(temporary, path, error);
	}
	std::filesystem::remove(temporary, error);
}
}
