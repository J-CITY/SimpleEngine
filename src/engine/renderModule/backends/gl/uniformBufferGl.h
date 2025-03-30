#pragma once

#ifdef OPENGL_BACKEND
#include <string>
#include "../interface/uniformBufferInterface.h"

namespace IKIGAI::RENDER {
	class UniformBufferGl : public UniformBufferInterface {
	public:
		using Id = unsigned;
	private:
		Id mId = 0;
	public:
		UniformBufferGl(const void* data, size_t sz);
		template <class T>
		UniformBufferGl(const T& data) : UniformBufferGl(static_cast<const void*>(&data), sizeof(T)) {}
		~UniformBufferGl() override;
		void setData(const void* data, size_t sz, size_t offset = 0) override;

		void bind();
		void bindToShader(unsigned slot);
		void unbind();
		unsigned getId() const;
	};
}
#endif
