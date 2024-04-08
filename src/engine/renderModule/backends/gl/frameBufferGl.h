#pragma once
#include "renderModule/backends/interface/frameBufferInterface.h"

#ifdef OPENGL_BACKEND
#include <memory>
#include <vector>

namespace IKIGAI::RENDER {
	class TextureGl;

	class FrameBufferGl: public FrameBufferInterface {
	public:
		unsigned id = 0;

		FrameBufferGl();
		~FrameBufferGl();

		void create(std::vector<std::shared_ptr<TextureGl>> textures, std::shared_ptr<TextureGl> depthTexture = nullptr);
		void create();

		void bind();
		void unbind();

		static void CopyDepth(const FrameBufferGl& from, const FrameBufferGl& to, unsigned w, unsigned h);

	};
}
#endif
