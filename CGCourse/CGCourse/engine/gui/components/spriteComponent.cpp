#define NOMINMAX
#include "spriteComponent.h"
#include "../guiObject.h"
#include "../../render/Material.h"
#include "../../resourceManager/shaderManager.h"
#include "../../core/core.h"

using namespace KUMA;
using namespace KUMA::GUI;

TransformComponentGui::TransformComponentGui(GuiObject& obj) : ComponentGui(obj) {

}

void TransformComponentGui::calculate() {
	if (!isDirty) {
		return;
	}
	auto screenSz = RESOURCES::ServiceManager::Get<WINDOW_SYSTEM::Window>().getSize();
	auto parentSize = obj.parent ? obj.parent->transform->size : MATHGL::Vector2f(screenSz.x, screenSz.y);

	model = MATHGL::Matrix4();
	model *= MATHGL::Matrix4::Translation(MATHGL::Vector3(parentSize.x * anchor.x, parentSize.y * anchor.y, 0.0f));
	model *= MATHGL::Matrix4::Translation(MATHGL::Vector3(size.x * -pivot.x * scale.x,
		size.y * -pivot.y * scale.y, 0.0f));
	model *= MATHGL::Matrix4::Translation(position);
	model *= MATHGL::Matrix4::Translation({size.x * pivot.x * scale.x, size.y * pivot.y * scale.y, 0.0f});
	model *= MATHGL::Quaternion::ToMatrix4(MATHGL::Quaternion::Normalize(rotation));
	model *= MATHGL::Matrix4::Translation({size.x * -pivot.x * scale.x, size.y * -pivot.y * scale.y, 0.0f});
	model *= MATHGL::Matrix4::Scaling(scale /** MATHGL::Vector3(size.x, size.y, 1.0f)*/);

	globalModel = obj.parent ? obj.parent->transform->globalModel * model : model;
}

