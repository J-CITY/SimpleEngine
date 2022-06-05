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
#include <typeindex>
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


		struct Character {
			glm::ivec2   Size;      // размер глифа
			glm::ivec2   Start;      // размер глифа
			glm::ivec2   Bearing;   // смещение от линии шрифта до верхнего/левого угла глифа
			unsigned int Advance;   // смещение до следующего глифа
		};

		struct Font {
			std::map<GLchar, Character> Characters;
			unsigned int texture;
			Font(std::string fontPath, int size);
		};

		class GuiObject {
		public:
			float childOffsetX = 0.0f;
			float childOffsetY = 0.0f;
			std::string name = "guiNode";
			GuiObject* parent = nullptr;
			std::vector<std::shared_ptr<GuiObject>> childs;
			std::shared_ptr<TransformComponentGui> transform;

			template<class T>
			void addComponent(std::shared_ptr<T> component) {
				components[typeid(T)] = component;
			}

			template<class T>
			std::shared_ptr<T> getComponent() {
				return std::dynamic_pointer_cast<T>(components[typeid(T)]);
			}

			std::unordered_map<std::type_index, std::shared_ptr<ComponentGui>> components;

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
				for (auto c : components) {
					c.second->onPreUpdate(dt);
				}
				for (auto& ch : childs) {
					ch->onPreUpdate(dt);
				}
			}
			virtual void onUpdate(float dt) {
				for (auto& c : components) {
					c.second->onUpdate(dt);
				}
				for (auto& c : components) {
					c.second->draw();
				}
				for (auto& ch : childs) {
					ch->onUpdate(dt);
				}
			}
			virtual void onPostUpdate(float dt) {
				for (auto& c : components) {
					c.second->onPostUpdate(dt);
				}
				for (auto& ch : childs) {
					ch->onPostUpdate(dt);
				}
			}

			void setPos(float x, float y) {
				transform->position = {x, y};
				setDirty(true);
				//calculateTransform();
			}

			void setScale(float x, float y) {
				transform->scale = {x, y};
				setDirty(true);
				//calculateTransform();
			}
			void setRotation(float r) {
				transform->rotation = r;
				setDirty(true);
				//calculateTransform();
			}
			void setAnchor(float x, float y) {
				transform->anchor = {x, y};
				setDirty(true);
				//calculateTransform();
			}
			void setRotation(float x, float y) {
				transform->pivot = {x, y};
				setDirty(true);
				//calculateTransform();
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
			GuiButton();
		};
		class GuiLabel : public GuiObject {
		public:
			GuiLabel(std::string text, std::shared_ptr<Font> font, std::shared_ptr<RENDER::Material> material);
		};
		class GuiImage : public GuiObject {
		public:
			GuiImage(std::shared_ptr<RENDER::Material> material);
		};

		class GuiLayout : public GuiObject {
		public:
			GuiLayout() :GuiObject() {
				addComponent(std::make_shared<LayoutComponentGui>(*this));
			}
			virtual void onPreUpdate(float dt) override {
				auto lc = getComponent<LayoutComponentGui>();
				//if (transform->isDirty) {
					//calculateTransformOwn();
					transform->calculate();
				//}
				float width = 0.0f;
				float height = 0.0f;
				auto startX = transform->globalModel(0, 3) + lc->hOffset;
				auto startY = transform->globalModel(1, 3) + lc->vOffset;
				for (auto& child : childs) {
					if (lc->type == Type::HORIZONTAL) {
						child->transform->globalModel(1, 3) = (transform->globalModel(1, 3) + lc->vOffset) +
							child->transform->position.y;
						child->transform->globalModel(0, 3) = startX + child->transform->position.x + lc->hOffset;
						startX += lc->hOffset + child->transform->size.x * transform->globalModel(0, 0);
						width += child->transform->size.x * child->transform->globalModel(0, 0) + lc->hOffset;
						height = std::max<float>(height, child->transform->size.y * child->transform->globalModel(1, 1));
					}
					else {
						child->transform->globalModel(0, 3) = (transform->globalModel(0, 3) + lc->hOffset) +
							child->transform->position.x;
						child->transform->globalModel(1, 3) = startY + child->transform->position.y;
						startY += child->transform->size.y * child->transform->globalModel(1, 1) + lc->vOffset;
						height += child->transform->size.y * child->transform->globalModel(1, 1) + lc->vOffset;
						width = std::max<float>(width, child->transform->size.x * child->transform->globalModel(0, 0));
					}
					
					child->transform->isDirty = false;
					for (auto& ch : child->childs) {
						ch->setDirty(true);
						ch->onPreUpdate(dt);
					}
				}
				transform->size = MATHGL::Vector2{width, height};
			}
		};

		class GuiClip: public GuiObject {
		public:
			GuiClip(float w, float h) {
				addComponent(std::make_shared<ClipComponentGui>(*this, w, h));
			}
			virtual void onUpdate(float dt) override;
		};
		
		class GuiScroll : public GuiObject {
		public:
			std::shared_ptr<GuiObject> selectedObj;
			MATHGL::Vector2f selectedObjPos;
			
			GuiScroll(float w, float h);

			bool contains(float left, float top, float width, float height, float x, float y) const;
			float prevShiftX = 0.0f;
			float prevShiftY = 0.0f;
		};
		
	} 
}

