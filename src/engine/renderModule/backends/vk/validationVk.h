#pragma once
#ifdef VULKAN_BACKEND
#include <vulkan/vulkan_core.h>
#include <vector>
//namespace IKIGAI::RENDER {
	const bool validationEnabled = true;

	// VK_LAYER_LUNARG_standard_validation = All standard validation layers
	const std::vector<const char*> validationLayers = {
		"VK_LAYER_LUNARG_standard_validation"
	};

	static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
		VkDebugReportFlagsEXT flags,
		VkDebugReportObjectTypeEXT objType,
		uint64_t obj,
		size_t location,
		int32_t code,
		const char* layerPrefix,
		const char* message,
		void* userData) {
		if (flags & VK_DEBUG_REPORT_ERROR_BIT_EXT) {
			printf("VALIDATION ERROR: %s\n", message);
			return VK_TRUE;
		}
		if (flags & VK_DEBUG_REPORT_WARNING_BIT_EXT) {
			printf("VALIDATION WARNING: %s\n", message);
			return VK_FALSE;
		}
		return VK_FALSE;
	}

	static VkResult CreateDebugReportCallbackEXT(VkInstance instance, const VkDebugReportCallbackCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugReportCallbackEXT* pCallback) {
		auto func = (PFN_vkCreateDebugReportCallbackEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugReportCallbackEXT");
		if (func != nullptr) {
			return func(instance, pCreateInfo, pAllocator, pCallback);
		}
		else {
			return VK_ERROR_EXTENSION_NOT_PRESENT;
		}
	}

	static void DestroyDebugReportCallbackEXT(VkInstance instance, VkDebugReportCallbackEXT callback, const VkAllocationCallbacks* pAllocator) {
		auto func = (PFN_vkDestroyDebugReportCallbackEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugReportCallbackEXT");
		if (func != nullptr) {
			func(instance, callback, pAllocator);
		}
	}
//}
#endif
