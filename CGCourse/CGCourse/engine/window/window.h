#pragma once
#include <string>

//need for right including dependencies
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "../utils/event.h"
#include "../utils/gamepad/gamepad.h"
#include "serdepp/include/serdepp/utility.hpp"

import glmath;

namespace KUMA::DEBUG {
	class DebugRender;
}

namespace KUMA::CORE_SYSTEM {
	class Core;
}

namespace KUMA::WINDOW_SYSTEM {
	struct WindowSettings {
		bool isFullscreen = false;
		bool isCursorVisible = true;
		bool isCursorLock = true;
		int depthBits = 24;
		int stencilBits = 8;
		int antialiasingLevel = 4;
		int majorVersion = 4;
		int minorVersion = 3;
		std::string title;
		MATHGL::Vector2u size = MATHGL::Vector2u(800, 600);
		int refreshRate = 60;

		DERIVE_SERDE(WindowSettings,
			(&Self::isFullscreen, "isFullscreen")
			(&Self::isFullscreen, "isCursorVisible")
			(&Self::isFullscreen, "isCursorLock")
			(&Self::depthBits, "depthBits")
			(&Self::stencilBits, "stencilBits")
			(&Self::antialiasingLevel, "antialiasingLevel")
			(&Self::majorVersion, "majorVersion")
			(&Self::minorVersion, "minorVersion")
			(&Self::title, "title")
			(&Self::size, "size")
			(&Self::refreshRate, "refreshRate")
		)
	};

	class Window {
		friend DEBUG::DebugRender;
	public:
		EVENT::Event<int> keyPressedEvent;
		EVENT::Event<int> keyReleasedEvent;
		EVENT::Event<int> mouseButtonPressedEvent;
		EVENT::Event<int> mouseButtonReleasedEvent;
		EVENT::Event<INPUT::Gamepad::GamepadData> gamepadEvent;
		
		explicit Window(const WindowSettings& p_windowSettings);
		Window() = delete;
		~Window();

		MATHGL::Vector2i getMousePos() const;
		void setSize(unsigned int width, unsigned int height);
		MATHGL::Vector2u getSize() const;
		void setPosition(int x, int y);
		MATHGL::Vector2i getPosition() const;
		void setTitle(const std::string& title);
		[[nodiscard]] std::string getTitle() const;
		void setDepthBits(int val);
		[[nodiscard]] int getDepathBits() const;
		void setStencilBits(int val);
		[[nodiscard]] int getStencilBits() const;
		void setMajorVersion(int val);
		[[nodiscard]] int getMajorVersion() const;
		void setMinorVersion(int val);
		[[nodiscard]] int getMinorVersion() const;
		void setAntialiasingLevel(int val);
		[[nodiscard]] int getAntialiasingLevel() const;
		void setRefreshRate(int val);
		[[nodiscard]] int getRefreshRate() const;
		void setFullscreen(bool val);
		[[nodiscard]] bool getIsFullscreen() const;
		void toggleFullscreen();

		void hide() const;
		void show() const;
		void focus() const;
		[[nodiscard]] bool hasFocus() const;
		void pollEvent();
		void draw() const;
#if defined(VULKAN_BACKEND) || defined(OPENGL_BACKEND)
		EVENT::Event<GLFWwindow*, int, int, int, int> keyEvent;
		EVENT::Event<GLFWwindow*, int, int, int> mouseButtonEvent;
		[[nodiscard]] GLFWwindow& getContext() const;
#endif
		[[nodiscard]] bool isClosed() const;

		void setCursorVisible(bool isVisible, bool isLock) const;

#ifdef VULKAN_BACKEND
		GLFWwindow* getGLFWWin() {
			return window.get();
		}
#endif

	private:
		

		[[nodiscard]] WindowSettings& getSetting();;
		void updateWindow();

		void create();
		WindowSettings windowSettings;
#if defined(VULKAN_BACKEND) || defined(OPENGL_BACKEND)
		struct DestroyGLFW {
			void operator()(GLFWwindow* ptr) const;
		};
		void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
		void mouseCallback(GLFWwindow* window, int button, int action, int mods);

		std::unique_ptr<GLFWwindow, DestroyGLFW> window;
#endif
		MATHGL::Vector2i position{};
		bool isClose = false;
	};
}
