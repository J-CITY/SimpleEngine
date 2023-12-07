

#include "driverVk.h"

import logger;
//#include "raytracing/dw/include/extensions_vk.h"
//#include "Render/vk/raytracing/dw/include/macros.h"
#ifdef VULKAN_BACKEND
#include <array>
#include <iostream>
#include <set>
#include <stdexcept>
#include <assimp/Importer.hpp>

#include <../../3rd/imgui/imgui/imgui.h>
#include "DebugMessanger.h"
#include "uniformBufferVk.h"
#include <assimp/postprocess.h>
#include "modelVk.h"
#include "textureVk.h"
#include "shaderVk.h"
#include "frameBufferVk.h"
//#include "../../GUI.h"


#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <windowModule/window/window.h>

using namespace IKIGAI;
using namespace IKIGAI::RENDER;
/*
 * TODO:
 * 1) remove all unnececery code
 * 2) SSBO
 * 3) Cubemap
 * 4) Multisamplint on/off
 * -5) depth on/off
 * 6) texture 3d
 * 7) different shader typee support
 * 8) add Opengl
 * 9) move all render pipeline from engine {
 *	...
 * }
 * 10) add new features {
 *	...
 * }
 * 11) add ray tracing
 */


DriverVk::DriverVk() {
	m_VulkanInstance					= 0;
	m_Surface							= 0;
	m_PushCostantRange					= {};
	m_GraphicsQueue						= 0;
	m_PresentationQueue					= 0;
	m_MainDevice.LogicalDevice			= 0;
	m_MainDevice.PhysicalDevice			= 0;
	//m_VPData.proj		= glm::mat4(1.f);
	//m_VPData.view			= glm::mat4(1.f);
	m_MainDevice.MinUniformBufferOffset	= 0;
	
	//m_Descriptors				= Descriptors(&m_MainDevice.LogicalDevice);
	m_SwapChain					= SwapChain(&m_MainDevice, &m_Surface, m_QueueFamilyIndices);
}


