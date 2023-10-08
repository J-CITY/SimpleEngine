#pragma once

#ifdef VULKAN_BACKEND
#include <vector>
#include <climits>
#include <vulkan/vulkan_core.h>

namespace IKIGAI::RENDER {
	class CommandHandler;
	class SwapChain;
	class DriverVk;
	int constexpr MAX_OBJECTS = 20;
	int constexpr MAX_FRAMES_IN_FLIGHT = 2;
	class TextureVk;

	struct QueueFamilyIndices {

		uint32_t GraphicsFamily = UINT_MAX;
		uint32_t PresentationFamily = UINT_MAX;

		bool isValid() const
		{
			return GraphicsFamily >= 0 && PresentationFamily >= 0;
		}
	};
	struct SubmissionSyncObjects {
		//VkSemaphore OffScreenAvailable;
		VkSemaphore ImageAvailable; // Avvisa quanto l'immagine è disponibile
		//VkSemaphore RenderFinished; // Avvisa quando il rendering è terminato
		VkFence		InFlight;		// Fence per il frame in esecuzione
	};
	struct MainDevice {
		VkPhysicalDevice PhysicalDevice;
		VkDevice		 LogicalDevice;
		VkDeviceSize	 MinUniformBufferOffset;
	};

	struct BufferSettings {
		VkDeviceSize			size;
		VkBufferUsageFlags		usage;
		VkMemoryAllocateFlags	properties;
	};

	struct VulkanRenderData {
		VkInstance			instance;

		MainDevice			main_device;
		VkPhysicalDevice	physical_device;
		VkDevice			device;

		uint32_t			graphic_queue_index;
		VkQueue				graphic_queue;

		uint32_t			min_image_count;
		uint32_t			image_count;

		VkDescriptorPool	imgui_descriptor_pool;
		VkDescriptorPool	texture_descriptor_pool;
		VkDescriptorSetLayout texture_descriptor_layout;
	};
	struct ImageInfo {
		uint32_t				width;
		uint32_t				height;
		VkFormat				format;
		VkImageTiling			tiling;
		VkImageUsageFlags		usage;
		VkMemoryPropertyFlags	properties;
	};
	struct UtilityVk {
		inline static MainDevice* device = nullptr;
		inline static bool useMSAA = false;
		inline static bool useSampleShading = false;
		inline static VkSampleCountFlagBits sampleCount = VK_SAMPLE_COUNT_1_BIT;
		inline static SwapChain* m_SwapChain = nullptr;
		inline static CommandHandler* m_CommandHandler = nullptr;
		inline static VkQueue* m_GraphicsQueue = nullptr;


		static void CreateBuffer(const BufferSettings& buffer_settings, VkBuffer* buffer_data, VkDeviceMemory* memory);
		static VkCommandBuffer BeginCommandBuffer();
		static void EndAndSubmitCommandBuffer(const VkCommandBuffer& command_buffer);
		static void CopyBufferCmd(const VkBuffer& src_buffer, const VkBuffer& dst_buffer, const VkDeviceSize& buffer_size);
		static void CleatImage(const VkImage& image);
		static void CopyImageBuffer(const VkBuffer& src, const VkImage& image, const uint32_t width, const uint32_t height);
		static uint32_t FindMemoryTypeIndex(uint32_t supportedMemoryTypes, const VkMemoryPropertyFlags& properties);
		static VkImageView CreateImageView(const VkImage& image, const VkFormat& format, const VkImageAspectFlags& aspect_flags);
		static void CreateDepthBufferImage(TextureVk& image, const VkExtent2D& image_extent);
		static VkImage CreateImage(const ImageInfo& image_info, VkDeviceMemory* imageMemory);
		static VkFormat ChooseSupportedFormat(const std::vector<VkFormat>& formats,
			const VkImageTiling& tiling, const VkFormatFeatureFlags& feature_flags);
		static VkSampler CreateSampler(const VkSamplerCreateInfo& sampler_create_info);
		static DriverVk* GetDriver();
		static void GetPossibleQueueFamilyIndices(VkSurfaceKHR& surface, const VkPhysicalDevice& possible_physical_device, QueueFamilyIndices& queue_family_indices);
		static bool CheckPossibleDeviceExtensionSupport(const VkPhysicalDevice& possible_physical_device, const std::vector<const char*>& requested_device_extensions);

	};
}
#endif
