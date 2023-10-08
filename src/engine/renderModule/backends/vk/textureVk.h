#pragma once

#ifdef VULKAN_BACKEND
#include <memory>
#include <string>
#include <vector>
#include <vulkan/vulkan.h>

#include "../interface/textureInterface.h"
namespace IKIGAI::RENDER {
	class TextureVk : public TextureInterface {
	public:
		//maybe make it static
		VkDescriptorPool		m_TexDescriptorPool;
		VkDescriptorSetLayout	m_TextureLayout;

		int width = 0;
		int height = 0;
		int depth = 0;
		unsigned long long imageSize = 0L;
		std::string path;

		VkDescriptorSet descriptor_set;
		std::vector<VkImage> Image;
		VkDeviceMemory Memory;
		std::vector<VkImageView> ImageView;
		std::vector<VkSampler> Sampler;

		VkFormat Format;


		static std::shared_ptr<TextureVk> create(std::string path);
		static std::shared_ptr<TextureVk> createForAttach(int texWidth, int texHeight);
		static std::shared_ptr<TextureVk> createDepthForAttach(unsigned int texWidth, unsigned int texHeight);

		static void TransitionImageLayout(const VkImage& image, const VkImageLayout& old_layout, const VkImageLayout& new_layout);

		static std::shared_ptr<TextureVk> createCubemap(std::array<std::string, 6> path);
		static std::shared_ptr<TextureVk> create3D(int texWidth, int texHeight, int arrSize);
	};
}
#endif
