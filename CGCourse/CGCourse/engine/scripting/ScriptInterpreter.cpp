#include "../utils/debug/logger.h"

#include "LuaBinder.h"
#include "ScriptInterpreter.h"

#include "../ecs/components/script.h"
#include <string>

using namespace KUMA;
using namespace KUMA::SCRIPTING;

ScriptInterpreter::ScriptInterpreter(const std::string& scriptRootFolder) :
	scriptRootFolder(scriptRootFolder) {
	createLuaContextAndBindGlobals();

	KUMA::ECS::Script::createdEvent.add(std::bind(&ScriptInterpreter::consider, this, std::placeholders::_1));
	KUMA::ECS::Script::destroyedEvent.add(std::bind(&ScriptInterpreter::unconsider, this, std::placeholders::_1));
}

ScriptInterpreter::~ScriptInterpreter() {
	destroyLuaContext();
}

void ScriptInterpreter::createLuaContextAndBindGlobals() {
	if (!luaState) {
		luaState = std::make_unique<sol::state>();
		luaState->open_libraries(sol::lib::base, sol::lib::math);
		LuaBinder::CallBinders(*luaState);
		checkOk = true;

		std::for_each(scripts.begin(), scripts.end(), [this](ECS::Script* s) {
			if (!s->registerToLuaContext(*luaState, scriptRootFolder)) {
				checkOk = false;
			}
		});

		if (!checkOk) {
			LOG_ERROR("Script interpreter failed to register scripts. Check your lua scripts");
		}
	}
}

void ScriptInterpreter::destroyLuaContext() {
	if (luaState) {
		std::for_each(scripts.begin(), scripts.end(), [this](ECS::Script* behaviour) {
			behaviour->unregisterFromLuaContext();
		});

		luaState.reset();
		checkOk = false;
	}
}

void ScriptInterpreter::consider(KUMA::ECS::Script* s) {
	if (luaState) {
		scripts.push_back(s);

		if (!s->registerToLuaContext(*luaState, scriptRootFolder)) {
			checkOk = false;
		}
	}
}

void ScriptInterpreter::unconsider(KUMA::ECS::Script* p_toUnconsider) {
	if (luaState) {
		p_toUnconsider->unregisterFromLuaContext();
	}
	scripts.erase(std::remove_if(scripts.begin(), scripts.end(), [p_toUnconsider](ECS::Script* s) {
		return p_toUnconsider == s;
	}));

	refreshAll();
}

void ScriptInterpreter::refreshAll() {
	destroyLuaContext();
	createLuaContextAndBindGlobals();
}

bool ScriptInterpreter::isOk() const {
	return checkOk;
}
