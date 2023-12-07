#include "helpers.h"


#ifdef VULKAN_BACKEND

#include "driverVk.h"
#include <stdexcept>
#include "../interface/driverInterface.h"

using namespace IKIGAI;
using namespace IKIGAI::RENDER;

#include "../../gameRendererVk.h"
#include <coreModule/resourceManager/ServiceManager.h>

void UtilityVk::CreateBuffer(const BufferSettings& buffer_settings, VkBuffer* buffer_data, VkDeviceMemory* memory) {
	
	VkBufferCreateInfo buffer_info = {};
	buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	buffer_info.size = buffer_settings.size;
	buffer_info.usage = buffer_settings.usage;
	buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	VkResult result = vkCreateBuffer(device->LogicalDevice, &buffer_info, nullptr, buffer_data);
	if (result != VK_SUCCESS) {
		throw std::runtime_error("Failed to create a Vertex Buffer!");
	}

	VkMemoryRequirements mem_requirements;
	vkGetBufferMemoryRequirements(UtilityVk::device->LogicalDevice, *buffer_data, &mem_requirements);

	VkMemoryAllocateInfo memory_alloc_info = {};
	memory_alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	memory_alloc_info.allocationSize = mem_requirements.size;
	memory_alloc_info.memoryTypeIndex = FindMemoryTypeIndex(mem_requirements.memoryTypeBits, buffer_settings.properties);
	
	result = vkAllocateMemory(UtilityVk::device->LogicalDevice, &memory_alloc_info, nullptr, memory);

	if (result != VK_SUCCESS) {
		throw std::runtime_error("Failed to allocate Vertex Buffer Memory!");
	}

	vkBindBufferMemory(UtilityVk::device->LogicalDevice, *buffer_data, *memory, 0);
}

VkCommandBuffer UtilityVk::BeginCommandBuffer() {
	
	VkCommandBuffer command_buffer;

	VkCommandBufferAllocateInfo alloc_info = {};
	alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	alloc_info.commandPool = m_CommandHandler->GetCommandPool();
	alloc_info.commandBufferCount = 1;

	vkAllocateCommandBuffers(UtilityVk::device->LogicalDevice, &alloc_info, &command_buffer);

	VkCommandBufferBeginInfo beginInfo = {};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT; // Ogni comando registrato nel Command Buffer verrà inviato soltanto una volta

	vkBeginCommandBuffer(command_buffer, &beginInfo);

	return command_buffer;
}

void UtilityVk::EndAndSubmitCommandBuffer(const VkCommandBuffer& command_buffer) {
	
	vkEndCommandBuffer(command_buffer);

	VkSubmitInfo submit_info = {};
	submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submit_info.commandBufferCount = 1;
	submit_info.pCommandBuffers = &command_buffer;

	// Invia li comando di copia alla Transfer Queue (nel nostro caso sarà la Graphics Queue) ed aspetta finchè termini
	vkQueueSubmit(*m_GraphicsQueue, 1, &submit_info, VK_NULL_HANDLE);
	vkQueueWaitIdle(*m_GraphicsQueue);

	vkFreeCommandBuffers(UtilityVk::device->LogicalDevice, m_CommandHandler->GetCommandPool(), 1, &command_buffer);
}

void UtilityVk::CopyBufferCmd(const VkBuffer& src_buffer, const VkBuffer& dst_buffer, const VkDeviceSize& buffer_size)
{
	VkCommandBuffer transfer_command_buffer = BeginCommandBuffer();

	VkBufferCopy buffer_copy_region = {};
	buffer_copy_region.srcOffset = 0;
	buffer_copy_region.dstOffset = 0;
	buffer_copy_region.size = buffer_size;

	vkCmdCopyBuffer(transfer_command_buffer, src_buffer, dst_buffer, 1, &buffer_copy_region);

	EndAndSubmitCommandBuffer(transfer_command_buffer);
}

void UtilityVk::CleatImage(const VkImage& image) {
	VkCommandBuffer transfer_command_buffer = BeginCommandBuffer();

	VkImageSubresourceRange subresource_range = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };

	VkClearColorValue color;

	color.float32[0] = 1.0f;
	color.float32[1] = 1.0f;
	color.float32[2] = 1.0f;
	color.float32[3] = 1.0f;

	vkCmdClearColorImage(transfer_command_buffer, image, VK_IMAGE_LAYOUT_GENERAL, &color, 1, &subresource_range);
	EndAndSubmitCommandBuffer(transfer_command_buffer);
}

