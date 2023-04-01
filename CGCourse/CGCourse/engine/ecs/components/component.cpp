#include "component.h"
#include "../object.h"

using namespace KUMA::ECS;

KUMA::ECS::Component::Component(Ref<ECS::Object> obj): obj(obj) {}

KUMA::ECS::Component::~Component() {
	onDisable();
	onDestroy();
}

std::string KUMA::ECS::Component::getName() {
	return __NAME__;
}

const Object& Component::getObject() {
	return obj.get();
}
