#define NOMINMAX
#include "spriteComponent.h"
#include "../guiObject.h"
#include "../../render/gameRenderer.h"
#include "../../resourceManager/shaderManager.h"
#include "../../core/core.h"
#include "../../ecs/object.h"
#ifdef OPENGL_BACKEND
#include "../../render/backends/gl/textureGl.h"
#endif
#include "../../../utils/loader.h"

using namespace KUMA;
using namespace KUMA::ECS;

SpriteComponent::SpriteComponent(Object& obj, const std::string& path): Component(obj), mPath(path) {
#ifdef OPENGL_BACKEND
	mTexture = RENDER::TextureGl::create(KUMA::UTILS::getRealPath(path.c_str()));
	obj.getTransform()->getTransform().setLocalSize(
		{ static_cast<RENDER::TextureGl*>(mTexture.get())->width, static_cast<RENDER::TextureGl*>(mTexture.get())->height });
#endif
}

LabelComponent::LabelComponent(Object& obj, std::string label, std::shared_ptr<GUI::Font> font) :
	Component(obj), font(font) {
	mLabel = label;

	float width = 0;
	float height = 0;
	for (auto c : label) {
		GUI::Character ch = font->Characters[c];
		width += (ch.Advance >> 6);
		height = std::max(height, static_cast<float>(ch.Size.y));
	}
	obj.getTransform()->getTransform().setLocalSize({ width, height });

	//TODO: uncoment it
	//auto shader = material->getShader();
	////shader = RESOURCES::ShaderLoader::CreateFromFile("Shaders/gui/text.glsl");
	//MATHGL::Matrix4 projection = MATHGL::Matrix4::CreateOrthographic(0.0f, static_cast<float>(800), static_cast<float>(600), 0.0f, 0, 1);
	//shader->bind();
	//shader->setUniformInt("u_engine_text", 0);
	//shader->setUniformMat4("u_engine_projection", projection);


	//glGenVertexArrays(1, &VAO);
	//glGenBuffers(1, &VBO);
	//glBindVertexArray(VAO);
	//glBindBuffer(GL_ARRAY_BUFFER, VBO);
	//glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
	//glEnableVertexAttribArray(0);
	//glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
	//glBindBuffer(GL_ARRAY_BUFFER, 0);
	//glBindVertexArray(0);
}

//void LabelComponentGui::draw() {
//	if (!isEnabled) {
//		return;
//	}
//	glEnable(GL_CULL_FACE);
//	glEnable(GL_BLEND);
//	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
//	//TODO: uncoment it
//	//auto shader = material->getShader();
//	//shader->bind();
//	//shader->setUniformVec3("textColor", {color.x, color.y, color.z});
//
//	glBindVertexArray(VAO);
//
//	glBindTexture(GL_TEXTURE_2D, font->texture);
//	//TODO: uncoment it
//	std::string::const_iterator c;
//	//shader->setUniformMat4("u_engine_model", obj.transform->globalModel);
//
//	float x = 0;//model(0, 3);
//	float y = 0;//model(1, 3);
//	for (auto c : label) {
//		Character ch = font->Characters[c];
//
//		float xpos = x + ch.Bearing.x * obj.transform->globalScale.x;
//		float ypos = y - (ch.Size.y - ch.Bearing.y) * obj.transform->globalScale.y;
//
//		float w = ch.Size.x * obj.transform->globalScale.x;
//		float h = ch.Size.y * obj.transform->globalScale.y;
//
//
//		// Обновляем VBO для каждого символа
//		float vertices[6][4] = {
//			{ xpos,     ypos + h, ch.Start.x / 1024.0,               (ch.Start.y + ch.Size.y) / 1024.0 },
//			{ xpos + w, ypos,     (ch.Start.x + ch.Size.x) / 1024.0,  ch.Start.y / 1024.0},
//			{ xpos,     ypos,     ch.Start.x / 1024.0,               ch.Start.y / 1024.0 },
//			{ xpos,     ypos + h, ch.Start.x / 1024.0,               (ch.Start.y + ch.Size.y) / 1024.0  },
//			{ xpos + w, ypos + h, (ch.Start.x + ch.Size.x) / 1024.0, (ch.Start.y + ch.Size.y) / 1024.0 },
//			{ xpos + w, ypos,     (ch.Start.x + ch.Size.x) / 1024.0, ch.Start.y / 1024.0 }
//		};
//
//		// Визуализируем текстуру глифа поверх прямоугольника
//		//glBindTexture(GL_TEXTURE_2D, ch.textureID);
//
//		// Обновляем содержимое памяти VBO
//		glBindBuffer(GL_ARRAY_BUFFER, VBO);
//		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
//		glBindBuffer(GL_ARRAY_BUFFER, 0);
//
//		// Рендерим прямоугольник
//		glDrawArrays(GL_TRIANGLES, 0, 6);
//
//		// Теперь производим смещение для отображения следующего глифа (обратите внимание, что данное смещение измеряется в единицах, составляющих 1/64 пикселя)
//		x += (ch.Advance >> 6) * obj.transform->globalScale.x; // побитовый сдвиг на 6, чтобы получить значение в пикселях (2^6 = 64)
//	}
//	glBindVertexArray(0);
//	glBindTexture(GL_TEXTURE_2D, 0);
//}

