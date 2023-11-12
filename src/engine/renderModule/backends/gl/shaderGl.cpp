#include "shaderGl.h"

#include <array>

#include "../interface/reflectionStructs.h"
#ifdef OPENGL_BACKEND
#include <filesystem>
#include <iostream>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/mat3x3.hpp>

//#include "../../utils/shaderUtils.h"

import logger;

namespace IKIGAI
{
	namespace RENDER
	{
		enum class UNIFORM_TYPE;
	}
}

using namespace IKIGAI;
using namespace IKIGAI::RENDER;

std::vector<uint32_t> load_spirv_file(const std::filesystem::path& path)
{
	std::ifstream in(path, std::ios::binary);

	if (!in.good()) {
		throw std::runtime_error("Failed to open file.");
	}

	auto fsize = in.tellg();
	in.seekg(0, std::ios::end);
	fsize = in.tellg() - fsize;
	in.seekg(0, std::ios::beg);

	std::vector<uint32_t> output;
	output.resize(static_cast<size_t>(fsize / 4));
	in.read(reinterpret_cast<char*>(output.data()), static_cast<size_t>(fsize));

	return output;
}

//void testSpirv()
//{
//	{
//		const std::filesystem::path filename = "./Shaders/second_vert.spv";
//		std::vector<uint32_t> buffer = load_spirv_file(filename);
//		std::string result = SHADER_UTILS::CompileSpirvToGlsl(buffer, false,
//			470, true, true);
//		std::cout << result << std::endl;
//	}
//	{
//		const std::filesystem::path filename = "./Shaders/second_frag.spv";
//		std::vector<uint32_t> buffer = load_spirv_file(filename);
//		std::string result = SHADER_UTILS::CompileSpirvToGlsl(buffer, false,
//			470, true, true);
//		std::cout << result << std::endl;
//	}
//}

#include <utilsModule/loader.h>


std::string readFile(const std::string& path) {
	std::ifstream ifs(IKIGAI::UTILS::getRealPath(path));
	std::string content((std::istreambuf_iterator<char>(ifs)),
		(std::istreambuf_iterator<char>()));
	return content;
}

std::string readFileWithInclude(const std::string& path) {
	auto dir = std::filesystem::path{ path }.parent_path();

	auto content = readFile(path);

	size_t start = 0;
	while (auto pos = content.find("#include", start)) {
		start = pos+1;
		if (pos == std::string::npos) {
			break;
		}
		if (pos > 0 && content[pos-1] == '/') {
			continue;
		}
		auto comma1 = content.find("\"", pos);
		if (comma1 == std::string::npos) {
			throw;
		}
		auto comma2 = content.find("\"", comma1+1);
		if (comma2 == std::string::npos) {
			throw;
		}

		auto includePath = std::string(content.begin() + comma1 + 1, content.begin() + comma2);

		auto newContent = readFileWithInclude(dir.string() + "/" + includePath);

		content.erase(pos, comma2 - pos + 1);
		content.insert(pos, newContent);
	}
	return content;
}

ShaderGl::ShaderGl(std::optional<std::string> vertexPath, std::optional<std::string> fragmentPath,
	std::optional<std::string> geometryPath, std::optional<std::string> tessControlPath,
	std::optional<std::string> tessEvalPath, std::optional<std::string> computePath)
{
	this->vertexPath = vertexPath;
	this->fragmentPath = fragmentPath;
	this->geometryPath = geometryPath;
	this->tessEvalPath = tessEvalPath;
	this->tessControlPath = tessControlPath;
	this->computePath = computePath;
	//TODO: add other

	auto res = read(vertexPath, fragmentPath,
		geometryPath, tessControlPath,
		tessEvalPath, computePath);
	compile(res[0], res[1],
		res[2], res[3],
		res[4], res[5]);

	getReflection();
}


