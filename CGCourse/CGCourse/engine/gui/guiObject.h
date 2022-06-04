#pragma once
//#include "../ecs/components/modelRenderer.h"
//#include "../resourceManager/modelManager.h"
//#include "../resourceManager/textureManager.h"
//#include "../resourceManager/parser/assimpParser.h"
//#include "../scene/scene.h"
//#include "../utils/idObject.h"
//#include "../utils/math/Vector2.h"
#include <memory>
#include <vector>
#include <algorithm>


#include "components/spriteComponent.h"

#include <ft2build.h>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

#include "../resourceManager/resource/shader.h"

#include FT_FREETYPE_H

namespace KUMA {
	namespace RESOURCES
	{
		class Shader;
	}

	namespace CORE_SYSTEM {
		class Core;
	}
	namespace GUI {
		class ComponentGui;
		
		enum class GuiEventType {
			NONE,
			
			HOVER,
			PRESS,
			RELEASE,
			UNHOVER
		};

		struct Character {
			unsigned int TextureID; // ID текстуры глифа
			glm::ivec2   Size;      // размер глифа
			glm::ivec2   Start;      // размер глифа
			glm::ivec2   Bearing;   // смещение от линии шрифта до верхнего/левого угла глифа
			unsigned int Advance;   // смещение до следующего глифа
		};

		struct Font {
			std::map<GLchar, Character> Characters;
			unsigned int texture;
			Font(std::string fontPath, int size);
		
			//void RenderText(std::string text, float x, float y, float scale, MATHGL::Vector3 color);
		
			//MATHGL::Vector2f anchor;
			//MATHGL::Vector2f pivot;
			//MATHGL::Vector3 position = {0, 0, 0};
			//MATHGL::Vector3 scale = {1, 1, 1};
			//MATHGL::Vector3 rotation = {0, 0, 0};
		};


		//struct Font {
		//	std::map<GLchar, Character> Characters;
		//	std::shared_ptr<RESOURCES::Shader> shader;
		//	unsigned int texture;
		//	unsigned int VAO, VBO;
		//	Font();
		//
		//	void RenderText(std::string text, float x, float y, float scale, MATHGL::Vector3 color);
		//
		//	MATHGL::Vector2f anchor;
		//	MATHGL::Vector2f pivot;
		//	MATHGL::Vector3 position = {0, 0, 0};
		//	MATHGL::Vector3 scale = {1, 1, 1};
		//	MATHGL::Vector3 rotation = {0, 0, 0};
		//};

		class Sprite {
		public:
			std::shared_ptr<RESOURCES::Shader> shader;
			std::shared_ptr<RESOURCES::Texture> texture;

			unsigned int quadVAO;
			Sprite(std::shared_ptr<RESOURCES::Texture> texture);
			~Sprite();
			void draw();

			MATHGL::Vector3 position = {400, 300, 0};
			MATHGL::Vector3 scale = {1, 1, 1};
			MATHGL::Vector3 rotation = {0, 0, 0};
			MATHGL::Vector4 color = {1, 1, 1, 1};
			MATHGL::Vector2f anchor;
			MATHGL::Vector2f pivot;
		};

		class GuiObject {
		public:
			float childOffsetX = 0.0f;
			float childOffsetY = 0.0f;
			std::string name = "guiNode";
			GuiObject* parent = nullptr;
			std::vector<std::shared_ptr<GuiObject>> childs;
			std::shared_ptr<TransformComponentGui> transform;
			
			GuiObject() {
				static int id = 0;
				name += std::to_string(id);
				id++;
				transform = std::make_shared<TransformComponentGui>(*this);
				setDirty(true);
			}

			virtual void onCreate() {}
			virtual void onDestroy() {}
			virtual void onEnabled() {}
			virtual void onDisable() {}
			virtual void onPreUpdate(float dt) {
				transform->calculate();
				for (auto& ch : childs) {
					ch->onPreUpdate(dt);
				}
			}
			virtual void onUpdate(float dt) {
				for (auto& ch : childs) {
					ch->onUpdate(dt);
				}
			}
			virtual void onEvent(GuiEventType etype) {}

			void setPos(float x, float y) {
				transform->position = {x, y};
				//setDirty(true);
				//calculateTransform();
			}

			void setScale(float x, float y) {
				transform->scale = {x, y};
				setDirty(true);
				calculateTransform();
			}
			void setRotation(float r) {
				transform->rotation = r;
				setDirty(true);
				calculateTransform();
			}

			void calculateTransformOwn() {
				transform->calculate();
				transform->isDirty = false;
			}
			
			virtual void calculateTransform() {
				if (transform->isDirty) {
					calculateTransformOwn();
				}
				for (auto& child : childs) {
					child->calculateTransform();
				}
			}

			void setDirty(bool b) {
				transform->isDirty = b;
				for (auto& child : childs) {
					child->setDirty(b);
				}
			}

			void addChild(std::shared_ptr<GuiObject> child) {
				child->setDirty(true);
				child->parent = this;
				childs.push_back(child);
			}
			
		};

		class GuiButton: public GuiObject {
		public:
			std::shared_ptr<SpriteComponentGui> sprite;
			std::shared_ptr<LabelComponentGui> label;
			std::shared_ptr<InteractionComponentGui> interaction;
		
