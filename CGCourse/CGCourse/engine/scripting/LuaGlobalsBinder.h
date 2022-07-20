#pragma once

namespace sol {
	class state;
}

namespace KUMA::SCRIPTING {
	class LuaGlobalsBinder {
	public:
		static void BindGlobals(sol::state& p_luaState);
	};
}
