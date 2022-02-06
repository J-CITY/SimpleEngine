#pragma once
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Texture.hpp>

#include "../../config.h"
#include "../../inputManager/inputManager.h"
#include "../../resourceManager/ServiceManager.h"
#include "../../window/window.h"

namespace KUMA
{
	namespace GUI
	{
		/*
		 * Progress
		 * Text edit
		 *
		 * primitive component
		 * 
		 * serealize/deserealize
		 */
		class GuiObject;

		enum class EAlign {
			LEFT,
			RIGHT,
			TOP,
			BOTTOM,
			CENTER
		};
		
		class ComponentGui {
		public:

			class Position {
			public:
				EAlign haligh = EAlign::CENTER;
				EAlign valigh = EAlign::CENTER;

				sf::Vector2f pos;
				sf::Vector2f scale = {1.0f, 1.0f};
				float rotation = 0.0f;
			};
			ComponentGui(GuiObject& obj) : obj(obj) {}
			
			bool isEnabled = true;
			virtual void draw(KUMA::WINDOW_SYSTEM::Window& win) {}

			virtual void onCreate() {
				isEnabled = true;
			}
			virtual void onDestroy() {}
			virtual void onEnabled() {
				isEnabled = true;
			}
			virtual void onDisable() {
				isEnabled = false;
			}
			virtual void onUpdate(float) {}

			virtual void calculateOwnPos() = 0;
			
			void _calculateOwnPos(float w, float h);
			
			GuiObject& obj;
			Position pos;
			Position posGlobal;
		};

		class TransformComponentGui : public ComponentGui {
		public:
			bool isDirty = true;

			EAlign haligh = EAlign::LEFT;
			EAlign valigh = EAlign::TOP;
			
			sf::Vector2f pos;
			sf::Vector2f scale = {1.0f, 1.0f};
			float rotate = 0.0f;

			sf::Vector2f posGlobal;
			sf::Vector2f scaleGlobal = {1.0f, 1.0f};;
			float rotateGlobal = 0.0f;
			TransformComponentGui(GuiObject& obj);

			void calculateOwnPos() {}
		};

		class SpriteComponentGui : public ComponentGui {
		public:
			sf::Texture texture;
			sf::Sprite sprite;
			std::string path;

			SpriteComponentGui(GuiObject& obj, std::string path);
			void draw(KUMA::WINDOW_SYSTEM::Window& win) override;

			void calculateOwnPos() override {
				_calculateOwnPos(sprite.getGlobalBounds().width, sprite.getGlobalBounds().height);
				sprite.setPosition(posGlobal.pos);
				sprite.setScale(posGlobal.scale);
				sprite.setRotation(posGlobal.rotation);
			}
		};

		class InteractionComponentGui : public ComponentGui {
		public:

			sf::FloatRect rect;
			InteractionComponentGui(GuiObject& obj, sf::FloatRect rect);
			void onUpdate(float dt) override;

			void calculateOwnPos() override {
				_calculateOwnPos(rect.width, rect.height);
				rect.left = posGlobal.pos.x;
				rect.top = posGlobal.pos.y;
				rect.width = rect.width * posGlobal.scale.x;
				rect.height = rect.height * posGlobal.scale.y;
			}
		private:
			bool contains(float x, float y) const;
		};

		class LabelComponentGui : public ComponentGui {
		public:
			std::string label;

			sf::Text text;
			sf::Font font;
			LabelComponentGui(GuiObject& obj, std::string label);
			void draw(KUMA::WINDOW_SYSTEM::Window& win) override;

			void calculateOwnPos() override {
				_calculateOwnPos(text.getGlobalBounds().width, text.getGlobalBounds().height);
				text.setPosition(posGlobal.pos);
				text.setScale(posGlobal.scale);
				text.setRotation(posGlobal.rotation);
			}
		};

		class ClipComponentGui : public ComponentGui {
		public:
			sf::View view;
			
			float width = 0.0f;
			float height = 0.0f;
			ClipComponentGui(GuiObject& obj, float w, float h): ComponentGui(obj),
				width(w), height(h), view(sf::FloatRect(0.f, 0.f, 10.f, 10.f)) {}
			void draw(KUMA::WINDOW_SYSTEM::Window& win) override;

			void calculateOwnPos() override;
		};

		class ScrollComponentGui : public ComponentGui {
		public:
			bool isScrollHorizontal = true;
			bool isScrollVertical = true;
			float width = 0.0f;
			float height = 0.0f;


			float startX = 0.0f;
			float startY = 0.0f;
			bool isPress = false;

			ScrollComponentGui(GuiObject& obj, float w, float h) : ComponentGui(obj),
				width(w), height(h) {
			}

			//todo:
			void calculateOwnPos() override {}
		};

		

		
	}
}

