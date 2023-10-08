#include "luaBinder.h"

#include <sol/sol.hpp>

#include "luaMathsBinder.h"
#include "luaActorBinder.h"
#include "luaComponentBinder.h"
#include "luaGlobalsBinder.h"

using namespace IKIGAI::SCRIPTING;

void LuaBinder::CallBinders(sol::state& p_luaState) {
	auto& L = p_luaState;
	LuaMathsBinder::BindMaths(L);
	LuaActorBinder::BindActor(L);
	LuaComponentBinder::BindComponent(L);
	LuaGlobalsBinder::BindGlobals(L);
}