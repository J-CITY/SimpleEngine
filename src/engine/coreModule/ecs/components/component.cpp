#include "component.h"
#include "../object.h"

using namespace IKIGAI::ECS;

IKIGAI::ECS::ComponentBase::ComponentBase(UTILS::Ref<ECS::Object> obj) : IKIGAI::ECS2::Component(), obj(obj) {}

IKIGAI::ECS::ComponentBase::~ComponentBase() {
	onDisable();
	onDestroy();
}

std::string IKIGAI::ECS::ComponentBase::getName() {
	return __NAME__;
}

std::string IKIGAI::ECS::ComponentBase::getTypeidName() {
	return typeid(*this).name();
}

const Object& ComponentBase::getObject() {
	return obj.get();
}
