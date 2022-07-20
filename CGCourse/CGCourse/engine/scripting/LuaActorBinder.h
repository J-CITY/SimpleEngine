#pragma once

namespace sol {
	class state;
}

namespace KUMA::SCRIPTING {
	class LuaActorBinder {
	public:
		static void BindActor(sol::state& p_luaState);
	};
}
