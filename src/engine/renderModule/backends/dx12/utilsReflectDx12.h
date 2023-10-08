#pragma once
#include <string>

namespace IKIGAI::RENDER::UTILS {
	const static inline std::string WHITESPACE = " \n\r\t\f\v";

	std::string ltrim(const std::string& s);
	std::string rtrim(const std::string& s);
	std::string trim(const std::string& s);
	bool isConsistInString(const std::string& s, const std::string& pattern);
	size_t findPos(const std::string& s, const std::string& pattern);
	bool isNumber(const std::string& s);
	std::string extractFromString(const std::string& s, const std::string& startPattern, const std::string& endPattern);
}
