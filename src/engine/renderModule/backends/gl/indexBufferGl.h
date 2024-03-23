#pragma once
#include <span>

#ifdef OPENGL_BACKEND
#include <vector>
#include <coreModule/graphicsWrapper.hpp>
#include "../interface/indexBufferInterface.h"

namespace IKIGAI::RENDER
{
	class IndexBufferGl : public IndexBufferInterface {
	public:
		enum class UsageType : uint8_t {
			STREAM_DRAW,
#ifndef USING_GLES
			STREAM_READ,
			STREAM_COPY,
#endif
			STATIC_DRAW,
#ifndef USING_GLES
			STATIC_READ,
			STATIC_COPY,
#endif
			DYNAMIC_DRAW,
#ifndef USING_GLES
			DYNAMIC_READ,
			DYNAMIC_COPY,
#endif
		};
		std::vector<GLenum> UsageTypeToEnum = {
			GL_STREAM_DRAW,
#ifndef USING_GLES
			GL_STREAM_READ,
			GL_STREAM_COPY,
#endif
			GL_STATIC_DRAW,
#ifndef USING_GLES
			GL_STATIC_READ,
			GL_STATIC_COPY,
#endif
			GL_DYNAMIC_DRAW,
#ifndef USING_GLES
			GL_DYNAMIC_READ,
			GL_DYNAMIC_COPY
#endif
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
