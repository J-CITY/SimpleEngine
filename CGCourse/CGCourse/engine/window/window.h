#pragma once
#include <string>
#include <unordered_map>

#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics.hpp>
//#include <SFML/Graphics.hpp>
//#include <SFML/System.hpp>
#include "../utils/event.h"
#include "../utils/pointers/objPtr.h"

#include "../resourceManager/serializerInterface.h"

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

		std::pair<int, int> getMousePos() {
			auto p = sf::Mouse::getPosition(*window);
			return std::make_pair(p.x, p.y);
		}
		std::pair<int, int> getMousePosGlobal() {
			auto p = sf::Mouse::getPosition();
			return std::make_pair(p.x, p.y);
		}
		Window(const WindowSettings& p_windowSettings);
		~Window();
		
		void setSize(unsigned int width, unsigned int height);
		std::pair<unsigned, unsigned> getSize() const {
			auto sz = window->getSize();
			return std::make_pair(sz.x, sz.y);
		}
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

		void draw();

		std::unique_ptr<sf::RenderWindow>& getSFMLContext() {
			return window;
		}

		virtual void onSerialize(nlohmann::json& j) override;
		virtual void onDeserialize(nlohmann::json& j) override;
	private:
		void create();

	private:
		WindowSettings windowSettings;
		std::unique_ptr<sf::RenderWindow> window;
		std::string winId = "";		

		std::string title;
		std::pair<int, int> size = std::make_pair(800, 600);
		std::pair<int, int> position;
		bool isFullscreen;
	};
}