int DriverVk::init() {
	try {
		// Instance + Surface + Physical Device + Logical Device
		CreateKernel();

		//init allocataor
		//VmaAllocatorCreateInfo allocator_info = {};
		//allocator_info.flags = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT;
		//allocator_info.physicalDevice = m_MainDevice.PhysicalDevice;
		//allocator_info.device = m_MainDevice.LogicalDevice;
		//allocator_info.instance = m_VulkanInstance;
		//if (vmaCreateAllocator(&allocator_info, &m_vma_allocator) != VK_SUCCESS) {
		//	throw std::runtime_error("(Vulkan) Failed to create Allocator.");
		//}


		//// Get ray tracing pipeline properties, which will be used later on in the sample
		//rayTracingPipelineProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_PROPERTIES_KHR;
		//VkPhysicalDeviceProperties2 deviceProperties2{};
		//deviceProperties2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
		//deviceProperties2.pNext = &rayTracingPipelineProperties;
		//vkGetPhysicalDeviceProperties2(m_MainDevice.PhysicalDevice, &deviceProperties2);
		//
		//// Get acceleration structure properties, which will be used later on in the sample
		//accelerationStructureFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_FEATURES_KHR;
		//VkPhysicalDeviceFeatures2 deviceFeatures2{};
		//deviceFeatures2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
		//deviceFeatures2.pNext = &accelerationStructureFeatures;
		//vkGetPhysicalDeviceFeatures2(m_MainDevice.PhysicalDevice, &deviceFeatures2);
		//
		//// Get the ray tracing and accelertion structure related function pointers required by this sample
		//vkGetBufferDeviceAddressKHR = reinterpret_cast<PFN_vkGetBufferDeviceAddressKHR>(vkGetDeviceProcAddr(m_MainDevice.LogicalDevice, "vkGetBufferDeviceAddressKHR"));
		//vkCmdBuildAccelerationStructuresKHR = reinterpret_cast<PFN_vkCmdBuildAccelerationStructuresKHR>(vkGetDeviceProcAddr(m_MainDevice.LogicalDevice, "vkCmdBuildAccelerationStructuresKHR"));
		//vkBuildAccelerationStructuresKHR = reinterpret_cast<PFN_vkBuildAccelerationStructuresKHR>(vkGetDeviceProcAddr(m_MainDevice.LogicalDevice, "vkBuildAccelerationStructuresKHR"));
		//vkCreateAccelerationStructureKHR = reinterpret_cast<PFN_vkCreateAccelerationStructureKHR>(vkGetDeviceProcAddr(m_MainDevice.LogicalDevice, "vkCreateAccelerationStructureKHR"));
		//vkDestroyAccelerationStructureKHR = reinterpret_cast<PFN_vkDestroyAccelerationStructureKHR>(vkGetDeviceProcAddr(m_MainDevice.LogicalDevice, "vkDestroyAccelerationStructureKHR"));
		//vkGetAccelerationStructureBuildSizesKHR = reinterpret_cast<PFN_vkGetAccelerationStructureBuildSizesKHR>(vkGetDeviceProcAddr(m_MainDevice.LogicalDevice, "vkGetAccelerationStructureBuildSizesKHR"));
		//vkGetAccelerationStructureDeviceAddressKHR = reinterpret_cast<PFN_vkGetAccelerationStructureDeviceAddressKHR>(vkGetDeviceProcAddr(m_MainDevice.LogicalDevice, "vkGetAccelerationStructureDeviceAddressKHR"));
		//vkCmdTraceRaysKHR = reinterpret_cast<PFN_vkCmdTraceRaysKHR>(vkGetDeviceProcAddr(m_MainDevice.LogicalDevice, "vkCmdTraceRaysKHR"));
		//vkGetRayTracingShaderGroupHandlesKHR = reinterpret_cast<PFN_vkGetRayTracingShaderGroupHandlesKHR>(vkGetDeviceProcAddr(m_MainDevice.LogicalDevice, "vkGetRayTracingShaderGroupHandlesKHR"));
		//vkCreateRayTracingPipelinesKHR = reinterpret_cast<PFN_vkCreateRayTracingPipelinesKHR>(vkGetDeviceProcAddr(m_MainDevice.LogicalDevice, "vkCreateRayTracingPipelinesKHR"));

		//load_VK_EXTENSION_SUBSET(m_VulkanInstance, vkGetInstanceProcAddr, m_MainDevice.LogicalDevice, vkGetDeviceProcAddr);
		////load_VK_EXT_buffer_device_address(m_VulkanInstance, vkGetInstanceProcAddr, m_MainDevice.LogicalDevice, vkGetDeviceProcAddr);
		//load_VK_KHR_acceleration_structure(m_VulkanInstance, vkGetInstanceProcAddr, m_MainDevice.LogicalDevice, vkGetDeviceProcAddr);

		//float priority = 1.0f;
		//for (int i = 0; i < m_selected_queues.queue_count; i++) {
		//	m_selected_queues.infos[i].pQueuePriorities = &priority;
		//}
		//
		//vkGetDeviceQueue(m_MainDevice.LogicalDevice, m_selected_queues.graphics_queue_index, 0, &m_vk_graphics_queue);
		//
		//for (int i = 0; i < MAX_COMMAND_THREADS; i++) {
		//	g_graphics_command_buffers[i] = std::make_shared<ThreadLocalCommandBuffers>(m_selected_queues.graphics_queue_index);
		//	//g_compute_command_buffers[i] = std::make_shared<ThreadLocalCommandBuffers>(shared_from_this(), m_selected_queues.compute_queue_index);
		//	//g_transfer_command_buffers[i] = std::make_shared<ThreadLocalCommandBuffers>(shared_from_this(), m_selected_queues.transfer_queue_index);
		//}

		sampleCount = getMaxUsableSampleCount();

		// Swapchain creation
		m_SwapChain.CreateSwapChain();

		m_CommandHandler.CreateCommandPool(m_QueueFamilyIndices);
		m_CommandHandler.CreateCommandBuffers(m_SwapChain.SwapChainImagesSize());

		// Creation of set layouts
		createImguiPool();
		//m_Descriptors.CreateSetLayouts();
		//VkDescriptorSetLayout vp_set_layout		= m_Descriptors.GetViewProjectionSetLayout();
		//VkDescriptorSetLayout tex_set_layout	= m_Descriptors.GetTextureSetLayout();
		//VkDescriptorSetLayout inp_set_layout	= m_Descriptors.GetInputSetLayout();
		//VkDescriptorSetLayout light_set_layout	= m_Descriptors.GetLightSetLayout();
		//VkDescriptorSetLayout settings_set_layout	= m_Descriptors.GetSettingsSetLayout();

		// Textures
		//m_PositionBufferImages = TextureVk::createForAttach(m_SwapChain.GetExtent().width, m_SwapChain.GetExtent().height);
		//m_ColorBufferImages = TextureVk::createForAttach(m_SwapChain.GetExtent().width, m_SwapChain.GetExtent().height);
		//m_NormalBufferImages = TextureVk::createForAttach(m_SwapChain.GetExtent().width, m_SwapChain.GetExtent().height);
		m_DepthBufferImage = TextureVk::createDepthForAttach(m_SwapChain.GetExtent().width, m_SwapChain.GetExtent().height);

		//Frame buffers
		defaultFb = std::make_shared<FrameBufferVk>();
		defaultFb->create(m_DepthBufferImage);
		//auto texs = std::vector{ m_PositionBufferImages, m_ColorBufferImages, m_NormalBufferImages };
		//offScreenFb.create(texs, m_DepthBufferImage);

		// Setting up PushCostant on the pipeline
		SetupPushCostantRange();
		//Shader::push_constant = m_PushCostantRange;
		//
		//Shader::depthWriteEnable = VK_TRUE;
		//Shader::cullMode = VK_CULL_MODE_BACK_BIT;
		//Shader::frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
		//shaderFirst = std::make_shared<Shader>(
		//	"./Shaders/vert.spv", "./Shaders/frag.spv", offScreenFb.m_RenderPass);
		//
		//Shader::push_constant = std::nullopt;
		//Shader::depthWriteEnable = VK_FALSE;
		//Shader::cullMode = VK_CULL_MODE_FRONT_BIT;
		//Shader::frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
		//shaderSecond = std::make_shared<Shader>(
		//	"./Shaders/second_vert.spv", "./Shaders/second_frag.spv", defaultFb.m_RenderPass);

		// Sampler
		//m_TextureObjects.CreateSampler(m_MainDevice);

		// Creation of the UBO for Lights, VP and Settings
		
		//viewProjUbo.create<LightData>();
		//lightUbo.create<LightData>(NUM_LIGHTS);
		//settingsUbo.create<SettingsData>();
		//
		//shaderFirst->createDescriptorSet(0, viewProjUbo);
		////shaderFirst->createDescriptorSet(1, viewProjUbo);
		//
		//shaderSecond->createDescriptorSet(0, {
		//	m_PositionBufferImages ,m_ColorBufferImages , m_NormalBufferImages });
		//shaderSecond->createDescriptorSet(1, lightUbo);
		//shaderSecond->createDescriptorSet(2, settingsUbo);
		
		// Creation of Descriptor Sets
		//CreateUniformBuffers();
		//m_Descriptors.CreateDescriptorPools(m_SwapChain.SwapChainImagesSize(), m_ViewProjectionUBO.size(), m_LightUBO.size(), m_SettingsUBO.size());
		//m_Descriptors.CreateViewProjectionDescriptorSets(m_ViewProjectionUBO, sizeof(ViewProjectionData), m_SwapChain.SwapChainImagesSize());
		//m_Descriptors.CreateInputAttachmentsDescriptorSets(m_SwapChain.SwapChainImagesSize(), m_PositionBufferImages, m_ColorBufferImages, m_NormalBufferImages);
		//m_Descriptors.CreateLightDescriptorSets(m_LightUBO, sizeof(LightData), m_SwapChain.SwapChainImagesSize());
		//m_Descriptors.CreateSettingsDescriptorSets(m_SettingsUBO, sizeof(SettingsData), m_SwapChain.SwapChainImagesSize());

		// Creation of Syn Objects
		CreateSynchronizationObjects();

		// Setting up data for the Data Structures (View-Projection, Lights, Settings)
		//SetUniformDataStructures();

		// Init the Textures
		//TextureLoader::GetInstance()->Init(GetRenderData(), &m_TextureObjects);

		// Loading the scene
		//m_Scene.PassRenderData(GetRenderData());
		//m_Scene.LoadScene(m_MeshList, m_TextureObjects);

		// Create the models
		//CreateMeshModel("Models/Vivi_Final.obj");
		//CreateMeshModel("Models/Vivi_Final.obj");
		//CreateMeshModel("Models/Vivi_Final.obj");
		//CreateMeshModel("Models/FloorTiledMarble.fbx");
	}
	catch (std::runtime_error& e) {
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}
	return 0;
}


