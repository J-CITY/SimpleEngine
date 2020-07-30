#pragma once
#include <iostream>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Window/ContextSettings.hpp>
#include <GL/glew.h>
#include "../3rd/imgui/imgui-SFML.h"
#include <SFML/Graphics.hpp>
#include <SFML/System.hpp>

namespace SE {
	class Window {
	public:
		Window(size_t w, size_t h, std::string title = "") {
			height = h;
			width = w;
			window = std::make_unique<sf::RenderWindow>(sf::VideoMode(w, h, 32),
				title, sf::Style::Titlebar | sf::Style::Close);
			window->setFramerateLimit(60);
			window->setActive();


			glewExperimental = GL_TRUE;
			if (GLEW_OK != glewInit()) {
				//std::cout << "Error:: glew not init =(" << endl;
				exit(-1);
			}

			ImGui::SFML::Init(*window.get());
		}

		~Window() {
			ImGui::SFML::Shutdown();
			window->close();
		}

		size_t getHeight() const { return height; }
		size_t getWidth() const { return width; }

		void update() {
			sf::Event windowEvent;
			while (window->pollEvent(windowEvent)) {
				ImGui::SFML::ProcessEvent(windowEvent);
				switch (windowEvent.type) {
				case sf::Event::Closed:

					break;
				default:
					break;
				}
			}
		}

		void draw() {
			window->pushGLStates();
			ImGui::SFML::Render(*window.get());
			window->popGLStates();


			window->display();
		}
		
	private:
		std::unique_ptr<sf::RenderWindow> window;

		size_t height = 0;
		size_t width = 0;
	};

}
