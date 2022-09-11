#include "window.h"

#include "../config.h"
#include "../utils/gamepad/gamepadManager.h"
#include "../core/core.h"

#include "../resourceManager/ServiceManager.h"
#include "../ecs/object.h"
#include "../gui/guiObject.h"
#include "../scene/sceneManager.h"
#include "../render/gameRenderer.h"

import logger;
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"
#include "serdepp/include/serdepp/adaptor/reflection.hpp"
using namespace KUMA;
using namespace KUMA::WINDOW_SYSTEM;

static void glfwErrorCallback(int error, const char* description) {
	LOG_ERROR("Glfw Error: " + std::to_string(error) + " " + description);
}

Window::Window(const WindowSettings& windowSettings): windowSettings(windowSettings) {
	create();
}


Window::~Window() {
	glfwTerminate();
}

bool Window::isClosed() const {
	return isClose;
}

void Window::setSize(unsigned int w, unsigned int h) {
	glfwSetWindowSize(window.get(), w, h);
}

void Window::setPosition(int x, int y) {
	glfwSetWindowPos(window.get(), x, y);
}

void Window::hide() const {
	glfwHideWindow(window.get());
}

void Window::show() const {
	glfwShowWindow(window.get());
}

void Window::focus() const {
	glfwFocusWindow(window.get());
}

bool Window::hasFocus() const {
	return glfwGetWindowAttrib(window.get(), GLFW_FOCUSED);
}

void Window::setFullscreen(bool val) {
	windowSettings.isFullscreen = val;
	glfwSetWindowMonitor(
		window.get(),
		val ? glfwGetPrimaryMonitor() : nullptr,
		position.x,
		position.y,
		windowSettings.size.x,
		windowSettings.size.y,
		windowSettings.refreshRate
	);
}

void Window::toggleFullscreen() {
	setFullscreen(!windowSettings.isFullscreen);
}

bool Window::getIsFullscreen() const {
	return windowSettings.isFullscreen;
}

void Window::setTitle(const std::string& _title) {
	windowSettings.appName = _title;
	glfwSetWindowTitle(window.get(), windowSettings.appName.c_str());
}

std::string Window::getTitle() const {
	return windowSettings.appName;
}

void Window::keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	if (action == GLFW_PRESS) {
		keyPressedEvent.run(key);
	}
	else if (action == GLFW_RELEASE) {
		keyReleasedEvent.run(key);
	}
}

void Window::mouseCallback(GLFWwindow* window, int button, int action, int mods) {
	if (action == GLFW_PRESS) {
		mouseButtonPressedEvent.run(button);
	}
	else if (action == GLFW_RELEASE) {
		mouseButtonReleasedEvent.run(button);
	}
}

void Window::DestroyGLFW::operator()(GLFWwindow* ptr) const {
	glfwDestroyWindow(ptr);
}

WindowSettings& Window::getSetting() {
	return windowSettings;
}

void Window::updateWindow() {
	setFullscreen(windowSettings.isFullscreen);
}

void Window::create() {
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, windowSettings.majorVersion);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, windowSettings.minorVersion);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	window = std::unique_ptr<GLFWwindow, DestroyGLFW>(glfwCreateWindow(
		windowSettings.size.x, windowSettings.size.y, 
		windowSettings.appName.c_str(), NULL, NULL));


	glfwMakeContextCurrent(window.get());
	glfwSwapInterval(1); //vsync
	glfwWindowHint(GLFW_REFRESH_RATE, 60);

	

	if (!window) {
		glfwTerminate();
		throw std::runtime_error("Failed to create GLFW window");
	}
	else {
		int x, y;
		glfwGetWindowPos(window.get(), &x, &y);
		position.x = x;
		position.y = y;
	}

	INPUT::GamepadManager::Instance();

	glfwSetErrorCallback(glfwErrorCallback);
	glfwSetKeyCallback(window.get(), [](GLFWwindow* window, int key, int scancode, int action, int mods) {
		ImGui_ImplGlfw_KeyCallback(window, key, scancode, action, mods);
		RESOURCES::ServiceManager::Get<WINDOW_SYSTEM::Window>().keyCallback(window, key, scancode, action, mods);
	});
	
	glfwSetMouseButtonCallback(window.get(), [](GLFWwindow* window, int button, int action, int mods) {
		ImGui_ImplGlfw_MouseButtonCallback(window, button, action, mods);
		RESOURCES::ServiceManager::Get<WINDOW_SYSTEM::Window>().mouseCallback(window, button, action, mods);
	});
	glfwSetWindowCloseCallback(window.get(), [](GLFWwindow* window) {
		RESOURCES::ServiceManager::Get<WINDOW_SYSTEM::Window>().isClose = true;
	});
}

void Window::update() {
	INPUT::GamepadManager::Instance().update([this](INPUT::Gamepad::GamepadData& data) {
		gamepadEvent.run(data);
	});
}


void Window::pollEvent() {
	glfwPollEvents();
}

void Window::draw() {
	if (!glfwWindowShouldClose(window.get())) {
		glfwSwapBuffers(window.get());
		//glfwPollEvents();
	}
}

GLFWwindow& Window::getContext() const {
	return *window;
}

MATHGL::Vector2i Window::getMousePos() {
	double x, y;
	glfwGetCursorPos(window.get(), &x, &y);
	return MATHGL::Vector2i(static_cast<unsigned>(x), static_cast<unsigned>(y));
}
MATHGL::Vector2u Window::getSize() const {
	auto width = 0;
	auto height = 0;
	glfwGetWindowSize(window.get(), &width, &height);
	return MATHGL::Vector2u(width, height);
}