#pragma once
#include <string>

//need for right including dependencies
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "../utils/event.h"
#include "../utils/gamepad/Gamepad.h"
#include "../resourceManager/serializerInterface.h"
#include "json_reflect.hpp"

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
	};
	REFLECT_NON_INTRUSIVE(WindowSettings, isFullscreen, depthBits, stencilBits, antialiasingLevel, majorVersion, minorVersion, appName);

	class Window : public KUMA::RESOURCES::Serializable {
	public:
		EVENT::Event<int> keyPressedEvent;
		EVENT::Event<int> keyReleasedEvent;
		EVENT::Event<int> mouseButtonPressedEvent;
		EVENT::Event<int> mouseButtonReleasedEvent;
		EVENT::Event<INPUT::Gamepad::GamepadData> gamepadEvent;

		MATHGL::Vector2i getMousePos();

		explicit Window(const WindowSettings& p_windowSettings);
		Window();
		~Window();
		
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

		void drawDebug(CORE_SYSTEM::Core& core);
		void draw();

		[[nodiscard]] GLFWwindow& getContext() const;

		virtual void onSerialize(nlohmann::json& j) override;
		virtual void onDeserialize(nlohmann::json& j) override;

		bool isClosed() const;
	private:
		struct DestroyglfwWin {
			void operator()(GLFWwindow* ptr) const;
		};

		void create();
		void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
		void mouseCallback(GLFWwindow* window, int button, int action, int mods);

		WindowSettings windowSettings;
		std::unique_ptr<GLFWwindow, DestroyglfwWin> window;
		std::string winId;
		std::string title;
		std::pair<int, int> size = std::make_pair(800, 600);
		std::pair<int, int> position;
		bool isFullscreen;
		int refreshRate = 60;
		bool m_isClosed = false;
	};
}
