#include "gamepad.h"

#include <iostream>
#include <GLFW/glfw3.h>

using namespace KUMA::INPUT;

bool _isButtonPressed(int gamepadNumber, GLFWgamepadstate& state, Gamepad::GAMEPAD_BUTTON btn) {
	if (glfwGetGamepadState(gamepadNumber, &state)) {
		return state.buttons[static_cast<int>(btn)];
	}
	return false;
}

float _getAxisPosition(int gamepadNumber, GLFWgamepadstate& state, Gamepad::GAMEPAD_AXIS axis) {
	if (glfwGetGamepadState(gamepadNumber, &state)) {
		return state.axes[static_cast<int>(axis)];
	}
	return 0.0f;
}

float _getTriggerValue(int gamepadNumber, GLFWgamepadstate& state, Gamepad::GAMEPAD_TRIGGER tgr) {
	if (glfwGetGamepadState(gamepadNumber, &state)) {
		return state.axes[static_cast<int>(tgr)];
	}
	return 0.0f;
}

Gamepad::Gamepad(int number) : gamepadNumber(number) {
	name = glfwGetGamepadName(gamepadNumber);
}

bool Gamepad::GamepadData::operator!=(const GamepadData& data) const {
	auto comparef = [](float x, float y, float epsilon = 0.00001f) {
		if (fabs(x - y) < epsilon)
			return true;
		return false;
	};
	if (!comparef(data.rightTrigger, rightTrigger) ||
		!comparef(data.leftTrigger, leftTrigger) ||
		!comparef(data.rightSticX, rightSticX) ||
		!comparef(data.rightSticY, rightSticY) ||
		!comparef(data.leftSticX, leftSticX) ||
		!comparef(data.leftSticY, leftSticY)) {
		return true;
	}
	return buttons == data.buttons;
}

bool Gamepad::isButtonPressed(GAMEPAD_BUTTON btn) const {
	GLFWgamepadstate state;
	if (glfwGetGamepadState(gamepadNumber, &state)) {
		return state.buttons[static_cast<int>(btn)];
	}
	return false;
}

float Gamepad::getAxisPosition(GAMEPAD_AXIS axis) const {
	GLFWgamepadstate state;
	if (glfwGetGamepadState(gamepadNumber, &state)) {
		return state.axes[static_cast<int>(axis)];
	}
	return 0.0f;
}

float Gamepad::getTriggerValue(GAMEPAD_TRIGGER tgr) const {
	GLFWgamepadstate state;
	if (glfwGetGamepadState(gamepadNumber, &state)) {
		return state.axes[static_cast<int>(tgr)];
	}
	return 0.0f;
}

Gamepad::GamepadData Gamepad::getData() const {
	return data;
}

bool Gamepad::updateData() {
	GLFWgamepadstate state;
	if (!glfwGetGamepadState(gamepadNumber, &state)) {
		return false;
	}

	GamepadData newData;
	for (auto& e : newData.buttons) {
		e.second = _isButtonPressed(gamepadNumber, state, e.first);
	}
	newData.leftSticX = _getAxisPosition(gamepadNumber, state, GAMEPAD_AXIS::leftStick_X);
	newData.leftSticY = _getAxisPosition(gamepadNumber, state, GAMEPAD_AXIS::leftStick_Y);
	newData.rightSticX = _getAxisPosition(gamepadNumber, state, GAMEPAD_AXIS::rightStick_X);
	newData.rightSticY = _getAxisPosition(gamepadNumber, state, GAMEPAD_AXIS::rightStick_Y);
	newData.leftTrigger = _getTriggerValue(gamepadNumber, state, GAMEPAD_TRIGGER::leftTrigger);
	newData.rightTrigger = _getTriggerValue(gamepadNumber, state, GAMEPAD_TRIGGER::rightTrigger);

	if (data != newData) {
		data = newData;
		return true;
	}
	return false;
}



