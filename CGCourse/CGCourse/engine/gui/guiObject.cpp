#include "guiObject.h"
#include "../core/core.h"
#include "../render/material.h"
#include "../render/gameRenderer.h"

using namespace KUMA;
using namespace KUMA::GUI;

Font::Font(std::string fontPath, int size) {
	FT_Library ft;

	// ������ ���, ����� ��������� ������, ������� ����� ���������� �������� �� ���� ��������
	if (FT_Init_FreeType(&ft)) {
		std::cout << "ERROR::FREETYPE: Could not init FreeType Library" << std::endl;
		return;
	}

	std::vector<std::array<unsigned char, 1024>> textureData(1024);
	int curMaxHeight = 0;
	int curX = 0, curY = 0;

	// ��������� ����� � face
	FT_Face face;
	if (FT_New_Face(ft, fontPath.c_str(), 0, &face)) {
		std::cout << "ERROR::FREETYPE: Failed to load font" << std::endl;
		return;
	}
	else {
		// ������ ������ ��� �������� ������
		FT_Set_Pixel_Sizes(face, 0, 48);

		// ��������� ����������� ������������ ������
		//glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

		// ��������� ����� �������� 
		for (unsigned int c = 0; c < 256; c++) {
			// ��������� ���� ������� 
			if (FT_Load_Char(face, c, FT_LOAD_RENDER)) {
				std::cout << "ERROR::FREETYTPE: Failed to load Glyph" << std::endl;
				continue;
			}

			for (int i = 0; i < face->glyph->bitmap.width; i++) {
				Character character = {
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
		// ���������� ��������

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
		
		// ������ ��� �������� ����������� �����
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glBindTexture(GL_TEXTURE_2D, 0);
	}

	// ����������� �������������� �������
	FT_Done_Face(face);
	FT_Done_FreeType(ft);
}

GuiButton::GuiButton() : GuiObject() {
	auto s = KUMA::RESOURCES::ShaderLoader::CreateFromFile("Shaders\\gui\\sprite.glsl");
	auto _m = KUMA::RESOURCES::MaterialLoader::Create("");
	_m->setShader(s);
	auto& data = _m->getUniformsData();
	auto tex1 = KUMA::RESOURCES::TextureLoader().CreateFromFile("textures\\gui\\btn.png");
	data["image"] = tex1;
	data["spriteColor"] = KUMA::MATHGL::Vector4{1, 1, 1, 1};

	auto sprite = std::make_shared<SpriteComponentGui>(*this, _m);
	auto interaction = std::make_shared<InteractionComponentGui>(*this, MATHGL::Vector2f{100.0f, 51.0f});
	interaction->onPress = [sprite]() {
		sprite->color = {1, 0, 0, 1};
	};
	interaction->onRelease = [sprite]() {
		sprite->color = {1, 1, 1, 1};
	};
	interaction->onCover = [sprite]() {
		sprite->color = {0, 1, 0, 1};
	};
	interaction->onUncover = [sprite]() {
		sprite->color = {1, 1, 1, 1};
	};

	addComponent(sprite);
	addComponent(interaction);
}

GuiImage::GuiImage(std::shared_ptr<RENDER::Material> material) : GuiObject() {
	addComponent(std::make_shared<SpriteComponentGui>(*this, material));

}

GuiLabel::GuiLabel(std::string text, std::shared_ptr<Font> font, std::shared_ptr<RENDER::Material> material) : GuiObject() {
	addComponent(std::make_shared<LabelComponentGui>(*this, text, font, material));
}

void GuiClip::onUpdate(float dt) {
	auto clip = getComponent<ClipComponentGui>();
	auto screenRes = RESOURCES::ServiceManager::Get<WINDOW_SYSTEM::Window>().getSize();
	glViewport(static_cast<int>(clip->globalX), screenRes.y - static_cast<int>(clip->globalY) - static_cast<int>(clip->height),
		static_cast<int>(clip->width), static_cast<int>(clip->height));
	RENDER::Renderer::guiProjection = MATHGL::Matrix4::CreateOrthographic(0.0f, static_cast<float>(clip->width), static_cast<float>(clip->height), 0.0f, -1, 1);
	GuiObject::onUpdate(dt);
	RENDER::Renderer::guiProjection = MATHGL::Matrix4::CreateOrthographic(0.0f, static_cast<float>(screenRes.x), static_cast<float>(screenRes.y), 0.0f, -1, 1);
	glViewport(0, 0, static_cast<int>(screenRes.x), static_cast<int>(screenRes.y));
}

GuiScroll::GuiScroll(float w, float h) : GuiObject() {
	auto interaction = std::make_shared<InteractionComponentGui>(*this, MATHGL::Vector2f(w, h));
	auto scroll = std::make_shared<ScrollComponentGui>(*this, w, h);
	addComponent(interaction);
	addComponent(scroll);
	interaction->onRelease = [this, scroll]() {
		scroll->isPress = false;
		scroll->startX = 0.0f;
		scroll->startY = 0.0f;
		selectedObj = nullptr;
	};
	interaction->onUncover = [this, scroll]() {
		scroll->isPress = false;
		scroll->startX = 0.0f;
		scroll->startY = 0.0f;
		selectedObj = nullptr;
	};
	interaction->onPressContinue = [this, interaction, scroll]() {
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
			//std::cout << shiftX << " " << shiftY << std::endl;
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
		}
	};

}

bool GuiScroll::contains(float left, float top, float width ,float height, float x, float y) const {
	float minX = std::min(left, (left + width));
	float maxX = std::max(left, (left + width));
	float minY = std::min(top, (top + height));
	float maxY = std::max(top, (top + height));

	return (x >= minX) && (x < maxX) && (y >= minY) && (y < maxY);
}

