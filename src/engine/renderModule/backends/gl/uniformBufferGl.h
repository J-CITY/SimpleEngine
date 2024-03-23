#pragma once

#ifdef OPENGL_BACKEND
#include "../interface/uniformBufferInterface.h"

namespace IKIGAI::RENDER {
	template<class T>
	class UniformBufferGl : public UniformBufferInterface {
	public:
		unsigned int id = 0;
		std::string name;
		int bindId = 0;
		//std::shared_ptr<ShaderInterface> shader;
		UniformBufferGl(std::string name, int bindId, int sz = 1) :
			name(name),
			bindId(bindId) {
			glGenBuffers(1, &id);
#ifndef USING_GLES
			glBindBuffer(GL_UNIFORM_BUFFER, id);
			glBufferData(GL_UNIFORM_BUFFER, sizeof(T) * sz, NULL, GL_STATIC_DRAW);
			glBindBuffer(GL_UNIFORM_BUFFER, 0);
			//static int bid = 0;
			glBindBufferRange(GL_UNIFORM_BUFFER, bindId, id, 0, sizeof(T) * sz);
			//bid++;
			//unsigned int _id = glGetUniformBlockIndex(shader->getId(), name.c_str());
			//glUniformBlockBinding(shader->getId(), _id, bindId);
#endif
		}

		UniformBufferGl(std::string name) :
			name(name)//,
			//bindId(bindId)
		{
#ifndef USING_GLES
			glGenBuffers(1, &id);
			glBindBuffer(GL_UNIFORM_BUFFER, id);
			glBufferData(GL_UNIFORM_BUFFER, sizeof(T), NULL, GL_STATIC_DRAW);
			glBindBuffer(GL_UNIFORM_BUFFER, 0);
			//static int bid = 0;
			//glBindBufferRange(GL_UNIFORM_BUFFER, bindId, id, 0, sizeof(T));
			//bid++;
			//unsigned int _id = glGetUniformBlockIndex(shader->getId(), name.c_str());
			//glUniformBlockBinding(shader->getId(), _id, bindId);
#endif
		}

		UniformBufferGl(std::string name, int bindId, const std::vector<unsigned char>& data) :
			name(name),
			bindId(bindId) {
			glGenBuffers(1, &id);
#ifndef USING_GLES
			glBindBuffer(GL_UNIFORM_BUFFER, id);
			glBufferData(GL_UNIFORM_BUFFER, sizeof(unsigned char) * data.size(), NULL, GL_STATIC_DRAW);
			glBindBuffer(GL_UNIFORM_BUFFER, 0);
			//static int bid = 0;
			glBindBufferRange(GL_UNIFORM_BUFFER, bindId, id, 0, sizeof(unsigned char) * data.size());
			//bid++;
			//unsigned int _id = glGetUniformBlockIndex(shader->getId(), name.c_str());
			//glUniformBlockBinding(shader->getId(), _id, bindId);
#endif
		}

		UniformBufferGl(std::string name, const std::vector<unsigned char>& data) :
			name(name)//,
			//bindId(bindId)
		{
			glGenBuffers(1, &id);
#ifndef USING_GLES
			glBindBuffer(GL_UNIFORM_BUFFER, id);
			glBufferData(GL_UNIFORM_BUFFER, sizeof(unsigned char) * data.size(), NULL, GL_STATIC_DRAW);
			glBindBuffer(GL_UNIFORM_BUFFER, 0);
			//static int bid = 0;
			//glBindBufferRange(GL_UNIFORM_BUFFER, bindId, id, 0, sizeof(unsigned char) * data.size());
			//bid++;
			//unsigned int _id = glGetUniformBlockIndex(shader->getId(), name.c_str());
			//glUniformBlockBinding(shader->getId(), _id, bindId);
#endif
		}

		// it`s only for variant in MaterialGl
		UniformBufferGl() = default;

		~UniformBufferGl() {

		}

		void bind() {
#ifndef USING_GLES
			glBindBuffer(GL_UNIFORM_BUFFER, id);
			//unsigned int _id = glGetUniformBlockIndex(shader->getId(), name.c_str());
			//glBindBufferBase(GL_UNIFORM_BUFFER, bindId, id);
#endif
		}

		void unbind() {
#ifndef USING_GLES
			glBindBuffer(GL_UNIFORM_BUFFER, 0);
			//glBindBufferBase(GL_UNIFORM_BUFFER, bindId, 0);
#endif
		}

		void set(const T& data) {
#ifndef USING_GLES
			bind();
			glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(T), std::addressof(data));
			unbind();
#endif
		}

		void setBytes(const std::vector<unsigned char>& data) {
#ifndef USING_GLES
			bind();
			glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(unsigned char) * data.size(), data.data());
			unbind();
#endif
		}

		void set(const std::vector<T>& data) {
#ifndef USING_GLES
			bind();
			glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(T) * data.size(), data.data());
			unbind();
#endif
		}

		void bindToShader(std::shared_ptr<ShaderInterface> shader, uint32_t p_uniformBlockLocation) {
#ifndef USING_GLES
			glUniformBlockBinding(shader->getId(), p_uniformBlockLocation, bindId);
#endif
		}

		void bindToShader(std::shared_ptr<ShaderInterface> shader, const std::string& name) {
#ifndef USING_GLES
			unsigned int _id = glGetUniformBlockIndex(shader->getId(), name.c_str());
			glUniformBlockBinding(shader->getId(), _id, bindId);
#endif
		}

		int getSetId() const override {
			return id;
		}
	};
}
#endif
