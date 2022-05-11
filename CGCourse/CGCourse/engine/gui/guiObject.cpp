#include "guiObject.h"
#include "../core/core.h"

using namespace KUMA;
using namespace KUMA::GUI;

GuiButton::GuiButton(): GuiObject() {
	sprite = std::make_shared<SpriteComponentGui>(*this, "textures\\gui\\btn.png");
	label = std::make_shared<LabelComponentGui>(*this, "TEST");
	interaction = std::make_shared<InteractionComponentGui>(*this, sf::FloatRect());

	transform->pos = {100.0f, 100.0f};
	transform->scale = {1.0f, 1.0f};

	interaction->rect = sf::FloatRect(transform->pos.x, transform->pos.y,
		sprite->sprite.getLocalBounds().width, sprite->sprite.getLocalBounds().height);

	sprite->sprite.setPosition(transform->pos.x, transform->pos.y);

	label->text.setPosition(transform->pos.x + 10.0f, transform->pos.y + 10.0f);

	onClick = []() {
		auto a = 10;
	};
}
//#include <iostream>
void GuiButton::onEvent(GuiEventType etype) {
	//std::cout << (int)etype << std::endl;
	if (cur == etype) {
		return;
	}
	if (etype == GuiEventType::PRESS) {
		sprite->sprite.setColor(sf::Color::Red);
	}
	else if (etype == GuiEventType::RELEASE) {
		sprite->sprite.setColor(sf::Color::Cyan);
	}
	else if (etype == GuiEventType::HOVER) {
		sprite->sprite.setColor(sf::Color::Yellow);
	}
	else if (etype == GuiEventType::UNHOVER) {
		sprite->sprite.setColor(sf::Color::White);
	}
	cur = etype;
}

void GuiButton::onUpdate(KUMA::CORE_SYSTEM::Core& context, float dt) {
	interaction->onUpdate(dt);
	sprite->draw(*context.window);
	label->draw(*context.window);
}


void GuiClip::onUpdate(KUMA::CORE_SYSTEM::Core& context, float dt) {
	context.window->getSFMLContext()->setView(clipComponent->view);
	for (auto& ch : childs) {
		ch->onUpdate(context, dt);
	}
	context.window->getSFMLContext()->setView(context.window->getSFMLContext()->getDefaultView());
}


GuiImage::GuiImage() : GuiObject() {
	sprite = std::make_shared<SpriteComponentGui>(*this, "textures\\gui\\btn.png");
	transform->pos = {0.0f, 0.0f};
	transform->scale = {1.0f, 1.0f};
	sprite->sprite.setPosition(transform->pos.x, transform->pos.y);
}

void GuiImage::onUpdate(KUMA::CORE_SYSTEM::Core& context, float dt) {
	sprite->draw(*context.window);
}


GuiLabel::GuiLabel() : GuiObject() {
	label = std::make_shared<LabelComponentGui>(*this, "TEST");
	transform->pos = {100.0f, 100.0f};
	transform->scale = {1.0f, 1.0f};
	label->text.setPosition(transform->pos.x + 10.0f, transform->pos.y + 10.0f);
}

void GuiLabel::onUpdate(KUMA::CORE_SYSTEM::Core& context, float dt) {
	label->draw(*context.window);
}


GuiScroll::GuiScroll(float w, float h) : GuiObject() {
	interaction = std::make_shared<InteractionComponentGui>(*this, sf::FloatRect());
	scroll = std::make_shared<ScrollComponentGui>(*this, w, h);

	//transform->pos = {100.0f, 100.0f};
	transform->scale = {1.0f, 1.0f};

	interaction->rect = sf::FloatRect(transform->posGlobal.x, transform->posGlobal.y, w, h);

}

bool GuiScroll::contains(sf::FloatRect rect, float x, float y) const {
	float minX = std::min(rect.left, (rect.left + rect.width));
	float maxX = std::max(rect.left, (rect.left + rect.width));
	float minY = std::min(rect.top, (rect.top + rect.height));
	float maxY = std::max(rect.top, (rect.top + rect.height));

	return (x >= minX) && (x < maxX) && (y >= minY) && (y < maxY);
}

void GuiScroll::onEvent(GuiEventType etype) {
	if (etype == GuiEventType::PRESS) {
		auto mpos = RESOURCES::ServiceManager::Get<INPUT_SYSTEM::InputManager>().getMousePosition();
		if (!scroll->isPress) {
			for (auto& ch : childs) {
				if (contains(sf::FloatRect(ch->transform->posGlobal.x, ch->transform->posGlobal.y, ch->getWidht(), ch->getHeight()), mpos.x, mpos.y)) {
					selectedObj = ch;
					selectedObjPos = ch->transform->pos;
					break;
				}
			}
			if (selectedObj) {
				scroll->startX = mpos.x;
				scroll->startY = mpos.y;

			}
			scroll->isPress = true;
		}

		if (selectedObj) {
			auto shiftX = scroll->isScrollHorizontal ? mpos.x - scroll->startX : 0.0f;
			auto shiftY = scroll->isScrollVertical ? mpos.y- scroll->startY : 0.0f;
			selectedObj->setPos(selectedObjPos.x + shiftX, selectedObjPos.y + shiftY);

			bool check = false;
			if (selectedObj->transform->posGlobal.x < transform->posGlobal.x) {
				check = true;
			}
			if (selectedObj->transform->posGlobal.x + selectedObj->getWidht() > transform->posGlobal.x + getWidht()) {
				check = true;
			}
			if (selectedObj->transform->posGlobal.y < transform->posGlobal.y) {
				check = true;
			}
			if (selectedObj->transform->posGlobal.y + selectedObj->getHeight() > transform->posGlobal.y + getHeight()) {
				check = true;
			}
			if (check) {
				selectedObj->setPos(selectedObjPos.x + prevShiftX, selectedObjPos.y + prevShiftY);
			}
			else {
				prevShiftX = shiftX;
				prevShiftY = shiftY;
			}
		}
	}
	else if (etype == GuiEventType::RELEASE || etype == GuiEventType::UNHOVER) {
		scroll->isPress = false;
		scroll->startX = 0.0f;
		scroll->startY = 0.0f;

		selectedObj = nullptr;
	}
}

void GuiScroll::onUpdate(KUMA::CORE_SYSTEM::Core& context, float dt) {
	interaction->onUpdate(dt);
	GuiObject::onUpdate(context, dt);
}

