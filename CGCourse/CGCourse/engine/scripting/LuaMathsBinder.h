#pragma once

namespace sol {
	class state;
}

namespace KUMA::SCRIPTING {
	class LuaMathsBinder {
	public:
		static void BindMaths(sol::state& p_luaState);
	};
}