//void DriverVk::flush(VkQueue queue, const std::vector<std::shared_ptr<CommandBuffer>>& cmd_bufs) {
//	VkCommandBuffer vk_cmd_bufs[32];
//
//	for (int i = 0; i < cmd_bufs.size(); i++)
//		vk_cmd_bufs[i] = cmd_bufs[i]->handle();
//
//	VkSubmitInfo submit_info;
//	DW_ZERO_MEMORY(submit_info);
//
//	submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
//	submit_info.commandBufferCount = 1;
//	submit_info.pCommandBuffers = &vk_cmd_bufs[0];
//
//	// Create fence to ensure that the command buffer has finished executing
//	VkFenceCreateInfo fence_info;
//	DW_ZERO_MEMORY(fence_info);
//
//	fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
//
//	VkFence fence;
//	vkCreateFence(m_MainDevice.LogicalDevice, &fence_info, nullptr, &fence);
//
//	// Submit to the queue
//	vkQueueSubmit(queue, 1, &submit_info, fence);
//
//	// Wait for the fence to signal that command buffer has finished executing
//	vkWaitForFences(m_MainDevice.LogicalDevice, 1, &fence, VK_TRUE, 100000000000);
//
//	vkDestroyFence(m_MainDevice.LogicalDevice, fence, nullptr);
//}
//
//void DriverVk::flush_graphics(const std::vector<std::shared_ptr<CommandBuffer>>& cmd_bufs) {
//	flush(m_vk_graphics_queue, cmd_bufs);
//
//	for (int i = 0; i < MAX_COMMAND_THREADS; i++) {
//		g_graphics_command_buffers[i]->reset(m_CurrentFrame);
//	}
//}

void DriverVk::begin() {
	VkResult result = vkWaitForFences(m_MainDevice.LogicalDevice, 1, 
		&m_SyncObjects[m_CurrentFrame].InFlight, VK_TRUE,
		std::numeric_limits<uint64_t>::max());

	vkResetFences(m_MainDevice.LogicalDevice, 1, &m_SyncObjects[m_CurrentFrame].InFlight);

	uint32_t image_idx;
	result = vkAcquireNextImageKHR(
		m_MainDevice.LogicalDevice, m_SwapChain.GetSwapChain(),
		std::numeric_limits<uint64_t>::max(),
		m_SyncObjects[m_CurrentFrame].ImageAvailable, VK_NULL_HANDLE, &image_idx);
	imageIndex = image_idx;
}

void DriverVk::submit() {
	VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };

	//TODO: create struct for create dependencies for generate more effective render
	
	while (!callShaderSequence.empty()) {
		auto shader = callShaderSequence.front();
		callShaderSequence.pop();
		VkSubmitInfo submitInfo = {};
		//DW_ZERO_MEMORY(submitInfo);

		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = (prev == nullptr ?
			&m_SyncObjects[m_CurrentFrame].ImageAvailable :
			&prev->sync[m_CurrentFrame]);
		submitInfo.pWaitDstStageMask = waitStages;
		submitInfo.commandBufferCount = 1;

		//TODO: it s hack while command buffer not in shader
		submitInfo.pCommandBuffers = &shader->m_CommandHandler.GetCommandBuffer(imageIndex);

		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = &shader->sync[m_CurrentFrame];

		auto result = vkQueueSubmit(m_GraphicsQueue, 1, &submitInfo, callShaderSequence.empty() ? m_SyncObjects[m_CurrentFrame].InFlight : VK_NULL_HANDLE);
		if (result != VK_SUCCESS) {
			throw std::runtime_error("Failed to submit Command Buffer to Queue!");
		}
		prev = shader;
	}
}

