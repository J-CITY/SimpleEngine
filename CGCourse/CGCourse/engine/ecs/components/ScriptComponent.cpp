
#include "../object.h"
#include "scriptComponent.h"
#include "../../scripting/luaBinder.h"

using namespace KUMA;
using namespace KUMA::ECS;

EVENT::Event<object_ptr<KUMA::ECS::ScriptComponent>> ScriptComponent::destroyedEvent;
EVENT::Event<object_ptr<KUMA::ECS::ScriptComponent>> ScriptComponent::createdEvent;

ScriptComponent::ScriptComponent(Ref<ECS::Object> obj, const std::string& name) :
	name(name), Component(obj) {
	__NAME__ = "ScriptComponent";
}

ScriptComponent::~ScriptComponent() = default;

sol::table& ScriptComponent::getTable() {
	return object;
}

std::string ScriptComponent::getName() const {
	return name;
}
