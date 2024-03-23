#pragma once
#include <string>
#include <unordered_map>
#ifdef USE_EDITOR
namespace IKIGAI::EDITOR {
	struct EditorLocale {
		static std::unordered_map<std::string, std::string> CurrentLocale;
	};
}
#endif
