#include "storage.h"
#include <array>
#include <atomic>
#include <bit>
#include <chrono>
#include <cstdlib>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <stdexcept>
#ifdef USE_SDL
#include <SDL3/SDL_filesystem.h>
#include <SDL3/SDL_stdinc.h>
#elif defined(_WIN32)
#include <windows.h>
#endif
#ifdef _WIN32
#include <windows.h>
#else
#include <fcntl.h>
#include <unistd.h>
#endif

namespace IKIGAI::RESOURCES::STORAGE {
std::filesystem::path Child(const std::filesystem::path& root, const std::string& relative) {
	auto path = std::filesystem::u8path(relative);
	if (relative.empty() || path.is_absolute() || path.has_root_name() || relative.find(':') != std::string::npos
		|| relative.find('\\') != std::string::npos || relative.find('\0') != std::string::npos)
		throw std::runtime_error("Expected a relative storage path");
	for (const auto& part : path)
		if (part == "..")
			throw std::runtime_error("Storage path traversal");
	auto result = root / path;
	// Reject links/junctions escaping a managed storage directory.
	auto base = std::filesystem::weakly_canonical(root);
	// SDL_GetPrefPath ends in a separator. A trailing empty component must not
	// be compared against the first component of the requested child path.
	if (base.filename().empty() && base != base.root_path())
		base = base.parent_path();
	const auto resolved = std::filesystem::weakly_canonical(result);
	auto it = resolved.begin();
	for (const auto& part : base) {
		if (it == resolved.end() || *it++ != part)
			throw std::runtime_error("Storage path escapes root");
	}
	return result;
}
std::filesystem::path AppDataPath(const std::string& org, const std::string& app) {
	if (org.empty() || app.empty() || org.find_first_of("/\\:") != std::string::npos || app.find_first_of("/\\:") != std::string::npos)
		throw std::runtime_error("Invalid application identity");
#ifdef USE_SDL
	char* value = SDL_GetPrefPath(org.c_str(), app.c_str());
	if (!value)
		throw std::runtime_error("SDL_GetPrefPath failed");
	auto result = std::filesystem::u8path(value);
	SDL_free(value);

	return result;
#elif defined(_WIN32)
	const wchar_t* value = _wgetenv(L"APPDATA");
	if (!value)
		throw std::runtime_error("APPDATA is unavailable");
	return std::filesystem::path(value) / std::filesystem::u8path(org) / std::filesystem::u8path(app);
#elif defined(__ANDROID__) || defined(__EMSCRIPTEN__)
	throw std::runtime_error("Supply an explicit persistent storage root on this platform");
#else
	const char* home = std::getenv("HOME");
	if (!home)
		throw std::runtime_error("HOME is unavailable");
#ifdef __APPLE__
	return std::filesystem::path(home) / "Library/Application Support" / org / app;
#else
	const char* xdg = std::getenv("XDG_DATA_HOME");
	return (xdg ? std::filesystem::path(xdg) : std::filesystem::path(home) / ".local/share") / org / app;
#endif
#endif
}
void WriteAtomic(const std::filesystem::path& path, std::span<const uint8_t> bytes) {
	if (path.filename().empty())
		throw std::runtime_error("Expected a storage file");
	std::filesystem::create_directories(path.parent_path());
	static std::atomic<uint64_t> sequence { 0 };
	auto temporary = path;
	temporary += ".tmp-" + std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()) + "-" + std::to_string(sequence++);
	struct Cleanup {
		std::filesystem::path path;
		~Cleanup() {
			std::error_code ec;
			std::filesystem::remove(path, ec);
		}
	} cleanup { temporary };
	{
		std::ofstream out(temporary, std::ios::binary | std::ios::trunc);
		out.exceptions(std::ios::badbit | std::ios::failbit);
		out.write(reinterpret_cast<const char*>(bytes.data()), static_cast<std::streamsize>(bytes.size()));
		out.close();
	}
#ifdef _WIN32
	if (!MoveFileExW(temporary.c_str(), path.c_str(), MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH))
		throw std::runtime_error("Atomic replace failed: " + std::to_string(GetLastError()));
#else
	const int fd = ::open(temporary.c_str(), O_RDONLY);
	if (fd < 0)
		throw std::runtime_error("Cannot open temporary file for sync");
	const int status = ::fsync(fd);
	::close(fd);
	if (status)
		throw std::runtime_error("Cannot sync temporary file");
	std::filesystem::rename(temporary, path);
	const int directory = ::open(path.parent_path().c_str(), O_RDONLY);
	if (directory >= 0) {
		::fsync(directory);
		::close(directory);
	}
#endif
}
void WriteTextAtomic(const std::filesystem::path& path, const std::string& text) {
	WriteAtomic(path, { reinterpret_cast<const uint8_t*>(text.data()), text.size() });
}
std::vector<uint8_t> Read(const std::filesystem::path& path, uint64_t limit) {
	const auto size = std::filesystem::file_size(path);
	if (size > limit)
		throw std::runtime_error("Storage file exceeds size limit");
	std::ifstream in(path, std::ios::binary);
	in.exceptions(std::ios::badbit | std::ios::failbit);
	std::vector<uint8_t> bytes(static_cast<size_t>(size));
	in.read(reinterpret_cast<char*>(bytes.data()), static_cast<std::streamsize>(size));
	return bytes;
}
namespace {
	std::string Hash(std::istream& input) {
		static constexpr uint32_t k[] = { 0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5, 0xd807aa98,
			0x12835b01, 0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174, 0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc,
			0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da, 0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147, 0x06ca6351,
			0x14292967, 0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85, 0xa2bfe8a1, 0xa81a664b,
			0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070, 0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3,
			0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3, 0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208, 0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2 };
		std::array<uint32_t, 8> hash { 0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a, 0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19 };
		auto block = [&](const uint8_t* bytes) {
			uint32_t w[64] { };
			for (int i = 0; i < 16; ++i)
				for (int j = 0; j < 4; ++j)
					w[i] = (w[i] << 8) | bytes[i * 4 + j];
			for (int i = 16; i < 64; ++i) {
				auto s0 = std::rotr(w[i - 15], 7) ^ std::rotr(w[i - 15], 18) ^ (w[i - 15] >> 3);
				auto s1 = std::rotr(w[i - 2], 17) ^ std::rotr(w[i - 2], 19) ^ (w[i - 2] >> 10);
				w[i] = w[i - 16] + s0 + w[i - 7] + s1;
			}
			auto [a, b, c, d, e, f, g, h] = hash;
			for (int i = 0; i < 64; ++i) {
				auto t1 = h + (std::rotr(e, 6) ^ std::rotr(e, 11) ^ std::rotr(e, 25)) + ((e & f) ^ ((~e) & g)) + k[i] + w[i];
				auto t2 = (std::rotr(a, 2) ^ std::rotr(a, 13) ^ std::rotr(a, 22)) + ((a & b) ^ (a & c) ^ (b & c));
				h = g;
				g = f;
				f = e;
				e = d + t1;
				d = c;
				c = b;
				b = a;
				a = t1 + t2;
			}
			hash[0] += a;
			hash[1] += b;
			hash[2] += c;
			hash[3] += d;
			hash[4] += e;
			hash[5] += f;
			hash[6] += g;
			hash[7] += h;
		};
		std::array<uint8_t, 64> bytes { };
		uint64_t size = 0;
		size_t tail = 0;
		while (input.read(reinterpret_cast<char*>(bytes.data()), bytes.size())) {
			size += 64;
			block(bytes.data());
		}
		if (!input.eof())
			throw std::runtime_error("Hash read failed");
		tail = static_cast<size_t>(input.gcount());
		size += tail;
		bytes[tail++] = 0x80;
		if (tail > 56) {
			std::fill(bytes.begin() + tail, bytes.end(), 0);
			block(bytes.data());
			tail = 0;
		}
		std::fill(bytes.begin() + tail, bytes.end(), 0);
		const auto bits = size * 8;
		for (int i = 0; i < 8; ++i)
			bytes[63 - i] = static_cast<uint8_t>(bits >> (i * 8));
		block(bytes.data());
		std::ostringstream output;
		output << std::hex << std::setfill('0');
		for (auto value : hash)
			output << std::setw(8) << value;
		return output.str();
	}
}
std::string Sha256(const std::filesystem::path& path) {
	std::ifstream input(path, std::ios::binary);
	if (!input)
		throw std::runtime_error("Cannot hash file");
	return Hash(input);
}
std::string Sha256(std::span<const uint8_t> bytes) {
	struct Buffer : std::streambuf {
		explicit Buffer(std::span<const uint8_t> bytes) {
			if (bytes.empty())
				return;
			auto* begin = const_cast<char*>(reinterpret_cast<const char*>(bytes.data()));
			setg(begin, begin, begin + bytes.size());
		}
	} buffer(bytes);
	std::istream input(&buffer);
	return Hash(input);
}
}
