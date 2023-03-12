#pragma once

#include "../../ecs/components/component.h"

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
		class TextureInterface;
		class MaterialInterface;
		//class Material;
	}

	namespace GUI
	{
		class Font;
	}
}

namespace KUMA {
	namespace ECS {
		class Object;

		class RootGuiComponent: public Component {
		public:
			RootGuiComponent(Object& obj): Component(obj) {}
		};

		class SpriteComponent : public Component {
		public:
			SpriteComponent(Object& obj, const std::string& path);
			MATHGL::Vector4 mColor = {1, 1, 1, 1};
			std::string mPath;
			std::shared_ptr<RENDER::TextureInterface> mTexture;
		};

		class LabelComponent : public Component {
		public:
			LabelComponent(Object& obj, std::string label, std::shared_ptr<GUI::Font> font);
			std::string mLabel;
			std::shared_ptr<GUI::Font> font;
			MATHGL::Vector4 color = {1, 1, 1, 1};
		};

		enum class GuiEventType {
			NONE,
			COVER,
			PRESS,
			PRESS_CONTINUE,
			RELEASE,
			UNCOVER
		};
		class InteractionComponent : public Component {
		public:
			GuiEventType mCurEvent = GuiEventType::NONE;

			std::function<void()> mOnPress;
			std::function<void()> mOnPressContinue;
			std::function<void()> mOnRelease;
			std::function<void()> mOnCover;
			std::function<void()> mOnUncover;

			float mGlobalX = 0.0f;
			float mGlobalY = 0.0f;

			float mWidth = 0.0f;
			float mHeight = 0.0f;
			InteractionComponent(Object& obj, float w, float h);
			
		//private:
			bool contains(float x, float y);
		};

		class ClipComponent : public Component {
		public:
			float mGlobalX = 0.0f;
			float mGlobalY = 0.0f;

			float mWidth = 0.0f;
			float mHeight = 0.0f;
			ClipComponent(Object& obj, float w, float h);
		};

		class ScrollComponent : public Component {
		public:
			bool mIsScrollHorizontal = true;
			bool mIsScrollVertical = false;
			float mWidth = 0.0f;
			float mHeight = 0.0f;


			float mStartX = 0.0f;
			float mStartY = 0.0f;
			bool mIsPress = false;

			std::shared_ptr<Object> mSelectedObj;
			MATHGL::Vector2f mSelectedObjPos;

			ScrollComponent(Object& obj, float w, float h);
		};
		
		class LayoutComponent : public Component {
		public:
			enum class Type {
				HORIZONTAL,
				VERTICAL
			};
			Type mType = Type::HORIZONTAL;

			float mHorizontalOffset = 0.0f;
			float mVerticalOffset = 0.0f;

			LayoutComponent(Object& obj, Type type) : Component(obj), mType(type) {}
		};
	}
}