ShaderGl::ShaderGl(const ShaderResource& res) {
	if (!res.vertex.empty()) this->vertexPath = res.vertex;
	if (!res.fragment.empty()) this->fragmentPath = res.fragment;
	if (!res.geometry.empty()) this->geometryPath = res.geometry;
	if (!res.tessEval.empty()) this->tessEvalPath = res.tessEval;
	if (!res.tessControl.empty()) this->tessControlPath = res.tessControl;
	if (!res.compute.empty()) this->computePath = res.compute;

	auto useBinary = res.useBinary;
	useBinary &= checkBinarySupport();
	auto binPath = res.path + ".bin";
	if (useBinary && std::filesystem::exists(binPath)) {
		ID = glCreateProgram();
		//GLenum format = 0;
		std::ifstream inputStream(binPath, std::ios::binary);
		std::istreambuf_iterator<char> startIt(inputStream), endIt;
		std::vector<char> buffer(startIt, endIt);
		inputStream.close();
		
		//memcpy(&format, buffer.data(), sizeof(GLenum));
		GLint formats = 0;
		glGetIntegerv(GL_NUM_PROGRAM_BINARY_FORMATS, &formats);
		std::vector<GLint> binaryFormats;
		binaryFormats.resize(formats);
		glGetIntegerv(GL_PROGRAM_BINARY_FORMATS, binaryFormats.data());
	
		glProgramBinary(ID, binaryFormats[0], buffer.data(), buffer.size());
		LOG_INFO("Reading from " + binPath + ", binary format = ");
		// Check for success/failure
		GLint status;
		glGetProgramiv(ID, GL_LINK_STATUS, &status);
		if (GL_FALSE == status) {
			// Handle failure ...
		}
		//glValidateProgram(ID);
	}
	else {
		auto source = read(
			fragmentPath,
			vertexPath,
			geometryPath,
			tessControlPath,
			tessEvalPath,
			computePath
		);
		compile(source[0], source[1],
			source[2], source[3],
			source[4], source[5]);
	}
	if (ID) {
		if (useBinary && !std::filesystem::exists(binPath)) {
			GLint length = 0;
			glGetProgramiv(ID, GL_PROGRAM_BINARY_LENGTH, &length);
	
			std::vector<GLubyte> buffer(length);
			GLenum format = 0;
			glGetProgramBinary(ID, length, NULL, &format, buffer.data());
			LOG_INFO("Writing to " + binPath + ", binary format = " + std::to_string(format));
			std::ofstream out(binPath.c_str(), std::ios::binary);
			out.write(reinterpret_cast<char*>(buffer.data()), length);
			out.close();
		}
	}

	getReflection();
}

bool ShaderGl::checkBinarySupport() {
	GLint formats = 0;
	glGetIntegerv(GL_NUM_PROGRAM_BINARY_FORMATS, &formats);
	if (formats < 1) {
		LOG_INFO("Driver does not support any binary formats.");
		return false;
	}
	return true;
}

std::array<std::string, 6> ShaderGl::read(std::optional<std::string> vertexPath, std::optional<std::string> fragmentPath,
	std::optional<std::string> geometryPath, std::optional<std::string> tessControlPath,
	std::optional<std::string> tessEvalPath, std::optional<std::string> computePath) {
	std::array<std::string, 6> res;
	// 1. retrieve the vertex/fragment source code from filePath
	try {
		if (vertexPath) {
			res[0] = readFileWithInclude(vertexPath.value());
		}
		if (fragmentPath) {
			res[1] = readFileWithInclude(fragmentPath.value());
		}
		if (geometryPath) {
			res[2] = readFileWithInclude(geometryPath.value());
		}
		if (tessControlPath) {
			res[3] = readFileWithInclude(tessControlPath.value());
		}
		if (tessEvalPath) {
			res[4] = readFileWithInclude(tessEvalPath.value());
		}
		if (computePath) {
			res[5] = readFileWithInclude(computePath.value());
		}
	}
	catch (std::ifstream::failure& e) {
		std::cout << "ERROR::SHADER::FILE_NOT_SUCCESSFULLY_READ: " << e.what() << std::endl;
	}

	return res;
}

