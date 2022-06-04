#pragma once
#include <string>
#include <unordered_map>

//#include <SFML/Graphics/RenderWindow.hpp>
//#include <SFML/Graphics.hpp>
//#include <SFML/Graphics.hpp>
//#include <SFML/System.hpp>


#include "../utils/event.h"
#include "../utils/pointers/objPtr.h"

#include "../utils/gamepad/GamepadMgr.h"
#include "../resourceManager/serializerInterface.h"
#include <GL/glew.h>
#include <GLFW/glfw3.h>
namespace KUMA
{
	namespace CORE_SYSTEM
	{
		class Core;
	}
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

	

	class Window : public KUMA::RESOURCES::Serializable {
	public:
		

		EVENT::Event<int> keyPressedEvent;
		EVENT::Event<int> keyReleasedEvent;
		EVENT::Event<int> mouseButtonPressedEvent;
		EVENT::Event<int> mouseButtonReleasedEvent;
		EVENT::Event<INPUT::Gamepad::GamepadData> gamepadEvent;

		MATHGL::Vector2i getMousePos();
		
		Window(const WindowSettings& p_windowSettings);
		~Window();
		
		void setSize(unsigned int width, unsigned int height);
		MATHGL::Vector2u getSize() const;
		void setPosition(int x, int y);
		
		void restore() const;
		void hide() const;
		void show() const;

		void focus() const;
		bool hasFocus() const;

		void setTitle(const std::string& p_title);
		std::string getTitle() const;

		void setFullscreen(bool val);
		void toggleFullscreen();
		bool getIsFullscreen() const;
		void update();

		bool isClosed = false;

		void drawDebug(CORE_SYSTEM::Core& core);
		void draw();

		GLFWwindow& getContext() {
			return *window;
		}

		virtual void onSerialize(nlohmann::json& j) override;
		virtual void onDeserialize(nlohmann::json& j) override;
	private:
		void create();
		void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
		void mouseCallback(GLFWwindow* window, int button, int action, int mods);

		WindowSettings windowSettings;

		struct DestroyglfwWin {
			void operator()(GLFWwindow* ptr) {
				glfwDestroyWindow(ptr);
			}
		};
		std::unique_ptr<GLFWwindow, DestroyglfwWin> window;
		std::string winId = "";		

		std::string title;
		std::pair<int, int> size = std::make_pair(800, 600);
		std::pair<int, int> position;
		bool isFullscreen;
		int refreshRate = 60;
	};
}
