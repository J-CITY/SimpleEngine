#pragma once
#include <cstdint>
#include <GL/glew.h>

#include "vertexBuffer.h"


namespace KUMA {
	namespace RENDER {
		enum class VertexAttributeInputRate {
			PER_VERTEX,
			PER_INSTANCE,
		};
		class VertexArray {
		public:
			enum class Type {
				BYTE = 0x1400,
				UNISGNED_BYTE = 0x1401,
				SHORT = 0x1402,
				UNSIGNED_SHORT = 0x1403,
				INT = 0x1404,
				UNSIGNED_INT = 0x1405,
				FLOAT = 0x1406,
				DOUBLE = 0x140A
			};
			
			VertexArray() {
				glGenVertexArrays(1, &ID);
				glBindVertexArray(ID);
			}

			~VertexArray() {
				glDeleteVertexArrays(1, &ID);
			}

			void bind() const {
				glBindVertexArray(ID);
			}

			void unbind() const {
				glBindVertexArray(0);
			}

			unsigned int getID() {
				return ID;
			}

			template <class T>
			inline void bindAttribute(unsigned int attribute, VertexBuffer<T>& p_vertexBuffer, Type p_type, unsigned int count, unsigned int p_stride, intptr_t offset) {
				bind();
				p_vertexBuffer.bind();
				if (p_type == Type::INT) {
					glVertexAttribIPointer(static_cast<GLuint>(attribute), static_cast<GLint>(count), static_cast<GLenum>(p_type), static_cast<GLsizei>(p_stride), reinterpret_cast<const GLvoid*>(offset));
				}
				else {
					glVertexAttribPointer(static_cast<GLuint>(attribute), static_cast<GLint>(count), static_cast<GLenum>(p_type), GL_FALSE, static_cast<GLsizei>(p_stride), reinterpret_cast<const GLvoid*>(offset));
				}
				glEnableVertexAttribArray(attribute);
			}

			size_t attributeIndex = 0;
			
		private:
			unsigned int ID;
		};
	}
}
