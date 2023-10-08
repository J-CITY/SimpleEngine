#include "indexBufferVk.h"

#ifdef VULKAN_BACKEND
#include <coreModule/resourceManager/ServiceManager.h>
#include "driverVk.h"
#include "shaderVk.h"
using namespace IKIGAI;
using namespace IKIGAI::RENDER;
IndexBufferVk::IndexBufferVk(std::span<uint32_t>& indices) {
	auto render = UtilityVk::GetDriver();
	m_indexCount = indices.size();
	VkDeviceSize bufferSize = sizeof(uint32_t) * indices.size();

	VkBuffer staging_buffer;
	VkDeviceMemory stagingBufferMemory;

	BufferSettings buffer_settings;
	buffer_settings.size = bufferSize;
	buffer_settings.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
	buffer_settings.properties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

	UtilityVk::CreateBuffer(buffer_settings, &staging_buffer, &stagingBufferMemory);

	// Mapping della memoria per l'index buffer
	void* data;
	vkMapMemory(render->m_MainDevice.LogicalDevice, stagingBufferMemory, 0, bufferSize, 0, &data);  // 2. Creao le associazioni (mapping) tra la memoria del vertex buffer ed il pointer
	memcpy(data, indices.data(), static_cast<size_t>(bufferSize));							// 3. Copio il vettore dei vertici in un punto in memoria
	vkUnmapMemory(render->m_MainDevice.LogicalDevice, stagingBufferMemory);							// 4. Disassocio il vertice dalla memoria

	buffer_settings.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
	buffer_settings.properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

	// Creazione del buffer per Index Data sulla GPU
	UtilityVk::CreateBuffer(buffer_settings, &m_indexBuffer, &m_indexBufferMemory);

	// Copia dello staging buffer sulla GPU
	UtilityVk::CopyBufferCmd(staging_buffer, m_indexBuffer, bufferSize);

	// Distruzione dello staging Buffer
	vkDestroyBuffer(render->m_MainDevice.LogicalDevice, staging_buffer, nullptr);
	vkFreeMemory(render->m_MainDevice.LogicalDevice, stagingBufferMemory, nullptr);

	//bool useInRt = true;
	//VkBufferDeviceAddressInfoKHR address_info;
	//DW_ZERO_MEMORY(address_info);
	//
	//address_info.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO_KHR;
	//address_info.buffer = m_indexBuffer;
	//
	////if ((usage & VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT) == VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT)
	//if (useInRt)
	//	m_device_address = vkGetBufferDeviceAddress(render->m_MainDevice.LogicalDevice, &address_info);

}

void IndexBufferVk::bind(const ShaderInterface& shader)
{
	auto render = UtilityVk::GetDriver();
	vkCmdBindIndexBuffer(static_cast<const ShaderVk&>(shader).m_CommandHandler.m_CommandBuffers[render->imageIndex], m_indexBuffer, 0, VK_INDEX_TYPE_UINT32);
}
#endif
