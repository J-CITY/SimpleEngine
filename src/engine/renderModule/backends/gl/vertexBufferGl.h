#pragma once
#include <iostream>

#ifdef OPENGL_BACKEND
#include <vector>
#include <span>

#include <coreModule/graphicsWrapper.hpp>

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
		VertexBufferGl() {
			glGenBuffers(1, &ID);
		}

		int sz = 0;
		inline VertexBufferGl(T* data, size_t elements) {
			sz = elements;
			glGenBuffers(1, &ID);
			glBindBuffer(GL_ARRAY_BUFFER, ID);
			glBufferData(GL_ARRAY_BUFFER, elements * sizeof(T), data, GL_STATIC_DRAW);
			std::cout << "CREATE VERTEX BUFF " << elements;
		}
		
		inline VertexBufferGl(T* data, size_t elements, UsageType type) {
			sz = elements;
			glGenBuffers(1, &ID);
			glBindBuffer(GL_ARRAY_BUFFER, ID);
			glBufferData(GL_ARRAY_BUFFER, elements * sizeof(T), data, UsageTypeToEnum[static_cast<int>(type)]);
		}
		
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
			glEnableVertexAttribArray(index);
			glVertexAttribPointer(index, size, type, normalized, stride, pointer);
		}

		void bindAttribute(GLuint index, GLint size, GLenum type, GLsizei stride, const GLvoid* pointer) const {
#ifndef USING_GLES
			bind();
			glVertexAttribIPointer(index, size, type, stride, pointer);
			glEnableVertexAttribArray(index);
#endif
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
#ifndef USING_GLES
			if (p_type == Type::INT) {
				glVertexAttribIPointer(static_cast<GLuint>(attribute), static_cast<GLint>(count), static_cast<GLenum>(p_type), static_cast<GLsizei>(p_stride), reinterpret_cast<const GLvoid*>(offset));
			}
			else {
#endif
				glVertexAttribPointer(static_cast<GLuint>(attribute), static_cast<GLint>(count), static_cast<GLenum>(p_type), GL_FALSE, static_cast<GLsizei>(p_stride), reinterpret_cast<const GLvoid*>(offset));
#ifndef USING_GLES
			}
#endif
			glEnableVertexAttribArray(attribute);
		}

		size_t attributeIndex = 0;

	private:
		unsigned int ID;
	};
}

#endif
