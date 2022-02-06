#include "LuaBinder.h"

#include "LuaMathsBinder.h"
#include "LuaActorBinder.h"
#include "LuaComponentBinder.h"
#include "LuaGlobalsBinder.h"

using namespace KUMA::SCRIPTING;

void LuaBinder::CallBinders(sol::state& p_luaState) {
	auto& L = p_luaState;

	LuaMathsBinder::BindMaths(L);
	LuaActorBinder::BindActor(L);
	LuaComponentBinder::BindComponent(L);
	LuaGlobalsBinder::BindGlobals(L);
}