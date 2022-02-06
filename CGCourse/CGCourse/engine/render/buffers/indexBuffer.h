#pragma once
#include <cstdint>
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
			IndexBuffer() {
			}
			IndexBuffer(const std::vector<unsigned>& data) {
				glGenBuffers(1, &m_bufferID);
				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_bufferID);
				glBufferData(GL_ELEMENT_ARRAY_BUFFER, data.size() * sizeof(unsigned int), data.data(), GL_STATIC_DRAW);
			}

			IndexBuffer(const std::vector<unsigned>& data, UsageType type) {
				glGenBuffers(1, &m_bufferID);
				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_bufferID);
				glBufferData(GL_ELEMENT_ARRAY_BUFFER, data.size() * sizeof(unsigned int), data.data(), UsageTypeToEnum[static_cast<int>(type)]);
			}
			
			~IndexBuffer() {
				glDeleteBuffers(1, &m_bufferID);
			}

			void bind() const {
				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_bufferID);
			}
			void unbind() {
				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
			}
			unsigned getID() {
				return m_bufferID;
			}


		private:
			uint32_t m_bufferID;
		};
	}
}
