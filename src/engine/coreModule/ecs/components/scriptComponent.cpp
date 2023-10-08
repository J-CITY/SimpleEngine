
#include "../object.h"
#include "scriptComponent.h"
#include <scriptModule/luaBinder.h>

using namespace IKIGAI;
using namespace IKIGAI::ECS;

EVENT::Event<object_ptr<IKIGAI::ECS::ScriptComponent>> ScriptComponent::destroyedEvent;
EVENT::Event<object_ptr<IKIGAI::ECS::ScriptComponent>> ScriptComponent::createdEvent;

ScriptComponent::ScriptComponent(Ref<ECS::Object> obj) : Component(obj) {
	__NAME__ = "ScriptComponent";
}

ScriptComponent::ScriptComponent(Ref<ECS::Object> obj, const std::string& name) :
	name(name), Component(obj) {
	__NAME__ = "ScriptComponent";
}

ScriptComponent::~ScriptComponent() = default;

sol::table& ScriptComponent::getTable() {
	return object;
}

const std::string& ScriptComponent::getName() const {
	return name;
}

void ScriptComponent::setScript(const std::string& _name) {
	name = _name;
	ScriptComponent::createdEvent.run(this);
}


#include <rttr/registration>

RTTR_REGISTRATION
{
	rttr::registration::class_<IKIGAI::ECS::ScriptComponent>("ScriptComponent")
	(
		rttr::metadata(MetaInfo::FLAGS, MetaInfo::SERIALIZABLE | MetaInfo::USE_IN_EDITOR)
	)
	.property("Path", &IKIGAI::ECS::ScriptComponent::getName, &IKIGAI::ECS::ScriptComponent::setScript)
	(
		rttr::metadata(MetaInfo::FLAGS, MetaInfo::SERIALIZABLE | MetaInfo::USE_IN_EDITOR | MetaInfo::USE_IN_COMPONENT_INSPECTOR)
	);
}
