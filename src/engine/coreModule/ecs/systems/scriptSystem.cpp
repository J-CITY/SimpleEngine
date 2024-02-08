#include "scriptSystem.h"
#include <sceneModule/sceneManager.h>

IKIGAI::ECS::ScriptSystem::ScriptSystem() = default;

void IKIGAI::ECS::ScriptSystem::onAwake() {
	for (auto& component : ECS::ComponentManager::GetInstance().getComponentArrayRef<ECS::ScriptComponent>()) {
		luaCall(component, "OnAwake");
	}
}

void IKIGAI::ECS::ScriptSystem::onStart() {
	for (auto& component : ECS::ComponentManager::GetInstance().getComponentArrayRef<ECS::ScriptComponent>()) {
		luaCall(component, "OnStart");
	}
}

void IKIGAI::ECS::ScriptSystem::onEnable() {
	for (auto& component : ECS::ComponentManager::GetInstance().getComponentArrayRef<ECS::ScriptComponent>()) {
		luaCall(component, "OnEnable");
	}
}

void IKIGAI::ECS::ScriptSystem::onDisable() {
	for (auto& component : ECS::ComponentManager::GetInstance().getComponentArrayRef<ECS::ScriptComponent>()) {
		luaCall(component, "OnDisable");
	}
}

void IKIGAI::ECS::ScriptSystem::onDestroy() {
	for (auto& component : ECS::ComponentManager::GetInstance().getComponentArrayRef<ECS::ScriptComponent>()) {
		luaCall(component, "OnDestroy");
	}
}

void IKIGAI::ECS::ScriptSystem::onUpdate(std::chrono::duration<double> dt) {
	for (auto& component : ECS::ComponentManager::GetInstance().getComponentArrayRef<ECS::ScriptComponent>()) {
		luaCall(component, "OnUpdate", static_cast<float>(dt.count()));
	}
}

void IKIGAI::ECS::ScriptSystem::onFixedUpdate(std::chrono::duration<double> dt) {
	for (auto& component : ECS::ComponentManager::GetInstance().getComponentArrayRef<ECS::ScriptComponent>()) {
		luaCall(component, "OnFixedUpdate", static_cast<float>(dt.count()));
	}
}

void IKIGAI::ECS::ScriptSystem::onLateUpdate(std::chrono::duration<double> dt) {
	for (auto& component : ECS::ComponentManager::GetInstance().getComponentArrayRef<ECS::ScriptComponent>()) {
		luaCall(component, "OnLateUpdate", static_cast<float>(dt.count()));
	}
}


bool IKIGAI::ECS::ScriptSystem::registerToLuaContext(ScriptComponent& component, sol::state& p_luaState, const std::string& p_scriptFolder) {
	using namespace IKIGAI::SCRIPTING;

	auto& object = component.getTable();
	auto result = p_luaState.safe_script_file(p_scriptFolder + component.getName() + ".lua", &sol::script_pass_on_error);
	if (!result.valid()) {
		sol::error err = result;
		LOG_ERROR(err.what());
		return false;
	}
	else {
		if (result.return_count() == 1 && result[0].is<sol::table>()) {
			object = result[0];
			object["owner"] = component.obj.getPtr();
			return true;
		}
		else {
			LOG_ERROR("'" + component.getName() + ".lua' missing return expression");
			return false;
		}
	}
}

void IKIGAI::ECS::ScriptSystem::unregisterFromLuaContext(ScriptComponent& component) {
	component.getTable() = sol::nil;
}