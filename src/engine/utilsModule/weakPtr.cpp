#include "weakPtr.h"

using namespace IKIGAI::UTILS;

ControlBlockHandler::ControlBlockHandler() {
	mCb = new UTILS::ControlBlock();
	mCb->mRC += 1;
	mCb->mPtr = this;
}

ControlBlockHandler::ControlBlockHandler(ECS2::Component* ptr) {
	mCb = new UTILS::ControlBlock();
	mCb->mRC += 1;
	mCb->mPtr = (ControlBlockHandler*)ptr;
}

ControlBlockHandler::ControlBlockHandler(const ControlBlockHandler& obj) {
	mCb = obj.mCb;
	if (mCb)
		++mCb->mRC;
}

ControlBlockHandler& ControlBlockHandler::operator=(const ControlBlockHandler& obj) {
	if (this == &obj) {
		return *this;
	}
	if (mCb) {
		if (mCb->mPtr == this)
			mCb->mPtr = nullptr;
		if (--mCb->mRC == 0)
			delete mCb;
	}
	mCb = obj.mCb;
	if (mCb)
		++mCb->mRC;
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
	if (mCb) {
		if (mCb->mPtr == this)
			mCb->mPtr = nullptr;
		if (--mCb->mRC == 0)
			delete mCb;
	}
	mCb = obj.mCb;
	obj.mCb = nullptr;
	return *this;
}

ControlBlockHandler::~ControlBlockHandler() {
	if (!mCb)
		return;
	if (mCb->mPtr == this)
		mCb->mPtr = nullptr;
	--(mCb->mRC);
	if (mCb->mRC == 0) {
		mCb->mPtr = nullptr;
		delete mCb;
	}
}

ControlBlock* ControlBlockHandler::getControlBlock() {
	return mCb;
}
