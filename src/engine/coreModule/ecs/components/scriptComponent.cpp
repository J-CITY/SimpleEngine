
#include "../object.h"
#include "scriptComponent.h"
#include <scriptModule/luaBinder.h>

using namespace IKIGAI;
using namespace IKIGAI::ECS;

EVENT::Event<UTILS::WeakPtr<IKIGAI::ECS::ScriptComponent>> ScriptComponentEvents::destroyedEvent;
EVENT::Event<UTILS::WeakPtr<IKIGAI::ECS::ScriptComponent>> ScriptComponentEvents::createdEvent;

ScriptComponent::ScriptComponent(UTILS::Ref<ECS::Object> obj) : Component(obj) {
	__NAME__ = "ScriptComponent";
}

ScriptComponent::ScriptComponent(UTILS::Ref<ECS::Object> obj, const std::string& name) :
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

std::string ScriptComponent::getScriptName() {
	return name;
}

void ScriptComponent::setScript(std::string _name) {
	name = _name;
	ScriptComponentEvents::createdEvent.run(getWeak<ScriptComponent>());
}


#include <rttr/registration>

RTTR_REGISTRATION
{
	rttr::registration::class_<IKIGAI::ECS::ScriptComponent>("ScriptComponent")
	(
		rttr::metadata(MetaInfo::FLAGS, MetaInfo::SERIALIZABLE)
	)
	.property("Path", &IKIGAI::ECS::ScriptComponent::getScriptName, &IKIGAI::ECS::ScriptComponent::setScript)
	(
		rttr::metadata(MetaInfo::FLAGS, MetaInfo::SERIALIZABLE | MetaInfo::USE_IN_EDITOR_COMPONENT_INSPECTOR),
		rttr::metadata(EditorMetaInfo::EDIT_WIDGET, EditorMetaInfo::WidgetType::STRING)
	);
}
