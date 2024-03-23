#include "stringUtils.h"

#include <algorithm>
#include <ranges>
#include <regex>

using namespace IKIGAI::UTILS;

void IKIGAI::UTILS::ReplaceSubstringsInPlace(std::string& subject, const std::string& search, const std::string& replace) {
	size_t pos = 0;
	while ((pos = subject.find(search, pos)) != std::string::npos) {
		subject.replace(pos, search.length(), replace);
		pos += replace.length();
	}
}

void IKIGAI::UTILS::ReplaceSubstringsInPlaceRegex(std::string& subject, const std::string& search, const std::string& replace) {

	std::regex vowel_re(search);
	subject = std::regex_replace(subject, vowel_re, replace);
}

std::string IKIGAI::UTILS::ReplaceSubstrings(const std::string& _subject, const std::string& search, const std::string& replace) {
	auto subject = _subject;
	size_t pos = 0;
	while ((pos = subject.find(search, pos)) != std::string::npos) {
		subject.replace(pos, search.length(), replace);
		pos += replace.length();
	}
	return subject;
}

std::string IKIGAI::UTILS::ReplaceSubstringsRegex(const std::string& subject, const std::string& search, const std::string& replace) {
	std::regex vowel_re(search);
	return std::regex_replace(subject, vowel_re, replace);
}

std::vector<std::string> IKIGAI::UTILS::Split(const std::string& str, char delimiter) {
	auto to_string = [](auto&& r) {
		const auto data = &*r.begin();
		const auto size = static_cast<std::size_t>(std::ranges::distance(r));
		return std::string{ data, size };
	};
	auto range = str |
		std::ranges::views::split(delimiter) |
		std::ranges::views::transform(to_string);

	return { std::ranges::begin(range), std::ranges::end(range) };
}

std::string IKIGAI::UTILS::ToLower(std::string_view s) {
	std::string data(s.begin(), s.end());
	std::transform(data.begin(), data.end(), data.begin(),
		[](unsigned char c) { return std::tolower(c); });
	return data;
}
std::string IKIGAI::UTILS::ToUpper(std::string_view s) {
	std::string data(s.begin(), s.end());
	std::transform(data.begin(), data.end(), data.begin(),
		[](unsigned char c) { return std::toupper(c); });
	return data;
}
