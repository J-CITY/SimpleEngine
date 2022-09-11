#pragma once
#include <cstdint>
#include <span>
#include <vector>
#include <GL/glew.h>

namespace KUMA {
	namespace RENDER {
		class IndexBuffer {
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
			//IndexBuffer() = default;
			IndexBuffer(std::span<unsigned> data);
			IndexBuffer(std::span<unsigned> data, UsageType type);
			~IndexBuffer();
			void bind() const;
			void unbind() const;
			[[nodiscard]] unsigned getID() const;
		private:
			uint32_t m_bufferID = 0;
		};
	}
}