void ShaderGl::compile(std::string vertexCode, std::string fragmentCode,
	std::string geometryCode, std::string tessControlCode,
	std::string tessEvalCode, std::string computeCode) {
	// 2. compile shaders
	unsigned int vertex=0, fragment=0, geometry=0, tessControl=0, tessEval=0, compute=0;
	// vertex shader
	if (vertexPath) {
		const char* vShaderCode = vertexCode.c_str();
		vertex = glCreateShader(GL_VERTEX_SHADER);
		glShaderSource(vertex, 1, &vShaderCode, NULL);
		glCompileShader(vertex);
		checkCompileErrors(vertex, "VERTEX: " + vertexPath.value());
	}
	// fragment Shader
	if (fragmentPath) {
		const char* fShaderCode = fragmentCode.c_str();
		fragment = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(fragment, 1, &fShaderCode, NULL);
		glCompileShader(fragment);
		checkCompileErrors(fragment, "FRAGMENT: " + fragmentPath.value());
	}
	//std::cout << "-------------------------------\n";
	//std::cout << fShaderCode;
	if (geometryPath) {
		const char* gShaderCode = geometryCode.c_str();
		geometry = glCreateShader(GL_GEOMETRY_SHADER);
		glShaderSource(geometry, 1, &gShaderCode, NULL);
		glCompileShader(geometry);
		checkCompileErrors(geometry, "GEOMETRY: " + geometryPath.value());
	}
	if (tessControlPath) {
		const char* tcShaderCode = tessControlCode.c_str();
		tessControl = glCreateShader(GL_TESS_CONTROL_SHADER);
		glShaderSource(tessControl, 1, &tcShaderCode, NULL);
		glCompileShader(tessControl);
		checkCompileErrors(tessControl, "TESS_CONTROL: " + tessControlPath.value());
	}
	if (tessEvalPath) {
		const char* teShaderCode = tessEvalCode.c_str();
		tessEval = glCreateShader(GL_TESS_EVALUATION_SHADER);
		glShaderSource(tessEval, 1, &teShaderCode, NULL);
		glCompileShader(tessEval);
		checkCompileErrors(tessEval, "TESS_EVAL: " + tessEvalPath.value());
	}
	if (computePath) {
		const char* cShaderCode = computeCode.c_str();
		compute = glCreateShader(GL_COMPUTE_SHADER);
		glShaderSource(compute, 1, &cShaderCode, NULL);
		glCompileShader(compute);
		checkCompileErrors(compute, "COMPUTE: " + computePath.value());
	}
	// if geometry shader is given, compile geometry shader
	//unsigned int geometry;
	//if (geometryPath != nullptr)
	//{
	//	const char* gShaderCode = geometryCode.c_str();
	//	geometry = glCreateShader(GL_GEOMETRY_SHADER);
	//	glShaderSource(geometry, 1, &gShaderCode, NULL);
	//	glCompileShader(geometry);
	//	checkCompileErrors(geometry, "GEOMETRY");
	//}
	// shader Program
	ID = glCreateProgram();
	if (vertexPath) {
		glAttachShader(ID, vertex);
	}
	if (fragmentPath) {
		glAttachShader(ID, fragment);
	}
	if (geometryPath) {
		glAttachShader(ID, geometry);
	}
	if (tessControlPath) {
		glAttachShader(ID, tessControl);
	}
	if (tessEvalPath) {
		glAttachShader(ID, tessEval);
	}
	if (computePath) {
		glAttachShader(ID, compute);
	}
	glLinkProgram(ID);
	checkCompileErrors(ID, "PROGRAM");

	if (vertexPath) {
		glDeleteShader(vertex);
	}
	if (fragmentPath) {
		glDeleteShader(fragment);
	}
	if (geometryPath) {
		glDeleteShader(geometry);
	}
	if (tessControlPath) {
		glDeleteShader(tessControl);
	}
	if (tessEvalPath) {
		glDeleteShader(tessEval);
	}
	if (computePath) {
		glDeleteShader(compute);
	}
}

