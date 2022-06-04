#include "guiObject.h"
#include "../core/core.h"

using namespace KUMA;
using namespace KUMA::GUI;

Font::Font(std::string fontPath, int size) {
	//auto shader = material->getShader();
	////shader = RESOURCES::ShaderLoader::CreateFromFile("Shaders/gui/text.glsl");
	//MATHGL::Matrix4 projection = MATHGL::Matrix4::CreateOrthographic(0.0f, static_cast<float>(800), 0.0f, static_cast<float>(600), 0, 1);
	//shader->bind();
	//shader->setUniformInt("text", 0);
	//shader->setUniformMat4("projection", projection);

	FT_Library ft;

	// Всякий раз, когда возникает ошибка, функции будут возвращать отличное от нуля значение
	if (FT_Init_FreeType(&ft)) {
		std::cout << "ERROR::FREETYPE: Could not init FreeType Library" << std::endl;
		return;
	}


	std::vector<unsigned char[1024]> textureData(1024);
	int curMaxHeight = 0;
	int curX = 0, curY = 0;

	// Загружаем шрифт в face
	FT_Face face;
	if (FT_New_Face(ft, fontPath.c_str(), 0, &face)) {
		std::cout << "ERROR::FREETYPE: Failed to load font" << std::endl;
		return;
	}
	else {
		// Задаем размер для загрузки глифов
		FT_Set_Pixel_Sizes(face, 0, 48);

		// Отключаем ограничение выравнивания байтов
		//glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

		// Загружаем глифы символов 
		for (unsigned int c = 0; c < 256; c++) {
			// Загружаем глиф символа 
			if (FT_Load_Char(face, c, FT_LOAD_RENDER)) {
				std::cout << "ERROR::FREETYTPE: Failed to load Glyph" << std::endl;
				continue;
			}

			for (int i = 0; i < face->glyph->bitmap.width; i++) {
				Character character = {
					0,
					glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
					glm::vec2(curX, curY),
					glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
					static_cast<unsigned int>(face->glyph->advance.x)
				};
				Characters.insert(std::pair<char, Character>(c, character));
				if (face->glyph->bitmap.rows > curMaxHeight) {
					curMaxHeight = face->glyph->bitmap.rows;
				}
				if (curX + face->glyph->bitmap.width > 1024) {
					curX = 0;
					curY += curMaxHeight;
				}
				if (curY + face->glyph->bitmap.rows > 1024) {
					auto a = 0;
				}
				for (int j = 0; j < face->glyph->bitmap.rows; j++) {
					textureData[curY + j][curX + i] = face->glyph->bitmap.buffer[j * face->glyph->bitmap.width + i];
				}
				//curY += face->glyph->bitmap.rows;
			}
			curX += face->glyph->bitmap.width;
		}
		// Генерируем текстуру

		glGenTextures(1, &texture);
		glBindTexture(GL_TEXTURE_2D, texture);
		glTexImage2D(
			GL_TEXTURE_2D,
			0,
			GL_RED,
			1024,
			1024,
			0,
			GL_RED,
			GL_UNSIGNED_BYTE,
			textureData.data()
		);
		
		// Задаем для текстуры необходимые опции
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glBindTexture(GL_TEXTURE_2D, 0);
	}

	// Освобождаем использованные ресурсы
	FT_Done_Face(face);
	FT_Done_FreeType(ft);
}
/*
Font::Font() {
	shader = RESOURCES::ShaderLoader::CreateFromFile("Shaders/gui/text.glsl");
	MATHGL::Matrix4 projection = MATHGL::Matrix4::CreateOrthographic(0.0f, static_cast<float>(800), 0.0f, static_cast<float>(600), 0, 1);
	shader->bind();
	shader->setUniformMat4("projection", projection);

	FT_Library ft;

	// Всякий раз, когда возникает ошибка, функции будут возвращать отличное от нуля значение
	if (FT_Init_FreeType(&ft)) {
		std::cout << "ERROR::FREETYPE: Could not init FreeType Library" << std::endl;
		return;
	}

	// Путь к файлу шрифта
	std::string font_name = "C:/Projects/SimpleEngine/CGCourse/CGCourse/Assets/Engine/Fonts/a_AlternaSw.TTF";
	if (font_name.empty()) {
		std::cout << "ERROR::FREETYPE: Failed to load font_name" << std::endl;
		return;
	}

	std::vector<unsigned char[1024]> textureData(1024);
	int curMaxHeight = 0;
	int curX = 0, curY = 0;

	// Загружаем шрифт в face
	FT_Face face;
	if (FT_New_Face(ft, font_name.c_str(), 0, &face)) {
		std::cout << "ERROR::FREETYPE: Failed to load font" << std::endl;
		return;
	}
	else {
		// Задаем размер для загрузки глифов
		FT_Set_Pixel_Sizes(face, 0, 48);

		// Отключаем ограничение выравнивания байтов
		//glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

		// Загружаем глифы символов 
		for (unsigned int c = 0; c < 256; c++) {
			// Загружаем глиф символа 
			if (FT_Load_Char(face, c, FT_LOAD_RENDER)) {
				std::cout << "ERROR::FREETYTPE: Failed to load Glyph" << std::endl;
				continue;
			}

			for (int i = 0; i < face->glyph->bitmap.width; i++) {
				Character character = {
					0,
					glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
					glm::vec2(curX, curY),
					glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
					static_cast<unsigned int>(face->glyph->advance.x)
				};
				Characters.insert(std::pair<char, Character>(c, character));
				if (face->glyph->bitmap.rows > curMaxHeight) {
					curMaxHeight = face->glyph->bitmap.rows;
				}
				if (curX + face->glyph->bitmap.width > 1024) {
					curX = 0;
					curY += curMaxHeight;
				}
				if (curY + face->glyph->bitmap.rows > 1024) {
					auto a = 0;
				}
				for (int j = 0; j < face->glyph->bitmap.rows; j++) {
					textureData[curY + j][curX + i] = face->glyph->bitmap.buffer[j * face->glyph->bitmap.width + i];
				}
				//curY += face->glyph->bitmap.rows;
			}
			curX += face->glyph->bitmap.width;
		}
		// Генерируем текстуру

		glGenTextures(1, &texture);
		glBindTexture(GL_TEXTURE_2D, texture);
		glTexImage2D(
			GL_TEXTURE_2D,
			0,
			GL_RED,
			1024,
			1024,
			0,
			GL_RED,
			GL_UNSIGNED_BYTE,
			textureData.data()
		);

		// Задаем для текстуры необходимые опции
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glBindTexture(GL_TEXTURE_2D, 0);
	}

	// Освобождаем использованные ресурсы
	FT_Done_Face(face);
	FT_Done_FreeType(ft);

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

void Font::RenderText(std::string text, float x, float y, float _scale, MATHGL::Vector3 color) {
	//position = MATHGL::Vector3(x, y, 0);
	//scale = MATHGL::Vector3(_scale, _scale, 1);
	//rotation = MATHGL::Vector3(0, 0, 0);
	//Активируем соответствующее состояние рендеринга	
	glEnable(GL_CULL_FACE);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	// Активируем соответствующее состояние рендеринга	
	shader->bind();
	shader->setUniformVec3("textColor", {color.x, color.y, color.z});
	shader->setUniformInt("text", 0);
	glBindVertexArray(VAO);

	glBindTexture(GL_TEXTURE_2D, texture);
	// Перебираем все символы
	std::string::const_iterator c;

	int width = 0;
	int height = 0;
	for (c = text.begin(); c != text.end(); c++) {
		Character ch = Characters[*c];
		width += (ch.Advance >> 6);
		height = std::max(height, ch.Size.y);
	}
	auto model = MATHGL::Matrix4();

	model *= MATHGL::Matrix4::Translation(MATHGL::Vector3(800 * anchor.x, 600 * anchor.y, 0.0f));
	model *= MATHGL::Matrix4::Translation(MATHGL::Vector3(width * -pivot.x * scale.x,
		height * -pivot.y * scale.y, 0.0f));
	model *= MATHGL::Matrix4::Translation(position);

	model *= MATHGL::Matrix4::Translation({width * pivot.x * scale.x, height * pivot.y * scale.y, 0.0f});
	model *= MATHGL::Quaternion::ToMatrix4(MATHGL::Quaternion::Normalize(rotation));
	model *= MATHGL::Matrix4::Translation({width * -pivot.x * scale.x, height * -pivot.y * scale.y, 0.0f});

	model *= MATHGL::Matrix4::Scaling(scale);
	shader->setUniformMat4("model", model);

	x = 0;//model(0, 3);
	y = 0;//model(1, 3);
	for (c = text.begin(); c != text.end(); c++) {
		Character ch = Characters[*c];

		float xpos = x + ch.Bearing.x * scale.x;
		float ypos = y - (ch.Size.y - ch.Bearing.y) * scale.y;

		float w = ch.Size.x * scale.x;
		float h = ch.Size.y * scale.y;

		// Обновляем VBO для каждого символа
		float vertices[6][4] = {
			{ xpos,     ypos + h, ch.Start.x / 1024.0,               ch.Start.y / 1024.0 },
			{ xpos,     ypos,     ch.Start.x / 1024.0,               (ch.Start.y + ch.Size.y) / 1024.0 },
			{ xpos + w, ypos,     (ch.Start.x + ch.Size.x) / 1024.0, (ch.Start.y + ch.Size.y) / 1024.0 },
			{ xpos,     ypos + h, ch.Start.x / 1024.0,               ch.Start.y / 1024.0 },
			{ xpos + w, ypos,     (ch.Start.x + ch.Size.x) / 1024.0, (ch.Start.y + ch.Size.y) / 1024.0 },
			{ xpos + w, ypos + h, (ch.Start.x + ch.Size.x) / 1024.0, ch.Start.y / 1024.0 }
		};
		//float vertices[6][4] = {
		//	{ 0,     600,   0.0f, 0.0f },
		//	{ 0,     0,       0.0f, 1.0f },
		//	{ 800, 0,       1.0f, 1.0f },
		//
		//	{ 0,     600,   0.0f, 0.0f },
		//	{ 800, 0,       1.0f, 1.0f },
		//	{ 800, 600,   1.0f, 0.0f }
		//};

		// Визуализируем текстуру глифа поверх прямоугольника
		//glBindTexture(GL_TEXTURE_2D, ch.textureID);

		// Обновляем содержимое памяти VBO
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		// Рендерим прямоугольник
		glDrawArrays(GL_TRIANGLES, 0, 6);

		// Теперь производим смещение для отображения следующего глифа (обратите внимание, что данное смещение измеряется в единицах, составляющих 1/64 пикселя)
		x += (ch.Advance >> 6) * scale.x; // побитовый сдвиг на 6, чтобы получить значение в пикселях (2^6 = 64)
	}
	glBindVertexArray(0);
	glBindTexture(GL_TEXTURE_2D, 0);
}
*/
Sprite::Sprite(std::shared_ptr<RESOURCES::Texture> texture) : texture(texture) {
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

	glGenVertexArrays(1, &this->quadVAO);
	glGenBuffers(1, &VBO);
	glBindVertexArray(this->quadVAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 24, vertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

Sprite::~Sprite() {
	glDeleteVertexArrays(1, &this->quadVAO);
}

void Sprite::draw() {
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	shader->bind();
	auto model = MATHGL::Matrix4();

	model *= MATHGL::Matrix4::Translation(MATHGL::Vector3(800 * anchor.x, 600 * anchor.y, 0.0f));
	model *= MATHGL::Matrix4::Translation(MATHGL::Vector3(texture->getWidth() * -pivot.x * scale.x,
		texture->getHeight() * -pivot.y * scale.y, 0.0f));
	model *= MATHGL::Matrix4::Translation(position);
	model *= MATHGL::Matrix4::Translation({texture->getWidth() * pivot.x * scale.x, texture->getHeight() * pivot.y * scale.y, 0.0f});
	model *= MATHGL::Quaternion::ToMatrix4(MATHGL::Quaternion::Normalize(rotation));
	model *= MATHGL::Matrix4::Translation({texture->getWidth() * -pivot.x * scale.x, texture->getHeight() * -pivot.y * scale.y, 0.0f});

	model *= MATHGL::Matrix4::Scaling(scale * MATHGL::Vector3(texture->getWidth(), texture->getHeight(), 1.0f)); // последним выполняется масштабирование

	shader->setUniformMat4("u_engine_model", model);

	// Рендерим текстурированный прямоугольник
	shader->setUniformVec4("spriteColor", color);

	//glActiveTexture(GL_TEXTURE0);
	texture->bind(0);

	glBindVertexArray(this->quadVAO);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindVertexArray(0);
	//glEnable(GL_DEPTH_TEST);
}


GuiButton::GuiButton() : GuiObject() {
	auto s = KUMA::RESOURCES::ShaderLoader::CreateFromFile("Shaders\\gui\\sprite.glsl");
	auto _m = KUMA::RESOURCES::MaterialLoader::Create("");
	_m->setShader(s);
	auto& data = _m->getUniformsData();
	auto tex1 = KUMA::RESOURCES::TextureLoader().CreateFromFile("textures\\gui\\btn.png");
	data["image"] = tex1;
	data["spriteColor"] = KUMA::MATHGL::Vector4{1, 1, 1, 1};

	sprite = std::make_shared<SpriteComponentGui>(*this, _m);
	{
		auto s = KUMA::RESOURCES::ShaderLoader::CreateFromFile("Shaders\\gui\\text.glsl");
		auto _m = KUMA::RESOURCES::MaterialLoader::Create("");
		_m->setShader(s);
		auto& data = _m->getUniformsData();
		data["textColor"] = KUMA::MATHGL::Vector4{1, 1, 1, 1};

		auto f = std::make_shared<KUMA::GUI::Font>("C:/Projects/SimpleEngine/CGCourse/CGCourse/Assets/Engine/Fonts/a_AlternaSw.TTF", 42);

		label = std::make_shared<LabelComponentGui>(*this, "press", f, _m);
	}
	interaction = std::make_shared<InteractionComponentGui>(*this, MATHGL::Vector2f{100.0f, 51.0f});

	//transform->pos = {100.0f, 100.0f};
	//transform->scale = {1.0f, 1.0f};

	//interaction->rect = sf::FloatRect(transform->pos.x, transform->pos.y,
	//	sprite->sprite.getLocalBounds().width, sprite->sprite.getLocalBounds().height);

	//sprite->sprite.setPosition(transform->pos.x, transform->pos.y);

	//label->text.setPosition(transform->pos.x + 10.0f, transform->pos.y + 10.0f);

	onClick = []() {
		LOG_INFO("Click");
	};
}
//#include <iostream>
void GuiButton::onEvent(GuiEventType etype) {
	//std::cout << (int)etype << std::endl;
	if (cur == etype) {
		return;
	}
	if (etype == GuiEventType::PRESS) {
		sprite->color = {1, 0, 0, 1};
	}
	else if (etype == GuiEventType::RELEASE) {
		sprite->color = {1, 1, 1, 1};
	}
	else if (etype == GuiEventType::HOVER) {
		sprite->color = {0, 1, 0, 1};
	}
	else if (etype == GuiEventType::UNHOVER) {
		sprite->color = {1, 1, 1, 1};
	}
	cur = etype;
}

void GuiButton::onUpdate(float dt) {
	interaction->onUpdate(dt);
	sprite->draw();
	label->draw();
	GuiObject::onUpdate(dt);
}

/*
void Font::RenderText(std::string text, float x, float y, float _scale, MATHGL::Vector3 color) {
	//position = MATHGL::Vector3(x, y, 0);
	//scale = MATHGL::Vector3(_scale, _scale, 1);
	//rotation = MATHGL::Vector3(0, 0, 0);
	//Активируем соответствующее состояние рендеринга	
	glEnable(GL_CULL_FACE);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	// Активируем соответствующее состояние рендеринга
	auto shader = material->getShader();
	shader->bind();
	shader->setUniformVec3("textColor", {color.x, color.y, color.z});
	
	glBindVertexArray(VAO);

	glBindTexture(GL_TEXTURE_2D, texture);
	// Перебираем все символы
	std::string::const_iterator c;

	int width = 0;
	int height = 0;
	for (c = text.begin(); c != text.end(); c++) {
		Character ch = Characters[*c];
		width += (ch.Advance >> 6);
		height = std::max(height, ch.Size.y);
	}
	auto model = MATHGL::Matrix4();

	model *= MATHGL::Matrix4::Translation(MATHGL::Vector3(800 * anchor.x, 600 * anchor.y, 0.0f));
	model *= MATHGL::Matrix4::Translation(MATHGL::Vector3(width * -pivot.x * scale.x,
		height * -pivot.y * scale.y, 0.0f));
	model *= MATHGL::Matrix4::Translation(position);
	
	model *= MATHGL::Matrix4::Translation({width * pivot.x * scale.x, height * pivot.y * scale.y, 0.0f});
	model *= MATHGL::Quaternion::ToMatrix4(MATHGL::Quaternion::Normalize(rotation));
	model *= MATHGL::Matrix4::Translation({width * -pivot.x * scale.x, height * -pivot.y * scale.y, 0.0f});

	model *= MATHGL::Matrix4::Scaling(scale);
	shader->setUniformMat4("model", model);

	x = 0;//model(0, 3);
	y = 0;//model(1, 3);
	for (c = text.begin(); c != text.end(); c++) {
		Character ch = Characters[*c];

		float xpos = x + ch.Bearing.x * scale.x;
		float ypos = y - (ch.Size.y - ch.Bearing.y) * scale.y;

		float w = ch.Size.x * scale.x;
		float h = ch.Size.y * scale.y;

		// Обновляем VBO для каждого символа
		float vertices[6][4] = {
			{ xpos,     ypos + h, ch.Start.x / 1024.0,               ch.Start.y / 1024.0 },
			{ xpos,     ypos,     ch.Start.x / 1024.0,               (ch.Start.y + ch.Size.y) / 1024.0 },
			{ xpos + w, ypos,     (ch.Start.x + ch.Size.x) / 1024.0, (ch.Start.y + ch.Size.y) / 1024.0 },
			{ xpos,     ypos + h, ch.Start.x / 1024.0,               ch.Start.y / 1024.0 },
			{ xpos + w, ypos,     (ch.Start.x + ch.Size.x) / 1024.0, (ch.Start.y + ch.Size.y) / 1024.0 },
			{ xpos + w, ypos + h, (ch.Start.x + ch.Size.x) / 1024.0, ch.Start.y / 1024.0 }
		};
		//float vertices[6][4] = {
		//	{ 0,     600,   0.0f, 0.0f },
		//	{ 0,     0,       0.0f, 1.0f },
		//	{ 800, 0,       1.0f, 1.0f },
		//
		//	{ 0,     600,   0.0f, 0.0f },
		//	{ 800, 0,       1.0f, 1.0f },
		//	{ 800, 600,   1.0f, 0.0f }
		//};

		// Визуализируем текстуру глифа поверх прямоугольника
		//glBindTexture(GL_TEXTURE_2D, ch.textureID);

		// Обновляем содержимое памяти VBO
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		// Рендерим прямоугольник
		glDrawArrays(GL_TRIANGLES, 0, 6);

		// Теперь производим смещение для отображения следующего глифа (обратите внимание, что данное смещение измеряется в единицах, составляющих 1/64 пикселя)
		x += (ch.Advance >> 6) * scale.x; // побитовый сдвиг на 6, чтобы получить значение в пикселях (2^6 = 64)
	}
	glBindVertexArray(0);
	glBindTexture(GL_TEXTURE_2D, 0);
}

*/

GuiImage::GuiImage(std::shared_ptr<RENDER::Material> material) : GuiObject() {
	sprite = std::make_shared<SpriteComponentGui>(*this, material);
	//transform->position = {0.0f, 0.0f};
	//transform->scale = {1.0f, 1.0f};
	//sprite->sprite.setPosition(transform->pos.x, transform->pos.y);
}

void GuiImage::onUpdate(float dt) {
	sprite->draw();
	GuiObject::onUpdate(dt);
}

void GuiImage::setProjection(float w, float h) {
	MATHGL::Matrix4 projection = MATHGL::Matrix4::CreateOrthographic(0.0f, w, h, 0.0f, -1, 1);
	sprite->shader->bind();
	sprite->shader->setUniformMat4("u_engine_projection", projection);
	sprite->shader->unbind();
}

GuiLabel::GuiLabel(std::string text, std::shared_ptr<Font> font, std::shared_ptr<RENDER::Material> material) : GuiObject() {
	label = std::make_shared<LabelComponentGui>(*this, text, font, material);
	//transform->pos = {100.0f, 100.0f};
	//transform->scale = {1.0f, 1.0f};
	//label->text.setPosition(transform->pos.x + 10.0f, transform->pos.y + 10.0f);
}

void GuiLabel::onUpdate(float dt) {
	label->draw();
}

void GuiClip::onUpdate(float dt) {
	auto screenRes = RESOURCES::ServiceManager::Get<WINDOW_SYSTEM::Window>().getSize();
	glViewport(globalX, screenRes.y - globalY - clipComponent->height,
		clipComponent->width, clipComponent->height);
	//glColorMask(1, 0, 0, 1);
	//glClear(GL_COLOR_BUFFER_BIT);
	GuiObject::onUpdate(dt);
	
	
	glViewport(0, 0, screenRes.x, screenRes.y);
}

GuiScroll::GuiScroll(float w, float h) : GuiObject() {
	interaction = std::make_shared<InteractionComponentGui>(*this, MATHGL::Vector2f(w, h));
	scroll = std::make_shared<ScrollComponentGui>(*this, w, h);

	//transform->pos = {100.0f, 100.0f};
	//transform->scale = {1.0f, 1.0f};

	//interaction->rect = sf::FloatRect(transform->posGlobal.x, transform->posGlobal.y, w, h);

}

bool GuiScroll::contains(float left, float top, float width ,float height, float x, float y) const {
	float minX = std::min(left, (left + width));
	float maxX = std::max(left, (left + width));
	float minY = std::min(top, (top + height));
	float maxY = std::max(top, (top + height));

	return (x >= minX) && (x < maxX) && (y >= minY) && (y < maxY);
}

void GuiScroll::onEvent(GuiEventType etype) {
	if (etype == GuiEventType::PRESS) {
		auto mpos = RESOURCES::ServiceManager::Get<INPUT_SYSTEM::InputManager>().getMousePosition();
		if (!scroll->isPress) {
			for (auto& ch : childs) {
				if (contains(ch->transform->globalModel(0, 3) + interaction->globalX, 
					ch->transform->globalModel(1, 3) + interaction->globalY,
					ch->transform->size.x, ch->transform->size.y, mpos.x, mpos.y)) {
					selectedObj = ch;
					selectedObjPos = {ch->transform->position.x, ch->transform->position.y};
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
			auto shiftY = scroll->isScrollVertical ? mpos.y - scroll->startY : 0.0f;
			selectedObj->transform->position.x = selectedObjPos.x + shiftX;
			selectedObj->transform->position.y = selectedObjPos.y + shiftY;
			selectedObj->transform->calculate();
			bool check = false;
			if (selectedObj->transform->globalModel(0, 3) < transform->globalModel(0, 3)) {
				selectedObj->transform->position.x = transform->globalModel(0, 3);
			}
			if (selectedObj->transform->globalModel(0, 3) + selectedObj->transform->size.x > transform->globalModel(0, 3) + transform->size.x) {
				selectedObj->transform->position.x = transform->globalModel(0, 3) + transform->size.x - selectedObj->transform->size.x;
			}
			if (selectedObj->transform->globalModel(1, 3) < transform->globalModel(1, 3)) {
				selectedObj->transform->position.y = transform->globalModel(1, 3);
			}
			if (selectedObj->transform->globalModel(1, 3) + selectedObj->transform->size.y > transform->globalModel(1, 3) + transform->size.y) {
				selectedObj->transform->position.y = transform->globalModel(1, 3) + transform->size.y - selectedObj->transform->size.y;
			}
			//if (check) {
			//	selectedObj->setPos(selectedObjPos.x + prevShiftX, selectedObjPos.y + prevShiftY);
			//}
			//else {
			//	prevShiftX = shiftX;
			//	prevShiftY = shiftY;
			//}
		}
	}
	else if (etype == GuiEventType::RELEASE || etype == GuiEventType::UNHOVER) {
		scroll->isPress = false;
		scroll->startX = 0.0f;
		scroll->startY = 0.0f;

		selectedObj = nullptr;
	}
}

void GuiScroll::onUpdate(float dt) {
	interaction->onUpdate(dt);
	GuiObject::onUpdate(dt);
}