SpriteComponentGui::SpriteComponentGui(GuiObject& obj, std::shared_ptr<RENDER::Material> material): ComponentGui(obj), material(material) {
	//auto shader = material->getShader();
	auto texture = std::get<std::shared_ptr<RESOURCES::Texture>>(material->getUniformsData()["image"]);
	obj.transform->size = { static_cast<float>(texture->getWidth()), static_cast<float>(texture->getHeight())};
	shader = RESOURCES::ShaderLoader::CreateFromFile("Shaders/gui/sprite.glsl");
	MATHGL::Matrix4 projection = MATHGL::Matrix4::CreateOrthographic(0.0f, static_cast<float>(800), static_cast<float>(600), 0.0f, -1, 1);
	shader->bind();
	shader->setUniformMat4("u_engine_projection", projection);
	shader->setUniformInt("image", 0);
	shader->unbind();
	unsigned int VBO;
	float vertices [] = {
		0.0f, 1.0f, 0.0f, 1.0f,
		1.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 0.0f,

		0.0f, 1.0f, 0.0f, 1.0f,
		1.0f, 1.0f, 1.0f, 1.0f,
		1.0f, 0.0f, 1.0f, 0.0f
	};

	glGenVertexArrays(1, &quadVAO);
	glGenBuffers(1, &VBO);
	glBindVertexArray(quadVAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 24, vertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

void SpriteComponentGui::draw() {
	if (isEnabled) {
		//auto shader = material->getShader();
		glDisable(GL_DEPTH_TEST);
		glEnable(GL_CULL_FACE);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		shader->bind();

		//auto model = MATHGL::Matrix4();
		//model *= MATHGL::Matrix4::Translation(MATHGL::Vector3(800 * anchor.x, 600 * anchor.y, 0.0f));
		//model *= MATHGL::Matrix4::Translation(MATHGL::Vector3(texture->getWidth() * -pivot.x * scale.x,
		//	texture->getHeight() * -pivot.y * scale.y, 0.0f));
		//model *= MATHGL::Matrix4::Translation(position);
		//model *= MATHGL::Matrix4::Translation({texture->getWidth() * pivot.x * scale.x, texture->getHeight() * pivot.y * scale.y, 0.0f});
		//model *= MATHGL::Quaternion::ToMatrix4(MATHGL::Quaternion::Normalize(rotation));
		//model *= MATHGL::Matrix4::Translation({texture->getWidth() * -pivot.x * scale.x, texture->getHeight() * -pivot.y * scale.y, 0.0f});
		//model *= MATHGL::Matrix4::Scaling(scale * MATHGL::Vector3(texture->getWidth(), texture->getHeight(), 1.0f)); // последним выполняется масштабирование
		shader->setUniformMat4("u_engine_projection", RENDER::Renderer::guiProjection);
		shader->setUniformMat4("u_engine_model", obj.transform->globalModel * 
			MATHGL::Matrix4::Scaling( MATHGL::Vector3(obj.transform->size.x, obj.transform->size.y, 1.0f)));

		// Рендерим текстурированный прямоугольник
		shader->setUniformVec4("spriteColor", color);

		//glActiveTexture(GL_TEXTURE0);
		//material->bind(nullptr, true);
		auto texture = std::get<std::shared_ptr<RESOURCES::Texture>>(material->getUniformsData()["image"]);
		texture->bind(0);

		glBindVertexArray(this->quadVAO);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		glBindVertexArray(0);
		glEnable(GL_DEPTH_TEST);
	}
}


LabelComponentGui::LabelComponentGui(GuiObject& obj, std::string label, std::shared_ptr<Font> font, std::shared_ptr<RENDER::Material> material) :
	ComponentGui(obj), font(font), material(material) {
	this->label = label;

	float width = 0;
	float height = 0;
	for (auto c : label) {
		Character ch = font->Characters[c];
		width += (ch.Advance >> 6);
		height = std::max(height, static_cast<float>(ch.Size.y));
	}
	obj.transform->size = {width, height};

	auto shader = material->getShader();
	//shader = RESOURCES::ShaderLoader::CreateFromFile("Shaders/gui/text.glsl");
	MATHGL::Matrix4 projection = MATHGL::Matrix4::CreateOrthographic(0.0f, static_cast<float>(800), static_cast<float>(600), 0.0f, 0, 1);
	shader->bind();
	shader->setUniformInt("u_engine_text", 0);
	shader->setUniformMat4("u_engine_projection", projection);


	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

void LabelComponentGui::draw() {
	if (!isEnabled) {
		return;
	}
	glEnable(GL_CULL_FACE);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	// Активируем соответствующее состояние рендеринга
	auto shader = material->getShader();
	shader->bind();
	shader->setUniformVec3("textColor", {color.x, color.y, color.z});

	glBindVertexArray(VAO);

	glBindTexture(GL_TEXTURE_2D, font->texture);
	// Перебираем все символы
	std::string::const_iterator c;
	
	shader->setUniformMat4("u_engine_model", obj.transform->globalModel);

	float x = 0;//model(0, 3);
	float y = 0;//model(1, 3);
	for (auto c : label) {
		Character ch = font->Characters[c];

		float xpos = x + ch.Bearing.x * obj.transform->globalScale.x;
		float ypos = y - (ch.Size.y - ch.Bearing.y) * obj.transform->globalScale.y;

		float w = ch.Size.x * obj.transform->globalScale.x;
		float h = ch.Size.y * obj.transform->globalScale.y;


		// Обновляем VBO для каждого символа
		float vertices[6][4] = {
			{ xpos,     ypos + h, ch.Start.x / 1024.0,               (ch.Start.y + ch.Size.y) / 1024.0 },
			{ xpos + w, ypos,     (ch.Start.x + ch.Size.x) / 1024.0,  ch.Start.y / 1024.0},
			{ xpos,     ypos,     ch.Start.x / 1024.0,               ch.Start.y / 1024.0 },
			{ xpos,     ypos + h, ch.Start.x / 1024.0,               (ch.Start.y + ch.Size.y) / 1024.0  },
			{ xpos + w, ypos + h, (ch.Start.x + ch.Size.x) / 1024.0, (ch.Start.y + ch.Size.y) / 1024.0 },
			{ xpos + w, ypos,     (ch.Start.x + ch.Size.x) / 1024.0, ch.Start.y / 1024.0 }
		};

		// Визуализируем текстуру глифа поверх прямоугольника
		//glBindTexture(GL_TEXTURE_2D, ch.textureID);

		// Обновляем содержимое памяти VBO
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		// Рендерим прямоугольник
		glDrawArrays(GL_TRIANGLES, 0, 6);

		// Теперь производим смещение для отображения следующего глифа (обратите внимание, что данное смещение измеряется в единицах, составляющих 1/64 пикселя)
		x += (ch.Advance >> 6) * obj.transform->globalScale.x; // побитовый сдвиг на 6, чтобы получить значение в пикселях (2^6 = 64)
	}
	glBindVertexArray(0);
	glBindTexture(GL_TEXTURE_2D, 0);
}

InteractionComponentGui::InteractionComponentGui(GuiObject& obj, MATHGL::Vector2f size) : ComponentGui(obj) {
	obj.transform->size = size;
}
bool InteractionComponentGui::contains(float x, float y) const {
	auto left = obj.transform->globalModel(0, 3) + globalX;
	auto top = obj.transform->globalModel(1, 3) + globalY;
	auto width = obj.transform->size.x * obj.transform->globalModel(0, 0);
	auto height = obj.transform->size.y * obj.transform->globalModel(1, 1);
	float minX = std::min(left, (left + width));
	float maxX = std::max(left, (left + width));
	float minY = std::min(top, (top + height));
	float maxY = std::max(top, (top + height));

	return (x >= minX) && (x < maxX) && (y >= minY) && (y < maxY);
}
void InteractionComponentGui::onUpdate(float dt) {
	auto ev = GuiEventType::NONE;
	auto mpos = RESOURCES::ServiceManager::Get<INPUT_SYSTEM::InputManager>().getMousePosition();
	if (contains(mpos.x, mpos.y)) {
		if (RESOURCES::ServiceManager::Get<INPUT_SYSTEM::InputManager>().isMouseButtonPressed(INPUT_SYSTEM::EMouseButton::MOUSE_BUTTON_1)) {
			ev = GuiEventType::PRESS;
		}
		else if (RESOURCES::ServiceManager::Get<INPUT_SYSTEM::InputManager>().isMouseButtonReleased(INPUT_SYSTEM::EMouseButton::MOUSE_BUTTON_1)) {
			ev = GuiEventType::RELEASE;
		}
		else {
			ev = GuiEventType::COVER;
		}
	}
	else {
		ev = GuiEventType::UNCOVER;
	}

	if (cur == ev && (cur == GuiEventType::PRESS || cur == GuiEventType::PRESS_CONTINUE)) {
		if (onPressContinue) onPressContinue();
	}

	if (cur == ev) {
		return;
	}
	cur = ev;
	switch (ev) {
	case GuiEventType::COVER: if (onCover) onCover();  break;
	case GuiEventType::PRESS: if (onPress) onPress();  break;
	case GuiEventType::RELEASE: if (onRelease) onRelease();  break;
	case GuiEventType::UNCOVER: if (onUncover) onUncover();  break;
	default: break;
	}
}

void InteractionComponentGui::onPreUpdate(float dt) {
	obj.transform->calculate();
	if (obj.parent) {
		globalX = obj.parent->childOffsetX;
		globalY = obj.parent->childOffsetY;
	}
};

ClipComponentGui::ClipComponentGui(GuiObject& obj, float w, float h) : ComponentGui(obj),
width(w), height(h) {
	obj.transform->size = {w, h};
}

void ClipComponentGui::draw() {
	
}

void ClipComponentGui::onPreUpdate(float dt) {
	obj.transform->calculate();
	globalX = obj.transform->globalModel(0, 3);
	globalY = obj.transform->globalModel(1, 3);
	obj.childOffsetX = globalX;
	obj.childOffsetY = globalY;
	obj.transform->globalModel(0, 3) = 0;
	obj.transform->globalModel(1, 3) = 0;
}



