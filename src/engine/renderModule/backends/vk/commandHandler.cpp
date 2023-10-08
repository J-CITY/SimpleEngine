
#include "commandHandler.h"

#ifdef VULKAN_BACKEND
#include <stdexcept>

#include "DriverVk.h"
#include "../../gameRendererVk.h"
#include <coreModule/resourceManager/ServiceManager.h>

using namespace IKIGAI;
using namespace IKIGAI::RENDER;

CommandHandler::CommandHandler() {
	m_GraphicsComandPool	= {};
	m_CommandBuffers		= {};
}

void CommandHandler::CreateCommandPool(QueueFamilyIndices& queueIndices) { 
	VkCommandPoolCreateInfo poolInfo = {};
	poolInfo.sType				= VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	poolInfo.flags				= VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	poolInfo.queueFamilyIndex	= queueIndices.GraphicsFamily;

	VkResult res = vkCreateCommandPool(UtilityVk::device->LogicalDevice, &poolInfo, nullptr, &m_GraphicsComandPool);

	if (res != VK_SUCCESS)
		throw std::runtime_error("Failed to create a Command Pool!");
}

void CommandHandler::CreateCommandBuffers(size_t numFrameBuffers) {
	m_CommandBuffers.resize(numFrameBuffers);

	VkCommandBufferAllocateInfo cbAllocInfo = {};
	cbAllocInfo.sType		= VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	cbAllocInfo.commandPool = m_GraphicsComandPool;
	cbAllocInfo.level		= VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	cbAllocInfo.commandBufferCount = static_cast<uint32_t>(m_CommandBuffers.size());

	VkResult res = vkAllocateCommandBuffers(UtilityVk::device->LogicalDevice, &cbAllocInfo, m_CommandBuffers.data());

	if (res != VK_SUCCESS) {
		throw std::runtime_error("Failed to allocate Command Buffers!");
	}
}

void CommandHandler::DestroyCommandPool() {
	auto render = reinterpret_cast<RENDER::GameRendererVk&>(RESOURCES::ServiceManager::Get<RENDER::GameRendererInterface>()).getDriver();
	vkDestroyCommandPool(render->m_MainDevice.LogicalDevice, m_GraphicsComandPool, nullptr);
}

void CommandHandler::FreeCommandBuffers() {
	auto render = reinterpret_cast<RENDER::GameRendererVk&>(RESOURCES::ServiceManager::Get<RENDER::GameRendererInterface>()).getDriver();
	vkFreeCommandBuffers(render->m_MainDevice.LogicalDevice, m_GraphicsComandPool, static_cast<uint32_t>(m_CommandBuffers.size()), m_CommandBuffers.data());
}

#endif