void DriverVk::end() {
	VkPresentInfoKHR presentInfo = {};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = &prev->sync[m_CurrentFrame];
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = m_SwapChain.GetSwapChainData();
	presentInfo.pImageIndices = &imageIndex;
	
	prev = nullptr;
	
	auto result = vkQueuePresentKHR(m_PresentationQueue, &presentInfo);
	if (result == VK_ERROR_OUT_OF_DATE_KHR) {
		HandleMinimization();
		return;
	}
	else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
		throw std::runtime_error("Failed to present the image!");
	}
	m_CurrentFrame = (m_CurrentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

void DriverVk::setClierColor() {
	
}

MATHGL::Vector4 DriverVk::getClearColor() {
	return clearColor;
}

void DriverVk::setViewport(const ShaderInterface& shader, float x, float y, float w, float h) {
	VkViewport viewport = { x, y, w, h, 0.0, 1.0 };
	vkCmdSetViewport(static_cast<const ShaderVk&>(shader).m_CommandHandler.m_CommandBuffers[imageIndex], 0, 1, &viewport);
}

void DriverVk::setScissor(const ShaderInterface& shader, int x, int y, unsigned w, unsigned h) {
	VkRect2D scissor = {};
	scissor.offset = { x, y };
	scissor.extent = { w, h };
	vkCmdSetScissor(static_cast<const ShaderVk&>(shader).m_CommandHandler.m_CommandBuffers[imageIndex], 0, 1, &scissor);

}

void DriverVk::beginCommandBuffer(const ShaderVk& shader) {
	VkCommandBufferBeginInfo buffer_begin_info = {};
	buffer_begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	buffer_begin_info.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT; // Il buffer può essere re-inviato al momento della resubmit

	VkResult res = vkBeginCommandBuffer(shader.m_CommandHandler.m_CommandBuffers[imageIndex], &buffer_begin_info);
	if (res != VK_SUCCESS) {
		throw std::runtime_error("Failed to start recording a Command Buffer!");
	}
}

void DriverVk::endCommandBuffer(const ShaderVk& shader) {
	auto res = vkEndCommandBuffer(shader.m_CommandHandler.m_CommandBuffers[imageIndex]);
	if (res != VK_SUCCESS) {
		throw std::runtime_error("Failed to stop recording offscreen Command Buffer!");
	}
}


void DriverVk::HandleMinimization()
{
	//int width = 0, height = 0;
	//glfwGetFramebufferSize(m_Window, &width, &height);
	//
	//while (width == 0 || height == 0) {
	//	glfwGetFramebufferSize(m_Window, &width, &height);
	//	glfwWaitEvents();
	//}
	//
	//vkDeviceWaitIdle(m_MainDevice.LogicalDevice);
	//
	////shaderSecond->m_CommandHandler.FreeCommandBuffers();
	//
	////m_GraphicPipeline.DestroyPipeline();
	//
	//m_SwapChain.DestroyFrameBuffers();
	//m_SwapChain.DestroySwapChainImageViews();
	//m_SwapChain.DestroySwapChain();
	//m_SwapChain.SetRecreationStatus(true);
	//m_SwapChain.CreateSwapChain();
	//m_SwapChain.SetRecreationStatus(false);

	//m_RenderPassHandler.CreateRenderPass();

	//m_GraphicPipeline.CreateGraphicPipeline();
	//m_PositionBufferImages = TextureVk::createForAttach(m_SwapChain.GetExtent().width, m_SwapChain.GetExtent().height);
	//m_ColorBufferImages = TextureVk::createForAttach(m_SwapChain.GetExtent().width, m_SwapChain.GetExtent().height);
	//m_NormalBufferImages = TextureVk::createForAttach(m_SwapChain.GetExtent().width, m_SwapChain.GetExtent().height);

	//m_PositionBufferImages.resize(m_SwapChain.SwapChainImagesSize());
	//m_ColorBufferImages.resize(m_SwapChain.SwapChainImagesSize());
	//m_NormalBufferImages.resize(m_SwapChain.SwapChainImagesSize());
	//
	//for (size_t i = 0; i < m_ColorBufferImages.size(); i++)
	//{
	//	Utility::CreatePositionBufferImage(m_PositionBufferImages[i], m_SwapChain.GetExtent());
	//	Utility::CreatePositionBufferImage(m_ColorBufferImages[i], m_SwapChain.GetExtent());
	//	Utility::CreatePositionBufferImage(m_NormalBufferImages[i], m_SwapChain.GetExtent());
	//}
	//m_DepthBufferImage = TextureVk::createDepthForAttach(m_SwapChain.GetExtent().width, m_SwapChain.GetExtent().height);

	//Utility::CreateDepthBufferImage(m_DepthBufferImage, m_SwapChain.GetExtent());

	//m_SwapChain.CreateFrameBuffers(m_DepthBufferImage->ImageView[0], *m_ColorBufferImages);
	//defaultFb.create(m_DepthBufferImage);
	//shaderSecond->m_CommandHandler.CreateCommandBuffers(defaultFb.swapChainFramebuffers.size());
}



void DriverVk::enableExtensions(std::vector<const char*>& enabledDeviceExtensions)
{
	bool require_ray_tracing = true;
	enabledDeviceExtensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);

	if (require_ray_tracing)
	{
		enabledDeviceExtensions.push_back(VK_KHR_DEDICATED_ALLOCATION_EXTENSION_NAME);
		enabledDeviceExtensions.push_back(VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME);
		enabledDeviceExtensions.push_back(VK_KHR_RAY_QUERY_EXTENSION_NAME);
		enabledDeviceExtensions.push_back(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME);
		enabledDeviceExtensions.push_back(VK_KHR_SPIRV_1_4_EXTENSION_NAME);
		enabledDeviceExtensions.push_back(VK_KHR_PIPELINE_LIBRARY_EXTENSION_NAME);
		enabledDeviceExtensions.push_back(VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME);
		enabledDeviceExtensions.push_back(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME);
	}
	
	enabledDeviceExtensions.push_back(VK_KHR_SHADER_FLOAT_CONTROLS_EXTENSION_NAME);
	enabledDeviceExtensions.push_back(VK_KHR_GET_MEMORY_REQUIREMENTS_2_EXTENSION_NAME);
	enabledDeviceExtensions.push_back(VK_KHR_MAINTENANCE3_EXTENSION_NAME);
	enabledDeviceExtensions.push_back(VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME);
	enabledDeviceExtensions.push_back(VK_EXT_SCALAR_BLOCK_LAYOUT_EXTENSION_NAME);


	

	//// Ray tracing related extensions required by this sample
	//enabledDeviceExtensions.push_back(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME);
	//bool rayQueryOnly = false;
	//if (!rayQueryOnly) {
	//	enabledDeviceExtensions.push_back(VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME);
	//}
	//
	//// Required by VK_KHR_acceleration_structure
	//enabledDeviceExtensions.push_back(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME);
	//enabledDeviceExtensions.push_back(VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME);
	//enabledDeviceExtensions.push_back(VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME);
	//
	//// Required for VK_KHR_ray_tracing_pipeline
	//enabledDeviceExtensions.push_back(VK_KHR_SPIRV_1_4_EXTENSION_NAME);
	//
	//// Required by VK_KHR_spirv_1_4
	//enabledDeviceExtensions.push_back(VK_KHR_SHADER_FLOAT_CONTROLS_EXTENSION_NAME);
}
#define ENABLED_VALIDATION_LAYERS
void DriverVk::CreateInstance()
{
	std::vector<const char*> validationLayers = { "VK_LAYER_KHRONOS_validation" };

#ifdef ENABLED_VALIDATION_LAYERS
	if (!CheckValidationLayerSupport(&validationLayers))
		throw std::runtime_error("VkInstance doesn't support the required validation layers");
#endif 

	VkApplicationInfo appInfo = {};

	appInfo.sType				= VK_STRUCTURE_TYPE_APPLICATION_INFO; 
	appInfo.pApplicationName	= "Vulkan Render Application";		  
	appInfo.applicationVersion  = VK_MAKE_VERSION(1, 0, 0);			  
	appInfo.pEngineName			= "VULKAN RENDERER";				  
	appInfo.engineVersion		= VK_MAKE_VERSION(1, 0, 0);			  
	appInfo.apiVersion			= VK_API_VERSION_1_2;				  

	VkInstanceCreateInfo createInfo = {};
	createInfo.sType				= VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;	
	createInfo.pApplicationInfo		= &appInfo;									

	std::vector <const char*> instanceExtensions = std::vector<const char*>(); 
	LoadGlfwExtensions(instanceExtensions);									   
	instanceExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
	instanceExtensions.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
	//for RT
	//enableExtensions(instanceExtensions);

	if (!CheckInstanceExtensionSupport(&instanceExtensions))								
		throw std::runtime_error("VkInstance doesn't support the required extensions");		
	
	createInfo.enabledExtensionCount   = static_cast<uint32_t>(instanceExtensions.size()); 
	createInfo.ppEnabledExtensionNames = instanceExtensions.data();						   
	
#ifdef ENABLED_VALIDATION_LAYERS
	createInfo.enabledLayerCount	= static_cast<uint32_t>(validationLayers.size());   
	createInfo.ppEnabledLayerNames	= validationLayers.data();						 
#else
	createInfo.enabledLayerCount = 0;								
	createInfo.ppEnabledLayerNames = nullptr;						
#endif
	VkResult res = vkCreateInstance(&createInfo, nullptr, &m_VulkanInstance);					   

	if (res != VK_SUCCESS)
		throw std::runtime_error("Failed to create Vulkan instance");

#ifdef ENABLED_VALIDATION_LAYERS
	DebugMessanger::GetInstance()->SetupDebugMessenger(m_VulkanInstance);
#endif
}

