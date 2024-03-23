#include "component.h"
#include "../object.h"

using namespace IKIGAI::ECS;

IKIGAI::ECS::Component::Component(UTILS::Ref<ECS::Object> obj) : ControlBlockHandler(this), obj(obj) {}

IKIGAI::ECS::Component::~Component() {
	onDisable();
	onDestroy();
}

std::string IKIGAI::ECS::Component::getName() {
	return __NAME__;
}

std::string IKIGAI::ECS::Component::getTypeidName() {
	return typeid(*this).name();
}

const Object& Component::getObject() {
	return obj.get();
}
