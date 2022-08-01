#include "gamepadManager.h"

#include <GLFW/glfw3.h>

using namespace KUMA::INPUT;

GamepadManager::GamepadManager() {
	initialize();
}

GamepadManager& GamepadManager::Instance() {
	static GamepadManager theGamepadMgr;
	return theGamepadMgr;
}

void GamepadManager::initialize() {
	gamepads.clear();
	for (auto i = GLFW_JOYSTICK_1; i < GLFW_JOYSTICK_1 + 15; i++) {
		if (glfwJoystickPresent(i) && glfwJoystickIsGamepad(i)) {
			gamepads.push_back(Gamepad(i));
		}
	}
}

void GamepadManager::update(std::function<void(Gamepad::GamepadData&)> cb) {
	for (auto& g : gamepads) {
		if (g.updateData()) {
			cb(g.getData());
		}
	}
}


