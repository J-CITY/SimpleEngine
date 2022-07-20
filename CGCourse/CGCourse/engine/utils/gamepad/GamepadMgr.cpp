#include "GamepadMgr.h"

#include <GLFW/glfw3.h>

using namespace KUMA::INPUT;

GamepadMgr::GamepadMgr() {
	initialize();
}

GamepadMgr& GamepadMgr::Instance() {
	static GamepadMgr theGamepadMgr;
	return theGamepadMgr;
}

void GamepadMgr::initialize() {
	gamepads.clear();
	for (auto i = GLFW_JOYSTICK_1; i < GLFW_JOYSTICK_1 + 15; i++) {
		if (glfwJoystickPresent(i) && glfwJoystickIsGamepad(i)) {
			gamepads.push_back(Gamepad(i));
		}
	}
}

void GamepadMgr::update(std::function<void(Gamepad::GamepadData&)> cb) {
	for (auto& g : gamepads) {
		if (g.updateData()) {
			cb(g.getData());
		}
	}
}


