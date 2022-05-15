#include "window.h"

#include <fstream>
#include <SFML/Window/Event.hpp>
#include "../utils/debug/logger.h"

#include "../config.h"
#include "imgui/imgui-SFML.h"
#include "imgui/imgui.h"

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
	ImGui::SFML::Init(*window);

	if (!window) {
		throw std::runtime_error("Failed to create GLFW window");
	}
	else {

		auto [x, y] = window->getPosition();
		position.first = x;
		position.second = y;
	}

	GamepadMgr::Instance().Initialize();
}

struct GamepadData {
	int id = 0;
	std::list<Gamepad::GAMEPAD_BUTTON> pressedButtons;
	float leftSticX = 0.0f;
	float leftSticY = 0.0f;
	float rightSticX = 0.0f;
	float rightSticY = 0.0f;
	float leftTrigger = 0.0f;
	float rightTrigger = 0.0f;
};

void Window::update() {
	sf::Event event;
	while (window->pollEvent(event)) {
		ImGui::SFML::ProcessEvent(event);
		if (event.type == sf::Event::Closed) {
			LOG_INFO("Window close: " + title);
			isClosed = true;
		}
		else if (event.type == sf::Event::KeyPressed) {
			keyPressedEvent.run(event.key.code);
		}
		else if (event.type == sf::Event::KeyReleased) {
			keyReleasedEvent.run(event.key.code);
		}
		else if (event.type == sf::Event::MouseButtonPressed) {
			mouseButtonPressedEvent.run(event.mouseButton.button);
		}
		else if (event.type == sf::Event::MouseButtonReleased) {
			mouseButtonReleasedEvent.run(event.mouseButton.button);
		}
	}
	auto g0 = GamepadMgr::Instance().GamepadOne();
	auto g1 = GamepadMgr::Instance().GamepadTwo();
	std::array btns = {Gamepad::btn_a, Gamepad::btn_b, Gamepad::btn_x, Gamepad::btn_y,
		Gamepad::btn_leftStick, Gamepad::btn_rightStick, Gamepad::btn_back, Gamepad::btn_start,
		Gamepad::btn_lb, Gamepad::btn_rb, Gamepad::dpad_up, Gamepad::dpad_down, Gamepad::dpad_left, Gamepad::dpad_right};
	if (g0) {
		GamepadData gd;
		gd.id = 0;
		gd.leftSticX    = g0->getAxisPosition(Gamepad::GAMEPAD_AXIS::leftStick_X);
		gd.leftSticY    = g0->getAxisPosition(Gamepad::GAMEPAD_AXIS::leftStick_Y);
		gd.rightSticX   = g0->getAxisPosition(Gamepad::GAMEPAD_AXIS::rightStick_X);
		gd.rightSticY   = g0->getAxisPosition(Gamepad::GAMEPAD_AXIS::rightStick_X);
		gd.leftTrigger  = g0->getTriggerValue(Gamepad::GAMEPAD_TRIGGER::leftTrigger);
		gd.rightTrigger = g0->getTriggerValue(Gamepad::GAMEPAD_TRIGGER::leftTrigger);
		for (auto& btn : btns) {
			if (g0->isButtonPressed(btn)) {
				gd.pressedButtons.push_back(btn);
			}
		}
		gamepadEvent.run(gd);
	}
	if (g1) {
		GamepadData gd;
		gd.id = 1;
		gd.leftSticX =    g1->getAxisPosition(Gamepad::GAMEPAD_AXIS::leftStick_X);
		gd.leftSticY =    g1->getAxisPosition(Gamepad::GAMEPAD_AXIS::leftStick_Y);
		gd.rightSticX =   g1->getAxisPosition(Gamepad::GAMEPAD_AXIS::rightStick_X);
		gd.rightSticY =   g1->getAxisPosition(Gamepad::GAMEPAD_AXIS::rightStick_X);
		gd.leftTrigger =  g1->getTriggerValue(Gamepad::GAMEPAD_TRIGGER::leftTrigger);
		gd.rightTrigger = g1->getTriggerValue(Gamepad::GAMEPAD_TRIGGER::leftTrigger);
		for (auto& btn : btns) {
			if (g1->isButtonPressed(btn)) {
				gd.pressedButtons.push_back(btn);
			}
		}
		gamepadEvent.run(gd);
	}
}

sf::Clock deltaClock;
void Window::draw() {


	window->pushGLStates();
	{//Debug
		ImGui::SFML::Update(*window, deltaClock.restart());

		ImGui::Begin("Sample window"); // создаём окно

		// Инструмент выбора цвета
		//if (ImGui::ColorEdit3("Background color", color)) {
		//	// код вызывается при изменении значения, поэтому всё
		//	// обновляется автоматически
		//	bgColor.r = static_cast<sf::Uint8>(color[0] * 255.f);
		//	bgColor.g = static_cast<sf::Uint8>(color[1] * 255.f);
		//	bgColor.b = static_cast<sf::Uint8>(color[2] * 255.f);
		//}

		static float lpx = 0.0f, lpy = 0.0f, lpz = 5.0f;
		ImGui::InputFloat("LightX", &lpx, 1.0f, 1.0f, "%.2f");
		ImGui::InputFloat("LightY", &lpy, 1.0f, 1.0f, "%.2f");
		ImGui::InputFloat("LightZ", &lpz, 1.0f, 1.0f, "%.2f");

		//ImGui::InputFloat("Shin", &materialShin, 2, 1.0f, "%.1f");
		//ImGui::InputFloat("Scale", &scale, 0.01f, 1.0f, "%.1f");
		//lightPos = glm::vec3(lpx, lpy, lpz);
		//char s[100] = "ImGui + SFML = <3";
		//ImGui::InputText("Window title", s, 255);
		//
		//if (ImGui::Button("Update window title")) {
		//	s[99] = '\0';
		//	window.setTitle("OKNO");
		//}
		ImGui::End(); // end window
	}
	ImGui::SFML::Render(*window);
	window->popGLStates();

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
