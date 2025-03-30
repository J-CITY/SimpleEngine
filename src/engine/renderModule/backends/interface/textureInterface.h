#pragma once
#include "renderEnums.h"
namespace IKIGAI::RENDER {
	enum class TextureType {
		TEXTURE_2D,
		TEXTURE_3D,
		TEXTURE_CUBE,
		TEXTURE_2D_ARRAY,
		DEPTH
	};

	class TextureInterface {
	protected:
		std::string mPath;

		TextureType mType = TextureType::TEXTURE_2D;
		PixelFormat mFormat = PixelFormat::RGBA_INT;

		size_t mWidth = 0;
		size_t mHeight = 0;
		size_t mDepth = 1;
		size_t mChannels = 1;

		uint32_t mMipCount = 0;

		MinMagFilter mMinFilter = MinMagFilter::LINEAR;
		MinMagFilter mMagFilter = MinMagFilter::LINEAR;

		WrapFilter mWrapS = WrapFilter::CLAMP_TO_EDGE;
		WrapFilter mWrapT = WrapFilter::CLAMP_TO_EDGE;
		WrapFilter mWrapR = WrapFilter::CLAMP_TO_EDGE;

	public:
		TextureType getType() const { return mType; }
		PixelFormat getFormat() const { return mFormat; }
		const std::string& getPath() const { return mPath; }
		size_t getWidth() const { return mWidth; }
		size_t getHeight() const { return mHeight; }
		size_t getDepth() const { return mDepth; }
		size_t getChannels() const { return mChannels; }

		virtual ~TextureInterface() = default;
		virtual void* getImguiId() = 0;
	};
}