void UtilityVk::CopyImageBuffer(const VkBuffer& src, const VkImage& image, const uint32_t width, const uint32_t height) {
	VkCommandBuffer transfer_command_buffer = BeginCommandBuffer();

	VkBufferImageCopy image_region = {};
	image_region.bufferOffset = 0;
	image_region.bufferRowLength = 0;
	image_region.bufferImageHeight = 0;
	image_region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	image_region.imageSubresource.mipLevel = 0;
	image_region.imageSubresource.baseArrayLayer = 0;
	image_region.imageSubresource.layerCount = 1;
	image_region.imageOffset = { 0, 0, 0 };
	image_region.imageExtent = { width, height, 1 };

	vkCmdCopyBufferToImage(transfer_command_buffer, src, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &image_region);

	EndAndSubmitCommandBuffer(transfer_command_buffer);
}


uint32_t UtilityVk::FindMemoryTypeIndex(uint32_t supportedMemoryTypes, const VkMemoryPropertyFlags& properties) {
	
	VkPhysicalDeviceMemoryProperties memory_properties;
	vkGetPhysicalDeviceMemoryProperties(device->PhysicalDevice, &memory_properties);
	
	for (uint32_t i = 0; i < memory_properties.memoryTypeCount; ++i) {
		unsigned int supported_memory = supportedMemoryTypes & (1U << i);
		bool supported_properties = (memory_properties.memoryTypes[i].propertyFlags & properties) == properties;
		if (supported_memory && supported_properties) {
			return i;
		}
	}
	return static_cast<uint32_t>(0);
}

VkImageView UtilityVk::CreateImageView(const VkImage& image, const VkFormat& format, const VkImageAspectFlags& aspect_flags) {
	VkImageViewCreateInfo viewCreateInfo = {};
	viewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	viewCreateInfo.image = image;
	viewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	viewCreateInfo.format = format;
	viewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
	viewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
	viewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
	viewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
	
	viewCreateInfo.subresourceRange.aspectMask = aspect_flags;
	viewCreateInfo.subresourceRange.baseMipLevel = 0;
	viewCreateInfo.subresourceRange.levelCount = 1;
	viewCreateInfo.subresourceRange.baseArrayLayer = 0;
	viewCreateInfo.subresourceRange.layerCount = 1;

	VkImageView imageView;
	VkResult res = vkCreateImageView(device->LogicalDevice, &viewCreateInfo, nullptr, &imageView);

	if (res != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create an Image View!");
	}

	return imageView;
}

void UtilityVk::CreateDepthBufferImage(TextureVk& image, const VkExtent2D& image_extent) {
	const std::vector<VkFormat> formats = { VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D32_SFLOAT, VK_FORMAT_D24_UNORM_S8_UINT };
	const VkImageTiling tiling = VK_IMAGE_TILING_OPTIMAL;
	const VkFormatFeatureFlags format_flags = VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT;

	image.Format = ChooseSupportedFormat(formats, tiling, format_flags);

	ImageInfo image_info = {};
	image_info.width = image_extent.width;
	image_info.height = image_extent.height;
	image_info.format = image.Format;
	image_info.tiling = tiling;
	image_info.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
	image_info.properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

	image.Image.push_back(CreateImage(image_info, &image.Memory));
	image.ImageView.push_back(CreateImageView(image.Image[0], image.Format, VK_IMAGE_ASPECT_DEPTH_BIT));
}


VkImage UtilityVk::CreateImage(const ImageInfo& image_info, VkDeviceMemory* imageMemory) {
	
	VkImageCreateInfo imageCreateInfo = {};
	imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
	imageCreateInfo.extent.width = image_info.width;
	imageCreateInfo.extent.height = image_info.height;
	imageCreateInfo.extent.depth = 1;
	imageCreateInfo.mipLevels = 1;
	imageCreateInfo.arrayLayers = 1;
	imageCreateInfo.format = image_info.format;
	imageCreateInfo.tiling = image_info.tiling;
	imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	imageCreateInfo.usage = image_info.usage;
	imageCreateInfo.samples = useMSAA ? sampleCount : VK_SAMPLE_COUNT_1_BIT;
	imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	VkImage image;
	VkResult result = vkCreateImage(device->LogicalDevice, &imageCreateInfo, nullptr, &image);

	if (result != VK_SUCCESS) {
		throw std::runtime_error("Failed to create an Image!");
	}

	// CREAZIONE DELLA MEMORIA PER L'IMMAGINE
	VkMemoryRequirements memoryRequirements;
	vkGetImageMemoryRequirements(device->LogicalDevice, image, &memoryRequirements);

	VkMemoryAllocateInfo memoryAllocInfo = {};
	memoryAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	memoryAllocInfo.allocationSize = memoryRequirements.size;
	memoryAllocInfo.memoryTypeIndex = FindMemoryTypeIndex(memoryRequirements.memoryTypeBits, image_info.properties);

	result = vkAllocateMemory(device->LogicalDevice, &memoryAllocInfo, nullptr, imageMemory);

	if (result != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to allocate memory for image!");
	}

	result = vkBindImageMemory(device->LogicalDevice, image, *imageMemory, 0);

	if (result != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to bind memory to the image!");
	}

	return image;
}

