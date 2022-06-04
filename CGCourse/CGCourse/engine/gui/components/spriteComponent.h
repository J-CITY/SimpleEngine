#pragma once

#include "../../config.h"
#include "../../inputManager/inputManager.h"
#include "../../resourceManager/ServiceManager.h"
//#include "../../window/window.h"

namespace KUMA
{
	namespace RESOURCES
	{
		class Shader;
	}
}

namespace KUMA
{
	namespace RENDER
	{
		class Material;
	}
}

namespace KUMA {
	namespace GUI {
		struct Font;
		class GuiObject;

		class ComponentGui {
		public:
			virtual ~ComponentGui() = default;
			ComponentGui(GuiObject& obj) : obj(obj) {}
			
			bool isEnabled = true;
			virtual void draw() {}

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
			
			GuiObject& obj;
		};

		class TransformComponentGui : public ComponentGui {
		public:
			bool isDirty = true;
			
			MATHGL::Vector3  position = {0, 0, 0};
			MATHGL::Vector3  scale = {1, 1, 1};
			MATHGL::Vector3  rotation = {0, 0, 0};
			MATHGL::Vector4  color = {1, 1, 1, 1};
			MATHGL::Vector2f anchor;
			MATHGL::Vector2f pivot;

			MATHGL::Vector3  globalPosition = {0, 0, 0};
			MATHGL::Vector3  globalScale = {1, 1, 1};
			MATHGL::Vector3  globalRotation = {0, 0, 0};
			MATHGL::Vector2f globalAnchor;
			MATHGL::Vector2f globalPivot;

			MATHGL::Vector2f size;

			TransformComponentGui(GuiObject& obj);

			void calculate();
			MATHGL::Matrix4 model;
			MATHGL::Matrix4 globalModel;
		};

		class SpriteComponentGui : public ComponentGui {
		public:
			SpriteComponentGui(GuiObject& obj, std::shared_ptr<RENDER::Material> material);
			void draw() override;
			std::shared_ptr<RESOURCES::Shader> shader;
		//private:
			MATHGL::Vector4 color = {1, 1, 1, 1};
			unsigned int quadVAO;
			std::shared_ptr<RENDER::Material> material;
		};
		class LabelComponentGui : public ComponentGui {
		public:
			std::string label;
			std::shared_ptr<Font> font;
			std::shared_ptr<RENDER::Material> material;
			unsigned int VAO, VBO;

			MATHGL::Vector4 color = {1, 1, 1, 1};

			LabelComponentGui(GuiObject& obj, std::string label, std::shared_ptr<Font> font, std::shared_ptr<RENDER::Material> material);
			void draw() override;

		};
		class InteractionComponentGui : public ComponentGui {
		public:
			float globalX = 0.0f;
			float globalY = 0.0f;
			InteractionComponentGui(GuiObject& obj, MATHGL::Vector2f size);
			void onUpdate(float dt) override;
		private:
			bool contains(float x, float y) const;
		};

		

		class ClipComponentGui : public ComponentGui {
		public:
			float width = 0.0f;
			float height = 0.0f;
			ClipComponentGui(GuiObject& obj, float w, float h);
			void draw() override;
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
			
		};
		
		

		
	}
}

