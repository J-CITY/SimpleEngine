#pragma once
#include <string>
#include <vector>

namespace IKIGAI::UTILS {
	void ReplaceSubstringsInPlace(std::string& subject, const std::string& search, const std::string& replace);
	void ReplaceSubstringsInPlaceRegex(std::string& subject, const std::string& search, const std::string& replace);
	std::string ReplaceSubstrings(const std::string& _subject, const std::string& search, const std::string& replace);
	std::string ReplaceSubstringsRegex(const std::string& _subject, const std::string& search, const std::string& replace);
	std::vector<std::string> Split(const std::string& str, char delimiter);
	std::string ToLower(std::string_view s);
	std::string ToUpper(std::string_view s);
}
