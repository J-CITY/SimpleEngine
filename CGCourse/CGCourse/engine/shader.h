#pragma once

#include <map>
#include <string>
#include <memory>
#include <GL/glew.h>

#include "../math.h"
namespace SE {
	class Shader {
	public:
		Shader();
		Shader(std::string path);
		Shader(const std::string& vertData, const std::string& fragData);
		~Shader();


		void addFragmentFromRaw(const std::string& fragData);
		void addVertexFromRaw(const std::string& vertData);

		//void addFragmentFromFile();
		//void addVertexFromFile();

		void link();

		void setAttribLocation(const char* name, int i);

		GLuint getProgram();

		void createUniform(const std::string& uniformName);
		GLuint getUniform(const std::string& uniformName);

		void bind() const;

		void setUniformVec3f(const std::string& uniformName, Mathgl::Vec3 vector);
		void setUniform1i(const std::string& uniformName, int value);
		void setUniform1f(const std::string& uniformName, float value);
		void setUniformMatrix4f(const std::string& uniformName, const  Mathgl::Mat4& matrix);


	private:
		GLuint  shVert;
		GLuint  shFrag;
		GLuint  program;

		std::map <std::string, GLuint> uniformLocation;
	};
};
