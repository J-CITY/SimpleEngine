
#include "../object.h"
#include "script.h"
#include "../../scripting/LuaBinder.h"

using namespace KUMA;
using namespace KUMA::ECS;

EVENT::Event<Script*> Script::createdEvent;
EVENT::Event<Script*> Script::destroyedEvent;

Script::Script(const ECS::Object& obj, const std::string& name) :
	name(name), Component(obj) {
	__NAME__ = "Script";
	createdEvent.run(this);
}

Script::~Script() {
	destroyedEvent.run(this);
}

bool Script::registerToLuaContext(sol::state& p_luaState, const std::string& p_scriptFolder) {
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

void Script::unregisterFromLuaContext() {
	object = sol::nil;
}

sol::table& Script::getTable() {
	return object;
}

void Script::onAwake() {
	luaCall("OnAwake");
}

void Script::onStart() {
	luaCall("OnStart");
}

void Script::onEnable() {
	luaCall("OnEnable");
}

void Script::onDisable() {
	luaCall("OnDisable");
}

void Script::onDestroy() {
	luaCall("OnDestroy");
}

void Script::onUpdate(std::chrono::duration<double> dt) {
	luaCall("OnUpdate", static_cast<float>(dt.count()));
}

void Script::onFixedUpdate(std::chrono::duration<double> dt)
{
	luaCall("OnFixedUpdate", static_cast<float>(dt.count()));
}

void Script::onLateUpdate(std::chrono::duration<double> dt) {
	luaCall("OnLateUpdate", static_cast<float>(dt.count()));
}
