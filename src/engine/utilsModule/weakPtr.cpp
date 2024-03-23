#include "weakPtr.h"

using namespace IKIGAI::UTILS;

ControlBlockHandler::ControlBlockHandler(ECS::Component* ptr) {
	mCb = new UTILS::ControlBlock();
	mCb->mRC += 1;
	mCb->mPtr = ptr;
}

ControlBlockHandler::ControlBlockHandler(const ControlBlockHandler& obj) {
	mCb = obj.mCb;
	++(mCb->mRC);
}

ControlBlockHandler& ControlBlockHandler::operator=(const ControlBlockHandler& obj) {
	if (this == &obj) {
		return *this;
	}
	mCb = obj.mCb;
	++(mCb->mRC);
	return *this;
}

ControlBlockHandler::ControlBlockHandler(ControlBlockHandler&& obj) noexcept {
	mCb = obj.mCb;
	obj.mCb = nullptr;
}

ControlBlockHandler& ControlBlockHandler::operator=(ControlBlockHandler&& obj) noexcept {
	if (this == &obj) {
		return *this;
	}
	mCb = obj.mCb;
	obj.mCb = nullptr;
	return *this;
}

ControlBlockHandler::~ControlBlockHandler() {
	--(mCb->mRC);
	if (mCb->mRC == 0) {
		mCb->mPtr = nullptr;
		delete mCb;
	}
}

ControlBlock* ControlBlockHandler::getControlBlock() {
	return mCb;
}