VkFormat UtilityVk::ChooseSupportedFormat(const std::vector<VkFormat>& formats,
	const VkImageTiling& tiling, const VkFormatFeatureFlags& feature_flags) {
	
	for (VkFormat format : formats)
	{
		VkFormatProperties properties;
		vkGetPhysicalDeviceFormatProperties(device->PhysicalDevice, format, &properties);

		bool const is_tiling_linear = tiling == VK_IMAGE_TILING_LINEAR;
		bool const linear_support_flags = (properties.linearTilingFeatures & feature_flags) == feature_flags;
		bool const is_tiling_optimal = tiling == VK_IMAGE_TILING_OPTIMAL;
		bool const optimal_support_flags = (properties.optimalTilingFeatures & feature_flags) == feature_flags;

		if (is_tiling_linear && linear_support_flags)
		{
			return format;
		}
		else if (is_tiling_optimal && optimal_support_flags)
		{
			return format;
		}
	}
	throw std::runtime_error("Failed to find a matching format!");
}

VkSampler  UtilityVk::CreateSampler(const VkSamplerCreateInfo& sampler_create_info) {
	
	VkSampler sampler;
	VkResult result = vkCreateSampler(device->LogicalDevice, &sampler_create_info, nullptr, &sampler);

	if (result != VK_SUCCESS)
		throw std::runtime_error("Failed to create a Texture Sampler!");

	return sampler;
}

DriverVk* UtilityVk::GetDriver() {
	return reinterpret_cast<RENDER::GameRendererVk&>(RESOURCES::ServiceManager::Get<RENDER::GameRendererInterface>()).getDriver();
}


void UtilityVk::GetPossibleQueueFamilyIndices(VkSurfaceKHR& surface, const VkPhysicalDevice& possible_physical_device, QueueFamilyIndices& queue_family_indices)
{
	uint32_t queue_family_count = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(possible_physical_device, &queue_family_count, nullptr);

	std::vector<VkQueueFamilyProperties> queue_family_list(queue_family_count);
	vkGetPhysicalDeviceQueueFamilyProperties(possible_physical_device, &queue_family_count, queue_family_list.data());

	int queue_index = 0;

	if (queue_family_count > 0)
	{
		for (const auto& queue_family : queue_family_list)
		{
			if (queue_family.queueFlags & VK_QUEUE_GRAPHICS_BIT)
			{
				queue_family_indices.GraphicsFamily = queue_index;
			}

			VkBool32 presentation_support = false;
			vkGetPhysicalDeviceSurfaceSupportKHR(possible_physical_device, queue_index, surface, &presentation_support);
			
			if (presentation_support)
			{
				queue_family_indices.PresentationFamily = queue_index;
			}
			if (queue_family.queueFlags & VK_QUEUE_COMPUTE_BIT)
			{
				queue_family_indices.ComputeFamily = queue_index;
			}
			if (queue_family.queueFlags & VK_QUEUE_TRANSFER_BIT)
			{
				queue_family_indices.TransferFamily = queue_index;
			}

			if (queue_family_indices.isValid())
			{
				break;
			}

			++queue_index;
		}
	}
}

bool UtilityVk::CheckPossibleDeviceExtensionSupport(const VkPhysicalDevice& possible_physical_device, const std::vector<const char*>& requested_device_extensions)
{
	uint32_t extensionCount = 0;
	vkEnumerateDeviceExtensionProperties(possible_physical_device, nullptr, &extensionCount, nullptr);

	if (extensionCount == 0)
		return false;

	std::vector<VkExtensionProperties> physicalDeviceExtensions(extensionCount);
	vkEnumerateDeviceExtensionProperties(possible_physical_device, nullptr, &extensionCount, physicalDeviceExtensions.data());

	for (const auto& extensionToCheck : requested_device_extensions)
	{
		bool hasExtension = false;

		for (const auto& extension : physicalDeviceExtensions)
		{
			if (strcmp(extensionToCheck, extension.extensionName) == 0)
			{
				hasExtension = true;
				break;
			}
		}

		if (!hasExtension)
			return false;
	}

	return true;
}
#endif
