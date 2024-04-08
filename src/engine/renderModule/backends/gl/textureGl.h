#pragma once
#include <map>
#include <vector>

#ifdef OPENGL_BACKEND
#include <memory>
#include <string>

#include "../interface/textureInterface.h"
#include <serdepp/serde.hpp>
namespace IKIGAI::RENDER
{
	struct TextureResource;

	class TextureGl : public TextureInterface {
	public:
		TextureGl() {}
		~TextureGl() override;
		unsigned id = 0;
		int slot = 0;
		

		static std::shared_ptr<TextureGl> Create(const std::string& path, bool generateMipmap);
		static std::shared_ptr<TextureGl> CreateFromResource(const RENDER::TextureResource& res);
		static std::shared_ptr<TextureGl> CreateHDREmptyCubemap(int width, int height);
		static std::shared_ptr<TextureGl> CreateHDR(const std::string& path, bool generateMipmap);
		static void CopyTexture(const TextureGl& from, const TextureGl& to);
		static std::shared_ptr<TextureGl> CreateFromMemory(uint8_t* data, uint32_t width, uint32_t height, bool generateMipmap);
		static std::shared_ptr<TextureGl> CreateForAttach(int texWidth, int texHeight, int type);
		static std::shared_ptr<TextureGl> CreateDepthForAttach(unsigned int texWidth, unsigned int texHeight);
		static std::shared_ptr<TextureGl> CreateCubemap(std::array<std::string, 6> path);
		static std::shared_ptr<TextureGl> CreateDepthForAttachCubemap(int texWidth, int texHeight, int type);
		static std::shared_ptr<TextureGl> CreateDepthForAttach2DArray(int texWidth, int texHeight, int arrSize);
		static std::shared_ptr<TextureGl> CreateEmpty3d(int texX, int texY, int texZ);
		static std::vector<unsigned char> GetPixels(const std::string& path);

		void bind(int slot);
		void unbind();
		void generateMipmaps();

		void bindImage(uint32_t unit, uint32_t mip_level, uint32_t layer, unsigned access, unsigned format);

		virtual void* getImguiId() override;
	};

	//TODO: add global struct and delete it
	struct AtlasRect {
		AtlasRect() = default;
		AtlasRect(float x, float y, float w, float h) : mX(x), mY(y), mW(w), mH(h) {};
		float getX()
		{
			return mX;
		}
		void setX(float x)
		{
			mX = x;
		}
		float mX = 0.0f;
		float mY = 0.0f;
		float mW = 0.0f;
		float mH = 0.0f;

		template<class Context>
		constexpr static auto serde(Context& context, AtlasRect& value) {
			using Self = AtlasRect;
			using namespace serde::attribute;
			serde::serde_struct(context, value)
				.field(&Self::mX, "X")
				.field(&Self::mY, "Y")
				.field(&Self::mW, "W")
				.field(&Self::mH, "H");
		}
	};

	struct AtlasData {
		AtlasData() = default;
		std::map<std::string, AtlasRect> mRects;
		std::string mPath;

		template<class Context>
		constexpr static auto serde(Context& context, AtlasData& value) {
			using Self = AtlasData;
			using namespace serde::attribute;
			serde::serde_struct(context, value)
				.field(&Self::mRects, "Files")
				.field(&Self::mPath, "Path");
		}

		void setRects(std::map<std::string, AtlasRect> rects)
		{
			mRects = rects;
		}

		std::map<std::string, AtlasRect> getRects()
		{
			return mRects;
		}
	};


	class TextureAtlas: public TextureGl {
	public:
		
	private:
		AtlasData mAtlas;
	public:
		TextureAtlas(): TextureGl() {}

		[[nodiscard]] AtlasRect getPiece(const std::string& name) const;
		[[nodiscard]] AtlasRect getPieceUV(const std::string& name) const;

		static std::shared_ptr<TextureAtlas> CreateAtlas(const std::string& path, bool generateMipmap);
		//static std::shared_ptr<TextureAtlas> CreateAtlasFromResource(const RENDER::TextureResource& res);
	};
}
#endif
