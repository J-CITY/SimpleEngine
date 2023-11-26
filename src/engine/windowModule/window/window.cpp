#include "window.h"

#include <coreModule/config.h>
#include <utilsModule/gamepad/gamepadManager.h>
#include <coreModule/core/core.h>

#include <coreModule/resourceManager/ServiceManager.h>
#include <coreModule/ecs/object.h>
#include <coreModule/gui/guiObject.h>
#include <sceneModule/sceneManager.h>
#include <renderModule/gameRenderer.h>

#ifdef OPENGL_BACKEND
#include <renderModule/gameRendererGl.h>
#endif
#ifdef VULKAN_BACKEND
#include "renderModule/gameRendererVk.h"
#endif

import logger;

using namespace IKIGAI;
using namespace IKIGAI::WINDOW_SYSTEM;

static void glfwErrorCallback(int error, const char* description) {
	LOG_ERROR("Glfw Error: " + std::to_string(error) + " " + description);
}

Window::Window(const WindowSettings& windowSettings) : windowSettings(windowSettings) {
	create();
}
bool Window::isClosed() const {
	return isClose;
}

void Window::toggleFullscreen() {
	setFullscreen(!windowSettings.isFullscreen);
}

int Window::getRefreshRate() const {
	return windowSettings.refreshRate;
}

bool Window::getIsFullscreen() const {
	return windowSettings.isFullscreen;
}


std::string Window::getTitle() const {
	return windowSettings.title;
}

WindowSettings& Window::getSetting() {
	return windowSettings;
}
#if defined(VULKAN_BACKEND) || defined(OPENGL_BACKEND)

Window::~Window() {
	glfwTerminate();
}


void Window::setSize(unsigned int w, unsigned int h) {
	windowSettings.size = { w, h };
	glfwSetWindowSize(window.get(), w, h);
}

void Window::setPosition(int x, int y) {
	position = { x, y };
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

void Window::setCursorVisible(bool isVisible, bool isLock) const {
	auto val = GLFW_CURSOR_NORMAL;
	if (!isVisible && isLock) {
		val = GLFW_CURSOR_DISABLED;
	}
	else if (!isVisible) {
		val = GLFW_CURSOR_HIDDEN;
	}
	glfwSetInputMode(window.get(), GLFW_CURSOR, val);
}

void Window::setDepthBits(int val) {
	windowSettings.depthBits = val;
	glfwWindowHint(GLFW_DEPTH_BITS, val);
}

int Window::getDepathBits() const {
	return windowSettings.depthBits;
}

void Window::setStencilBits(int val) {
	windowSettings.stencilBits = val;
	glfwWindowHint(GLFW_STENCIL_BITS, val);
}

int Window::getStencilBits() const {
	return windowSettings.stencilBits;
}

void Window::setMajorVersion(int val) {
	windowSettings.majorVersion = val;
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, val);
}

int Window::getMajorVersion() const {
	return windowSettings.majorVersion;
}

void Window::setMinorVersion(int val) {
	windowSettings.minorVersion = val;
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, val);
}

int Window::getMinorVersion() const {
	return windowSettings.minorVersion;
}

void Window::setAntialiasingLevel(int val) {
	windowSettings.antialiasingLevel = val;
	glfwWindowHint(GLFW_SAMPLES, val);
}

int Window::getAntialiasingLevel() const {
	return windowSettings.antialiasingLevel;
}

void Window::setRefreshRate(int val) {
	windowSettings.refreshRate = val;
	glfwWindowHint(GLFW_REFRESH_RATE, val);
}