/*ShaderGl::ShaderGl(std::string computePath) {
	auto readShader = [](std::string path) {
		std::string code;

		std::ifstream shaderFile;
		shaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
		shaderFile.open(IKIGAI::UTILS::getRealPath(path));
		std::stringstream shaderStream;
		shaderStream << shaderFile.rdbuf();
		shaderFile.close();
		// convert stream into string
		code = shaderStream.str();
		return code;
	};

	// 1. retrieve the vertex/fragment source code from filePath
	std::string computeCode;
	try {
		computeCode = readFileWithInclude(computePath);
	}
	catch (std::ifstream::failure& e) {
		std::cout << "ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ: " << e.what() << std::endl;
	}

	const char* cShaderCode = computeCode.c_str();
	unsigned int compute = glCreateShader(GL_COMPUTE_SHADER);
	glShaderSource(compute, 1, &cShaderCode, NULL);
	glCompileShader(compute);
	checkCompileErrors(compute, "COMPUTE");

	// shader Program
	ID = glCreateProgram();
	glAttachShader(ID, compute);
	glLinkProgram(ID);
	checkCompileErrors(ID, "PROGRAM");
	glDeleteShader(compute);

	getReflection();
}*/

ShaderGl::~ShaderGl()
{
	glDeleteShader(ID);
}

