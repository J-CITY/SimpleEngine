#include "stringUtils.h"

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
