#include "debugMessanger.h"
#ifdef VULKAN_BACKEND
#include <iostream>
#include <stdexcept>
using namespace IKIGAI;
using namespace IKIGAI::RENDER;
DebugMessanger* DebugMessanger::s_Instance = nullptr;

void DebugMessanger::Clear()
{
	DestroyDebugUtilsMessengerEXT(m_VulkanInstance, m_DebugMessenger, nullptr);
}

DebugMessanger* DebugMessanger::GetInstance() {
	if (s_Instance == 0)
		s_Instance = new DebugMessanger();
	return s_Instance;
}

void DebugMessanger::SetupDebugMessenger(VkInstance& instance) {
	m_VulkanInstance = instance;

	VkDebugUtilsMessengerCreateInfoEXT createInfo{};
	createInfo.sType			= VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	createInfo.flags			= 0;
	createInfo.messageSeverity	= 
		//VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |			
		//VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT	|
		VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;

	createInfo.messageType		= 
		VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT		|			
		VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT  |
		VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;

	createInfo.pfnUserCallback	= DebugCallback;
	createInfo.pUserData		= nullptr;  	

	if (CreateDebugUtilsMessengerEXT(instance, &createInfo, nullptr, &m_DebugMessenger) != VK_SUCCESS) // Creo il debug messanger, in caso di errore alzo un eccezione
	{
		throw std::runtime_error("Failed to set up debug messenger!");
	}
}

VKAPI_ATTR VkBool32 VKAPI_CALL DebugMessanger::DebugCallback(
	VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
	VkDebugUtilsMessageTypeFlagsEXT messageType,
	const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
	void* pUserData)
{
	switch (messageSeverity)
	{
	case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
		std::cerr << "[Verbose]";
		break;

	case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
		std::cerr << "[Info]";
		break;

	case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
		std::cerr << "[Warning]";
		break;

	case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
		std::cerr << "[Error]";
		break;
	}

	switch (messageType)
	{
	case VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT:
		std::cerr << "[General]";
		break;
	case VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT:
		std::cerr << "[Validation]";
		break;
	case VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT:
		std::cerr << "[Performance]";
		break;
	}

	std::cerr << "\nCount : " << pCallbackData->objectCount << "\n";
	std::cerr << "MessageID name : " << pCallbackData->pMessageIdName << "\n";
	std::cerr << "Type :" << pCallbackData->sType << "\n";
	std::cerr << "Message : " <<  pCallbackData->pMessage << "\n\n";

	return VK_FALSE;
}

VkResult DebugMessanger::CreateDebugUtilsMessengerEXT(
	VkInstance instance,
	const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
	const VkAllocationCallbacks* pAllocator,
	VkDebugUtilsMessengerEXT* pDebugMessenger)
{

	auto func = (PFN_vkCreateDebugUtilsMessengerEXT)						
		vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");	
																			
	if (func != nullptr)
	{
		VkResult result = func(instance, pCreateInfo, pAllocator, pDebugMessenger);
		return result;
	}

	return VK_ERROR_EXTENSION_NOT_PRESENT;
}

void DebugMessanger::DestroyDebugUtilsMessengerEXT(
	VkInstance &instance,
	VkDebugUtilsMessengerEXT &m_debugMessenger,
	const VkAllocationCallbacks* pAllocator)
{

	auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)						
		vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");	

	if (func != nullptr)
		func(instance, m_debugMessenger, pAllocator);
}
#endif