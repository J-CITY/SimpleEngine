#pragma once

#ifdef OPENGL_BACKEND
#include <vector>
#include <span>
#include <gl/glew.h>

#include "../interface/vertexBufferInterface.h"

namespace IKIGAI::RENDER
{
	enum class VertexAttributeInputRate {
		PER_VERTEX,
		PER_INSTANCE,
	};

	template <class T>
	class VertexBufferGl : public VertexBufferInterface {
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
		VertexBufferGl() {
			glGenBuffers(1, &ID);
		}

		int sz = 0;
		template <class T>
		inline VertexBufferGl(T* data, size_t elements) {
			sz = elements;
			glGenBuffers(1, &ID);
			glBindBuffer(GL_ARRAY_BUFFER, ID);
			glBufferData(GL_ARRAY_BUFFER, elements * sizeof(T), data, GL_STATIC_DRAW);
		}

		template <class T>
		inline VertexBufferGl(T* data, size_t elements, UsageType type) {
			sz = elements;
			glGenBuffers(1, &ID);
			glBindBuffer(GL_ARRAY_BUFFER, ID);
			glBufferData(GL_ARRAY_BUFFER, elements * sizeof(T), data, UsageTypeToEnum[static_cast<int>(type)]);
		}

		template<class T>
		inline VertexBufferGl(std::span<T> data) : VertexBufferGl(data.data(), data.size()) {}

		inline ~VertexBufferGl() {
			glDeleteBuffers(1, &ID);
		}

		inline void bind() const {
			glBindBuffer(GL_ARRAY_BUFFER, ID);
		}

		inline void unbind() const {
			glBindBuffer(GL_ARRAY_BUFFER, 0);
		}

		inline unsigned int getID() const {
			return ID;
		}

		void bindAttribute(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const GLvoid* pointer) const {
			bind();
			glVertexAttribPointer(index, size, type, normalized, stride, pointer);
			glEnableVertexAttribArray(index);
		}

		void bindAttribute(GLuint index, GLint size, GLenum type, GLsizei stride, const GLvoid* pointer) const {
			bind();
			glVertexAttribIPointer(index, size, type, stride, pointer);
			glEnableVertexAttribArray(index);
		}

		void bufferData(GLsizeiptr size, void* data, GLenum usage) const {
			bind();
			glBufferData(GL_ARRAY_BUFFER, size, data, usage);
		}

		void bufferSubData(GLintptr offset, GLsizeiptr size, void* data) const {
			bind();
			(glBufferSubData(GL_ARRAY_BUFFER, offset, size, data));
		}

		void bufferDataWithResize(const T* data, size_t sizeInScalars) const {
			bind();
			glBufferSubData(GL_ARRAY_BUFFER, 0, sizeInScalars * sizeof(T), (const uint8_t*)data);
		}

		void bind(const ShaderInterface& shader) override {
			bind();
		};
		int getVertexCount() override {
			return sz;
		};

	private:
		unsigned int ID;
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

		VertexArray();
		~VertexArray();
		void bind() const;
		void unbind() const;
		unsigned int getID() const;

		template <class T>
		inline void bindAttribute(unsigned int attribute, VertexBufferGl<T>& p_vertexBuffer, Type p_type, unsigned int count, unsigned int p_stride, intptr_t offset) {
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

#endif
