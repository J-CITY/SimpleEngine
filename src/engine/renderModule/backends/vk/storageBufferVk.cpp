#include "storageBufferVk.h"
#ifdef VULKAN_BACKEND
#include "bufferUploadVk.h"
#include "driverVk.h"
#include "helpers.h"
#include <stdexcept>

namespace IKIGAI::RENDER {
StorageBufferVk::StorageBufferVk(void* data, size_t count, size_t stride)
	: StorageBufferInterface(0, stride) {
	StorageBufferVk::setData(data, count, stride);
}
StorageBufferVk::~StorageBufferVk() {
	if (mBuffer) {
		UtilityVk::GetDriver()->destroyDeferred(std::move(mBuffer));
	}
}
void StorageBufferVk::setData(const void* data, size_t count, size_t stride) {
	const size_t bytes = CheckedBufferSizeVk(count, stride);
	if (!bytes) {
		return;
	}
	ReserveBufferVk(mBuffer, mCapacityByte, bytes, vk::BufferUsageFlagBits::eStorageBuffer);
	mSize = count;
	mStride = stride;
	mSizeByte = bytes;
	if (data) {
		UploadBufferVk(mBuffer, data, bytes);
	}
}
void StorageBufferVk::setSubData(const void* data, size_t bytes, size_t offset) {
	if (!bytes) {
		return;
	}
	if (!data || offset > mSizeByte || bytes > mSizeByte - offset) {
		throw std::invalid_argument("Storage buffer update exceeds its logical range");
	}
	UploadBufferVk(mBuffer, data, bytes, offset);
}
void StorageBufferVk::bind() { }
void StorageBufferVk::unbind() { }
}
#endif
