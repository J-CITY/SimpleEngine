#include "utilsReflectDx12.h"

using namespace IKIGAI::RENDER::UTILS;

std::string IKIGAI::RENDER::UTILS::ltrim(const std::string& s) {
	size_t start = s.find_first_not_of(WHITESPACE);
	return (start == std::string::npos) ? "" : s.substr(start);
}

std::string IKIGAI::RENDER::UTILS::rtrim(const std::string& s) {
	size_t end = s.find_last_not_of(WHITESPACE);
	return (end == std::string::npos) ? "" : s.substr(0, end + 1);
}

std::string IKIGAI::RENDER::UTILS::trim(const std::string& s) {
	return rtrim(ltrim(s));
}

bool IKIGAI::RENDER::UTILS::isConsistInString(const std::string& s, const std::string& pattern) {
	return s.find(pattern) != std::string::npos;
}

size_t IKIGAI::RENDER::UTILS::findPos(const std::string& s, const std::string& pattern) {
	return s.find(pattern);
}

bool IKIGAI::RENDER::UTILS::isNumber(const std::string& s) {
	return !s.empty() && std::find_if(s.begin(),
		s.end(), [](unsigned char c) { return !std::isdigit(c); }) == s.end();
}

std::string IKIGAI::RENDER::UTILS::extractFromString(const std::string& s, const std::string& startPattern, const std::string& endPattern) {
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
