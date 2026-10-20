#pragma once

#ifdef VULKAN_BACKEND
#include "../interface/vertexBufferInterface.h"
#include "vmaVk.h"
#include <vector>

namespace IKIGAI::RENDER {
class VertexBufferVk : public VertexBufferInterface {
	VmaBuffer mBuffer;
	size_t mCapacityByte = 0;

public:
	VertexBufferVk(void* data, size_t size, size_t stride);

	template <class T>
	VertexBufferVk(const std::vector<T>& vertices)
		: VertexBufferVk((void*)vertices.data(), vertices.size(), sizeof(T)) { }

	~VertexBufferVk() override;

	void setData(const void* data, size_t sz, size_t stride) override;

	void bind() override;

	void unbind() override;

	VmaBuffer& getBuffer() {
		return mBuffer;
	}
};
}
#endif
