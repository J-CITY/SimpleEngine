#pragma once

#ifdef VULKAN_BACKEND
#include <vulkan/vulkan_core.h>
namespace KUMA::RENDER {
	class DebugMessanger
	{
	public:
		static DebugMessanger* GetInstance();
		void SetupDebugMessenger(VkInstance& instance);
		void Clear();


	private:
		DebugMessanger() = default;

		static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(
			VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
			VkDebugUtilsMessageTypeFlagsEXT messageType,
			const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
			void* pUserData);

		VkResult CreateDebugUtilsMessengerEXT(
			VkInstance instance,
			const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
			const VkAllocationCallbacks* pAllocator,
			VkDebugUtilsMessengerEXT* pDebugMessenger);

		void DestroyDebugUtilsMessengerEXT(
			VkInstance& instance,
			VkDebugUtilsMessengerEXT& debugMessenger,
			const VkAllocationCallbacks* pAllocator);

	private:
		static DebugMessanger* s_Instance;
		VkInstance m_VulkanInstance;
		VkDebugUtilsMessengerEXT m_DebugMessenger;
	};
}
#endif