void ShaderGl::getReflection() {
	GLint numActiveUniforms = 0;
	//uniforms.clear();
	struct UData {
		int id = 0;
		std::string name;
		IKIGAI::RENDER::UNIFORM_TYPE type = IKIGAI::RENDER::UNIFORM_TYPE::NONE;
		int arraySize = 0;
		int offset = 0;
		int blockIndex = 0;
	};
	std::vector<UData> data;

	const std::unordered_map<GLenum, IKIGAI::RENDER::UNIFORM_TYPE> fromGlType = {
		{ GL_FLOAT_MAT4, IKIGAI::RENDER::UNIFORM_TYPE::MAT4 },
		{ GL_FLOAT_MAT3, IKIGAI::RENDER::UNIFORM_TYPE::MAT3 },
		{ GL_FLOAT_VEC4, IKIGAI::RENDER::UNIFORM_TYPE::VEC4 },
		{ GL_FLOAT_VEC3, IKIGAI::RENDER::UNIFORM_TYPE::VEC3 },
		{ GL_FLOAT_VEC2, IKIGAI::RENDER::UNIFORM_TYPE::VEC2 },
		{ GL_INT, IKIGAI::RENDER::UNIFORM_TYPE::INT },
		{ GL_FLOAT, IKIGAI::RENDER::UNIFORM_TYPE::FLOAT },
		{ GL_BOOL, IKIGAI::RENDER::UNIFORM_TYPE::BOOL },
		{ GL_SAMPLER_2D, IKIGAI::RENDER::UNIFORM_TYPE::SAMPLER_2D },
		{ GL_SAMPLER_3D, IKIGAI::RENDER::UNIFORM_TYPE::SAMPLER_3D },
		{ GL_SAMPLER_2D_ARRAY, IKIGAI::RENDER::UNIFORM_TYPE::SAMPLER_2D_ARRAY},
		{ GL_SAMPLER_CUBE, IKIGAI::RENDER::UNIFORM_TYPE::SAMPLER_CUBE },
		{ GL_IMAGE_3D, IKIGAI::RENDER::UNIFORM_TYPE::IMAGE_3D }
	};
	const std::unordered_map<IKIGAI::RENDER::UNIFORM_TYPE, unsigned> typeToSize = {
		{ IKIGAI::RENDER::UNIFORM_TYPE::MAT4, sizeof(float) * 16 },
		{ IKIGAI::RENDER::UNIFORM_TYPE::MAT3, sizeof(float) * 9 },
		{ IKIGAI::RENDER::UNIFORM_TYPE::VEC4, sizeof(float) * 4 },
		{ IKIGAI::RENDER::UNIFORM_TYPE::VEC3, sizeof(float) * 3 },
		{ IKIGAI::RENDER::UNIFORM_TYPE::VEC2, sizeof(float) * 2 },
		{ IKIGAI::RENDER::UNIFORM_TYPE::INT, sizeof(int) * 1 },
		{ IKIGAI::RENDER::UNIFORM_TYPE::FLOAT, sizeof(float) * 1 },
		{ IKIGAI::RENDER::UNIFORM_TYPE::BOOL, sizeof(bool) * 1 },
		{ IKIGAI::RENDER::UNIFORM_TYPE::SAMPLER_2D, sizeof(int) * 1 },
		{ IKIGAI::RENDER::UNIFORM_TYPE::SAMPLER_3D, sizeof(int) * 1 },
		{ IKIGAI::RENDER::UNIFORM_TYPE::SAMPLER_CUBE, sizeof(int) * 1 },
		{ IKIGAI::RENDER::UNIFORM_TYPE::SAMPLER_2D_ARRAY, sizeof(int) * 1 },
		{ IKIGAI::RENDER::UNIFORM_TYPE::IMAGE_3D, sizeof(int) * 1 },
	};

	std::array<IKIGAI::RENDER::SHADER_TYPE, 6> shaderTypes = {
		IKIGAI::RENDER::SHADER_TYPE::VERTEX,
		IKIGAI::RENDER::SHADER_TYPE::TESSELLATION_CONTROL,
		IKIGAI::RENDER::SHADER_TYPE::TESSELLATION_EVALUATION,
		IKIGAI::RENDER::SHADER_TYPE::GEOMETRY,
		IKIGAI::RENDER::SHADER_TYPE::FRAGMENT,
		IKIGAI::RENDER::SHADER_TYPE::COMPUTE
	};

	glGetProgramiv(ID, GL_ACTIVE_UNIFORMS, &numActiveUniforms);
	std::vector<GLchar> nameData(256);
	for (int unif = 0; unif < numActiveUniforms; ++unif) {
		GLint arraySize = 0;
		GLenum type = 0;
		GLsizei actualLength = 0;
		glGetActiveUniform(ID, unif, static_cast<GLsizei>(nameData.size()), &actualLength, &arraySize, &type, &nameData[0]);
		std::string name(static_cast<char*>(nameData.data()), actualLength);

		std::array<GLenum, 8> blockProperties{ GL_OFFSET, GL_BLOCK_INDEX,
			GL_REFERENCED_BY_VERTEX_SHADER, GL_REFERENCED_BY_TESS_CONTROL_SHADER,
			GL_REFERENCED_BY_TESS_EVALUATION_SHADER, GL_REFERENCED_BY_GEOMETRY_SHADER,
			GL_REFERENCED_BY_FRAGMENT_SHADER, GL_REFERENCED_BY_COMPUTE_SHADER };
		std::array<GLint, 8> blockData{};
		glGetProgramResourceiv(ID, GL_UNIFORM, unif, blockProperties.size(), blockProperties.data(), blockData.size(), nullptr, blockData.data());

		if (name.back() == '\0') {
			name.pop_back();
		}
		

		//std::cout << "UNIFORT INFO" << std::endl;
		//std::cout << "id:" << unif << std::endl;
		//std::cout << "name:" << name << std::endl;
		//std::cout << "type:" << type << std::endl;
		//std::cout << "array size:" << arraySize << std::endl;
		//std::cout << "offset:" << blockData[0] << std::endl;
		//std::cout << "block index:" << blockData[1] << std::endl;

		data.push_back({ unif, name, fromGlType.at(type), arraySize, blockData[0], blockData[1] });

		// if it is UBO
		if (blockData[1] >= 0) {
			continue;
		}
		IKIGAI::RENDER::UniformInform uniform;
		uniform.name = data.back().name;
		uniform.type = IKIGAI::RENDER::UniformInform::TYPE::UNIFORM;
		for (int i = 2; i < blockData.size(); i++) {
			if (blockData[i]) {
				uniform.shaderType |= shaderTypes[i-2];
			}
		}
		//uniform.shaderType;
		uniform.size = typeToSize.at(data.back().type);
		uniform.members.push_back({ 
			data.back().type, data.back().name,
			data.back().offset, uniform.size, data.back().arraySize });
		mUniforms[uniform.name] = uniform;

		//auto p =name.find("[");
		//if (p != std::string::npos) {
		//	name = name.substr(0, p);
		//}

		//if (!isEngineUBOMember(name) && !isEngineUniformMember(name)) {
		//	RENDER::ShaderUniform defaultValue;
		//
		//	switch (static_cast<RENDER::UniformType>(type)) {
		//	case RENDER::UniformType::UNIFORM_BOOL:			defaultValue = static_cast<bool>(getUniformInt(name));					break;
		//	case RENDER::UniformType::UNIFORM_INT:			defaultValue = (getUniformInt(name));						break;
		//	case RENDER::UniformType::UNIFORM_FLOAT:		defaultValue = (getUniformFloat(name));					break;
		//	case RENDER::UniformType::UNIFORM_FLOAT_VEC2:	defaultValue = (getUniformVec2(name));		break;
		//	case RENDER::UniformType::UNIFORM_FLOAT_VEC3:	defaultValue = (getUniformVec3(name));		break;
		//	case RENDER::UniformType::UNIFORM_FLOAT_VEC4:	defaultValue = (getUniformVec4(name));		break;
		//	case RENDER::UniformType::UNIFORM_SAMPLER_2D: {
		//		std::shared_ptr<RESOURCES::Texture> t;
		//		defaultValue = t;
		//		break;
		//	}
		//	}
		//
		//	//if (defaultValue) {
		//	uniforms.push_back({
		//		static_cast<RENDER::UniformType>(type),
		//		name,
		//		getUniformLocation(nameData.data()),
		//		defaultValue
		//		});
		//	//}
		//}
	}
	std::array<GLenum, 10> blockProperties{
		GL_NAME_LENGTH, GL_NUM_ACTIVE_VARIABLES,
		GL_BUFFER_DATA_SIZE, GL_BUFFER_BINDING,
		GL_REFERENCED_BY_VERTEX_SHADER, GL_REFERENCED_BY_TESS_CONTROL_SHADER,
		GL_REFERENCED_BY_TESS_EVALUATION_SHADER, GL_REFERENCED_BY_GEOMETRY_SHADER,
		GL_REFERENCED_BY_FRAGMENT_SHADER, GL_REFERENCED_BY_COMPUTE_SHADER
	};
	std::array<GLint, 10> blockData{};
	GLint numUniformBlocks = 0;
	glGetProgramInterfaceiv(ID, GL_UNIFORM_BLOCK, GL_ACTIVE_RESOURCES, &numUniformBlocks);
	for (int blockIx = 0; blockIx < numUniformBlocks; ++blockIx) {
		glGetProgramResourceiv(ID, GL_UNIFORM_BLOCK, blockIx, blockProperties.size(), blockProperties.data(), blockData.size(), nullptr, blockData.data());

		//Retrieve name
		std::string blockName(blockData[0], '\0');
		glGetProgramResourceName(ID, GL_UNIFORM_BLOCK, blockIx, blockName.size() + 1, nullptr, blockName.data());

		//Retrieve indices of uniforms that are a member of this block.
		std::vector<GLint> uniformIxs(blockData[1]);
		GLenum member = GL_ACTIVE_VARIABLES;
		glGetProgramResourceiv(ID, GL_UNIFORM_BLOCK, blockIx, 1, &member, uniformIxs.size(), nullptr, uniformIxs.data());

		//We already retrieved the size.
		auto bufferDataSize = blockData[2];
		auto bind  = blockData[3];

		if (blockName.back() == '\0') {
			blockName.pop_back();
		}

		//std::cout << "UNIFORM BUFFER INFO" << std::endl;
		//std::cout << "Name: " << blockName << std::endl;
		//std::cout << "Size: " << bufferDataSize << std::endl;
		//std::cout << "Bind id: " << bind << std::endl;
		//std::cout << "Members: " << std::endl;
		//for (auto i : uniformIxs)
		//	std::cout << i << " " << std::endl;

		IKIGAI::RENDER::UniformInform uniform;
		uniform.name = blockName;
		uniform.type = IKIGAI::RENDER::UniformInform::TYPE::UNIFORM_BUFFER;
		uniform.binding = bind;
		//uniform.shaderType;
		uniform.size = bufferDataSize-8; //Wtf, I dont know why struct always bigger on 8 bytes
		for (auto i : uniformIxs) {
			uniform.members.push_back({
				data[i].type, data[i].name,
				data[i].offset, (int)typeToSize.at(data[i].type), data[i].arraySize});
		}
		for (int i = 4; i < blockData.size(); i++) {
			if (blockData[i]) {
				uniform.shaderType |= shaderTypes[i - 4];
			}
		}
		std::sort(uniform.members.begin(), uniform.members.end(), [](const auto& a, const auto& b) {
			return a.offset < b.offset;
		});
		mUniforms[uniform.name] = uniform;
	}
}

