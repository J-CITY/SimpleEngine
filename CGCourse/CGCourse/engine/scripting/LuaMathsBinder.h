#pragma once

#include <sol/sol.hpp>

namespace KUMA::SCRIPTING {
	class LuaMathsBinder {
	public:
		static void BindMaths(sol::state& p_luaState);
	};
}