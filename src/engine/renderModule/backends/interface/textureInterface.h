#pragma once

namespace IKIGAI::RENDER {
	enum class TextureType {
		TEXTURE_2D,
		TEXTURE_3D,
		TEXTURE_CUBE,
		TEXTURE_2D_ARRAY,
	};

	class TextureInterface {
	protected:
		TextureType mType = TextureType::TEXTURE_2D;
		std::string mPath;
		size_t mWidth = 0;
		size_t mHeight = 0;
		size_t mDepth = 1;
		size_t mChannels = 1;
	public:
		TextureType getType() const { return mType; }
		const std::string& getPath() const { return mPath; }
		float getWidth() const { return mWidth; }
		float getHeight() const { return mHeight; }
		float getDepth() const { return mDepth; }
		size_t getChannels() const { return mChannels; }

		virtual ~TextureInterface() = default;


		virtual void* getImguiId() = 0;
	};
}
