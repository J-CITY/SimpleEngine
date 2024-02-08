#pragma once

#include "../../ecs/components/component.h"
#include <renderModule/spine/spineDrawable.h>
#include <rttr/registration_friend.h>

#include "renderModule/backends/gl/textureGl.h"


import glmath;

namespace IKIGAI
{
	namespace RESOURCES
	{
		class Shader;
	}
}

namespace IKIGAI
{
	namespace RENDER
	{
		class VertexArray;
		struct AtlasRect;
		class ShaderInterface;
		class VertexBufferInterface;
		class TextureInterface;
		class MaterialInterface;
		//class Material;
	}

	namespace GUI
	{
		class Font;
	}
}

namespace IKIGAI {
	namespace ECS {
		class Object;

		struct BatchVertex {
			MATHGL::Vector3 position;
			MATHGL::Vector2f texCoord;
			MATHGL::Vector4 color;
		};

		class SpriteBatcher
		{

		public:
			SpriteBatcher();
			~SpriteBatcher();

			void Draw(const std::array<MATHGL::Vector4, 6>& verts, const RENDER::AtlasRect& uv, MATHGL::Vector4 color,
				std::shared_ptr<RENDER::TextureInterface> texture, std::shared_ptr<RENDER::ShaderInterface> shader, bool is3D = false);
			void Draw(const std::array<BatchVertex, 6>& verts, std::shared_ptr<RENDER::TextureInterface> texture, std::shared_ptr<RENDER::ShaderInterface> shader, bool is3D = false);
			void Flush();
			

		private:

			// Collection of vertices
			std::vector<BatchVertex> mVertexBuffer;

			std::shared_ptr<RENDER::VertexBufferInterface> mVbo;
			std::shared_ptr<RENDER::VertexArray> mVertexArray;

			//unsigned m_vbo = 0;

			// Shaders.
			//Shader m_vertexShader;
			//Shader m_fragmentShader;

			// GL index for the shader program
			//unsigned m_shaderProgram = 0;

			// Texture and uniform location
			std::shared_ptr<RENDER::TextureInterface> mTexture;
			std::shared_ptr<RENDER::ShaderInterface> mShader;
			bool mIs3D = false;
			unsigned m_textureUniform = 0;

			// stuff to send word matrices
			//MATHGL::Matrix4 m_screenTransform;
			//unsigned m_screenTransformUniform = 0;
		};


		class RootGuiComponent: public Component {
		public:
			RootGuiComponent(Object& obj): Component(obj) {}
		};

		class ColorComponent : public Component {
		public:
			MATHGL::Vector4 mColor = { 1, 1, 1, 1 };
		};

		class SpriteComponent : public Component {
		public:
			SpriteComponent(Object& obj);
			SpriteComponent(Object& obj, const std::string& path);
			void setTexture(std::string path);
			void setTextureAtlas(std::string path);
			std::string getTexture();

			void setAtlasPiece(std::string name);

			std::string getAtlasPiece();

			MATHGL::Vector4 mColor = {1, 1, 1, 1};
			std::string mPath;
			std::shared_ptr<RENDER::TextureInterface> mTexture;
			std::string mTexturePiece;

			//move to component
			bool mIs3D = false;
		};

		class SpriteAnimateComponent : public SpriteComponent {
		public:
			SpriteAnimateComponent(Object& obj): SpriteComponent(obj) {};
			SpriteAnimateComponent(Object& obj, const std::string& path): SpriteComponent(obj, path) {};


			void setGridSize(MATHGL::Vector2f sz)
			{
				mGridSize = sz;
			}
			MATHGL::Vector2f getGridSize()
			{
				return mGridSize;
			}

			void setFrameCount(int e)
			{
				mFrameCount = e;
			}
			int getFrameCount()
			{
				return mFrameCount;
			}

			//TODO: move to system
			void updateAnim();
			void nextFrame();

			int mFrameCount = 0;
			int mCurrentFreme = 0;
			MATHGL::Vector2f mGridSize;

			float mTime = 1000 / 25.0f;
			float mCutTime = mTime;

			float mTimeScale = 200.0f;
			RENDER::AtlasRect mUVRect;
		};

		class SpriteParticleComponent : public SpriteComponent {
		public:
			SpriteParticleComponent(Object& obj);;
			SpriteParticleComponent(Object& obj, const std::string& path);;

			struct Particle
			{
				MATHGL::Vector3 pos;
				MATHGL::Vector3 speed;
				float life = 0.0f;
				float size = 0.0f, weight = 0.0f;

				int emmiterId = 0;
				bool isAlive = false;
			};

			struct Emmiter {
				int count = 0;

				MATHGL::Vector3 localPos;
				MATHGL::Vector3 gravity;
				MATHGL::Vector4 color;


				MATHGL::Vector2f size;
				MATHGL::Vector2f weight;
				MATHGL::Vector2f angle;
				MATHGL::Vector2f angleForce;
				MATHGL::Vector2f lifeTime;
				//TODO: param for change alpha

				std::string piece;

				float spawnTime = 0.0f;
				int spawnCount = 0;
				float curSpawnTime = 0.0f;
				int curParticleCount = 0;
			};

			void setEmmiters(std::vector<Emmiter> e);

			std::vector<Emmiter> getEmmiters();

			void spawnParticle(Emmiter& emmiter, int i);
			//TODO: move to system
			void update();

			//TODO: add play/pause, add support animation, add change alpha by life time

			std::vector<Particle> particles;
			std::vector<int> freeParticlesId;

			std::vector<Emmiter> emmiters;
		};

		class LabelComponent : public Component {
		public:
			LabelComponent(Object& obj, std::string label, std::shared_ptr<GUI::Font> font);
			std::string mLabel;
			std::shared_ptr<GUI::Font> font;
			MATHGL::Vector4 color = {1, 1, 1, 1};

			//move to component
			bool mIs3D = false;
		};

		struct SpineRefl
		{
			std::string skelPath;
			std::string atlasPath;
		};
		class SpineComponent : public Component {
			RTTR_REGISTRATION_FRIEND

			void setSpine(SpineRefl data);

			SpineRefl getSpine();

		public:
			SpineComponent(Object& obj);
			SpineComponent(Object& obj, std::string skelPath, std::string atlasPath);
			std::shared_ptr<RENDER::SPINE::SpineController> spine;

			std::string skelPath;
			std::string atlasPath;

			//move to component TODO:add support 3d in shader
			bool mIs3D = false;
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

