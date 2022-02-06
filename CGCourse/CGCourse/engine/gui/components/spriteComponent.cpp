#include "spriteComponent.h"
#include "../guiObject.h"

using namespace KUMA;
using namespace KUMA::GUI;

void ComponentGui::_calculateOwnPos(float w, float h) {

	if (pos.haligh == EAlign::LEFT) {
		posGlobal.pos.x = obj.transform->posGlobal.x + pos.pos.x;
	}
	else if (pos.haligh == EAlign::CENTER) {
		posGlobal.pos.x = obj.transform->posGlobal.x + obj.getWidht() / 2.0f + pos.pos.x - w / 2.0f;
	}
	else if (pos.haligh == EAlign::RIGHT) {
		posGlobal.pos.x = obj.transform->posGlobal.x + obj.getWidht() + pos.pos.x - w;
	}
	if (pos.valigh == EAlign::TOP) {
		posGlobal.pos.y = obj.transform->posGlobal.y + pos.pos.y;
	}
	else if (pos.valigh == EAlign::CENTER) {
		posGlobal.pos.y = obj.transform->posGlobal.y + obj.getHeight() / 2.0f + pos.pos.y - h / 2.0f;
	}
	else if (pos.valigh == EAlign::BOTTOM) {
		posGlobal.pos.y = obj.transform->posGlobal.y + obj.getHeight() + pos.pos.y - h;
	}

	posGlobal.scale.x = obj.transform->scaleGlobal.x * pos.scale.x;
	posGlobal.scale.y = obj.transform->scaleGlobal.y * pos.scale.y;
	posGlobal.rotation = obj.transform->rotateGlobal + pos.rotation;
}

TransformComponentGui::TransformComponentGui(GuiObject& obj) : ComponentGui(obj) {

}

SpriteComponentGui::SpriteComponentGui(GuiObject& obj, std::string path) : ComponentGui(obj) {
	this->path = path;
	if (texture.loadFromFile(KUMA::Config::ROOT + Config::USER_ASSETS_PATH + path)) {
		sprite.setTexture(texture);
		return;
	}
	//err
}

void SpriteComponentGui::draw(KUMA::WINDOW_SYSTEM::Window& win) {
	if (isEnabled) {
		win.getSFMLContext()->draw(sprite);
	}
}


InteractionComponentGui::InteractionComponentGui(GuiObject& obj, sf::FloatRect rect) : ComponentGui(obj) {
	this->rect = rect;
}
bool InteractionComponentGui::contains(float x, float y) const {
	float minX = std::min(rect.left, (rect.left + rect.width));
	float maxX = std::max(rect.left, (rect.left + rect.width));
	float minY = std::min(rect.top, (rect.top + rect.height));
	float maxY = std::max(rect.top, (rect.top + rect.height));

	return (x >= minX) && (x < maxX) && (y >= minY) && (y < maxY);
}
void InteractionComponentGui::onUpdate(float dt) {
	auto mpos = RESOURCES::ServiceManager::Get<INPUT_SYSTEM::InputManager>().getMousePosition();
	if (contains(mpos.first, mpos.second)) {
		if (RESOURCES::ServiceManager::Get<INPUT_SYSTEM::InputManager>().isMouseButtonPressed(INPUT_SYSTEM::EMouseButton::MOUSE_BUTTON_1)) {
			obj.onEvent(GuiEventType::PRESS);
		}
		else if (RESOURCES::ServiceManager::Get<INPUT_SYSTEM::InputManager>().isMouseButtonReleased(INPUT_SYSTEM::EMouseButton::MOUSE_BUTTON_1)) {
			obj.onEvent(GuiEventType::RELEASE);
		}
		else {
			obj.onEvent(GuiEventType::HOVER);
		}
	}
	else {
		obj.onEvent(GuiEventType::UNHOVER);
	}
}

LabelComponentGui::LabelComponentGui(GuiObject& obj, std::string label) : ComponentGui(obj) {
	this->label = label;

	if (!font.loadFromFile(KUMA::Config::ROOT + Config::USER_ASSETS_PATH + "fonts\\arcade.TTF")) {
		// error
	}
	text.setFont(font);
	text.setString(label);
	text.setCharacterSize(24);
	text.setFillColor(sf::Color::Red);
	text.setStyle(sf::Text::Bold | sf::Text::Underlined);
}

void LabelComponentGui::draw(KUMA::WINDOW_SYSTEM::Window& win) {
	if (isEnabled) {
		win.getSFMLContext()->draw(text);
	}
}

void ClipComponentGui::draw(KUMA::WINDOW_SYSTEM::Window& win) {
	
}
void ClipComponentGui::calculateOwnPos() {
	sf::FloatRect rect(obj.transform->posGlobal.x, obj.transform->posGlobal.y, obj.getWidht(), obj.getHeight());
	_calculateOwnPos(rect.width, rect.height);
	rect.left = posGlobal.pos.x;
	rect.top = posGlobal.pos.y;
	rect.width = rect.width * posGlobal.scale.x;
	rect.height = rect.height * posGlobal.scale.y;
	view = sf::View(rect);

	auto windowSize = RESOURCES::ServiceManager::Get<WINDOW_SYSTEM::Window>().getSize();
	view.setViewport(sf::FloatRect(rect.left / (float)windowSize.first, rect.top / (float)windowSize.second, 
		rect.width / (float)windowSize.first, rect.height / (float)windowSize.second));
}