#pragma once
#include <string>
namespace IKIGAI::SCRIPTING {
// VFS reads are binary; unlike std::ifstream in text mode they retain Windows
// CRLFs and UTF-8 BOMs, which the embedded language lexer does not accept.
inline std::string NormalizeScriptSource(const std::string& input) {
	std::string result;
	result.reserve(input.size());
	size_t start = input.starts_with("\xef\xbb\xbf") ? 3 : 0;
	for (size_t i = start; i < input.size(); ++i) {
		if (input[i] == '\r') {
			if (i + 1 < input.size() && input[i + 1] == '\n')
				continue;
			result += '\n';
		}
		else
			result += input[i];
	}
	return result;
}
}
