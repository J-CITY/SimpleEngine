#pragma once
#include <nlohmann/json.hpp>
#include <stdexcept>
namespace IKIGAI::SCRIPTING {
inline void ValidateScriptFeatures(const nlohmann::json& data) {
#if (defined(USE_LUA) && !USE_LUA) || (defined(USE_IKIGAI_SCRIPT) && !USE_IKIGAI_SCRIPT)
	if (data.is_object()) {
#if defined(USE_LUA) && !USE_LUA
		if (data.contains("ScriptComponentType"))
			throw std::runtime_error("Scene requires Lua ScriptComponent, but USE_LUA=OFF");
#endif
#if defined(USE_IKIGAI_SCRIPT) && !USE_IKIGAI_SCRIPT
		if (data.contains("IkigaiScriptComponentType"))
			throw std::runtime_error("Scene requires IkigaiScriptComponent, but USE_IKIGAI_SCRIPT=OFF");
#endif
	}
	if (data.is_structured())
		for (const auto& value : data)
			ValidateScriptFeatures(value);
#endif
}
}
