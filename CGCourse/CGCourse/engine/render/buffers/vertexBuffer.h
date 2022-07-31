#pragma once
#include <GL/glew.h>


namespace KUMA {
	namespace RENDER {

		
		template <class T>
		class VertexBuffer {
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
			VertexBuffer() {
				glGenBuffers(1, &ID);
			}
			
			template <class T>
			inline VertexBuffer(T* data, size_t elements) {
				glGenBuffers(1, &ID);
				glBindBuffer(GL_ARRAY_BUFFER, ID);
				glBufferData(GL_ARRAY_BUFFER, elements * sizeof(T), data, GL_STATIC_DRAW);
			}

			template <class T>
			inline VertexBuffer(T* data, size_t elements, UsageType type) {
				glGenBuffers(1, &ID);
				glBindBuffer(GL_ARRAY_BUFFER, ID);
				glBufferData(GL_ARRAY_BUFFER, elements * sizeof(T), data, UsageTypeToEnum[static_cast<int>(type)]);
			}

			template<class T>
			inline VertexBuffer(const std::vector<T>& data) : VertexBuffer(data.data(), data.size()) {}

			inline ~VertexBuffer() {
				glDeleteBuffers(1, &ID);
			}

			inline void bind() {
				glBindBuffer(GL_ARRAY_BUFFER, ID);
			}
		
			inline void unbind() {
				glBindBuffer(GL_ARRAY_BUFFER, 0);
			}

			inline unsigned int getID() {
				return ID;
			}

			void bindAttribute(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const GLvoid* pointer) {
				bind();
				glVertexAttribPointer(index, size, type, normalized, stride, pointer);
				glEnableVertexAttribArray(index);
			}

			void bindAttribute(GLuint index, GLint size, GLenum type, GLsizei stride, const GLvoid* pointer) {
				bind();
				glVertexAttribIPointer(index, size, type, stride, pointer);
				glEnableVertexAttribArray(index);
			}

			void bufferData(GLsizeiptr size, void* data, GLenum usage) {
				bind();
				glBufferData(GL_ARRAY_BUFFER, size, data, usage);
			}

			void bufferSubData(GLintptr offset, GLsizeiptr size, void* data) {
				bind();
				(glBufferSubData(GL_ARRAY_BUFFER, offset, size, data));
			}

			void bufferDataWithResize(const T* data, size_t sizeInScalars) {
				//BufferBase::BufferDataWithResize((const uint8_t*)data, sizeInScalars * sizeof(T));


				//if (this->byteSize < byteSize)
				//	this->Load(this->type, byteData, byteSize, this->usage);
				//else
				bind();
				glBufferSubData(GL_ARRAY_BUFFER, 0, sizeInScalars * sizeof(T), (const uint8_t*)data);
					//this->BufferSubData(byteData, byteSize);
			}
		private:
			unsigned int ID;
		};
		
#define BONE_ID_LOCATION     10
#define BONE_WEIGHT_LOCATION 11
		
		template <class T>
		class VertexAttribBuffer {
		public:

			template <class T>
			inline VertexAttribBuffer(T* data, size_t elements) {
				glGenBuffers(1, &ID);
				glBindBuffer(GL_ARRAY_BUFFER, ID);
				glBufferData(GL_ARRAY_BUFFER, sizeof(data[0]) * elements, &data[0], GL_STATIC_DRAW);
				glEnableVertexAttribArray(BONE_ID_LOCATION);
				//glVertexAttribIPointer(BONE_ID_LOCATION, 4, GL_INT, sizeof(data[0]), nullptr);
				glVertexAttribPointer(BONE_ID_LOCATION, 4, GL_FLOAT, GL_FALSE, sizeof(data[0]), nullptr);
				glEnableVertexAttribArray(BONE_WEIGHT_LOCATION);
				glVertexAttribPointer(BONE_WEIGHT_LOCATION, 4, GL_FLOAT, GL_FALSE, sizeof(data[0]), (float*)16);
			}

			template<class T>
			inline VertexAttribBuffer(std::vector<T>& data) : VertexAttribBuffer(data.data(), data.size()) {}

			inline ~VertexAttribBuffer() {
				glDeleteBuffers(1, &ID);
			}

			inline void bind() {
				glBindBuffer(GL_ARRAY_BUFFER, ID);
			}

			inline void unbind() {
				glBindBuffer(GL_ARRAY_BUFFER, 0);
			}

			inline unsigned int getID() {
				return ID;
			}

		private:
			unsigned int ID;
		};
	}
}
