#pragma once

#include <vector>
#include <memory>

namespace IKIGAI::RENDER {
	class TextureInterface;

	class FrameBufferInterface {
	protected:
		size_t mWidth = 0;
		size_t mHeight = 0;
		unsigned mId = 0;
	public:
		virtual ~FrameBufferInterface() {}
		size_t getWidth() const {
			return mWidth;
		}
		size_t getHeight() const {
			return mHeight;
		}
		unsigned getId() const {
			return mId;
		}

		virtual const std::vector<std::shared_ptr<TextureInterface>>& getTextures() const = 0;
		virtual const std::shared_ptr<TextureInterface>& getDepth() const = 0;
	};
}
