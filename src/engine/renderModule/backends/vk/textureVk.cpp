#include "textureVk.h"

#include <iostream>

#ifdef VULKAN_BACKEND
#include "driverVk.h"

#include <coreModule/resourceManager/textureManager.h>
#include "../../gameRendererVk.h"
#include <coreModule/resourceManager/ServiceManager.h>

using namespace IKIGAI;
using namespace IKIGAI::RENDER;
std::shared_ptr<IKIGAI::RENDER::TextureVk> TextureVk::createDepthForAttach(unsigned int texWidth, unsigned int texHeight) {
	auto texture = std::make_shared<TextureVk>();
	UtilityVk::CreateDepthBufferImage(*texture, { texWidth, texHeight });
	texture->width = texWidth;
	texture->height = texHeight;
	return texture;
}

void TextureVk::TransitionImageLayout(const VkImage& image, const VkImageLayout& old_layout, const VkImageLayout& new_layout) {
	VkCommandBuffer command_buffer = UtilityVk::BeginCommandBuffer();

	VkImageMemoryBarrier image_memory_barrier = {};
	image_memory_barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	image_memory_barrier.oldLayout = old_layout;					// Layout da cui spostarsi
	image_memory_barrier.newLayout = new_layout;					// Layout in cui spostarsi
	image_memory_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;		// Queue family da cui spostarsi
	image_memory_barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;		// Queue family in cui spostarsi
	image_memory_barrier.image = image;						// Immagine su cui wrappare la barriera
	image_memory_barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	image_memory_barrier.subresourceRange.baseMipLevel = 0;
	image_memory_barrier.subresourceRange.levelCount = 1;
	image_memory_barrier.subresourceRange.baseArrayLayer = 0;
	image_memory_barrier.subresourceRange.layerCount = 1;

	bool const old_layout_undefined = old_layout == VK_IMAGE_LAYOUT_UNDEFINED;
	bool const new_layout_transfer_dst_optimal = new_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;

	bool const old_layout_transfer_dst_optimal = old_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	bool const new_layout_shader_read_only = new_layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

	VkPipelineStageFlags src_stage = 0;	// Stage dal quale è possibile iniziare la transizione
	VkPipelineStageFlags dst_stage = 0;	// Stage nel quale la transizione deve essere già terminata

	if (old_layout_undefined && new_layout_transfer_dst_optimal) {
		image_memory_barrier.srcAccessMask = 0;								// Qualsiasi stage iniziale
		image_memory_barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;	// copyBufferImage è una operazione di write

		src_stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;	// qualsiasi momento dopo l'inizio della pipeline
		dst_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;		// Primache provi a fare una write nel transfer stage!
	}
	else if (old_layout_transfer_dst_optimal && new_layout_shader_read_only) {
		image_memory_barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		image_memory_barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		src_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;			// al termine delle operazioni di scrittura del transfer stage
		dst_stage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;	// prima che provi a a leggere il fragment shader
	}

	const VkDependencyFlags dep_flags = 0;

	const uint32_t memory_barrier_count = 0;
	const uint32_t buffer_memory_barrier_count = 0;
	const uint32_t image_memory_barrier_count = 1;

	vkCmdPipelineBarrier(command_buffer, src_stage, dst_stage, dep_flags,
		memory_barrier_count, nullptr,						// no global memory barrier
		buffer_memory_barrier_count, nullptr,				// no buffer memory barrier
		image_memory_barrier_count, &image_memory_barrier
	);

	UtilityVk::EndAndSubmitCommandBuffer(command_buffer);
}