void ShaderGl::bind()
{
	glUseProgram(ID);
}
// utility uniform functions
// ------------------------------------------------------------------------
void ShaderGl::setBool(const std::string& name, bool value) const
{
	glUniform1i(glGetUniformLocation(ID, name.c_str()), (int)value);
}
// ------------------------------------------------------------------------
void ShaderGl::setInt(const std::string& name, int value) const
{
	glUniform1i(glGetUniformLocation(ID, name.c_str()), value);
}
// ------------------------------------------------------------------------
void ShaderGl::setFloat(const std::string& name, float value) const
{
	glUniform1f(glGetUniformLocation(ID, name.c_str()), value);
}
// ------------------------------------------------------------------------
void ShaderGl::setVec2(const std::string& name, const glm::vec2& value) const
{
	glUniform2fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]);
}
void ShaderGl::setVec2(const std::string& name, float x, float y) const
{
	glUniform2f(glGetUniformLocation(ID, name.c_str()), x, y);
}
// ------------------------------------------------------------------------
void ShaderGl::setVec3(const std::string& name, const glm::vec3& value) const
{
	glUniform3fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]);
}
void ShaderGl::setVec3(const std::string& name, float x, float y, float z) const
{
	glUniform3f(glGetUniformLocation(ID, name.c_str()), x, y, z);
}
// ------------------------------------------------------------------------
void ShaderGl::setVec4(const std::string& name, const glm::vec4& value) const
{
	glUniform4fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]);
}
void ShaderGl::setVec4(const std::string& name, float x, float y, float z, float w)
{
	glUniform4f(glGetUniformLocation(ID, name.c_str()), x, y, z, w);
}
// ------------------------------------------------------------------------
void ShaderGl::setMat2(const std::string& name, const glm::mat2& mat) const
{
	glUniformMatrix2fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
}
// ------------------------------------------------------------------------
void ShaderGl::setMat3(const std::string& name, const glm::mat3& mat) const
{
	glUniformMatrix3fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
}
// ------------------------------------------------------------------------
void ShaderGl::setMat4(const std::string& name, const glm::mat4& mat) const
{
	glUniformMatrix4fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
}

