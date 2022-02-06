#pragma once

#include <sol/sol.hpp>

namespace KUMA::SCRIPTING {
	class LuaComponentBinder {
	public:
		static void BindComponent(sol::state& p_luaState);
	};
}