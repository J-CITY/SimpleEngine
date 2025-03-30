#pragma once
#ifdef OPENGL_BACKEND
#include "renderModule/backends/interface/storageBufferInterface.h"
namespace IKIGAI::RENDER {
	class StorageBufferGl : public StorageBufferInterface {
	public:
		using Id = unsigned;

		StorageBufferGl(const void* data, size_t sz, size_t stride);
		template <class T>
		StorageBufferGl(const std::vector<T>& vertices) : StorageBufferGl((void*)vertices.data(), vertices.size(), sizeof(T)) {}
		~StorageBufferGl() override;
		void setData(const void* data, size_t sz, size_t stride) override;

		void bind() override {};
		void unbind() override {};
	private:
		Id mId = 0;
		Id mBindId = 0;
	};
}
#endif
