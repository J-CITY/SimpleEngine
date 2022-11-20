#include "inputComponent.h"

using namespace KUMA::ECS;

InputComponent::InputComponent(Ref<ECS::Object> obj, std::function<void(std::chrono::duration<double>)> _inputEventFun):
	Component(obj) {
	inputEventFun = _inputEventFun;
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
