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

namespace KUMA {
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
		class GuiObject {
		public:
			GuiObject* parent = nullptr;
			std::vector<std::shared_ptr<GuiObject>> childs;

			std::shared_ptr<TransformComponentGui> transform;
			
			GuiObject() {
				transform = std::make_shared<TransformComponentGui>(*this);
			}

			virtual float getWidht() = 0;
			virtual float getHeight() = 0;
			virtual void applyTransform() = 0;
			
			virtual void onCreate() {}
			virtual void onDestroy() {}
			virtual void onEnabled() {}
			virtual void onDisable() {}
			virtual void onUpdate(KUMA::CORE_SYSTEM::Core& context, float dt) {
				for (auto& ch : childs) {
					ch->onUpdate(context, dt);
				}
			}
			virtual void onEvent(GuiEventType etype) {}

			void setPos(float x, float y) {
				transform->pos = {x, y};
				setDirty(true);
				calculateTransform();
			}

			void setScale(float x, float y) {
				transform->scale = {x, y};
				setDirty(true);
				calculateTransform();
			}
			void setRotation(float r) {
				transform->rotate = r;
				setDirty(true);
				calculateTransform();
			}

			void calculateTransformOwn() {
				if (parent) {
					transform->scaleGlobal.x = parent->transform->scaleGlobal.x * transform->scale.x;
					transform->scaleGlobal.y = parent->transform->scaleGlobal.y * transform->scale.y;
					transform->rotateGlobal = static_cast<int>(parent->transform->rotateGlobal + transform->rotate) % 360;

					if (transform->haligh == EAlign::LEFT) {
						transform->posGlobal.x = parent->transform->posGlobal.x + transform->pos.x;
					}
					else if (transform->haligh == EAlign::CENTER) {
						transform->posGlobal.x = parent->transform->posGlobal.x + parent->getWidht() / 2.0f + transform->pos.x - getWidht() / 2.0f;
					}
					else if (transform->haligh == EAlign::RIGHT) {
						transform->posGlobal.x = parent->transform->posGlobal.x + parent->getWidht() + transform->pos.x - getWidht();
					}
					if (transform->valigh == EAlign::TOP) {
						transform->posGlobal.y = parent->transform->posGlobal.y + transform->pos.y;
					}
					else if (transform->valigh == EAlign::CENTER) {
						transform->posGlobal.y = parent->transform->posGlobal.y + parent->getHeight() / 2.0f + transform->pos.y - getHeight() / 2.0f;
					}
					else if (transform->valigh == EAlign::BOTTOM) {
						transform->posGlobal.y = parent->transform->posGlobal.y + parent->getHeight() + transform->pos.y - getHeight();
					}
				}
				else {
					auto windowSize = RESOURCES::ServiceManager::Get<WINDOW_SYSTEM::Window>().getSize();
					transform->scaleGlobal.x = transform->scale.x;
					transform->scaleGlobal.y = transform->scale.y;
					transform->rotateGlobal = transform->rotate;

					if (transform->haligh == EAlign::LEFT) {
						transform->posGlobal.x = transform->pos.x;
					}
					else if (transform->haligh == EAlign::CENTER) {
						transform->posGlobal.x = windowSize.x / 2.0f + transform->pos.x - getWidht() / 2.0f;
					}
					else if (transform->haligh == EAlign::RIGHT) {
						transform->posGlobal.x = windowSize.x + transform->pos.x - getWidht();
					}
					if (transform->valigh == EAlign::TOP) {
						transform->posGlobal.y = transform->pos.y;
					}
					else if (transform->valigh == EAlign::CENTER) {
						transform->posGlobal.y = windowSize.y / 2.0f + transform->pos.y - getHeight() / 2.0f;
					}
					else if (transform->valigh == EAlign::BOTTOM) {
						transform->posGlobal.y = windowSize.y + transform->pos.y - getHeight();
					}
				}
				applyTransform();
				transform->isDirty = false;
			}
			
