#include "scriptSystem.h"
#include "../../scene/sceneManager.h"

KUMA::ECS::ScriptSystem::ScriptSystem() = default;

void KUMA::ECS::ScriptSystem::onAwake() {
	for (auto& component : *ECS::ComponentManager::getInstance()->getComponentArray<ECS::ScriptComponent>()) {
		luaCall(component, "OnAwake");
	}
}

void KUMA::ECS::ScriptSystem::onStart() {
	for (auto& component : *ECS::ComponentManager::getInstance()->getComponentArray<ECS::ScriptComponent>()) {
		luaCall(component, "OnStart");
	}
}

void KUMA::ECS::ScriptSystem::onEnable() {
	for (auto& component : *ECS::ComponentManager::getInstance()->getComponentArray<ECS::ScriptComponent>()) {
		luaCall(component, "OnEnable");
	}
}

void KUMA::ECS::ScriptSystem::onDisable() {
	for (auto& component : *ECS::ComponentManager::getInstance()->getComponentArray<ECS::ScriptComponent>()) {
		luaCall(component, "OnDisable");
	}
}

void KUMA::ECS::ScriptSystem::onDestroy() {
	for (auto& component : *ECS::ComponentManager::getInstance()->getComponentArray<ECS::ScriptComponent>()) {
		luaCall(component, "OnDestroy");
	}
}

void KUMA::ECS::ScriptSystem::onUpdate(std::chrono::duration<double> dt) {
	for (auto& component : *ECS::ComponentManager::getInstance()->getComponentArray<ECS::ScriptComponent>()) {
		luaCall(component, "OnUpdate", static_cast<float>(dt.count()));
	}
}

void KUMA::ECS::ScriptSystem::onFixedUpdate(std::chrono::duration<double> dt) {
	for (auto& component : *ECS::ComponentManager::getInstance()->getComponentArray<ECS::ScriptComponent>()) {
		luaCall(component, "OnFixedUpdate", static_cast<float>(dt.count()));
	}
}

void KUMA::ECS::ScriptSystem::onLateUpdate(std::chrono::duration<double> dt) {
	for (auto& component : *ECS::ComponentManager::getInstance()->getComponentArray<ECS::ScriptComponent>()) {
		luaCall(component, "OnLateUpdate", static_cast<float>(dt.count()));
	}
}


bool KUMA::ECS::ScriptSystem::registerToLuaContext(ScriptComponent& component, sol::state& p_luaState, const std::string& p_scriptFolder) {
	using namespace KUMA::SCRIPTING;

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
			object["owner"] = component.obj.getPtr().get();
			return true;
		}
		else {
			LOG_ERROR("'" + component.getName() + ".lua' missing return expression");
			return false;
		}
	}
}

void KUMA::ECS::ScriptSystem::unregisterFromLuaContext(ScriptComponent& component) {
	component.getTable() = sol::nil;
}