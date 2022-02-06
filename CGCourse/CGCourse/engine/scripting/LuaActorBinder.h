#pragma once

#include <sol/sol.hpp>

namespace KUMA::SCRIPTING {
	class LuaActorBinder {
	public:
		static void BindActor(sol::state& p_luaState);
	};
}