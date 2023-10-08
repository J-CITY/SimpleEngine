#include "weakPtr.h"

using namespace IKIGAI::UTILS;

ControlBlockHandler::ControlBlockHandler(ECS::Component* ptr) {
	mCb = new UTILS::ControlBlock();
	mCb->m_rc = 1;
	mCb->m_ptr = ptr;
}

ControlBlockHandler::~ControlBlockHandler() {
	mCb->m_rc -= 1;

	if (mCb->m_rc == 0) {
		mCb->m_ptr = nullptr;
		delete mCb;
	}
}
