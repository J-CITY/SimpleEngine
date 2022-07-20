#pragma once

namespace sol {
	class state;
}

namespace KUMA::SCRIPTING {
	class LuaComponentBinder {
	public:
		static void BindComponent(sol::state& p_luaState);
	};
}