void Window::setTitle(const std::string& _title) {
	windowSettings.title = _title;
	glfwSetWindowTitle(window.get(), windowSettings.title.c_str());
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

void Window::updateWindow() {
	setFullscreen(windowSettings.isFullscreen);
	setTitle(windowSettings.title);
	setMajorVersion(windowSettings.majorVersion);
	setMinorVersion(windowSettings.minorVersion);
	setAntialiasingLevel(windowSettings.antialiasingLevel);
	setDepthBits(windowSettings.depthBits);
	setStencilBits(windowSettings.stencilBits);
}
#include <renderModule/backends/interface/driverInterface.h>
void Window::create() {
	glfwInit();
#ifdef OPENGL_BACKEND
	if (RENDER::DriverInterface::settings.backend == RENDER::RenderSettings::Backend::OPENGL) {
		setMajorVersion(windowSettings.majorVersion);
		setMinorVersion(windowSettings.minorVersion);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	}
#endif
#ifdef VULKAN_BACKEND
	if (RENDER::DriverInterface::settings.backend == RENDER::RenderSettings::Backend::VULKAN) {
		if (!glfwVulkanSupported()) {
			std::cerr << "GLFW: Vulkan not supported\n" << std::endl;
			throw;
		}
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

	}
#endif
	setAntialiasingLevel(windowSettings.antialiasingLevel);
	setDepthBits(windowSettings.depthBits);
	setStencilBits(windowSettings.stencilBits);

	window = std::unique_ptr<GLFWwindow, DestroyGLFW>(glfwCreateWindow(
		windowSettings.size.x, windowSettings.size.y, 
		windowSettings.title.c_str(), NULL, NULL));

#ifdef VULKAN_BACKEND
	//glfwGetFramebufferSize(window, &m_BufferWidth, &m_BufferHeight);
#endif

	glfwMakeContextCurrent(window.get());
	glfwSwapInterval(1); //vsync
	setRefreshRate(windowSettings.refreshRate);
	setCursorVisible(windowSettings.isCursorVisible, windowSettings.isCursorLock);

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

	setFullscreen(windowSettings.isFullscreen);

	//TODO mode to service
	INPUT::GamepadManager::Instance();

	glfwSetErrorCallback(glfwErrorCallback);
	glfwSetKeyCallback(window.get(), [](GLFWwindow* window, int key, int scancode, int action, int mods) {
		RESOURCES::ServiceManager::Get<WINDOW_SYSTEM::Window>().keyEvent.run(window, key, scancode, action, mods);
		RESOURCES::ServiceManager::Get<WINDOW_SYSTEM::Window>().keyCallback(window, key, scancode, action, mods);
	});
	glfwSetMouseButtonCallback(window.get(), [](GLFWwindow* window, int button, int action, int mods) {
		RESOURCES::ServiceManager::Get<WINDOW_SYSTEM::Window>().mouseButtonEvent.run(window, button, action, mods);
		RESOURCES::ServiceManager::Get<WINDOW_SYSTEM::Window>().mouseCallback(window, button, action, mods);
	});
	glfwSetWindowCloseCallback(window.get(), [](GLFWwindow* window) {
		RESOURCES::ServiceManager::Get<WINDOW_SYSTEM::Window>().isClose = true;
	});

	glfwSetFramebufferSizeCallback(window.get(), [](GLFWwindow* window, int width, int height) {
		auto& renderer = RESOURCES::ServiceManager::Get<RENDER::GameRendererInterface>();
#ifdef OPENGL_BACKEND
		auto _renderer = reinterpret_cast<RENDER::GameRendererGl*>(&renderer);
		_renderer->resize();
#endif
#ifdef VULKAN_BACKEND
		auto _renderer = reinterpret_cast<RENDER::GameRendererVk*>(&renderer);
		_renderer->resize();
#endif
		//TODO:

	});
}

void Window::pollEvent() {
	glfwPollEvents();
	INPUT::GamepadManager::Instance().update([this](const INPUT::Gamepad::GamepadData& data) {
		gamepadEvent.run(data);
	});
}

void Window::draw() const {
#ifdef OPENGL_BACKEND
	if (RENDER::DriverInterface::settings.backend == RENDER::RenderSettings::Backend::OPENGL) {
		if (!glfwWindowShouldClose(window.get())) {
			glfwSwapBuffers(window.get());
		}
	}
#endif
}

GLFWwindow& Window::getContext() const {
	return *window;
}

MATHGL::Vector2i Window::getMousePos() const {
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

MATHGL::Vector2i Window::getPosition() const {
	int x, y;
	glfwGetWindowPos(window.get(), &x, &y);
	return MATHGL::Vector2i(x, y);
}
#endif

#ifdef DX12_BACKEND


Window::~Window() {
	//glfwTerminate();
}


void Window::setSize(unsigned int w, unsigned int h) {
	windowSettings.size = { w, h };
	//glfwSetWindowSize(window.get(), w, h);
}

void Window::setPosition(int x, int y) {
	position = { x, y };
	//glfwSetWindowPos(window.get(), x, y);
}

void Window::hide() const {
	//glfwHideWindow(window.get());
}

void Window::show() const {
	//glfwShowWindow(window.get());
}

void Window::focus() const {
	//glfwFocusWindow(window.get());
}

bool Window::hasFocus() const {
	return true;//glfwGetWindowAttrib(window.get(), GLFW_FOCUSED);
}

void Window::setFullscreen(bool val) {
	windowSettings.isFullscreen = val;
	//glfwSetWindowMonitor(
	//	window.get(),
	//	val ? glfwGetPrimaryMonitor() : nullptr,
	//	position.x,
	//	position.y,
	//	windowSettings.size.x,
	//	windowSettings.size.y,
	//	windowSettings.refreshRate
	//);
}

void Window::setCursorVisible(bool isVisible, bool isLock) const {
	//auto val = GLFW_CURSOR_NORMAL;
	//if (!isVisible && isLock) {
	//	val = GLFW_CURSOR_DISABLED;
	//}
	//else if (!isVisible) {
	//	val = GLFW_CURSOR_HIDDEN;
	//}
	//glfwSetInputMode(window.get(), GLFW_CURSOR, val);
}


void Window::setDepthBits(int val) {
	windowSettings.depthBits = val;
	glfwWindowHint(GLFW_DEPTH_BITS, val);
}

int Window::getDepathBits() const {
	return windowSettings.depthBits;
}

void Window::setStencilBits(int val) {
	windowSettings.stencilBits = val;
	glfwWindowHint(GLFW_STENCIL_BITS, val);
}

int Window::getStencilBits() const {
	return windowSettings.stencilBits;
}

void Window::setMajorVersion(int val) {
	windowSettings.majorVersion = val;
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, val);
}

int Window::getMajorVersion() const {
	return windowSettings.majorVersion;
}

void Window::setMinorVersion(int val) {
	windowSettings.minorVersion = val;
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, val);
}

