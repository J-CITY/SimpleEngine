#pragma once
#include <string>

//need for right including dependencies
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "../utils/event.h"
#include "../utils/gamepad/gamepad.h"
#include "../resourceManager/serializerInterface.h"
#include "serdepp/include/serdepp/utility.hpp"

namespace KUMA
{
	namespace DEBUG
	{
		class DebugRender;
	}
}

namespace KUMA::CORE_SYSTEM {
	class Core;
}

namespace KUMA::WINDOW_SYSTEM {
	struct WindowSettings {
		bool isFullscreen = false;
		int depthBits = 24;
		int stencilBits = 8;
		int antialiasingLevel = 4;
		int majorVersion = 4;
		int minorVersion = 3;
		std::string appName;
		MATHGL::Vector2u size = MATHGL::Vector2u(800, 600);
		int refreshRate = 60;

		DERIVE_SERDE(WindowSettings,
			(&Self::isFullscreen, "isFullscreen")
			(&Self::depthBits, "depthBits")
			(&Self::stencilBits, "stencilBits")
			(&Self::antialiasingLevel, "antialiasingLevel")
			(&Self::majorVersion, "majorVersion")
			(&Self::minorVersion, "minorVersion")
			(&Self::appName, "appName")
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

		MATHGL::Vector2i getMousePos();
		void setSize(unsigned int width, unsigned int height);
		MATHGL::Vector2u getSize() const;
		void setPosition(int x, int y);
		
		void hide() const;
		void show() const;

		void focus() const;
		[[nodiscard]] bool hasFocus() const;

		void setTitle(const std::string& p_title);
		[[nodiscard]] std::string getTitle() const;

		void setFullscreen(bool val);
		void toggleFullscreen();
		[[nodiscard]] bool getIsFullscreen() const;
		void update();

		void pollEvent();
		void draw();

		[[nodiscard]] GLFWwindow& getContext() const;
		[[nodiscard]] bool isClosed() const;


	private:
		struct DestroyGLFW {
			void operator()(GLFWwindow* ptr) const;
		};

		[[nodiscard]] WindowSettings& getSetting();;
		void updateWindow();

		void create();
		void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
		void mouseCallback(GLFWwindow* window, int button, int action, int mods);

		WindowSettings windowSettings;
		std::unique_ptr<GLFWwindow, DestroyGLFW> window;
		std::string winId;
		
		MATHGL::Vector2u position;
		bool isClose = false;
	};
}