InteractionComponent::InteractionComponent(Object& obj, float w, float h) : Component(obj), mWidth(w), mHeight(h) {
	//obj.transform->size = size;
	obj.getTransform()->getTransform().setLocalSize({ w, h });
}
bool InteractionComponent::contains(float x, float y) {
	auto left = obj->getTransform()->getWorldPosition().x;
	auto top = obj->getTransform()->getWorldPosition().y;
	auto width = obj->getTransform()->getTransform().getLocalSize().x * obj->getTransform()->getWorldScale().x;
	auto height = obj->getTransform()->getTransform().getLocalSize().y * obj->getTransform()->getWorldScale().y;
	float minX = std::min(left, (left + width));
	float maxX = std::max(left, (left + width));
	float minY = std::min(top, (top + height));
	float maxY = std::max(top, (top + height));
	
	return (x >= minX) && (x < maxX) && (y >= minY) && (y < maxY);
}
//void InteractionComponent::onUpdate(float dt) {
//	auto ev = GuiEventType::NONE;
//	auto mpos = RESOURCES::ServiceManager::Get<INPUT_SYSTEM::InputManager>().getMousePosition();
//	if (contains(mpos.x, mpos.y)) {
//		if (RESOURCES::ServiceManager::Get<INPUT_SYSTEM::InputManager>().isMouseButtonPressed(INPUT_SYSTEM::EMouseButton::MOUSE_BUTTON_1)) {
//			ev = GuiEventType::PRESS;
//		}
//		else if (RESOURCES::ServiceManager::Get<INPUT_SYSTEM::InputManager>().isMouseButtonReleased(INPUT_SYSTEM::EMouseButton::MOUSE_BUTTON_1)) {
//			ev = GuiEventType::RELEASE;
//		}
//		else {
//			ev = GuiEventType::COVER;
//		}
//	}
//	else {
//		ev = GuiEventType::UNCOVER;
//	}
//
//	if (cur == ev && (cur == GuiEventType::PRESS || cur == GuiEventType::PRESS_CONTINUE)) {
//		if (onPressContinue) onPressContinue();
//	}
//
//	if (cur == ev) {
//		return;
//	}
//	cur = ev;
//	switch (ev) {
//	case GuiEventType::COVER: if (onCover) onCover();  break;
//	case GuiEventType::PRESS: if (onPress) onPress();  break;
//	case GuiEventType::RELEASE: if (onRelease) onRelease();  break;
//	case GuiEventType::UNCOVER: if (onUncover) onUncover();  break;
//	default: break;
//	}
//}
//
//void InteractionComponent::onPreUpdate(float dt) {
//	obj.transform->calculate();
//	if (obj.parent) {
//		globalX = obj.parent->childOffsetX;
//		globalY = obj.parent->childOffsetY;
//	}
//};
//
ClipComponent::ClipComponent(Object& obj, float w, float h) : Component(obj), mWidth(w), mHeight(h) {
	obj.getTransform()->getTransform().setLocalSize({ w, h });
}

ScrollComponent::ScrollComponent(Object& obj, float w, float h): Component(obj), mWidth(w), mHeight(h) {
	obj.getTransform()->getTransform().setLocalSize({ w, h });
}

////void ClipComponent::draw() {
////	
////}
//
//void ClipComponent::onPreUpdate(float dt) {
//	obj.transform->calculate();
//	globalX = obj.transform->globalModel(0, 3);
//	globalY = obj.transform->globalModel(1, 3);
//	obj.childOffsetX = globalX;
//	obj.childOffsetY = globalY;
//	obj.transform->globalModel(0, 3) = 0;
//	obj.transform->globalModel(1, 3) = 0;
//}



