#pragma once

#ifdef VULKAN_BACKEND
#include <queue>
#include "swapChainHandler.h"
#include "commandHandler.h"
#include <assimp/scene.h>
#include "../interface/driverInterface.h"

import glmath;

namespace KUMA::RENDER {
	class MeshInterface;
	class ShaderVk;
	class FrameBufferVk;
	struct ImDrawData;
	class CommandBuffer;
	
	class DriverVk : public DriverInterface {
	public:
		//PFN_vkGetBufferDeviceAddressKHR vkGetBufferDeviceAddressKHR;
		//PFN_vkCreateAccelerationStructureKHR vkCreateAccelerationStructureKHR;
		//PFN_vkDestroyAccelerationStructureKHR vkDestroyAccelerationStructureKHR;
		//PFN_vkGetAccelerationStructureBuildSizesKHR vkGetAccelerationStructureBuildSizesKHR;
		//PFN_vkGetAccelerationStructureDeviceAddressKHR vkGetAccelerationStructureDeviceAddressKHR;
		//PFN_vkCmdBuildAccelerationStructuresKHR vkCmdBuildAccelerationStructuresKHR;
		//PFN_vkBuildAccelerationStructuresKHR vkBuildAccelerationStructuresKHR;
		//PFN_vkCmdTraceRaysKHR vkCmdTraceRaysKHR;
		//PFN_vkGetRayTracingShaderGroupHandlesKHR vkGetRayTracingShaderGroupHandlesKHR;
		//PFN_vkCreateRayTracingPipelinesKHR vkCreateRayTracingPipelinesKHR;
		//
		//VkPhysicalDeviceRayTracingPipelinePropertiesKHR  rayTracingPipelineProperties{};
		//VkPhysicalDeviceAccelerationStructureFeaturesKHR accelerationStructureFeatures{};
		//
		//VkPhysicalDeviceBufferDeviceAddressFeatures enabledBufferDeviceAddresFeatures{};
		//VkPhysicalDeviceRayTracingPipelineFeaturesKHR enabledRayTracingPipelineFeatures{};
		//VkPhysicalDeviceAccelerationStructureFeaturesKHR enabledAccelerationStructureFeatures{};

	public:
		static const uint32_t kMaxFramesInFlight = 2;
		MATHGL::Vector4 clearColor = { 0.0f,0.0f ,0.0f ,0.0f };

		std::queue<std::shared_ptr<ShaderVk>> callShaderSequence;
		std::shared_ptr<ShaderVk> prev;


		DriverVk();
	public:
		CommandHandler m_CommandHandler;
		std::shared_ptr<FrameBufferVk> defaultFb;
		uint32_t imageIndex = 0;

		~DriverVk();

		void begin();
		void end();
		void setClierColor();
		MATHGL::Vector4 getClearColor();
		void setViewport(const ShaderInterface& shader, float x, float y, float w, float h) override;
		void setScissor(const ShaderInterface& shader, int x, int y, unsigned w, unsigned h) override;
		void beginCommandBuffer(const ShaderVk& shader);
		void endCommandBuffer(const ShaderVk& shader);
		void submit() override;
		int init() override;
		void Draw(ImDrawData* draw_data);
		void cleanup() override;

		const VulkanRenderData GetRenderData();
		int const GetCurrentFrame() const;

		VkDescriptorPool m_ImguiDescriptorPool;
		void createImguiPool();

		static VkAttachmentDescription InputPositionAttachment(const VkFormat& imageFormat);
		static VkAttachmentDescription InputDepthAttachment();

		static std::array<VkSubpassDependency, 2> SetSubpassDependencies();

		static VkAttachmentDescription SwapchainColourAttachment(const VkFormat& imageFormat);

		void drawIndexed(std::shared_ptr<ShaderInterface> shader, size_t indexCount) override;
		void draw(std::shared_ptr<ShaderInterface> shader, size_t vertexCount) override;
		void draw(std::shared_ptr<ShaderInterface> shader, const MeshInterface& mesh, size_t instanceCount);
		bool useMSAA = false;
		bool useSampleShading = false;

		VkSampleCountFlagBits sampleCount = VK_SAMPLE_COUNT_1_BIT;
		VkSampleCountFlagBits getMaxUsableSampleCount();
		
	public:
		VkInstance			m_VulkanInstance;
		MainDevice			m_MainDevice;
		VkSurfaceKHR		m_Surface;
		SwapChain			m_SwapChain;

		std::vector<const char*> m_RequestedDeviceExtensions = {
			VK_KHR_SWAPCHAIN_EXTENSION_NAME
		};

		int	m_CurrentFrame = 0;
		
		QueueFamilyIndices m_QueueFamilyIndices;
		VkQueue	m_GraphicsQueue;
		VkQueue	m_PresentationQueue;
		
		std::shared_ptr<TextureVk> m_DepthBufferImage;

		VkPushConstantRange			 m_PushCostantRange;

		std::vector<SubmissionSyncObjects> m_SyncObjects;

		//void CreateOffScreenFrameBuffer();

		/* Core Renderer Functions */
		void CreateKernel();
		void CreateInstance();
		void RetrievePhysicalDevice();
		void CreateLogicalDevice();
		void CreateSurface();
		void CreateSynchronizationObjects();


		//void CreateModel(std::vector<std::shared_ptr<ModelVk>>& m_MeshModelList, const std::string& file);

		/* Auxiliary function for creation */
		void LoadGlfwExtensions(std::vector<const char*>& instanceExtensions);

		/* Uniform Data */
		void SetupPushCostantRange();
		//void CreateUniformBuffers();
		//void UpdateUniformBuffersWithData(uint32_t imageIndex);
		//void SetUniformDataStructures();
		//void SetLightsDataStructures();

		void HandleMinimization();
		bool CheckInstanceExtensionSupport(std::vector<const char*>* checkExtension);
		bool CheckDeviceSuitable(VkPhysicalDevice device);
		bool CheckValidationLayerSupport(std::vector<const char*>* validationLayers);
		void enableExtensions(std::vector<const char*>& enabledDeviceExtensions);
	};
}

#endif
