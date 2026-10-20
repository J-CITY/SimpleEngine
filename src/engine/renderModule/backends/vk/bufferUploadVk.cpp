#include "bufferUploadVk.h"
#ifdef VULKAN_BACKEND
#include "driverVk.h"
#include "helpers.h"
#include <cstdint>
#include <limits>
#include <stdexcept>

namespace IKIGAI::RENDER {
size_t CheckedBufferSizeVk(size_t count, size_t stride) {
	if (count && (!stride || count > std::numeric_limits<size_t>::max() / stride)) {
		throw std::invalid_argument("Invalid or overflowing Vulkan buffer size");
	}
	return count * stride;
}
void ReserveBufferVk(VmaBuffer& buffer, size_t& capacity, size_t bytes, vk::BufferUsageFlags usage) {
	if (!bytes || (buffer && bytes <= capacity)) {
		return;
	}
	auto replacement = UtilityVk::CreateBuffer(bytes, usage | vk::BufferUsageFlagBits::eTransferDst, UtilityVk::MemoryUsage::GpuOnly);
	if (buffer) {
		UtilityVk::GetDriver()->destroyDeferred(std::move(buffer));
	}
	buffer = std::move(replacement);
	capacity = bytes;
}
void UploadBufferVk(const VmaBuffer& buffer, const void* source, size_t bytes, size_t offset) {
	if (!bytes) {
		return;
	}
	if (!source || !buffer) {
		throw std::invalid_argument("Missing Vulkan buffer upload source or destination");
	}
	auto& driver = *UtilityVk::GetDriver();
	driver.deactivateRenderPass();
	auto& commands = driver.getCurrentFrame().mCommandBuffer;
	UtilityVk::SetMemoryBarrier(commands, driver.mCurrentMemoryStage, vk::PipelineStageFlagBits2::eTransfer);
	driver.mCurrentMemoryStage = vk::PipelineStageFlagBits2::eTransfer;
	// vkCmdUpdateBuffer requires word-aligned offset/length and at most 64 KiB.
	// Byte ranges use vkCmdCopyBuffer, which has no such alignment restriction.
	if (bytes <= 65536 && bytes % 4 == 0 && offset % 4 == 0) {
		commands.updateBuffer<uint8_t>(*buffer, offset, { static_cast<uint32_t>(bytes), static_cast<const uint8_t*>(source) });
		return;
	}
	auto staging = UtilityVk::CreateBuffer(bytes, vk::BufferUsageFlagBits::eTransferSrc, UtilityVk::MemoryUsage::Staging);
	staging.upload(source, bytes);
	vk::BufferCopy copy;
	copy.setSrcOffset(0).setDstOffset(offset).setSize(bytes);
	commands.copyBuffer(*staging, *buffer, { copy });
	driver.destroyDeferred(std::move(staging));
}
}
#endif
