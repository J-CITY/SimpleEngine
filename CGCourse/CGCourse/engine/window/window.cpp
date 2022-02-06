#include "window.h"

#include <fstream>
#include <SFML/Window/Event.hpp>
#include "../utils/debug/logger.h"

#include "../config.h"

using namespace KUMA::WINDOW_SYSTEM;

Window::Window(const WindowSettings& p_windowSettings): windowSettings(p_windowSettings) {
	nlohmann::json j;
	std::ifstream i(Config::ENGINE_ASSETS_PATH + "Configs\\" + "app.json");
	i >> j;
	onDeserialize(j);
	create();
}

Window::~Window() {
	window->close();
}

void Window::setSize(unsigned int w, unsigned int h) {
	window->setSize(sf::Vector2u(w, h));
}

void Window::setPosition(int x, int y) {
	window->setPosition(sf::Vector2i(x, y));
}

void Window::hide() const {
	window->setVisible(false);
}

void Window::show() const {
	window->setVisible(true);
}

void Window::focus() const {
	window->requestFocus();
}

bool Window::hasFocus() const {
	return window->hasFocus();
}

void Window::setFullscreen(bool val) {
	isFullscreen = val;
	if (isFullscreen) {
		window->create(sf::VideoMode(window->getSize().x, window->getSize().y, 32),
			title, sf::Style::Fullscreen);
	}
	else {
		window->create(sf::VideoMode(window->getSize().x, window->getSize().y, 32),
			title, sf::Style::Default);
	}

}

void Window::toggleFullscreen() {
	setFullscreen(!isFullscreen);
}

bool Window::getIsFullscreen() const {
	return isFullscreen;
}

void Window::setTitle(const std::string& _title) {
	title = _title;
	window->setTitle(title);
}

std::string Window::getTitle() const {
	return title;
}

void Window::create() {
	window = std::make_unique<sf::RenderWindow>();

	auto settings = window->getSettings();
	settings.depthBits = windowSettings.depthBits;
	settings.stencilBits = windowSettings.stencilBits;
	settings.antialiasingLevel = windowSettings.antialiasingLevel;
	settings.majorVersion = windowSettings.majorVersion;
	settings.minorVersion = windowSettings.minorVersion;
	//settings.attributeFlags = sf::ContextSettings::Core;

	auto mask = sf::Style::Titlebar | sf::Style::Close;
	if (windowSettings.isFullscreen) {
		mask |= sf::Style::Fullscreen;
	}

	window->create(sf::VideoMode(size.first, size.second, 32),
		sf::String(title.c_str()), mask, settings);
	window->setFramerateLimit(60);
	window->setActive();
	

	if (!window) {
		throw std::runtime_error("Failed to create GLFW window");
	}
	else {

		auto [x, y] = window->getPosition();
		position.first = x;
		position.second = y;
	}
}

void Window::update() {
	sf::Event event;
	window->pollEvent(event);
	if (event.type == sf::Event::Closed) {
		LOG_INFO("Window close: " + title);
		isClosed = true;
	}
	if (event.type == sf::Event::KeyPressed) {
		keyPressedEvent.run(event.key.code);
	}
	if (event.type == sf::Event::KeyReleased) {
		keyReleasedEvent.run(event.key.code);
	}
	if (event.type == sf::Event::MouseButtonPressed) {
		mouseButtonPressedEvent.run(event.mouseButton.button);
	}
	if (event.type == sf::Event::MouseButtonReleased) {
		mouseButtonReleasedEvent.run(event.mouseButton.button);
	}
}

void Window::draw() {
	window->display();
}

void Window::onSerialize(nlohmann::json& j) {
	j["window"]["depthBits"] = windowSettings.depthBits;
	j["window"]["stencilBits"] = windowSettings.stencilBits;
	j["window"]["antialiasingLevel"] = windowSettings.antialiasingLevel;
	j["window"]["majorVersion"] = windowSettings.majorVersion;
	j["window"]["minorVersion"] = windowSettings.minorVersion;
	j["window"]["isFillscreen"] = windowSettings.isFullscreen;
	j["window"]["appName"] = windowSettings.appName;
}

void Window::onDeserialize(nlohmann::json& j) {
	windowSettings.depthBits = j["window"]["depthBits"];
	windowSettings.stencilBits = j["window"]["stencilBits"];
	windowSettings.antialiasingLevel = j["window"]["antialiasingLevel"];
	windowSettings.majorVersion = j["window"]["majorVersion"];
	windowSettings.minorVersion = j["window"]["minorVersion"];
	windowSettings.isFullscreen = j["window"]["isFillscreen"];
	windowSettings.appName = j["window"]["appName"];
}
