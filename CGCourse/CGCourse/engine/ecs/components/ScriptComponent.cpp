
#include "../object.h"
#include "ScriptComponent.h"
#include "../../scripting/luaBinder.h"

using namespace KUMA;
using namespace KUMA::ECS;

EVENT::Event<object_ptr<KUMA::ECS::ScriptComponent>> ScriptComponent::destroyedEvent;
EVENT::Event<object_ptr<KUMA::ECS::ScriptComponent>> ScriptComponent::createdEvent;

ScriptComponent::ScriptComponent(Ref<ECS::Object> obj, const std::string& name) :
	name(name), Component(obj) {
	__NAME__ = "ScriptComponent";
}

ScriptComponent::~ScriptComponent() {
}

bool ScriptComponent::registerToLuaContext(sol::state& p_luaState, const std::string& p_scriptFolder) {
	using namespace KUMA::SCRIPTING;

	auto result = p_luaState.safe_script_file(p_scriptFolder + name + ".lua", &sol::script_pass_on_error);

	if (!result.valid()) {
		sol::error err = result;
		LOG_ERROR(err.what());
		return false;
	}
	else {
		if (result.return_count() == 1 && result[0].is<sol::table>()) {
			object = result[0];
			object["owner"] = &obj;
			return true;
		}
		else {
			LOG_ERROR("'" + name + ".lua' missing return expression");
			return false;
		}
	}
}

void ScriptComponent::unregisterFromLuaContext() {
	object = sol::nil;
}

sol::table& ScriptComponent::getTable() {
	return object;
}

void ScriptComponent::onAwake() {
	luaCall("OnAwake");
}

void ScriptComponent::onStart() {
	luaCall("OnStart");
}

void ScriptComponent::onEnable() {
	luaCall("OnEnable");
}

void ScriptComponent::onDisable() {
	luaCall("OnDisable");
}

void ScriptComponent::onDestroy() {
	luaCall("OnDestroy");
}

void ScriptComponent::onUpdate(std::chrono::duration<double> dt) {
	luaCall("OnUpdate", static_cast<float>(dt.count()));
}

void ScriptComponent::onFixedUpdate(std::chrono::duration<double> dt)
{
	luaCall("OnFixedUpdate", static_cast<float>(dt.count()));
}

void ScriptComponent::onLateUpdate(std::chrono::duration<double> dt) {
	luaCall("OnLateUpdate", static_cast<float>(dt.count()));
}

std::string ScriptComponent::getName() const {
	return name;
}
