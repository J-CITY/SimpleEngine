#pragma once

namespace sol {
	class state;
}

namespace KUMA::SCRIPTING {
	class LuaBinder {
	public:
		static void CallBinders(sol::state& p_luaState);
	};
}
