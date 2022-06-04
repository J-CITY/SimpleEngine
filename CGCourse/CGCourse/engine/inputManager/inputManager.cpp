#include "inputManager.h"

using namespace KUMA::INPUT_SYSTEM;


InputManager::InputManager(KUMA::WINDOW_SYSTEM::Window& _window) : window(_window) {
	keyPressedListener = window.keyPressedEvent.add(std::bind(&InputManager::onKeyPressed, this, std::placeholders::_1));
	keyReleasedListener = window.keyReleasedEvent.add(std::bind(&InputManager::onKeyReleased, this, std::placeholders::_1));
	mouseButtonPressedListener = window.mouseButtonPressedEvent.add(std::bind(&InputManager::onMouseButtonPressed, this, std::placeholders::_1));
	mouseButtonReleasedListener = window.mouseButtonReleasedEvent.add(std::bind(&InputManager::onMouseButtonReleased, this, std::placeholders::_1));
	gamepadListener = window.gamepadEvent.add(std::bind(&InputManager::onGamepad, this, std::placeholders::_1));
}

InputManager::~InputManager() {
	window.keyPressedEvent.removeListener(keyPressedListener);
	window.keyReleasedEvent.removeListener(keyReleasedListener);
	window.mouseButtonPressedEvent.removeListener(mouseButtonPressedListener);
	window.mouseButtonReleasedEvent.removeListener(mouseButtonReleasedListener);
	window.gamepadEvent.removeListener(gamepadListener);
}

bool InputManager::isKeyPressed(EKey p_key) const {
	return keyEvents.find(p_key) != keyEvents.end() && keyEvents.at(p_key) == EKeyState::KEY_DOWN;
}

bool InputManager::isKeyReleased(EKey p_key) const {
	return keyEvents.find(p_key) != keyEvents.end() && keyEvents.at(p_key) == EKeyState::KEY_UP;
}

bool InputManager::isMouseButtonPressed(EMouseButton p_button) const {
	return mouseButtonEvents.find(p_button) != mouseButtonEvents.end() && mouseButtonEvents.at(p_button) == EMouseButtonState::MOUSE_DOWN;
}

bool InputManager::isMouseButtonReleased(EMouseButton p_button) {
	bool res = mouseButtonEvents.find(p_button) != mouseButtonEvents.end() && mouseButtonEvents.at(p_button) == EMouseButtonState::MOUSE_UP;
	mouseButtonEvents.erase(p_button);
	return res;
}

KUMA::MATHGL::Vector2i InputManager::getMousePosition() const {
	return window.getMousePos();
}

void InputManager::clearEvents() {
	keyEvents.clear();
	mouseButtonEvents.clear();
}

void InputManager::onKeyPressed(int p_key) {
	keyEvents[static_cast<EKey>(p_key)] = EKeyState::KEY_DOWN;
}

void InputManager::onKeyReleased(int p_key) {
	keyEvents[static_cast<EKey>(p_key)] = EKeyState::KEY_UP;
}

void InputManager::onMouseButtonPressed(int p_button) {
	mouseButtonEvents[static_cast<EMouseButton>(p_button)] = EMouseButtonState::MOUSE_DOWN;
}

void InputManager::onMouseButtonReleased(int p_button) {
	mouseButtonEvents[static_cast<EMouseButton>(p_button)] = EMouseButtonState::MOUSE_UP;
}

void InputManager::onGamepad(INPUT::Gamepad::GamepadData gd) {
	gamepads[gd.id] = gd;
}

bool InputManager::isGamepadExist(int id) const {
	return gamepads.count(id);
}

KUMA::INPUT::Gamepad::GamepadData InputManager::getGamepad(int id) {
	if (!gamepads.count(id)) {
		throw std::invalid_argument("Wrong gamepad id " + std::to_string(id));
	}
	return gamepads[id];
}