std::shared_ptr<TextureVk> TextureVk::createCubemap(std::array<std::string, 6> path) {
	auto render = reinterpret_cast<RENDER::GameRendererVk&>(RESOURCES::ServiceManager::Get<RENDER::GameRendererInterface>()).getDriver();

	unsigned char* textureData[6];

	int width{ 0 };
	int height{ 0 };
	int numberOfChannels{ 0 };

	textureData[0] = IKIGAI::RESOURCES::stbiLoad(path[0].c_str(), &width, &height, &numberOfChannels, 4);
	textureData[1] = IKIGAI::RESOURCES::stbiLoad(path[1].c_str(), &width, &height, &numberOfChannels, 4);
	textureData[2] = IKIGAI::RESOURCES::stbiLoad(path[2].c_str(), &width, &height, &numberOfChannels, 4);
	textureData[3] = IKIGAI::RESOURCES::stbiLoad(path[3].c_str(), &width, &height, &numberOfChannels, 4);
	textureData[4] = IKIGAI::RESOURCES::stbiLoad(path[4].c_str(), &width, &height, &numberOfChannels, 4);
	textureData[5] = IKIGAI::RESOURCES::stbiLoad(path[5].c_str(), &width, &height, &numberOfChannels, 4);
	

	//Calculate the image size and the layer size.
	const VkDeviceSize imageSize = width * height * 4 * 6;
	const VkDeviceSize layerSize = imageSize / 6;

	//Set up the staging buffer.
	BufferSettings m_BufferSettings;
	m_BufferSettings.size = imageSize;
	m_BufferSettings.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
	m_BufferSettings.properties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

	VkBuffer		m_StagingBuffer;
	VkDeviceMemory	m_StagingBufferMemory;
	UtilityVk::CreateBuffer(m_BufferSettings, &m_StagingBuffer, &m_StagingBufferMemory);

	//Map the memory.
	void* data;
	vkMapMemory(render->m_MainDevice.LogicalDevice,
		m_StagingBufferMemory, 0, imageSize, 0, &data);

	//Copy the data into the staging buffer.
	for (int i = 0; i < 6; ++i) {
		memcpy((unsigned char*)data+(layerSize * i), textureData[i], static_cast<size_t>(layerSize));
	}

	vkUnmapMemory(render->m_MainDevice.LogicalDevice, m_StagingBufferMemory);

	IKIGAI::RESOURCES::stbiImageFree(textureData[0]);
	IKIGAI::RESOURCES::stbiImageFree(textureData[1]);
	IKIGAI::RESOURCES::stbiImageFree(textureData[2]);
	IKIGAI::RESOURCES::stbiImageFree(textureData[3]);
	IKIGAI::RESOURCES::stbiImageFree(textureData[4]);
	IKIGAI::RESOURCES::stbiImageFree(textureData[5]);

	auto texture = std::make_shared<TextureVk>();
	texture->path = path[0];
	texture->width = width;
	texture->height = height;

	//CRESTE TEXTURE IMAGE
	VkDeviceMemory m_TextureImageMemory;

	ImageInfo image_info = {};
	image_info.width = texture->width;
	image_info.height = texture->height;
	image_info.format = VK_FORMAT_R8G8B8A8_UNORM;
	image_info.tiling = VK_IMAGE_TILING_OPTIMAL;
	image_info.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
	image_info.properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

	VkImage texture_image = UtilityVk::CreateImage(image_info, &m_TextureImageMemory);

	TransitionImageLayout(texture_image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
	UtilityVk::CopyImageBuffer(m_StagingBuffer, texture_image, texture->width, texture->height);
	TransitionImageLayout(texture_image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

	texture->Image.push_back(texture_image);
	texture->Memory = m_TextureImageMemory;

	vkDestroyBuffer(render->m_MainDevice.LogicalDevice, m_StagingBuffer, nullptr);
	vkFreeMemory(render->m_MainDevice.LogicalDevice, m_StagingBufferMemory, nullptr);


	const VkImageView image_view = UtilityVk::CreateImageView(texture->Image[0], VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT);

	texture->ImageView.push_back(image_view);

	//SAMPLER
	VkSamplerCreateInfo samplerCreateInfo = {};
	samplerCreateInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	samplerCreateInfo.magFilter = VK_FILTER_LINEAR;
	samplerCreateInfo.minFilter = VK_FILTER_LINEAR;
	samplerCreateInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerCreateInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerCreateInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerCreateInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
	samplerCreateInfo.unnormalizedCoordinates = VK_FALSE;
	samplerCreateInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	samplerCreateInfo.mipLodBias = 0.0f;
	samplerCreateInfo.minLod = 0.0f;
	samplerCreateInfo.maxLod = 0.0f;
	samplerCreateInfo.anisotropyEnable = VK_TRUE;
	samplerCreateInfo.maxAnisotropy = 16;

	VkSampler s;
	VkResult result = vkCreateSampler(render->m_MainDevice.LogicalDevice, &samplerCreateInfo, nullptr, &s);
	texture->Sampler.push_back(s);

	if (result != VK_SUCCESS) {
		throw std::runtime_error("Failed to create a Texture Sampler!");
	}


	VkDescriptorPoolSize samplerPoolSize = {};
	samplerPoolSize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	samplerPoolSize.descriptorCount = MAX_OBJECTS;

	VkDescriptorPoolCreateInfo samplerPoolCreateInfo = {};
	samplerPoolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	samplerPoolCreateInfo.maxSets = MAX_OBJECTS;
	samplerPoolCreateInfo.poolSizeCount = 1;
	samplerPoolCreateInfo.pPoolSizes = &samplerPoolSize;

	result = vkCreateDescriptorPool(render->m_MainDevice.LogicalDevice, &samplerPoolCreateInfo, nullptr, &texture->m_TexDescriptorPool);
	if (result != VK_SUCCESS) {
		throw std::runtime_error("Failed to create a Descriptor Pool!");
	}

	VkDescriptorSetLayoutBinding samplerLayoutBinding = {};
	samplerLayoutBinding.binding = 0;
	samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	samplerLayoutBinding.descriptorCount = 1;
	samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
	samplerLayoutBinding.pImmutableSamplers = nullptr;

	VkDescriptorSetLayoutCreateInfo textureLayoutCreateInfo = {};
	textureLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	textureLayoutCreateInfo.bindingCount = 1;
	textureLayoutCreateInfo.pBindings = &samplerLayoutBinding;

	result = vkCreateDescriptorSetLayout(render->m_MainDevice.LogicalDevice, &textureLayoutCreateInfo, nullptr, &texture->m_TextureLayout);
	if (result != VK_SUCCESS) {
		throw std::runtime_error("Failed to create the Texture Descriptor Set Layout!");
	}
	//descriptor set
	VkDescriptorSet descriptor_set;

	VkDescriptorSetAllocateInfo set_alloc_info = {};
	set_alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	set_alloc_info.descriptorPool = texture->m_TexDescriptorPool;
	set_alloc_info.descriptorSetCount = 1;
	set_alloc_info.pSetLayouts = &texture->m_TextureLayout;

	result = vkAllocateDescriptorSets(render->m_MainDevice.LogicalDevice, &set_alloc_info, &descriptor_set);
	if (result != VK_SUCCESS) {
		throw std::runtime_error("Failed to allocate Texture Descriptor Set!");
	}

	VkDescriptorImageInfo imageInfo = {};
	imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	imageInfo.imageView = texture->ImageView.back();
	imageInfo.sampler = texture->Sampler.back();

	VkWriteDescriptorSet descriptorWrite = {};
	descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptorWrite.dstSet = descriptor_set;
	descriptorWrite.dstBinding = 0;
	descriptorWrite.dstArrayElement = 0;
	descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	descriptorWrite.descriptorCount = 1;
	descriptorWrite.pImageInfo = &imageInfo;

	vkUpdateDescriptorSets(render->m_MainDevice.LogicalDevice, 1, &descriptorWrite, 0, nullptr);
	texture->descriptor_set = descriptor_set;

	texture->type = TextureType::TEXTURE_CUBE;
	return texture;
}

std::shared_ptr<TextureVk> TextureVk::create3D(int width, int height, int arrSize) {
	auto render = reinterpret_cast<RENDER::GameRendererVk&>(RESOURCES::ServiceManager::Get<RENDER::GameRendererInterface>()).getDriver();

	auto texture = std::make_shared<TextureVk>();
	texture->path = "";
	texture->width = width;
	texture->height = height;
	texture->depth = arrSize;
	texture->type = TextureType::TEXTURE_3D;
	
	// A 3D texture is described as width x height x depth
	auto mipLevels = 1;
	auto format = VK_FORMAT_R8G8B8A8_UNORM;

	// Format support check
	// 3D texture support in Vulkan is mandatory (in contrast to OpenGL) so no need to check if it's supported
	VkFormatProperties formatProperties;
	vkGetPhysicalDeviceFormatProperties(render->m_MainDevice.PhysicalDevice, format, &formatProperties);
	// Check if format supports transfer
	if (!(formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_TRANSFER_DST_BIT)) {
		std::cout << "Error: Device does not support flag TRANSFER_DST for selected texture format!" << std::endl;
		//return;
	}
	// Check if GPU supports requested 3D texture dimensions
	//uint32_t maxImageDimension3D(vulkanDevice->properties.limits.maxImageDimension3D);
	//if (width > maxImageDimension3D || height > maxImageDimension3D || depth > maxImageDimension3D) {
	//	std::cout << "Error: Requested texture dimensions is greater than supported 3D texture dimension!" << std::endl;
	//	return;
	//}

	auto& device = render->m_MainDevice.LogicalDevice;

	// Create optimal tiled target image
	VkImageCreateInfo imageCreateInfo;
	imageCreateInfo.imageType = VK_IMAGE_TYPE_3D;
	imageCreateInfo.format = format;
	imageCreateInfo.mipLevels = mipLevels;
	imageCreateInfo.arrayLayers = 1;
	imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	imageCreateInfo.extent.width = texture->width;
	imageCreateInfo.extent.height = texture->height;
	imageCreateInfo.extent.depth = texture->depth;
	// Set initial layout of the image to undefined
	imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	imageCreateInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;

	texture->Image.push_back({});
	vkCreateImage(device, &imageCreateInfo, nullptr, &texture->Image[0]);


	// Device local memory to back up image
	VkMemoryAllocateInfo memAllocInfo;
	VkMemoryRequirements memReqs = {};
	vkGetImageMemoryRequirements(device, texture->Image[0], &memReqs);
	memAllocInfo.allocationSize = memReqs.size;
	//TODO:
	//memAllocInfo.memoryTypeIndex = vulkanDevice->getMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	vkAllocateMemory(device, &memAllocInfo, nullptr, &texture->Memory);
	vkBindImageMemory(device, texture->Image[0], texture->Memory, 0);

	// Create sampler
	VkSamplerCreateInfo sampler;
	sampler.magFilter = VK_FILTER_LINEAR;
	sampler.minFilter = VK_FILTER_LINEAR;
	sampler.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	sampler.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	sampler.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	sampler.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	sampler.mipLodBias = 0.0f;
	sampler.compareOp = VK_COMPARE_OP_NEVER;
	sampler.minLod = 0.0f;
	sampler.maxLod = 0.0f;
	sampler.maxAnisotropy = 1.0;
	sampler.anisotropyEnable = VK_FALSE;
	sampler.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
	texture->Sampler.push_back({});
	vkCreateSampler(device, &sampler, nullptr, &texture->Sampler[0]);

	// Create image view
	VkImageViewCreateInfo view;
	view.image = texture->Image[0];
	view.viewType = VK_IMAGE_VIEW_TYPE_3D;
	view.format = format;
	view.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	view.subresourceRange.baseMipLevel = 0;
	view.subresourceRange.baseArrayLayer = 0;
	view.subresourceRange.layerCount = 1;
	view.subresourceRange.levelCount = 1;
	texture->ImageView.push_back({});
	vkCreateImageView(device, &view, nullptr, &texture->ImageView[0]);

	// Fill image descriptor image info to be used descriptor set setup
	//texture->descriptor_set.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	//texture.descriptor.imageView = texture.view;
	//texture.descriptor.sampler = texture.sampler;

	return texture;
}

std::shared_ptr<TextureVk> TextureVk::createForAttach(int texWidth, int texHeight) {
	
	auto texture = std::make_shared<TextureVk>();
	for (size_t i = 0; i < UtilityVk::m_SwapChain->SwapChainImagesSize(); i++) {
		const std::vector<VkFormat> formats = { VK_FORMAT_R32G32B32A32_SFLOAT };
		const VkImageTiling tiling = VK_IMAGE_TILING_OPTIMAL;
		const VkFormatFeatureFlags format_flags = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		texture->Format = UtilityVk::ChooseSupportedFormat(formats, tiling, format_flags);

		ImageInfo image_info = {};
		image_info.width = texWidth;
		image_info.height = texHeight;
		image_info.format = texture->Format;
		image_info.tiling = tiling;
		image_info.usage = VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
		image_info.properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

		texture->Image.push_back(UtilityVk::CreateImage(image_info, &texture->Memory));

		texture->ImageView.push_back(UtilityVk::CreateImageView(texture->Image.back(), texture->Format, VK_IMAGE_ASPECT_COLOR_BIT));

		{
			VkSamplerCreateInfo samplerCreateInfo = {};
			samplerCreateInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
			samplerCreateInfo.magFilter = VK_FILTER_NEAREST;		// linear interpolation between the texels
			samplerCreateInfo.minFilter = VK_FILTER_NEAREST;		// quando viene miniaturizzata come renderizzarla (lerp)
			samplerCreateInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
			samplerCreateInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
			samplerCreateInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
			samplerCreateInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
			samplerCreateInfo.unnormalizedCoordinates = VK_FALSE;			// è normalizzata
			samplerCreateInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
			samplerCreateInfo.mipLodBias = 0.0f;
			samplerCreateInfo.minLod = 0.0f;
			samplerCreateInfo.maxLod = 1.0f;
			samplerCreateInfo.anisotropyEnable = VK_TRUE;
			samplerCreateInfo.maxAnisotropy = 16;
			samplerCreateInfo.compareEnable = VK_FALSE;
			samplerCreateInfo.compareOp = VK_COMPARE_OP_NEVER;
			texture->Sampler.push_back(UtilityVk::CreateSampler(samplerCreateInfo));
		}
	}

	///


	VkDescriptorPoolSize samplerPoolSize = {};
	samplerPoolSize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	samplerPoolSize.descriptorCount = MAX_OBJECTS;

	VkDescriptorPoolCreateInfo samplerPoolCreateInfo = {};
	samplerPoolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	samplerPoolCreateInfo.maxSets = MAX_OBJECTS;
	samplerPoolCreateInfo.poolSizeCount = 1;
	samplerPoolCreateInfo.pPoolSizes = &samplerPoolSize;

	auto result = vkCreateDescriptorPool(UtilityVk::device->LogicalDevice, &samplerPoolCreateInfo, nullptr, &texture->m_TexDescriptorPool);
	if (result != VK_SUCCESS) {
		throw std::runtime_error("Failed to create a Descriptor Pool!");
	}

	VkDescriptorSetLayoutBinding samplerLayoutBinding = {};
	samplerLayoutBinding.binding = 0;
	samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	samplerLayoutBinding.descriptorCount = 1;
	samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
	samplerLayoutBinding.pImmutableSamplers = nullptr;

	VkDescriptorSetLayoutCreateInfo textureLayoutCreateInfo = {};
	textureLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	textureLayoutCreateInfo.bindingCount = 1;
	textureLayoutCreateInfo.pBindings = &samplerLayoutBinding;

	result = vkCreateDescriptorSetLayout(UtilityVk::device->LogicalDevice, &textureLayoutCreateInfo, nullptr, &texture->m_TextureLayout);
	if (result != VK_SUCCESS) {
		throw std::runtime_error("Failed to create the Texture Descriptor Set Layout!");
	}
	//descriptor set
	VkDescriptorSet descriptor_set;

	VkDescriptorSetAllocateInfo set_alloc_info = {};
	set_alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	set_alloc_info.descriptorPool = texture->m_TexDescriptorPool;
	set_alloc_info.descriptorSetCount = 1;
	set_alloc_info.pSetLayouts = &texture->m_TextureLayout;

	result = vkAllocateDescriptorSets(UtilityVk::device->LogicalDevice, &set_alloc_info, &descriptor_set);
	if (result != VK_SUCCESS) {
		throw std::runtime_error("Failed to allocate Texture Descriptor Set!");
	}

	VkDescriptorImageInfo imageInfo = {};
	imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	imageInfo.imageView = texture->ImageView.back();
	imageInfo.sampler = texture->Sampler.back();

	VkWriteDescriptorSet descriptorWrite = {};
	descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptorWrite.dstSet = descriptor_set;
	descriptorWrite.dstBinding = 0;
	descriptorWrite.dstArrayElement = 0;
	descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	descriptorWrite.descriptorCount = 1;
	descriptorWrite.pImageInfo = &imageInfo;

	vkUpdateDescriptorSets(UtilityVk::device->LogicalDevice, 1, &descriptorWrite, 0, nullptr);
	texture->descriptor_set = descriptor_set;
	///
	return texture;
}

std::shared_ptr<TextureVk> TextureVk::create(std::string path) {
	
	auto texture = std::make_shared<TextureVk>();
	texture->path = path;

	//int const texture_image_location = CreateTextureImage();

	int nChannels;
	unsigned char* image = IKIGAI::RESOURCES::stbiLoad(texture->path.c_str(), &texture->width, &texture->height, &nChannels, 4);

	if (!image) {
		throw std::runtime_error("Failed to load a Texture file! (" + path + ")");
	}

	texture->imageSize = static_cast<uint64_t>((texture->width)) * static_cast<uint64_t>((texture->height)) * 4L;

	BufferSettings m_BufferSettings;
	m_BufferSettings.size = texture->imageSize;
	m_BufferSettings.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
	m_BufferSettings.properties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

	VkBuffer		m_StagingBuffer;
	VkDeviceMemory	m_StagingBufferMemory;
	UtilityVk::CreateBuffer(m_BufferSettings, &m_StagingBuffer, &m_StagingBufferMemory);

	// copy image data to staging buffer
	void* data;
	vkMapMemory(UtilityVk::device->LogicalDevice, m_StagingBufferMemory, 0, texture->imageSize, 0, &data);
	memcpy(data, image, static_cast<size_t>(texture->imageSize));
	vkUnmapMemory(UtilityVk::device->LogicalDevice, m_StagingBufferMemory);

	IKIGAI::RESOURCES::stbiImageFree(image);

	//CRESTE TEXTURE IMAGE
	VkDeviceMemory m_TextureImageMemory;

	ImageInfo image_info = {};
	image_info.width = texture->width;
	image_info.height = texture->height;
	image_info.format = VK_FORMAT_R8G8B8A8_UNORM;
	image_info.tiling = VK_IMAGE_TILING_OPTIMAL;
	image_info.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
	image_info.properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
	
	VkImage texture_image = UtilityVk::CreateImage(image_info, &m_TextureImageMemory);

	TransitionImageLayout(texture_image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
	UtilityVk::CopyImageBuffer(m_StagingBuffer, texture_image, texture->width, texture->height);
	TransitionImageLayout(texture_image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

	texture->Image.push_back(texture_image);
	texture->Memory = m_TextureImageMemory;

	vkDestroyBuffer(UtilityVk::device->LogicalDevice, m_StagingBuffer, nullptr);
	vkFreeMemory(UtilityVk::device->LogicalDevice, m_StagingBufferMemory, nullptr);


	const VkImageView image_view = UtilityVk::CreateImageView(texture->Image[0], VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT);

	texture->ImageView.push_back(image_view);

	//SAMPLER
	VkSamplerCreateInfo samplerCreateInfo = {};
	samplerCreateInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	samplerCreateInfo.magFilter = VK_FILTER_LINEAR;		// linear interpolation between the texels
	samplerCreateInfo.minFilter = VK_FILTER_LINEAR;		// quando viene miniaturizzata come renderizzarla (lerp)
	samplerCreateInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerCreateInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerCreateInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerCreateInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
	samplerCreateInfo.unnormalizedCoordinates = VK_FALSE;
	samplerCreateInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	samplerCreateInfo.mipLodBias = 0.0f;
	samplerCreateInfo.minLod = 0.0f;
	samplerCreateInfo.maxLod = 0.0f;
	samplerCreateInfo.anisotropyEnable = VK_TRUE;
	samplerCreateInfo.maxAnisotropy = 16;

	VkSampler s;
	VkResult result = vkCreateSampler(UtilityVk::device->LogicalDevice, &samplerCreateInfo, nullptr, &s);
	texture->Sampler.push_back(s);

	if (result != VK_SUCCESS) {
		throw std::runtime_error("Failed to create a Texture Sampler!");
	}


	VkDescriptorPoolSize samplerPoolSize = {};
	samplerPoolSize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	samplerPoolSize.descriptorCount = MAX_OBJECTS;

	VkDescriptorPoolCreateInfo samplerPoolCreateInfo = {};
	samplerPoolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	samplerPoolCreateInfo.maxSets = MAX_OBJECTS;
	samplerPoolCreateInfo.poolSizeCount = 1;
	samplerPoolCreateInfo.pPoolSizes = &samplerPoolSize;

	result = vkCreateDescriptorPool(UtilityVk::device->LogicalDevice, &samplerPoolCreateInfo, nullptr, &texture->m_TexDescriptorPool);
	if (result != VK_SUCCESS) {
		throw std::runtime_error("Failed to create a Descriptor Pool!");
	}

	VkDescriptorSetLayoutBinding samplerLayoutBinding = {};
	samplerLayoutBinding.binding = 0;
	samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	samplerLayoutBinding.descriptorCount = 1;
	samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
	samplerLayoutBinding.pImmutableSamplers = nullptr;

	VkDescriptorSetLayoutCreateInfo textureLayoutCreateInfo = {};
	textureLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	textureLayoutCreateInfo.bindingCount = 1;
	textureLayoutCreateInfo.pBindings = &samplerLayoutBinding;

	result = vkCreateDescriptorSetLayout(UtilityVk::device->LogicalDevice, &textureLayoutCreateInfo, nullptr, &texture->m_TextureLayout);
	if (result != VK_SUCCESS) {
		throw std::runtime_error("Failed to create the Texture Descriptor Set Layout!");
	}
	//descriptor set
	VkDescriptorSet descriptor_set;

	VkDescriptorSetAllocateInfo set_alloc_info = {};
	set_alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	set_alloc_info.descriptorPool = texture->m_TexDescriptorPool;
	set_alloc_info.descriptorSetCount = 1;
	set_alloc_info.pSetLayouts = &texture->m_TextureLayout;

	result = vkAllocateDescriptorSets(UtilityVk::device->LogicalDevice, &set_alloc_info, &descriptor_set);
	if (result != VK_SUCCESS) {
		throw std::runtime_error("Failed to allocate Texture Descriptor Set!");
	}

	VkDescriptorImageInfo imageInfo = {};
	imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	imageInfo.imageView = texture->ImageView.back();
	imageInfo.sampler = texture->Sampler.back();

	VkWriteDescriptorSet descriptorWrite = {};
	descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptorWrite.dstSet = descriptor_set;
	descriptorWrite.dstBinding = 0;
	descriptorWrite.dstArrayElement = 0;
	descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	descriptorWrite.descriptorCount = 1;
	descriptorWrite.pImageInfo = &imageInfo;

	vkUpdateDescriptorSets(UtilityVk::device->LogicalDevice, 1, &descriptorWrite, 0, nullptr);
	texture->descriptor_set = descriptor_set;
	return texture;
}
#endif