			virtual void calculateTransform() {
				if (transform->isDirty) {
					calculateTransformOwn();
				}
				else {
					applyTransform();
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
			virtual void onUpdate(KUMA::CORE_SYSTEM::Core& context, float dt) override;

			void applyTransform() override {
				sprite->calculateOwnPos();
				label->calculateOwnPos();
				interaction->calculateOwnPos();
			}
			virtual float getWidht() override {
				return sprite->sprite.getGlobalBounds().width;
			}
			virtual float getHeight() override {
				return sprite->sprite.getGlobalBounds().height;
			};
		};

		class GuiLabel : public GuiObject {
		public:
			std::shared_ptr<LabelComponentGui> label;
			GuiLabel();
			virtual void onUpdate(KUMA::CORE_SYSTEM::Core& context, float dt) override;
			void applyTransform() override {
				label->calculateOwnPos();
			}
			virtual float getWidht() override {
				return label->text.getGlobalBounds().width;
			}
			virtual float getHeight() override {
				return label->text.getGlobalBounds().height;
			};
		};

		
		class GuiImage : public GuiObject {
		public:
			std::shared_ptr<SpriteComponentGui> sprite;
			GuiImage();
			virtual void onUpdate(KUMA::CORE_SYSTEM::Core& context, float dt) override;
			void applyTransform() override {
				sprite->calculateOwnPos();
			}
			virtual float getWidht() override {
				return sprite->sprite.getGlobalBounds().width;
			}
			virtual float getHeight() override {
				return sprite->sprite.getGlobalBounds().height;
			};
		};

		class GuiLayout: public GuiObject {
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

			virtual void calculateTransform() override {
				if (transform->isDirty) {
					calculateTransformOwn();
				}
				width = 0.0f;
				height = 0.0f;
				auto startX = transform->posGlobal.x + hOffset;
				auto startY = transform->posGlobal.y + vOffset;
				for (auto& child : childs) {
					child->calculateTransform(); // to known width and height
					if (type == Type::HORIZONTAL) {
						child->transform->posGlobal.y = transform->posGlobal.y + vOffset * transform->scaleGlobal.y + child->transform->pos.y;
						child->transform->posGlobal.x = startX + child->transform->pos.x;
						startX += child->getWidht() + hOffset * transform->scaleGlobal.x;
						width += child->getWidht();
						height = std::max<float>(height, child->getHeight());
					}
					else {
						child->transform->posGlobal.x = transform->posGlobal.x + hOffset * transform->scaleGlobal.x + child->transform->pos.x;
						child->transform->posGlobal.y = startY + child->transform->pos.y;
						startY += child->getHeight() + vOffset * transform->scaleGlobal.y;
						height += child->getHeight();
						width = std::max<float>(height, child->getWidht());
					}
					
					child->transform->scaleGlobal.x *= (transform->scaleGlobal.x * child->transform->scale.x);
					child->transform->scaleGlobal.y *= (transform->scaleGlobal.y * child->transform->scale.y);

					child->transform->rotateGlobal = static_cast<int>(transform->rotateGlobal + child->transform->rotate) % 360;

					child->transform->isDirty = false;
					for (auto& ch : child->childs) {
						ch->setDirty(true);
					}
					child->calculateTransform();
				}
			}

			float getWidht() override {
				return width * transform->scaleGlobal.x;
			}
			float getHeight() override {
				return height * transform->scaleGlobal.y;
			}

			void applyTransform() override {

			}
		};

		class GuiClip: public GuiObject {
		public:

			std::shared_ptr<ClipComponentGui> clipComponent;
			GuiClip(float w, float h) {
				clipComponent = std::make_shared<ClipComponentGui>(*this, w, h);
			}


			virtual void onUpdate(KUMA::CORE_SYSTEM::Core& context, float dt) override;
			void applyTransform() override {
				clipComponent->calculateOwnPos();
			}
			virtual float getWidht() override {
				return clipComponent->width;
			}
			virtual float getHeight() override {
				return clipComponent->height;
			};
		};

		class GuiScroll : public GuiObject {
		public:
			std::shared_ptr<InteractionComponentGui> interaction;
			std::shared_ptr<ScrollComponentGui> scroll;

			std::shared_ptr<GuiObject> selectedObj;
			sf::Vector2f selectedObjPos;
			
			GuiScroll(float w, float h);

			virtual void onEvent(GuiEventType etype) override;
			virtual void onUpdate(KUMA::CORE_SYSTEM::Core& context, float dt) override;

			bool contains(sf::FloatRect rect, float x, float y) const;
			void applyTransform() override {
				interaction->calculateOwnPos();
			}
			virtual float getWidht() override {
				return scroll->width;
			}
			virtual float getHeight() override {
				return scroll->height;
			};

			float prevShiftX = 0.0f;
			float prevShiftY = 0.0f;
		};
		
		/*class GuiObject {
		public:
			GuiObject(std::string path, KUMA::MATHGL::Vector2 pos, KUMA::MATHGL::Vector2 scale, std::shared_ptr<KUMA::RESOURCES::Shader> shader) :
				pos(pos), scale(scale), shader(shader) {
				obj = std::make_shared<KUMA::ECS::Object>(ObjectId<ECS::Object>(1000), "guiElem", "");

				obj->transform->setLocalPosition({pos.x, pos.y, 0.0f});
				obj->transform->setLocalRotation({0.0f, 0.0f, 0.0f, 1.0f});
				obj->transform->setLocalScale({scale.x, scale.y, 1.0f});

				tex = KUMA::RESOURCES::TextureLoader().createResource(path);

				mat = std::make_unique<KUMA::RENDER::Material>();
				mat->setShader(shader);
				
				auto model = obj->addComponent<KUMA::ECS::ModelRenderer>();
				KUMA::RESOURCES::ModelParserFlags flags = KUMA::RESOURCES::ModelParserFlags::TRIANGULATE;
				auto m = KUMA::RESOURCES::ModelLoader::Create("Cube.fbx", flags);
				model->setModel(m);
				model->setFrustumBehaviour(KUMA::ECS::ModelRenderer::EFrustumBehaviour::CULL_MODEL);
			}
			
			std::shared_ptr<KUMA::RESOURCES::Shader> shader;
			std::unique_ptr<KUMA::RENDER::Material> mat;
			std::shared_ptr<KUMA::RESOURCES::Texture> tex;
			int texture;
			KUMA::MATHGL::Vector2 pos;
			KUMA::MATHGL::Vector2 scale;
			std::shared_ptr<KUMA::ECS::Object> obj;
		};*/
	}
}

