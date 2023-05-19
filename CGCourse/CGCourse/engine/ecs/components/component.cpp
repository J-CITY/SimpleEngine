#include "component.h"
#include "../object.h"

using namespace KUMA::ECS;

KUMA::ECS::Component::Component(Ref<ECS::Object> obj): obj(obj) {
	mCb = new UTILS::ControlBlock();
	mCb->rc = 1;
	mCb->ptr = this;
}

KUMA::ECS::Component::~Component() {
	onDisable();
	onDestroy();

	mCb->rc -= 1;
	mCb->ptr = nullptr;

	if (mCb->rc == 0) {
		delete mCb;
	}
}

std::string KUMA::ECS::Component::getName() {
	return __NAME__;
}

const Object& Component::getObject() {
	return obj.get();
}

KUMA::UTILS::ControlBlock* Component::getControlBlock() {
	return mCb;
}
