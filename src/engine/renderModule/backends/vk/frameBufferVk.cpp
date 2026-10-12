#include "frameBufferVk.h"

#ifdef VULKAN_BACKEND
#include "textureVk.h"
#include "driverVk.h"
#include <stdexcept>

IKIGAI::RENDER::FrameBufferVk::FrameBufferVk(const std::vector<std::shared_ptr<TextureInterface>>& textures, std::shared_ptr<TextureInterface> depth, unsigned mip, unsigned layer) : mTextures(textures), mDepth(depth) {
	mMip=mip; mLayer=layer;
	if (textures.empty() && !depth) {
		throw std::runtime_error("Framebuffer needs a color or depth attachment");
	}
	static size_t ID = 0;
	mId = ID;
	ID++;
	auto sizeTexture = mTextures.empty() ? mDepth : mTextures.front();
	mWidth = std::max<size_t>(1,sizeTexture->getWidth() >> mMip);
	mHeight = std::max<size_t>(1,sizeTexture->getHeight() >> mMip);
}

const std::vector<std::shared_ptr<IKIGAI::RENDER::TextureInterface>>& IKIGAI::RENDER::FrameBufferVk::getTextures() const {
	return mTextures;
}

const std::shared_ptr<IKIGAI::RENDER::TextureInterface>& IKIGAI::RENDER::FrameBufferVk::getDepth() const {
	return mDepth;
}

#endif
