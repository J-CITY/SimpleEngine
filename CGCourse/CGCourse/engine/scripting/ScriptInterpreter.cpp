#include "scriptInterpreter.h"

#include <string>
#include <sol/sol.hpp>

import logger;

#include "luaBinder.h"
#include "../ecs/components/ScriptComponent.h"

using namespace KUMA;
using namespace KUMA::SCRIPTING;

ScriptInterpreter::ScriptInterpreter(const std::string& scriptRootFolder) :
	scriptRootFolder(scriptRootFolder) {
	createLuaContextAndBindGlobals();

	KUMA::ECS::ScriptComponent::createdEvent.add(std::bind(&ScriptInterpreter::consider, this, std::placeholders::_1));
	KUMA::ECS::ScriptComponent::destroyedEvent.add(std::bind(&ScriptInterpreter::unconsider, this, std::placeholders::_1));
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
		
		for (const auto& s : scripts) {
			if (!s->registerToLuaContext(*luaState, scriptRootFolder)) {
				checkOk = false;
			}
		}

		if (!checkOk) {
			LOG_ERROR("ScriptComponent interpreter failed to register scripts. Check your lua scripts");
		}
	}
}

void ScriptInterpreter::destroyLuaContext() {
	if (luaState) {
		for (const auto& s : scripts) {
			s->unregisterFromLuaContext();
		}

		luaState.reset();
		checkOk = false;
	}
}

void ScriptInterpreter::consider(std::shared_ptr<KUMA::ECS::ScriptComponent> s) {
	if (luaState) {
		scripts.push_back(s);

		if (!s->registerToLuaContext(*luaState, scriptRootFolder)) {
			checkOk = false;
		}
	}
}

void ScriptInterpreter::unconsider(std::shared_ptr<KUMA::ECS::ScriptComponent> p_toUnconsider) {
	if (luaState) {
		p_toUnconsider->unregisterFromLuaContext();
	}
	scripts.erase(std::remove_if(scripts.begin(), scripts.end(), [p_toUnconsider](std::shared_ptr<KUMA::ECS::ScriptComponent> s) {
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
