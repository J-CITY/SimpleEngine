#include "stringUtils.h"

#include <algorithm>
#include <ranges>

using namespace IKIGAI::UTILS;

void IKIGAI::UTILS::ReplaceSubstringsInPlace(std::string& subject, const std::string& search, const std::string& replace) {
	size_t pos = 0;
	while ((pos = subject.find(search, pos)) != std::string::npos) {
		subject.replace(pos, search.length(), replace);
		pos += replace.length();
	}
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

std::vector<std::string> IKIGAI::UTILS::split(const std::string& str, char delimiter) {
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

std::string IKIGAI::UTILS::toLower(const std::string_view s) {
	std::string data(s.begin(), s.end());
	std::transform(data.begin(), data.end(), data.begin(),
		[](unsigned char c) { return std::tolower(c); });
	return data;
}
std::string IKIGAI::UTILS::toUpper(const std::string_view s) {
	std::string data(s.begin(), s.end());
	std::transform(data.begin(), data.end(), data.begin(),
		[](unsigned char c) { return std::toupper(c); });
	return data;
}
