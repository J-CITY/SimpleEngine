#include "component.h"
#include "../object.h"

using namespace IKIGAI::ECS;

//Object dummy = Object(Object::Id{ 0 }, "", "");
//Component::Component(): ControlBlockHandler(this), obj(dummy) {  }

IKIGAI::ECS::Component::Component(Ref<ECS::Object> obj) : ControlBlockHandler(this), obj(obj) {}

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
