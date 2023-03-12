#pragma once

#ifdef OPENGL_BACKEND
#include <memory>
#include <vector>

namespace KUMA::RENDER {
	class TextureGl;

	class FrameBufferGl {
	public:
		unsigned id = 0;

		void create(std::vector<std::shared_ptr<TextureGl>> textures, std::shared_ptr<TextureGl> depthTexture = nullptr);
		void create();

		void bind();
		void unbind();

		static void CopyDepth(const FrameBufferGl& from, const FrameBufferGl& to, unsigned w, unsigned h);

	};
}
#endif
