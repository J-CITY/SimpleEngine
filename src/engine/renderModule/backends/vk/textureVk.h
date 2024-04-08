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

		unsigned long long imageSize = 0L;

		VkDescriptorSet descriptor_set;
		std::vector<VkImage> Image;
		VkDeviceMemory Memory;
		std::vector<VkImageView> ImageView;
		std::vector<VkSampler> Sampler;

		VkFormat Format;


		static std::shared_ptr<TextureVk> Create(std::string path);
		static std::shared_ptr<TextureVk> CreateForAttach(int texWidth, int texHeight);
		static std::shared_ptr<TextureVk> CreateDepthForAttach(unsigned int texWidth, unsigned int texHeight);

		static void TransitionImageLayout(const VkImage& image, const VkImageLayout& old_layout, const VkImageLayout& new_layout);

		static std::shared_ptr<TextureVk> CreateCubemap(std::array<std::string, 6> path);
		static std::shared_ptr<TextureVk> Create3D(int texWidth, int texHeight, int arrSize);

		void* getImguiId() override;
	};
}
#endif