			std::function<void()> onClick;
			GuiButton();
		
			GuiEventType cur = GuiEventType::NONE;
			virtual void onEvent(GuiEventType etype) override;
			virtual void onUpdate(float dt) override;
		};

		class GuiLabel : public GuiObject {
		public:
			std::shared_ptr<LabelComponentGui> label;
			GuiLabel(std::string text, std::shared_ptr<Font> font, std::shared_ptr<RENDER::Material> material);
			virtual void onUpdate(float dt) override;

		};

		
		class GuiImage : public GuiObject {
		public:
			std::shared_ptr<SpriteComponentGui> sprite;
			GuiImage(std::shared_ptr<RENDER::Material> material);
			virtual void onUpdate(float dt) override;
			void setProjection(float w, float h);
		};

		class GuiLayout : public GuiObject {
		public:
			GuiLayout() :GuiObject() {}
			//TODO: move to layout component
			enum class Type {
				HORIZONTAL,
				VERTICAL
			};
			Type type = Type::HORIZONTAL;

			float hOffset = 0.0f;
			float vOffset = 0.0f;

			//TODO: move to transform
			float width = 0.0f;
			float height = 0.0f;

			virtual void onPreUpdate(float dt) override {
				//if (transform->isDirty) {
					//calculateTransformOwn();
					transform->calculate();
				//}
				width = 0.0f;
				height = 0.0f;
				auto startX = transform->globalModel(0, 3) + hOffset;
				auto startY = transform->globalModel(1, 3) + vOffset;
				for (auto& child : childs) {
					//child->calculateTransform(); // to known width and height
					//if (!child->transform->isDirty) {
					//	for (auto& ch : child->childs) {
					//		//ch->setDirty(true);
					//		ch->onPreUpdate(dt);
					//	}
					//	continue;
					//}
					if (type == Type::HORIZONTAL) {
						child->transform->globalModel(1, 3) = (transform->globalModel(1, 3) + vOffset) + 
							child->transform->position.y;
						child->transform->globalModel(0, 3) = startX + child->transform->position.x + hOffset;
						startX += hOffset + child->transform->size.x * transform->globalModel(0, 0);
						width += child->transform->size.x * child->transform->globalModel(0, 0) + hOffset;
						height = std::max<float>(height, child->transform->size.y * child->transform->globalModel(1, 1));
					}
					else {
						child->transform->globalModel(0, 3) = (transform->globalModel(0, 3) + hOffset) + 
							child->transform->position.x;
						child->transform->globalModel(1, 3) = startY + child->transform->position.y;
						startY += child->transform->size.y * child->transform->globalModel(1, 1) + vOffset;
						height += child->transform->size.y * child->transform->globalModel(1, 1) + vOffset;
						width = std::max<float>(width, child->transform->size.x * child->transform->globalModel(0, 0));
					}

					//child->transform->scaleGlobal.x *= (transform->scaleGlobal.x * child->transform->scale.x);
					//child->transform->scaleGlobal.y *= (transform->scaleGlobal.y * child->transform->scale.y);
					//child->transform->rotateGlobal = static_cast<int>(transform->rotateGlobal + child->transform->rotate) % 360;

					child->transform->isDirty = false;
					for (auto& ch : child->childs) {
						ch->setDirty(true);
						ch->onPreUpdate(dt);
					}
					//child->calculateTransform();
				}

				transform->size = MATHGL::Vector2{width, height};

			}
		};

		
		

		class GuiClip: public GuiObject {
		public:
			float globalX = 0.0f;
			float globalY = 0.0f;
			std::shared_ptr<ClipComponentGui> clipComponent;
			GuiClip(float w, float h) {
				clipComponent = std::make_shared<ClipComponentGui>(*this, w, h);
			}
			virtual void onPreUpdate(float dt) override {
				transform->calculate();
				globalX = transform->globalModel(0, 3);
				globalY = transform->globalModel(1, 3);
				childOffsetX = globalX;
				childOffsetY = globalY;
				transform->globalModel(0, 3) = 0;
				transform->globalModel(1, 3) = 0;
				for (auto& ch : childs) {
					ch->onPreUpdate(dt);
				}
			}
			virtual void onUpdate(float dt) override;
		};
		
		class GuiScroll : public GuiObject {
		public:
			std::shared_ptr<InteractionComponentGui> interaction;
			std::shared_ptr<ScrollComponentGui> scroll;

			std::shared_ptr<GuiObject> selectedObj;
			MATHGL::Vector2f selectedObjPos;
			
			GuiScroll(float w, float h);

			virtual void onEvent(GuiEventType etype) override;
			virtual void onUpdate(float dt) override;
			virtual void onPreUpdate(float dt) override {
				transform->calculate();
				if (parent) {
					interaction->globalX = parent->childOffsetX;
					interaction->globalY = parent->childOffsetY;
				}
				for (auto& ch : childs) {
					ch->onPreUpdate(dt);
				}
			}
			bool contains(float left, float top, float width, float height, float x, float y) const;

			float prevShiftX = 0.0f;
			float prevShiftY = 0.0f;
		};
		
	} 
}

