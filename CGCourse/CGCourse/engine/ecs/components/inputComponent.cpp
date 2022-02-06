#include "inputComponent.h"

using namespace KUMA::ECS;

InputComponent::InputComponent(const ECS::Object& obj, std::function<void(float)> _inputEventFun):
	Component(obj) {
	inputEventFun = _inputEventFun;
	__NAME__ = "Input";
}
