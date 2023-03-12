#include "utilsReflectDx12.h"

using namespace KUMA::RENDER::UTILS;

std::string KUMA::RENDER::UTILS::ltrim(const std::string& s) {
	size_t start = s.find_first_not_of(WHITESPACE);
	return (start == std::string::npos) ? "" : s.substr(start);
}

std::string KUMA::RENDER::UTILS::rtrim(const std::string& s) {
	size_t end = s.find_last_not_of(WHITESPACE);
	return (end == std::string::npos) ? "" : s.substr(0, end + 1);
}

std::string KUMA::RENDER::UTILS::trim(const std::string& s) {
	return rtrim(ltrim(s));
}

bool KUMA::RENDER::UTILS::isConsistInString(const std::string& s, const std::string& pattern) {
	return s.find(pattern) != std::string::npos;
}

size_t KUMA::RENDER::UTILS::findPos(const std::string& s, const std::string& pattern) {
	return s.find(pattern);
}

bool KUMA::RENDER::UTILS::isNumber(const std::string& s) {
	return !s.empty() && std::find_if(s.begin(),
		s.end(), [](unsigned char c) { return !std::isdigit(c); }) == s.end();
}

std::string KUMA::RENDER::UTILS::extractFromString(const std::string& s, const std::string& startPattern, const std::string& endPattern) {
	auto start = s.find(startPattern);
	if (start == std::string::npos) {
		throw;
	}
	auto end = s.find(endPattern, start + 1);
	if (end == std::string::npos) {
		throw;
	}
	auto result = s.substr(start + 1, end - start - 1);
	return result;
}
