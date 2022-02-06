#pragma once

#include <sol/sol.hpp>

namespace KUMA::SCRIPTING {
	class LuaBinder {
	public:
		static void CallBinders(sol::state& p_luaState);
	};
}