void ShaderGl::setMat4(const std::string& name, const IKIGAI::MATHGL::Matrix4& mat) const
{
	glUniformMatrix4fv(glGetUniformLocation(ID, name.c_str()), 1, GL_TRUE, mat.data._values);
}


void ShaderGl::setVec2(const std::string& name, const IKIGAI::MATHGL::Vector2f& vec2) {
	glUniform2f(getUniformLocation(name), vec2.x, vec2.y);
}

void ShaderGl::setVec3(const std::string& name, const IKIGAI::MATHGL::Vector3& vec3) {
	glUniform3f(getUniformLocation(name), vec3.x, vec3.y, vec3.z);
}

void ShaderGl::setVec4(const std::string& name, const IKIGAI::MATHGL::Vector4& vec4) {
	glUniform4f(getUniformLocation(name), vec4.x, vec4.y, vec4.z, vec4.w);
}

void ShaderGl::setMat4(const std::string& name, const IKIGAI::MATHGL::Matrix4& mat4) {
	glUniformMatrix4fv(getUniformLocation(name), 1, GL_TRUE, mat4.data._values);
}

void ShaderGl::setMat3(const std::string& name, const IKIGAI::MATHGL::Matrix3& mat3) {
	glUniformMatrix3fv(getUniformLocation(name), 1, GL_TRUE, mat3.data._values);
}


