#pragma once
#include <string>
#include <vector>

namespace IKIGAI::UTILS {
	void ReplaceSubstringsInPlace(std::string& subject, const std::string& search, const std::string& replace);

	std::string ReplaceSubstrings(const std::string& _subject, const std::string& search, const std::string& replace);

	std::vector<std::string> split(const std::string& str, char delimiter);

	std::string toLower(const std::string_view s);
	std::string toUpper(const std::string_view s);
}
