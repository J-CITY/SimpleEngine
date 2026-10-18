#pragma once
#include "gpuMemoryGl.h"
#include "renderModule/backends/interface/frameBufferInterface.h"

#ifdef OPENGL_BACKEND
#include <memory>
#include <vector>

namespace IKIGAI::RENDER {
	void BindDefaultFramebufferGl();
	class TextureGl;

	class FrameBufferGl: public FrameBufferInterface {
	private:
		std::vector<std::shared_ptr<TextureInterface>> mTextures;
		std::shared_ptr<TextureInterface> mDepth;
		unsigned mDepthRenderbuffer = 0;
#if IKIGAI_ENABLE_MEMORY_TRACKING
        PROFILER::GpuAllocationToken mDepthMemory;
#endif
	public:
		FrameBufferGl(const std::vector<std::shared_ptr<TextureInterface>>& textures, std::shared_ptr<TextureInterface> depth = nullptr, unsigned mip = 0, unsigned layer = 0);
		~FrameBufferGl() override;
		const std::vector<std::shared_ptr<TextureInterface>>& getTextures() const override;
		const std::shared_ptr<TextureInterface>& getDepth() const override;

		void bind();
		void unbind();

		static void CopyDepth(const FrameBufferGl& from, const FrameBufferGl& to, unsigned w, unsigned h);
	private:
		void create();
	};
}
#endif
