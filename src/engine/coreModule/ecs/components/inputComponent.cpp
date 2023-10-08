#include "inputComponent.h"

#include "transform.h"

using namespace IKIGAI::ECS;

InputComponent::InputComponent(Ref<ECS::Object> obj, std::function<void(std::chrono::duration<double>)> _inputEventFun):
	Component(obj) {
	inputEventFun = _inputEventFun;
	__NAME__ = "Input";
}

InputComponent::InputComponent(Ref<ECS::Object> obj) :
	Component(obj) {
	__NAME__ = "Input";
}

void InputComponent::setActive(bool val) {
	isActive = val;
}

bool InputComponent::getActive() const {
	return isActive;
}

const std::function<void(std::chrono::duration<double>)>& InputComponent::getEventFunc() {
	return inputEventFun;
}

#include <rttr/registration>

RTTR_REGISTRATION
{
	rttr::registration::class_<IKIGAI::ECS::InputComponent>("InputComponent")
	(
		rttr::metadata(MetaInfo::FLAGS, MetaInfo::SERIALIZABLE | MetaInfo::USE_IN_EDITOR)
	);
}