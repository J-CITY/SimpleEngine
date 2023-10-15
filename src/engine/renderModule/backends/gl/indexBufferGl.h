#pragma once
#include <span>

#ifdef OPENGL_BACKEND
#include <vector>
#include <gl/glew.h>

#include "../interface/indexBufferInterface.h"

namespace IKIGAI::RENDER
{
	class IndexBufferGl : public IndexBufferInterface {
	public:
		enum class UsageType : uint8_t {
			STREAM_DRAW,
			STREAM_READ,
			STREAM_COPY,
			STATIC_DRAW,
			STATIC_READ,
			STATIC_COPY,
			DYNAMIC_DRAW,
			DYNAMIC_READ,
			DYNAMIC_COPY,
		};
		std::vector<GLenum> UsageTypeToEnum = {
			GL_STREAM_DRAW,
			GL_STREAM_READ,
			GL_STREAM_COPY,
			GL_STATIC_DRAW,
			GL_STATIC_READ,
			GL_STATIC_COPY,
			GL_DYNAMIC_DRAW,
			GL_DYNAMIC_READ,
			GL_DYNAMIC_COPY
		};
		IndexBufferGl(std::span<unsigned> data);
		IndexBufferGl(std::span<unsigned> data, UsageType type);
		virtual ~IndexBufferGl() override;
		void bind() const;
		void unbind() const;
		[[nodiscard]] unsigned getID() const;

		int sz = 0;
		int getIndexCount() override
		{
			return sz;
		}
		virtual void bind(const ShaderInterface& shader) override
		{
			bind();
		}
	private:
		uint32_t m_bufferID = 0;
	};
}
#endif