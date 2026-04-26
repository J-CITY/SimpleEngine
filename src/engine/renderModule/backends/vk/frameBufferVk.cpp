#include "frameBufferVk.h"

#ifdef VULKAN_BACKEND
#include "textureVk.h"
#include "driverVk.h"

IKIGAI::RENDER::FrameBufferVk::FrameBufferVk(const std::vector<std::shared_ptr<TextureInterface>>& textures, std::shared_ptr<TextureInterface> depth) : mTextures(textures), mDepth(depth) {
	if (textures.empty()) {
		throw;
	}
	static size_t ID = 0;
	mId = ID;
	ID++;
	mWidth = mTextures[0]->getWidth();
	mHeight = mTextures[0]->getHeight();
}

const std::vector<std::shared_ptr<IKIGAI::RENDER::TextureInterface>>& IKIGAI::RENDER::FrameBufferVk::getTextures() const {
	return mTextures;
}

const std::shared_ptr<IKIGAI::RENDER::TextureInterface>& IKIGAI::RENDER::FrameBufferVk::getDepth() const {
	return mDepth;
}

#endif
