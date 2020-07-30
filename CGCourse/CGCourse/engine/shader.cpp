#include "asset.h"
#include "Shader.h"

using namespace SE;

Shader::Shader() {
	program = glCreateProgram();
}

Shader::Shader(std::string shaderAssetName) {
	program = glCreateProgram();

	addVertexFromRaw(Asset(shaderAssetName + ".vs").read());
	addFragmentFromRaw(Asset(shaderAssetName + ".fs").read());
}

Shader::Shader(const std::string& vertData, const std::string& fragData) {
	program = glCreateProgram();
	addVertexFromRaw(vertData);
	addFragmentFromRaw(fragData);
}

Shader::~Shader(void) {
	glDetachShader(program, shVert);
	glDeleteShader(shVert);

	glDetachShader(program, shFrag);
	glDeleteShader(shFrag);

	glDeleteProgram(program);
}

void Shader::addVertexFromRaw(const std::string& vertData) {
	char shErr[1024];
	int errlen;
	GLint res;

	// Generate some IDs for our shader programs
	shVert = glCreateShader(GL_VERTEX_SHADER);

	// Assign our above shader source code to these IDs
	const char* data = vertData.c_str();
	glShaderSource(shVert, 1, &data, nullptr);

	// Attempt to compile the source code
	glCompileShader(shVert);

	// check if compilation was successful
	glGetShaderiv(shVert, GL_COMPILE_STATUS, &res);
	if (GL_FALSE == res) {
		glGetShaderInfoLog(shVert, 1024, &errlen, shErr);
		//log_err("Failed to compile vertex shader: %s", shErr);
		return;
	}

	// Attach these shaders to the shader program
	glAttachShader(program, shVert);

	// flag the shaders to be deleted when the shader program is deleted
	glDeleteShader(shVert);
}

void Shader::addFragmentFromRaw(const std::string& fragData) {
	char shErr[1024];
	int errlen;
	GLint res;

	// Generate some IDs for our shader programs
	shFrag = glCreateShader(GL_FRAGMENT_SHADER);

	// Assign our above shader source code to these IDs
	const char* data = fragData.c_str();
	glShaderSource(shFrag, 1, &data, nullptr);

	// Attempt to compile the source code
	glCompileShader(shFrag);

	// check if compilation was successful
	glGetShaderiv(shFrag, GL_COMPILE_STATUS, &res);
	if (GL_FALSE == res) {
		glGetShaderInfoLog(shFrag, 1024, &errlen, shErr);
		//log_err("Failed to compile fragment shader: %s", shErr);
		return;
	}

	// Attach these shaders to the shader program
	glAttachShader(program, shFrag);

	// flag the shaders to be deleted when the shader program is deleted
	glDeleteShader(shFrag);
}

void Shader::link() {
	char shErr[1024];
	int errlen;
	GLint res;
	// Link the shaders
	glLinkProgram(program);
	glGetProgramiv(program, GL_LINK_STATUS, &res);

	if (GL_FALSE == res) {
		//log_err("Failed to link shader program");
	}
	glValidateProgram(program);
	glGetProgramiv(program, GL_VALIDATE_STATUS, &res);
	if (GL_FALSE == res) {
		glGetProgramInfoLog(program, 1024, &errlen, shErr);
		//log_err("Error validating shader: %s", shErr);
	}
}

GLuint Shader::getProgram() {
	return program;
}

void Shader::createUniform(const std::string& uniformName) {
	uniformLocation[uniformName] = glGetUniformLocation(program, uniformName.c_str());
}

GLuint Shader::getUniform(const std::string& uniformName) {
	return uniformLocation[uniformName];
}

void Shader::setAttribLocation(const char* name, int i) {
	glBindAttribLocation(program, i, name);
}

void Shader::bind() const {
	glUseProgram(program);
}

void Shader::setUniform1i(const std::string& uniformName, int value) {
	bind();

	glUniform1i(getUniform(uniformName), value);
}

void Shader::setUniform1f(const std::string& uniformName, float value) {
	bind();

	glUniform1f(getUniform(uniformName), value);
}

void Shader::setUniformVec3f(const std::string& uniformName, Mathgl::Vec3 vector) {
	bind();

	glUniform3f(getUniform(uniformName), vector.x, vector.y, vector.z);
}

void Shader::setUniformMatrix4f(const std::string& uniformName, const Mathgl::Mat4& matrix) {
	bind();

	glUniformMatrix4fv(getUniform(uniformName), 1, GL_FALSE, &(matrix)[0][0]);
}