void DriverVk::CreateKernel() {
	CreateInstance();
	CreateSurface();
	RetrievePhysicalDevice();
	CreateLogicalDevice();

	UtilityVk::device = &m_MainDevice;
	UtilityVk::useMSAA = useMSAA ;
	UtilityVk::sampleCount = sampleCount;
	UtilityVk::m_SwapChain = &m_SwapChain;
	UtilityVk::m_CommandHandler = &m_CommandHandler;
	UtilityVk::m_GraphicsQueue = &m_GraphicsQueue;;
}

void DriverVk::LoadGlfwExtensions(std::vector<const char*>& instanceExtensions)
{
	uint32_t glfwExtensionCount = 0;	
	const char** glfwExtensions;		

	glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount); 
	for (size_t i = 0; i < glfwExtensionCount; ++i)							 
		instanceExtensions.push_back(glfwExtensions[i]);
}


bool DriverVk::CheckInstanceExtensionSupport(std::vector<const char*>* extensionsToCheck) {
	uint32_t nAvailableExt = 0;
	vkEnumerateInstanceExtensionProperties(nullptr, &nAvailableExt, nullptr);
	
	std::vector<VkExtensionProperties> availableExt(nAvailableExt);						
	vkEnumerateInstanceExtensionProperties(nullptr, &nAvailableExt, availableExt.data());

	for (const auto& proposedExt : *extensionsToCheck)			
	{																
		bool hasExtension = false;
		for (const auto& extension : availableExt)
		{
			if (strcmp(proposedExt, extension.extensionName))
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

bool DriverVk::CheckValidationLayerSupport(std::vector<const char*>* validationLayers)
{
	uint32_t nAvailableLayers;
	vkEnumerateInstanceLayerProperties(&nAvailableLayers, nullptr);				 

	std::vector<VkLayerProperties> availableLayers(nAvailableLayers);				 
	vkEnumerateInstanceLayerProperties(&nAvailableLayers, availableLayers.data());

	for (const auto& layerName : *validationLayers)					
	{																
		bool layerFound = false;

		for (const auto& layerProperties : availableLayers)
		{
			if (strcmp(layerName, layerProperties.layerName) == 0)
			{
				layerFound = true;
				break;
			}
		}

		if (!layerFound) {
			return false;
		}
	}

	return true;
}

void DriverVk::RetrievePhysicalDevice()
{
	uint32_t deviceCount = 0;

	vkEnumeratePhysicalDevices(m_VulkanInstance, &deviceCount, nullptr);

	if (deviceCount == 0)										  
		throw std::runtime_error("Can't find GPU that support Vulkan Instance!");

	std::vector<VkPhysicalDevice> deviceList(deviceCount);
	vkEnumeratePhysicalDevices(m_VulkanInstance, &deviceCount, deviceList.data()); 

	for (const auto& device : deviceList)			
	{												
		if (CheckDeviceSuitable(device))
		{
			m_MainDevice.PhysicalDevice = device;
			break;
		}
	}

	VkPhysicalDeviceProperties deviceProperties;
	vkGetPhysicalDeviceProperties(m_MainDevice.PhysicalDevice, &deviceProperties);

	m_MainDevice.MinUniformBufferOffset = deviceProperties.limits.minUniformBufferOffsetAlignment;// serve per DYNAMIC UBO
	
	LOG_INFO("(Vulkan) Name   : " + std::string(deviceProperties.deviceName));
}

bool DriverVk::CheckDeviceSuitable(VkPhysicalDevice possibleDevice)
{
	 /*Al momento non ci interessano particolari caratteristiche della GPU

	// Informazioni generiche a riguardo del dispositivo
	VkPhysicalDeviceProperties deviceProperties;
	vkGetPhysicalDeviceProperties(device, &deviceProperties);*/

/*	// Informazioni rispetto ai servizi che offre il dispositvo
	VkPhysicalDeviceFeatures deviceFeatures;
	vkGetPhysicalDeviceFeatures(device, &deviceFeatures);
	*/

	// Preleva dal dispositivo fisico gli indici delle QueueFamily per la Grafica e la Presentazione
	UtilityVk::GetPossibleQueueFamilyIndices(m_Surface, possibleDevice, m_QueueFamilyIndices);

	// Controlla che le estensioni richieste siano disponibili nel dispositivo fisico
	bool const extensionSupported = UtilityVk::CheckPossibleDeviceExtensionSupport(possibleDevice, m_RequestedDeviceExtensions);


	bool swapChainValid	= false;

	// Se le estensioni richieste sono supportate (quindi Surface compresa), si procede con la SwapChain
	if (extensionSupported)
	{						
		SwapChainDetails swapChainDetails = m_SwapChain.GetSwapChainDetails(possibleDevice, m_Surface);
		swapChainValid = !swapChainDetails.presentationModes.empty() && !swapChainDetails.formats.empty();
	}

	return m_QueueFamilyIndices.isValid() && extensionSupported && swapChainValid;
}
bool require_ray_tracing = true;
void DriverVk::CreateLogicalDevice()
{
	VkPhysicalDeviceRayQueryFeaturesKHR device_ray_query_features;
	
	device_ray_query_features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_QUERY_FEATURES_KHR;
	device_ray_query_features.pNext = nullptr;
	device_ray_query_features.rayQuery = VK_TRUE;

	// Acceleration Structure Features
	VkPhysicalDeviceAccelerationStructureFeaturesKHR device_acceleration_structure_features;
	
	device_acceleration_structure_features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_FEATURES_KHR;
	device_acceleration_structure_features.pNext = &device_ray_query_features;
	device_acceleration_structure_features.accelerationStructure = VK_TRUE;

	// Ray Tracing Features
	VkPhysicalDeviceRayTracingPipelineFeaturesKHR device_ray_tracing_pipeline_features;
	
	device_ray_tracing_pipeline_features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_FEATURES_KHR;
	device_ray_tracing_pipeline_features.pNext = &device_acceleration_structure_features;
	device_ray_tracing_pipeline_features.rayTracingPipeline = VK_TRUE;

	// Vulkan 1.1/1.2 Features
	VkPhysicalDeviceVulkan11Features features11;
	VkPhysicalDeviceVulkan12Features features12;

	features11.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES;
	features11.pNext = &features12;

	features12.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES;

	if (require_ray_tracing)
		features12.pNext = &device_ray_tracing_pipeline_features;

	// Physical Device Features 2
	VkPhysicalDeviceFeatures2 physical_device_features_2;

	physical_device_features_2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
	physical_device_features_2.pNext = &features11;

	vkGetPhysicalDeviceFeatures2(m_MainDevice.PhysicalDevice, &physical_device_features_2);

	physical_device_features_2.features.robustBufferAccess = VK_FALSE;



	std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
	std::set<uint32_t> queueFamilyIndices = {
		m_QueueFamilyIndices.GraphicsFamily , m_QueueFamilyIndices.PresentationFamily,
		m_QueueFamilyIndices.ComputeFamily, m_QueueFamilyIndices.TransferFamily
	};

	for (int queueFamilyIndex : queueFamilyIndices)
	{
		VkDeviceQueueCreateInfo queueCreateInfo = {};
		queueCreateInfo.sType			 = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfo.queueFamilyIndex = queueFamilyIndex;
		queueCreateInfo.queueCount		 = 1;
		const float priority			 = 1.f;
		queueCreateInfo.pQueuePriorities = &priority;
	
		queueCreateInfos.push_back(queueCreateInfo);
	}

	//for RT
	enableExtensions(m_RequestedDeviceExtensions);

	// LOGICAL DEVICE
	VkDeviceCreateInfo deviceCreateInfo = {};
	deviceCreateInfo.sType					 = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	deviceCreateInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
	deviceCreateInfo.pQueueCreateInfos		 = queueCreateInfos.data();
	deviceCreateInfo.enabledExtensionCount   = static_cast<uint32_t>(m_RequestedDeviceExtensions.size());
	deviceCreateInfo.ppEnabledExtensionNames = m_RequestedDeviceExtensions.data();

	VkPhysicalDeviceFeatures deviceFeatures = {};
	deviceFeatures.samplerAnisotropy	= VK_TRUE;
	deviceFeatures.geometryShader = true;
	deviceFeatures.tessellationShader = true;

	//deviceCreateInfo.pEnabledFeatures	= &deviceFeatures;
	deviceCreateInfo.pEnabledFeatures = nullptr;
	deviceCreateInfo.pNext = &physical_device_features_2;

	
	//vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

	VkResult result	= vkCreateDevice(m_MainDevice.PhysicalDevice, &deviceCreateInfo, nullptr, &m_MainDevice.LogicalDevice);	// Creo il device logico

	if (result != VK_SUCCESS)	// Nel caso in cui il Dispositivo Logico non venga creato con successo alzo un eccezione a runtime.
		throw std::runtime_error("Failed to create Logical Device!");

	vkGetDeviceQueue(
		m_MainDevice.LogicalDevice,
		m_QueueFamilyIndices.GraphicsFamily, 
		0,
		&m_GraphicsQueue);
		
	vkGetDeviceQueue(
		m_MainDevice.LogicalDevice,
		m_QueueFamilyIndices.PresentationFamily,
		0,
		&m_PresentationQueue);

	vkGetDeviceQueue(
		m_MainDevice.LogicalDevice,
		m_QueueFamilyIndices.ComputeFamily,
		0,
		&m_ComputeQueue);

	vkGetDeviceQueue(
		m_MainDevice.LogicalDevice,
		m_QueueFamilyIndices.TransferFamily,
		0,
		&m_TransferQueue);
}

#include <coreModule/resourceManager/ServiceManager.h>
#include <GLFW/glfw3.h>
void DriverVk::CreateSurface()
{
	auto& win = RESOURCES::ServiceManager::Get<WINDOW_SYSTEM::Window>();
	
	VkResult res = glfwCreateWindowSurface(m_VulkanInstance, win.getGLFWWin(), nullptr, &m_Surface);
																		
	if (res != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create the surface!");
	}
}


const VulkanRenderData DriverVk::GetRenderData() {
	VulkanRenderData data = {};
	data.main_device = m_MainDevice;
	data.instance = m_VulkanInstance;
	data.physical_device = m_MainDevice.PhysicalDevice;
	data.device = m_MainDevice.LogicalDevice;
	data.graphic_queue_index = m_QueueFamilyIndices.GraphicsFamily;
	data.graphic_queue = m_GraphicsQueue;
	data.imgui_descriptor_pool = m_ImguiDescriptorPool;
	data.min_image_count = 3;	// setup correct practice
	data.image_count = 3;	// setup correct practice
	//data.render_pass				= m_RenderPassHandler.GetRenderPass();
	//data.command_pool				= m_CommandHandler.GetCommandPool();
	//data.command_buffers			= m_CommandHandler.GetCommandBuffers();
	//data.texture_descriptor_layout	= m_Descriptors.GetTextureSetLayout();
	//data.texture_descriptor_pool	= m_Descriptors.GetTexturePool();

	return data;
}

const int DriverVk::GetCurrentFrame() const
{
	return m_CurrentFrame;
}

void DriverVk::CreateSynchronizationObjects()
{
	m_SyncObjects.resize(MAX_FRAMES_IN_FLIGHT);

	VkSemaphoreCreateInfo semaphore_info = {};
	semaphore_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	VkFenceCreateInfo fence_info = {};
	fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fence_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;	

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
	{
		//VkResult offscreen_available_sem = vkCreateSemaphore(m_MainDevice.LogicalDevice, &semaphore_info, nullptr, &m_SyncObjects[i].OffScreenAvailable);
		VkResult image_available_sem	 = vkCreateSemaphore(m_MainDevice.LogicalDevice, &semaphore_info, nullptr, &m_SyncObjects[i].ImageAvailable);
		//VkResult render_finished_sem	 = vkCreateSemaphore(m_MainDevice.LogicalDevice, &semaphore_info, nullptr, &m_SyncObjects[i].RenderFinished);
		VkResult in_flight_fence		 = vkCreateFence(m_MainDevice.LogicalDevice, &fence_info, nullptr, &m_SyncObjects[i].InFlight);

		if (image_available_sem		!= VK_SUCCESS ||
			in_flight_fence			!= VK_SUCCESS)
			throw std::runtime_error("Failed to create semaphores and/or Fence!");
	}
}

void DriverVk::SetupPushCostantRange()
{
	m_PushCostantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;  // Stage dove finiranno le push costant
	m_PushCostantRange.offset	  = 0;
	m_PushCostantRange.size		  = sizeof(ModelVk);
}

void DriverVk::cleanup()
{
	// Aspetta finchè nessun azione sia eseguita sul device senza distruggere niente
	// Tutte le operazioni effettuate all'interno della draw() sono in asincrono.
	// Questo significa che all'uscita del loop della init(), le operazionio di drawing
	// e di presentazione potrebbero ancora essere in corso ed eliminare le risorse mentre esse sono in corso è una pessima idea
	// quindi è corretto aspettare che il dispositivo sia inattivo prima di eliminare gli oggetti.
	vkDeviceWaitIdle(m_MainDevice.LogicalDevice);

	//for (size_t i = 0; i < m_MeshModelList.size(); i++) {
	//	m_MeshModelList[i].DestroyMeshModel();
	//}

	//GUI::GetInstance()->Destroy();

	//TODO: clean up pools
	//m_Descriptors.DestroyImguiPool();
	//m_Descriptors.DestroyTexturePool();
	//m_Descriptors.DestroyTextureLayout();

	//vkDestroySampler(m_MainDevice.LogicalDevice, m_TextureObjects.TextureSampler, nullptr);
	//
	//for (size_t i = 0; i < m_TextureObjects.TextureImages.size(); i++)
	//{
	//	vkDestroyImageView(m_MainDevice.LogicalDevice, m_TextureObjects.TextureImageViews[i], nullptr);
	//	vkDestroyImage(m_MainDevice.LogicalDevice, m_TextureObjects.TextureImages[i], nullptr);
	//	vkFreeMemory(m_MainDevice.LogicalDevice, m_TextureObjects.TextureImageMemory[i], nullptr);
	//}

	//m_Descriptors.DestroyInputPool();
	//m_Descriptors.DestroyInputAttachmentsLayout();
	//for (size_t i = 0; i < m_ColorBufferImages->ImageView.size(); i++)
	//{
		//m_PositionBufferImages[i].DestroyAndFree(m_MainDevice);
		//m_ColorBufferImages[i].DestroyAndFree(m_MainDevice);
		//m_NormalBufferImages[i].DestroyAndFree(m_MainDevice);
	//}

	//m_DepthBufferImage.DestroyAndFree(m_MainDevice);
	
	//m_Descriptors.DestroyViewProjectionPool();
	//m_Descriptors.DestroyViewProjectionLayout();

	//for (size_t i = 0; i < m_ViewProjectionUBO.size(); ++i)
	//{
	//	vkDestroyBuffer(m_MainDevice.LogicalDevice, m_ViewProjectionUBO[i], nullptr);
	//	vkFreeMemory(m_MainDevice.LogicalDevice, m_ViewProjectionUBOMemory[i], nullptr);
	//}

	//m_Descriptors.DestroyLightPool();
	//m_Descriptors.DestroyLightLayout();

	//for (size_t i = 0; i < m_LightUBO.size(); ++i)
	//{
	//	vkDestroyBuffer(m_MainDevice.LogicalDevice, m_LightUBO[i], nullptr);
	//	vkFreeMemory(m_MainDevice.LogicalDevice, m_LightUBOMemory[i], nullptr);
	//}

	//m_Descriptors.DestroySettingsPool();
	//m_Descriptors.DestroySettingsLayout();

	//for (size_t i = 0; i < m_SettingsUBO.size(); ++i)
	//{
	//	vkDestroyBuffer(m_MainDevice.LogicalDevice, m_SettingsUBO[i], nullptr);
	//	vkFreeMemory(m_MainDevice.LogicalDevice, m_SettingsUBOMemory[i], nullptr);
	//}



	//for (size_t i = 0; i < m_MeshList.size(); i++)
	//{
	//	m_MeshList[i].destroyBuffers();
	//}
	//TODO: destroy sync object in shader
	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
	{
		//vkDestroySemaphore(m_MainDevice.LogicalDevice, m_SyncObjects[i].RenderFinished, nullptr);
		vkDestroySemaphore(m_MainDevice.LogicalDevice, m_SyncObjects[i].ImageAvailable, nullptr);
		//vkDestroySemaphore(m_MainDevice.LogicalDevice, m_SyncObjects[i].OffScreenAvailable, nullptr);
		vkDestroyFence(m_MainDevice.LogicalDevice, m_SyncObjects[i].InFlight, nullptr);
	}

	//TODO: add destroy to shader
	//m_OffScreenCommandHandler.DestroyCommandPool();
	//m_CommandHandler.DestroyCommandPool();

	//for (auto framebuffer : offScreenFb.swapChainFramebuffers)
	//	vkDestroyFramebuffer(m_MainDevice.LogicalDevice, framebuffer, nullptr);
	m_SwapChain.DestroyFrameBuffers();

	//m_GraphicPipeline.DestroyPipeline();

	//TODO: destroy fb and rander pass
	//m_RenderPassHandler.DestroyRenderPass();

	m_SwapChain.DestroySwapChainImageViews();
	m_SwapChain.DestroySwapChain();

	vkDestroySurfaceKHR(m_VulkanInstance, m_Surface, nullptr);	// Distrugge la Surface (GLFW si utilizza solo per settarla)

#ifdef ENABLED_VALIDATION_LAYERS
	DebugMessanger::GetInstance()->Clear();
#endif

	vkDestroyDevice(m_MainDevice.LogicalDevice, nullptr);
	vkDestroyInstance(m_VulkanInstance, nullptr);					
}

void DriverVk::createImguiPool() {
	VkDescriptorPoolSize imguiPoolSize[] = {
		{ VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
		{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
		{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
		{ VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
	};

	VkDescriptorPoolCreateInfo imgui_pool = {};
	imgui_pool.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	imgui_pool.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
	imgui_pool.maxSets = 1000 * IM_ARRAYSIZE(imguiPoolSize);
	imgui_pool.poolSizeCount = (uint32_t)IM_ARRAYSIZE(imguiPoolSize);
	imgui_pool.pPoolSizes = imguiPoolSize;

	VkResult result = vkCreateDescriptorPool(m_MainDevice.LogicalDevice, &imgui_pool, nullptr, &m_ImguiDescriptorPool);
	if (result != VK_SUCCESS) {
		throw std::runtime_error("Failed to create a Descriptor Pool!");
	}
}

//const VulkanRenderData DriverVk::GetRenderData()
//{
//	VulkanRenderData data = {};
//	data.main_device				= m_MainDevice;
//	data.instance					= m_VulkanInstance;
//	data.physical_device			= m_MainDevice.PhysicalDevice;
//	data.device						= m_MainDevice.LogicalDevice;
//	data.graphic_queue_index		= m_QueueFamilyIndices.GraphicsFamily;
//	data.graphic_queue				= m_GraphicsQueue;
//	data.imgui_descriptor_pool		= m_ImguiDescriptorPool;
//	data.min_image_count			= 3;	// setup correct practice
//	data.image_count				= 3;	// setup correct practice
//	//data.render_pass				= m_RenderPassHandler.GetRenderPass();
//	//data.command_pool				= m_CommandHandler.GetCommandPool();
//	//data.command_buffers			= m_CommandHandler.GetCommandBuffers();
//	//data.texture_descriptor_layout	= m_Descriptors.GetTextureSetLayout();
//	//data.texture_descriptor_pool	= m_Descriptors.GetTexturePool();
//
//	return data;
//}

//SettingsData* DriverVk::GetUBOSettingsRef()
//{
//	return &m_SettingsData;
//}

DriverVk::~DriverVk()
{
	//if (m_vma_allocator) {
	//	vmaDestroyAllocator(m_vma_allocator);
	//	m_vma_allocator = nullptr;
	//}
	//for (int i = 0; i < MAX_COMMAND_THREADS; i++) {
	//	g_graphics_command_buffers[i].reset();
	//}
	cleanup();
}

VkAttachmentDescription DriverVk::InputPositionAttachment(const VkFormat& imageFormat)
{
	VkAttachmentDescription color_attachment_input = {};

	color_attachment_input.format = UtilityVk::ChooseSupportedFormat(
		{ VK_FORMAT_R32G32B32A32_SFLOAT },
		VK_IMAGE_TILING_OPTIMAL,
		VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

	color_attachment_input.samples = VK_SAMPLE_COUNT_1_BIT;
	color_attachment_input.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	color_attachment_input.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	color_attachment_input.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	color_attachment_input.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	color_attachment_input.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	color_attachment_input.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL; // VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	return color_attachment_input;
}

VkAttachmentDescription DriverVk::InputDepthAttachment()
{
	std::vector<VkFormat> formats = { VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D32_SFLOAT, VK_FORMAT_D24_UNORM_S8_UINT };

	VkAttachmentDescription depth_attachment_desc = {};

	depth_attachment_desc.format = UtilityVk::ChooseSupportedFormat(formats, VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
	depth_attachment_desc.samples = VK_SAMPLE_COUNT_1_BIT;
	depth_attachment_desc.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	depth_attachment_desc.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	depth_attachment_desc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	depth_attachment_desc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depth_attachment_desc.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	depth_attachment_desc.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	return depth_attachment_desc;
}

std::array<VkSubpassDependency, 2> DriverVk::SetSubpassDependencies() {
	std::array<VkSubpassDependency, 2> dependencies = {};

	// VK_IMAGE_LAYOUT_UNDEFINED -> VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
	dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
	dependencies[0].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
	dependencies[0].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
	dependencies[0].dstSubpass = 0;
	dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	dependencies[0].dependencyFlags = 0;

	dependencies[1].srcSubpass = 0;
	dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
	dependencies[1].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
	dependencies[1].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
	dependencies[1].dependencyFlags = 0;

	return dependencies;
}

VkAttachmentDescription DriverVk::SwapchainColourAttachment(const VkFormat& imageFormat)
{
	VkAttachmentDescription colour_attachment = {};
	colour_attachment.flags = 0;
	colour_attachment.format = imageFormat;
	colour_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
	colour_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	colour_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	colour_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colour_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	colour_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	colour_attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	return colour_attachment;
}

void DriverVk::drawIndexed(std::shared_ptr<ShaderInterface> shader, size_t indexCount) {
	vkCmdDrawIndexed(std::dynamic_pointer_cast<ShaderVk>(shader)->m_CommandHandler.m_CommandBuffers[imageIndex], indexCount, 1, 0, 0, 0);
}

void DriverVk::draw(std::shared_ptr<ShaderInterface> shader, size_t vertexCount) {
	vkCmdDraw(std::dynamic_pointer_cast<ShaderVk>(shader)->m_CommandHandler.m_CommandBuffers[imageIndex], vertexCount, 1, 0, 0);
}

void DriverVk::draw(std::shared_ptr<ShaderInterface> shader, const MeshInterface& mesh, size_t instanceCount) {
	reinterpret_cast<const MeshVk&>(mesh).bind(*shader);
	drawIndexed(shader, mesh.getIndexCount());
}

VkSampleCountFlagBits DriverVk::getMaxUsableSampleCount() {
	VkPhysicalDeviceProperties physicalDeviceProperties;
	vkGetPhysicalDeviceProperties(m_MainDevice.PhysicalDevice, &physicalDeviceProperties);

	VkSampleCountFlags counts = physicalDeviceProperties.limits.framebufferColorSampleCounts & physicalDeviceProperties.limits.framebufferDepthSampleCounts;
	if (counts & VK_SAMPLE_COUNT_64_BIT) { return VK_SAMPLE_COUNT_64_BIT; }
	if (counts & VK_SAMPLE_COUNT_32_BIT) { return VK_SAMPLE_COUNT_32_BIT; }
	if (counts & VK_SAMPLE_COUNT_16_BIT) { return VK_SAMPLE_COUNT_16_BIT; }
	if (counts & VK_SAMPLE_COUNT_8_BIT) { return VK_SAMPLE_COUNT_8_BIT; }
	if (counts & VK_SAMPLE_COUNT_4_BIT) { return VK_SAMPLE_COUNT_4_BIT; }
	if (counts & VK_SAMPLE_COUNT_2_BIT) { return VK_SAMPLE_COUNT_2_BIT; }

	return VK_SAMPLE_COUNT_1_BIT;
}

//VmaAllocator_T* DriverVk::allocator() {
//	return m_vma_allocator;
//}
#endif