void ShaderGl::unbind()
{
	glUseProgram(0);
}

void ShaderGl::checkCompileErrors(GLuint shader, std::string type)
{
	GLint success;
	GLchar infoLog[1024];
	if (type != "PROGRAM")
	{
		glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
		if (!success)
		{
			glGetShaderInfoLog(shader, 1024, NULL, infoLog);
			std::cout << "ERROR::SHADER_COMPILATION_ERROR of type: " << type << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
		}
	}
	else
	{
		glGetProgramiv(shader, GL_LINK_STATUS, &success);
		if (!success)
		{
			glGetProgramInfoLog(shader, 1024, NULL, infoLog);
			std::cout << "ERROR::PROGRAM_LINKING_ERROR of type: " << type << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
		}
	}
}


IKIGAI::MATHGL::Vector2f ShaderGl::getUniformVec2(const std::string& name) {
	GLfloat values[2];
	glGetnUniformfv(ID, getUniformLocation(name), 2 * sizeof(float), values);
	return reinterpret_cast<IKIGAI::MATHGL::Vector2f&>(values);
}

IKIGAI::MATHGL::Vector3 ShaderGl::getUniformVec3(const std::string& name) {
	GLfloat values[3];
	glGetnUniformfv(ID, getUniformLocation(name), 3 * sizeof(float), values);
	return reinterpret_cast<IKIGAI::MATHGL::Vector3&>(values);
}

IKIGAI::MATHGL::Vector4 ShaderGl::getUniformVec4(const std::string& name) {
	GLfloat values[4];
	glGetnUniformfv(ID, getUniformLocation(name), 4 * sizeof(float), values);
	return reinterpret_cast<IKIGAI::MATHGL::Vector4&>(values);
}

IKIGAI::MATHGL::Matrix4 ShaderGl::getUniformMat4(const std::string& name) {
	GLfloat values[16];
	glGetnUniformfv(ID, getUniformLocation(name), 16 * sizeof(float), values);
	return reinterpret_cast<IKIGAI::MATHGL::Matrix4&>(values);
}


int ShaderGl::getUniformInt(const std::string& name) {
	int value;
	glGetUniformiv(ID, getUniformLocation(name), &value);
	return value;
}

float ShaderGl::getUniformFloat(const std::string& name) {
	float value;
	glGetUniformfv(ID, getUniformLocation(name), &value);
	return value;
}

int ShaderGl::getUniformLocation(const std::string& name) {
	if (uniformLocationCache.count(name))
		return uniformLocationCache.at(name);

	const int location = glGetUniformLocation(ID, name.c_str());
	if (location == -1) {
		//LOG_WARNING("Uniform: '" + name + "' doesn't exist\n");
	}
	uniformLocationCache[name] = location;
	return location;
}
#endif