int Window::getMinorVersion() const {
	return windowSettings.minorVersion;
}

void Window::setAntialiasingLevel(int val) {
	windowSettings.antialiasingLevel = val;
	glfwWindowHint(GLFW_SAMPLES, val);
}

int Window::getAntialiasingLevel() const {
	return windowSettings.antialiasingLevel;
}

void Window::setRefreshRate(int val) {
	windowSettings.refreshRate = val;
	//glfwWindowHint(GLFW_REFRESH_RATE, val);
}


void Window::setTitle(const std::string& _title) {
	windowSettings.title = _title;
	//glfwSetWindowTitle(window.get(), windowSettings.title.c_str());
}

//void Window::keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
//	if (action == GLFW_PRESS) {
//		keyPressedEvent.run(key);
//	}
//	else if (action == GLFW_RELEASE) {
//		keyReleasedEvent.run(key);
//	}
//}
//
//void Window::mouseCallback(GLFWwindow* window, int button, int action, int mods) {
//	if (action == GLFW_PRESS) {
//		mouseButtonPressedEvent.run(button);
//	}
//	else if (action == GLFW_RELEASE) {
//		mouseButtonReleasedEvent.run(button);
//	}
//}

//void Window::DestroyGLFW::operator()(GLFWwindow* ptr) const {
//	//glfwDestroyWindow(ptr);
//}


void Window::updateWindow() {
	setFullscreen(windowSettings.isFullscreen);
	setTitle(windowSettings.title);
	setMajorVersion(windowSettings.majorVersion);
	setMinorVersion(windowSettings.minorVersion);
	setAntialiasingLevel(windowSettings.antialiasingLevel);
	setDepthBits(windowSettings.depthBits);
	setStencilBits(windowSettings.stencilBits);
}
#include "../render/backends/interface/driverInterface.h"
void Window::create() {
	
}

void Window::pollEvent() {
	//glfwPollEvents();
	INPUT::GamepadManager::Instance().update([this](const INPUT::Gamepad::GamepadData& data) {
		gamepadEvent.run(data);
	});
}

void Window::draw() const {

}

MATHGL::Vector2i Window::getMousePos() const {
	double x=0.0, y=0.0;
	//glfwGetCursorPos(window.get(), &x, &y);
	return MATHGL::Vector2i(static_cast<unsigned>(x), static_cast<unsigned>(y));
}
MATHGL::Vector2u Window::getSize() const {
	auto width = 0;
	auto height = 0;
	//glfwGetWindowSize(window.get(), &width, &height);
	return MATHGL::Vector2u(800, 600);
}

MATHGL::Vector2i Window::getPosition() const {
	int x=0, y=0;
	//glfwGetWindowPos(window.get(), &x, &y);
	return MATHGL::Vector2i(x, y);
}

#endif