#pragma once

#include <sol/sol.hpp>

namespace KUMA::SCRIPTING {
	class LuaGlobalsBinder {
	public:
		static void BindGlobals(sol::state& p_luaState);
	};
}