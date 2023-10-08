#pragma once
#ifdef VULKAN_BACKEND

#include <vector>
#include <vulkan/vulkan_core.h>


namespace IKIGAI::RENDER {
	struct QueueFamilyIndices;

	class TextureVk;
	class CommandHandler
	{
	public:
		CommandHandler();
		void CreateCommandPool(QueueFamilyIndices& queueIndices);
		void CreateCommandBuffers(size_t numFrameBuffers);

		void DestroyCommandPool();
		void FreeCommandBuffers();

		VkCommandPool& GetCommandPool() { return m_GraphicsComandPool; }
		VkCommandBuffer& GetCommandBuffer(uint32_t const index) { return m_CommandBuffers[index]; }
		std::vector<VkCommandBuffer>& GetCommandBuffers() { return m_CommandBuffers; }

		VkCommandPool	m_GraphicsComandPool;
		std::vector<VkCommandBuffer> m_CommandBuffers;
	};
}
#endif