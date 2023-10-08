#pragma once

#ifdef VULKAN_BACKEND
#include <memory>
#include <vector>
#include <vulkan/vulkan.h>
namespace IKIGAI::RENDER {
	class TextureVk;
	class ShaderInterface;
	class TextureInterface;

	class FrameBufferVk {
	public:
		int attachmentsSize = 1;
		VkRenderPass m_RenderPass = {};
		std::vector<VkFramebuffer> swapChainFramebuffers;

		//std::vector<std::shared_ptr<TextureVk>> textures;
		//std::shared_ptr<TextureVk> depthTexture;

		void create(const std::vector<std::shared_ptr<TextureVk>>& textures, std::shared_ptr<TextureVk> depthTexture);
		void create(std::shared_ptr<TextureVk> depthTexture);

		void bind(const ShaderInterface& shader);
		void unbind(const ShaderInterface& shader);
	};
}
#endif

