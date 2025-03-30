#include "shaderGl.h"
#ifdef OPENGL_BACKEND

#include "../interface/reflectionStructs.h"
#include "utilsModule/pathGetter.h"
#include "utilsModule/stringUtils.h"
#include "utilsModule/log/loggerDefine.h"
#include <array>
#include <filesystem>
#include <iostream>
#include "coreModule/glmWrapper.hpp"

//std::vector<uint32_t> load_spirv_file(const std::filesystem::path& path)
//{
//	std::ifstream in(path, std::ios::binary);
//
//	if (!in.good()) {
//		throw std::runtime_error("Failed to open file.");
//	}
//
//	auto fsize = in.tellg();
//	in.seekg(0, std::ios::end);
//	fsize = in.tellg() - fsize;
//	in.seekg(0, std::ios::beg);
//
//	std::vector<uint32_t> output;
//	output.resize(static_cast<size_t>(fsize / 4));
//	in.read(reinterpret_cast<char*>(output.data()), static_cast<size_t>(fsize));
//
//	return output;
//}

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



std::string readFile(const std::string& path) {
	std::ifstream ifs(IKIGAI::UTILS::GetRealPath(path));
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

std::shared_ptr<IKIGAI::RENDER::ShaderGl> IKIGAI::RENDER::ShaderGl::CreateFromSource(const std::map<ShaderType, std::string>& source) {
	ShaderResource res;
	return std::make_shared<ShaderGl>(res, source);
}

std::shared_ptr<IKIGAI::RENDER::ShaderGl> IKIGAI::RENDER::ShaderGl::CreateFromPath(const std::map<ShaderType, std::string>& path) {
	ShaderResource res;
	res.fragment = path.contains(ShaderType::FRAGMENT) ?  path.at(ShaderType::FRAGMENT) : "";
	res.vertex = path.contains(ShaderType::VERTEX) ?  path.at(ShaderType::VERTEX) : "";
	res.geometry = path.contains(ShaderType::GEOMETRY) ?  path.at(ShaderType::GEOMETRY) : "";
	res.tessControl = path.contains(ShaderType::TESSELLATION_CONTROL) ?  path.at(ShaderType::TESSELLATION_CONTROL) : "";
	res.tessEval = path.contains(ShaderType::TESSELLATION_EVALUATION) ?  path.at(ShaderType::TESSELLATION_EVALUATION) : "";
	res.compute = path.contains(ShaderType::COMPUTE) ?  path.at(ShaderType::COMPUTE) : "";
	return CreateFromPath(res);
}

std::shared_ptr<IKIGAI::RENDER::ShaderGl> IKIGAI::RENDER::ShaderGl::CreateFromPath(const ShaderResource& resource) {
	if (resource.useBinary) {
		bool useBinary = resource.useBinary;
		useBinary &= CheckBinarySupport();
		const auto binPath = resource.path + ".bin";
		if (useBinary && std::filesystem::exists(binPath)) {
			//TODO: ask it in file system
			std::ifstream inputStream(binPath, std::ios::binary);
			std::istreambuf_iterator<char> startIt(inputStream), endIt;
			std::vector<std::byte> buffer(startIt, endIt);
			inputStream.close();
			return std::make_shared<ShaderGl>(resource, buffer);
		}
	}
	std::map<ShaderType, std::string> source;
	if (!resource.compute.empty()) {
		source[ShaderType::COMPUTE] = readFileWithInclude(ConstructRealPath(resource.compute));
	}
	else {
		if (!resource.vertex.empty()) {
			source[ShaderType::VERTEX] = readFileWithInclude(ConstructRealPath(resource.vertex));
		}
		if (!resource.fragment.empty()) {
			source[ShaderType::FRAGMENT] = readFileWithInclude(ConstructRealPath(resource.fragment));
		}
		if (!resource.tessControl.empty()) {
			source[ShaderType::TESSELLATION_CONTROL] = readFileWithInclude(ConstructRealPath(resource.tessControl));
		}
		if (!resource.tessEval.empty()) {
			source[ShaderType::TESSELLATION_EVALUATION] = readFileWithInclude(ConstructRealPath(resource.tessEval));
		}
		if (!resource.geometry.empty()) {
			source[ShaderType::GEOMETRY] = readFileWithInclude(ConstructRealPath(resource.geometry));
		}
	}
	return std::make_shared<ShaderGl>(resource, source);
}

IKIGAI::RENDER::ShaderGl::ShaderGl(const ShaderResource& res, const std::map<ShaderType, std::string>& source) {
	create(res, source);
}

//For binary shader
IKIGAI::RENDER::ShaderGl::ShaderGl(const ShaderResource& res, const std::vector<std::byte>& source) {
	//Set paths
	mPath = res.path;
	if (!res.vertex.empty()) this->vertexPath = ConstructRealPath(res.vertex);
	if (!res.fragment.empty()) this->fragmentPath = ConstructRealPath(res.fragment);
	if (!res.geometry.empty()) this->geometryPath = ConstructRealPath(res.geometry);
	if (!res.tessEval.empty()) this->tessEvalPath = ConstructRealPath(res.tessEval);
	if (!res.tessControl.empty()) this->tessControlPath = ConstructRealPath(res.tessControl);
	if (!res.compute.empty()) this->computePath = ConstructRealPath(res.compute);
	loadBinaryShader(source);
	readReflection();
}

void IKIGAI::RENDER::ShaderGl::loadBinaryShader(const std::vector<std::byte>& buffer) {
#ifndef USING_GLES
	mId = glCreateProgram();
	GLint formats = 0;
	glGetIntegerv(GL_NUM_PROGRAM_BINARY_FORMATS, &formats);
	std::vector<GLint> binaryFormats;
	binaryFormats.resize(formats);
	glGetIntegerv(GL_PROGRAM_BINARY_FORMATS, binaryFormats.data());

	glProgramBinary(static_cast<unsigned>(mId), binaryFormats[0], buffer.data(), buffer.size());
	// Check for success/failure
	GLint status;
	glGetProgramiv(static_cast<unsigned>(mId), GL_LINK_STATUS, &status);
	if (GL_FALSE == status) {
		// Handle failure ...
	}
	//glValidateProgram(ID);
#endif
}

bool IKIGAI::RENDER::ShaderGl::CheckBinarySupport() {
#ifndef USING_GLES
	GLint formats = 0;
	glGetIntegerv(GL_NUM_PROGRAM_BINARY_FORMATS, &formats);
	if (formats < 1) {
		//LOG_INFO("Driver does not support any binary formats.");
		return false;
	}
	return true;
#else
	return false;
#endif
}

void IKIGAI::RENDER::ShaderGl::recompile(const ShaderResource& res, const std::map<ShaderType, std::string>& source) {
	clear();
	create(res, source);
}

void IKIGAI::RENDER::ShaderGl::create(const ShaderResource& res, const std::map<ShaderType, std::string>& source) {
	mPath = res.path;
	if (!res.vertex.empty()) this->vertexPath = ConstructRealPath(res.vertex);
	if (!res.fragment.empty()) this->fragmentPath = ConstructRealPath(res.fragment);
	if (!res.geometry.empty()) this->geometryPath = ConstructRealPath(res.geometry);
	if (!res.tessEval.empty()) this->tessEvalPath = ConstructRealPath(res.tessEval);
	if (!res.tessControl.empty()) this->tessControlPath = ConstructRealPath(res.tessControl);
	if (!res.compute.empty()) this->computePath = ConstructRealPath(res.compute);

	compile(source);

	//TODO: save it in thread pool
	if (mId) {
		bool useBinary = res.useBinary;
		useBinary &= CheckBinarySupport();
		const auto binPath = res.path + ".bin";
		if (useBinary && !std::filesystem::exists(binPath)) {
#ifndef USING_GLES
			GLint length = 0;
			glGetProgramiv(static_cast<unsigned>(mId), GL_PROGRAM_BINARY_LENGTH, &length);

			std::vector<GLubyte> buffer(length);
			GLenum format = 0;
			glGetProgramBinary(static_cast<unsigned>(mId), length, nullptr, &format, buffer.data());
			std::ofstream out(binPath.c_str(), std::ios::binary);
			out.write(reinterpret_cast<char*>(buffer.data()), length);
			out.close();
#endif
		}
	}

	readReflection();
}

void IKIGAI::RENDER::ShaderGl::clear() const {
	glDeleteShader(static_cast<unsigned>(mId));
}

void IKIGAI::RENDER::ShaderGl::compile(const std::map<ShaderType, std::string>& source) {
	mId = glCreateProgram();
#ifndef USING_GLES
	//Check if compute shader
	if (source.contains(ShaderType::COMPUTE)) {
		const char* shaderCode = source.at(ShaderType::COMPUTE).c_str();
		auto compute = glCreateShader(GL_COMPUTE_SHADER);
		glShaderSource(compute, 1, &shaderCode, nullptr);
		glCompileShader(compute);
		checkCompileErrors(compute, "COMPUTE: " + computePath.value());
		glAttachShader(static_cast<unsigned>(mId), compute);
		glLinkProgram(static_cast<unsigned>(mId));
		checkCompileErrors(static_cast<unsigned>(mId), "PROGRAM");
		glDeleteShader(compute);
		return;
	}
#endif

	unsigned int vertex = 0, fragment = 0, geometry = 0, tessControl = 0, tessEval = 0;
	if (source.contains(ShaderType::VERTEX)) {
		const char* shaderCode = source.at(ShaderType::VERTEX).c_str();
		vertex = glCreateShader(GL_VERTEX_SHADER);
		glShaderSource(vertex, 1, &shaderCode, nullptr);
		glCompileShader(vertex);
		checkCompileErrors(vertex, "VERTEX: " + vertexPath.value());
	}
	if (source.contains(ShaderType::FRAGMENT)) {
		const char* shaderCode = source.at(ShaderType::FRAGMENT).c_str();
		fragment = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(fragment, 1, &shaderCode, nullptr);
		glCompileShader(fragment);
		checkCompileErrors(fragment, "FRAGMENT: " + fragmentPath.value());
	}
#ifndef USING_GLES
	if (source.contains(ShaderType::GEOMETRY)) {
		const char* shaderCode = source.at(ShaderType::GEOMETRY).c_str();
		geometry = glCreateShader(GL_GEOMETRY_SHADER);
		glShaderSource(geometry, 1, &shaderCode, nullptr);
		glCompileShader(geometry);
		checkCompileErrors(geometry, "GEOMETRY: " + geometryPath.value());
	}
	if (source.contains(ShaderType::TESSELLATION_CONTROL)) {
		const char* shaderCode = source.at(ShaderType::TESSELLATION_CONTROL).c_str();
		tessControl = glCreateShader(GL_TESS_CONTROL_SHADER);
		glShaderSource(tessControl, 1, &shaderCode, nullptr);
		glCompileShader(tessControl);
		checkCompileErrors(tessControl, "TESS_CONTROL: " + tessControlPath.value());
	}
	if (source.contains(ShaderType::TESSELLATION_EVALUATION)) {
		const char* shaderCode = source.at(ShaderType::TESSELLATION_EVALUATION).c_str();
		tessEval = glCreateShader(GL_TESS_EVALUATION_SHADER);
		glShaderSource(tessEval, 1, &shaderCode, nullptr);
		glCompileShader(tessEval);
		checkCompileErrors(tessEval, "TESS_EVAL: " + tessEvalPath.value());
	}
#endif

	mId = glCreateProgram();
	if (vertexPath) {
		glAttachShader(static_cast<unsigned>(mId), vertex);
	}
	if (fragmentPath) {
		glAttachShader(static_cast<unsigned>(mId), fragment);
	}
#ifndef USING_GLES
	if (geometryPath) {
		glAttachShader(static_cast<unsigned>(mId), geometry);
	}
	if (tessControlPath) {
		glAttachShader(static_cast<unsigned>(mId), tessControl);
	}
	if (tessEvalPath) {
		glAttachShader(static_cast<unsigned>(mId), tessEval);
	}
#endif

	//for old opengl
	//auto ver = glGetString(GL_SHADING_LANGUAGE_VERSION);
	//if (vertexCode.find("#version 120") != std::string::npos || vertexCode.find("#version 100") != std::string::npos) {
	//	glBindAttribLocation(mId, 0, "geo_Pos");
	//	glBindAttribLocation(mId, 1, "geo_TexCoords");
	//	glBindAttribLocation(mId, 2, "geo_Normal");
	//	glBindAttribLocation(mId, 3, "geo_Tangent");
	//	glBindAttribLocation(mId, 4, "geo_Bitangent");
	//	glBindAttribLocation(mId, 5, "boneIds");
	//	glBindAttribLocation(mId, 6, "weights");
	//}
	glLinkProgram(static_cast<unsigned>(mId));
	checkCompileErrors(static_cast<unsigned>(mId), "PROGRAM");

	if (vertexPath) {
		glDeleteShader(vertex);
	}
	if (fragmentPath) {
		glDeleteShader(fragment);
	}
#ifndef USING_GLES
	if (geometryPath) {
		glDeleteShader(geometry);
	}
	if (tessControlPath) {
		glDeleteShader(tessControl);
	}
	if (tessEvalPath) {
		glDeleteShader(tessEval);
	}
#endif
}

IKIGAI::RENDER::ShaderGl::~ShaderGl() {
	clear();
}

void IKIGAI::RENDER::ShaderGl::readReflection() {
	mReflection = ShaderReflection();

	struct UData {
		int id = 0;
		std::string name;
		IKIGAI::RENDER::ShaderReflection::UniformType type = IKIGAI::RENDER::ShaderReflection::UniformType::NONE;
		int arraySize = 0;
		int offset = 0;
		int blockIndex = 0;
		size_t shaderMask = 0;
	};

	static const std::map<GLenum, IKIGAI::RENDER::ShaderReflection::UniformType> fromGlType = {
		{GL_FLOAT_MAT4, IKIGAI::RENDER::ShaderReflection::UniformType::MAT4},
		{GL_FLOAT_MAT3, IKIGAI::RENDER::ShaderReflection::UniformType::MAT3},
		{GL_FLOAT_VEC4, IKIGAI::RENDER::ShaderReflection::UniformType::VEC4},
		{GL_FLOAT_VEC3, IKIGAI::RENDER::ShaderReflection::UniformType::VEC3},
		{GL_FLOAT_VEC2, IKIGAI::RENDER::ShaderReflection::UniformType::VEC2},
		{GL_INT, IKIGAI::RENDER::ShaderReflection::UniformType::INT},
		{GL_FLOAT, IKIGAI::RENDER::ShaderReflection::UniformType::FLOAT},
		{GL_BOOL, IKIGAI::RENDER::ShaderReflection::UniformType::BOOL},
		{GL_SAMPLER_2D, IKIGAI::RENDER::ShaderReflection::UniformType::SAMPLER_2D},
		{GL_SAMPLER_CUBE, IKIGAI::RENDER::ShaderReflection::UniformType::SAMPLER_CUBE},
#ifndef USING_GLES
		{GL_SAMPLER_3D, IKIGAI::RENDER::ShaderReflection::UniformType::SAMPLER_3D},
		{GL_SAMPLER_2D_ARRAY, IKIGAI::RENDER::ShaderReflection::UniformType::SAMPLER_2D_ARRAY},
		{GL_SAMPLER_2D_ARRAY, IKIGAI::RENDER::ShaderReflection::UniformType::IMAGE_2D},
		{GL_SAMPLER_2D_ARRAY, IKIGAI::RENDER::ShaderReflection::UniformType::IMAGE_2D_ARRAY},
		{GL_SAMPLER_2D_ARRAY, IKIGAI::RENDER::ShaderReflection::UniformType::IMAGE_3D},
		{GL_SAMPLER_2D_ARRAY, IKIGAI::RENDER::ShaderReflection::UniformType::IMAGE_CUBE},
#endif
	};

	static const std::map<IKIGAI::RENDER::ShaderReflection::UniformType, unsigned> typeToSize = {
		{IKIGAI::RENDER::ShaderReflection::UniformType::MAT4, sizeof(float) * 16},
		{IKIGAI::RENDER::ShaderReflection::UniformType::MAT3, sizeof(float) * 9},
		{IKIGAI::RENDER::ShaderReflection::UniformType::VEC4, sizeof(float) * 4},
		{IKIGAI::RENDER::ShaderReflection::UniformType::VEC3, sizeof(float) * 3},
		{IKIGAI::RENDER::ShaderReflection::UniformType::VEC2, sizeof(float) * 2},
		{IKIGAI::RENDER::ShaderReflection::UniformType::INT, sizeof(uint32_t) * 1},
		{IKIGAI::RENDER::ShaderReflection::UniformType::FLOAT, sizeof(float) * 1},
		{IKIGAI::RENDER::ShaderReflection::UniformType::BOOL, sizeof(uint32_t) * 1},
		{IKIGAI::RENDER::ShaderReflection::UniformType::SAMPLER_2D, sizeof(uint32_t) * 1},
		{IKIGAI::RENDER::ShaderReflection::UniformType::SAMPLER_CUBE, sizeof(uint32_t) * 1},
#ifndef USING_GLES
		{IKIGAI::RENDER::ShaderReflection::UniformType::SAMPLER_3D, sizeof(uint32_t) * 1},
		{IKIGAI::RENDER::ShaderReflection::UniformType::SAMPLER_2D_ARRAY, sizeof(uint32_t) * 1},
		{IKIGAI::RENDER::ShaderReflection::UniformType::IMAGE_2D, sizeof(uint32_t) * 1},
		{IKIGAI::RENDER::ShaderReflection::UniformType::IMAGE_2D_ARRAY, sizeof(uint32_t) * 1},
		{IKIGAI::RENDER::ShaderReflection::UniformType::IMAGE_3D, sizeof(uint32_t) * 1},
		{IKIGAI::RENDER::ShaderReflection::UniformType::IMAGE_CUBE, sizeof(uint32_t) * 1},
#endif
	};

	std::array<IKIGAI::RENDER::ShaderType, 6> shaderTypes = {
		IKIGAI::RENDER::ShaderType::VERTEX,
		IKIGAI::RENDER::ShaderType::FRAGMENT,
		IKIGAI::RENDER::ShaderType::TESSELLATION_CONTROL,
		IKIGAI::RENDER::ShaderType::TESSELLATION_EVALUATION,
		IKIGAI::RENDER::ShaderType::GEOMETRY,
		IKIGAI::RENDER::ShaderType::COMPUTE
	};

	std::vector<UData> data;
	GLint numActiveUniforms = 0;
	glGetProgramiv(static_cast<unsigned>(mId), GL_ACTIVE_UNIFORMS, &numActiveUniforms);

	std::vector<GLchar> nameData(256);
	for (auto i = 0; i < numActiveUniforms; ++i) {
		GLint arraySize = 0;
		GLenum type = 0;
		GLsizei actualLength = 0;
		glGetActiveUniform(static_cast<unsigned>(mId), i, static_cast<GLsizei>(nameData.size()), &actualLength, &arraySize, &type, nameData.data());
		std::string name(nameData.data(), actualLength);

		std::array<GLint, 8> blockData{};
#ifndef USING_GLES
		std::array<GLenum, 8> blockProperties{ GL_OFFSET, GL_BLOCK_INDEX,
			GL_REFERENCED_BY_VERTEX_SHADER, GL_REFERENCED_BY_FRAGMENT_SHADER,
			GL_REFERENCED_BY_TESS_CONTROL_SHADER, GL_REFERENCED_BY_TESS_EVALUATION_SHADER,
			GL_REFERENCED_BY_GEOMETRY_SHADER, GL_REFERENCED_BY_COMPUTE_SHADER };
		glGetProgramResourceiv(static_cast<unsigned>(mId), GL_UNIFORM, i, blockProperties.size(), blockProperties.data(), blockData.size(), nullptr, blockData.data());
#endif
		if (name.back() == '\0') {
			name.pop_back();
		}

		data.push_back({i, name, fromGlType.at(type), arraySize, blockData[0], blockData[1]});

		// if it is UBO
#ifndef USING_GLES
		if (blockData[1] >= 0) {
			//UBO construct later
			continue;
		}
#endif

		for (int i = 2; i < blockData.size(); i++) {
			if (blockData[i]) {
				data.back().shaderMask |= static_cast<size_t>(shaderTypes[i - 2]);
			}
		}
	}
#ifndef USING_GLES
	std::array<GLenum, 10> blockProperties{
		GL_NAME_LENGTH, GL_NUM_ACTIVE_VARIABLES,
		GL_BUFFER_DATA_SIZE, GL_BUFFER_BINDING,
		GL_REFERENCED_BY_VERTEX_SHADER, GL_REFERENCED_BY_TESS_CONTROL_SHADER,
		GL_REFERENCED_BY_TESS_EVALUATION_SHADER, GL_REFERENCED_BY_GEOMETRY_SHADER,
		GL_REFERENCED_BY_FRAGMENT_SHADER, GL_REFERENCED_BY_COMPUTE_SHADER
	};
	std::array<GLint, 10> blockData{};
	GLint numUniformBlocks = 0;
	glGetProgramInterfaceiv(static_cast<unsigned>(mId), GL_UNIFORM_BLOCK, GL_ACTIVE_RESOURCES, &numUniformBlocks);
	std::set<int> usedIds;
	for (int blockIx = 0; blockIx < numUniformBlocks; ++blockIx) {
		glGetProgramResourceiv(static_cast<unsigned>(mId), GL_UNIFORM_BLOCK, blockIx, blockProperties.size(), blockProperties.data(), blockData.size(), nullptr, blockData.data());

		//Retrieve name
		std::string blockName(blockData[0], '\0');
		glGetProgramResourceName(static_cast<unsigned>(mId), GL_UNIFORM_BLOCK, blockIx, blockName.size() + 1, nullptr, blockName.data());

		//Retrieve indices of uniforms that are a member of this block.
		std::vector<GLint> uniformIxs(blockData[1]);
		GLenum member = GL_ACTIVE_VARIABLES;
		glGetProgramResourceiv(static_cast<unsigned>(mId), GL_UNIFORM_BLOCK, blockIx, 1, &member, uniformIxs.size(), nullptr, uniformIxs.data());

		//We already retrieved the size.
		auto bufferDataSize = blockData[2];
		auto bind  = blockData[3];

		if (blockName.back() == '\0') {
			blockName.pop_back();
		}

		// std::cout << "UNIFORM BUFFER INFO" << std::endl;
		// std::cout << "Name: " << blockName << std::endl;
		// std::cout << "Size: " << bufferDataSize << std::endl;
		// std::cout << "Bind id: " << bind << std::endl;
		// std::cout << "Members: " << std::endl;
		// for (auto i : uniformIxs)
		//	std::cout << i << " " << std::endl;

		IKIGAI::RENDER::ShaderReflection::Uniform uniform;
		uniform.mName = blockName;
		uniform.mType = IKIGAI::RENDER::ShaderReflection::UniformType::UNIFORM_BUFFER;
		uniform.mBind = bind;
		uniform.mSize = bufferDataSize - 8; // Wtf, I dont know why struct always bigger on 8 bytes
		for (auto i : uniformIxs) {
			ShaderReflection::UniformMember member;
			member.mType = data[i].type;
			member.mName = data[i].name;
			member.mOffset = data[i].offset;
			member.mSize = (int)typeToSize.at(data[i].type);
			member.mArraySize = data[i].arraySize;
			usedIds.insert(i);
		}
		for (int i = 4; i < blockData.size(); i++) {
			if (blockData[i]) {
				uniform.mShaderMask |= (size_t)shaderTypes[i - 4];
			}
		}
		std::sort(uniform.mMembers.begin(), uniform.mMembers.end(),
		[](const auto &a, const auto &b) {
				return a.mOffset < b.mOffset;
		});
		mReflection.mUniforms.push_back(uniform);
		mReflection.mNameToUniforms[uniform.mName] = mReflection.mUniforms.size() - 1;
	}
#endif
	for (int i = 0; i < data.size(); ++i) {
		if (usedIds.contains(i)) {
			continue;
		}
		IKIGAI::RENDER::ShaderReflection::Uniform uniform;
		uniform.mName = data[i].name;
		uniform.mType = data[i].type;
		uniform.mShaderMask = data[i].shaderMask;
		uniform.mSize = typeToSize.at(uniform.mType);
		mReflection.mUniforms.push_back(uniform);
		mReflection.mNameToUniforms[uniform.mName] = mReflection.mUniforms.size() - 1;
	}
}

//TODO: use logger
void IKIGAI::RENDER::ShaderGl::checkCompileErrors(GLuint shader, std::string type) {
	GLint success;
	GLchar infoLog[1024];
	if (type != "PROGRAM") {
		glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
		if (!success) {
			glGetShaderInfoLog(shader, 1024, NULL, infoLog);
			std::cout << "ERROR::SHADER_COMPILATION_ERROR of type: " << type << "\n" << infoLog << "\n --------------------------------------------------- -- " << std::endl;
		}
	} else {
		glGetProgramiv(shader, GL_LINK_STATUS, &success);
		if (!success) {
			glGetProgramInfoLog(shader, 1024, NULL, infoLog);
			std::cout << "ERROR::PROGRAM_LINKING_ERROR of type: " << type << "\n" << infoLog << "\n --------------------------------------------------- -- " << std::endl;
		}
	}
}

void IKIGAI::RENDER::ShaderGl::bind() {
	glUseProgram(static_cast<unsigned>(mId));
}

void IKIGAI::RENDER::ShaderGl::unbind() {
	glUseProgram(0);
}

void IKIGAI::RENDER::ShaderGl::setBool(const std::string& name, bool value) const {
	glUniform1i(glGetUniformLocation(mId, name.c_str()), (int)value);
}

void IKIGAI::RENDER::ShaderGl::setInt(const std::string& name, int value) const {
	glUniform1i(glGetUniformLocation(mId, name.c_str()), value);
}

void IKIGAI::RENDER::ShaderGl::setFloat(const std::string& name, float value) const {
	glUniform1f(glGetUniformLocation(mId, name.c_str()), value);
}

void IKIGAI::RENDER::ShaderGl::setVec2(const std::string& name, const glm::vec2& value) const {
	glUniform2fv(glGetUniformLocation(mId, name.c_str()), 1, &value[0]);
}

void IKIGAI::RENDER::ShaderGl::setVec2(const std::string& name, float x, float y) const {
	glUniform2f(glGetUniformLocation(mId, name.c_str()), x, y);
}

void IKIGAI::RENDER::ShaderGl::setVec3(const std::string& name, const glm::vec3& value) const {
	glUniform3fv(glGetUniformLocation(mId, name.c_str()), 1, &value[0]);
}

void IKIGAI::RENDER::ShaderGl::setVec3(const std::string& name, float x, float y, float z) const {
	glUniform3f(glGetUniformLocation(mId, name.c_str()), x, y, z);
}

void IKIGAI::RENDER::ShaderGl::setVec4(const std::string& name, const glm::vec4& value) const {
	glUniform4fv(glGetUniformLocation(mId, name.c_str()), 1, &value[0]);
}

void IKIGAI::RENDER::ShaderGl::setVec4(const std::string& name, float x, float y, float z, float w) {
	glUniform4f(glGetUniformLocation(mId, name.c_str()), x, y, z, w);
}

void IKIGAI::RENDER::ShaderGl::setMat2(const std::string& name, const glm::mat2& mat) const {
	glUniformMatrix2fv(glGetUniformLocation(mId, name.c_str()), 1, GL_FALSE, &mat[0][0]);
}

void IKIGAI::RENDER::ShaderGl::setMat3(const std::string& name, const glm::mat3& mat) const {
	glUniformMatrix3fv(glGetUniformLocation(mId, name.c_str()), 1, GL_FALSE, &mat[0][0]);
}

void IKIGAI::RENDER::ShaderGl::setMat4(const std::string& name, const glm::mat4& mat) const {
	glUniformMatrix4fv(glGetUniformLocation(mId, name.c_str()), 1, GL_FALSE, &mat[0][0]);
}

void IKIGAI::RENDER::ShaderGl::setVec2(const std::string& name, const IKIGAI::MATH::Vector2f& vec2) const {
	glUniform2f(getUniformLocation(name), vec2.x, vec2.y);
}

void IKIGAI::RENDER::ShaderGl::setVec3(const std::string& name, const IKIGAI::MATH::Vector3f& vec3) const {
	glUniform3f(getUniformLocation(name), vec3.x, vec3.y, vec3.z);
}

void IKIGAI::RENDER::ShaderGl::setVec4(const std::string& name, const IKIGAI::MATH::Vector4f& vec4) const {
	glUniform4f(getUniformLocation(name), vec4.x, vec4.y, vec4.z, vec4.w);
}

void IKIGAI::RENDER::ShaderGl::setMat4(const std::string& name, const IKIGAI::MATH::Matrix4f& mat4) const {
#ifndef USING_GLES
	glUniformMatrix4fv(getUniformLocation(name), 1, GL_TRUE, mat4.getData());
#else
	auto m = IKIGAI::MATH::Matrix4f::Transpose(mat4);
	glUniformMatrix4fv(getUniformLocation(name), 1, GL_FALSE, m.getData());
#endif
}

void IKIGAI::RENDER::ShaderGl::setMat3(const std::string& name, const IKIGAI::MATH::Matrix3f& mat3) const {
#ifndef USING_GLES
	glUniformMatrix3fv(getUniformLocation(name), 1, GL_TRUE, mat3.getData());
#else
	auto m = IKIGAI::MATH::Matrix4f::Transpose(mat3);
	glUniformMatrix3fv(getUniformLocation(name), 1, GL_FALSE, m.getData());
#endif
}

IKIGAI::MATH::Vector2f IKIGAI::RENDER::ShaderGl::getUniformVec2(const std::string& name) const {
	GLfloat values[2];
	glGetnUniformfv(static_cast<unsigned>(mId), getUniformLocation(name), 2 * sizeof(float), values);
	return reinterpret_cast<IKIGAI::MATH::Vector2f&>(values);
}

IKIGAI::MATH::Vector3f IKIGAI::RENDER::ShaderGl::getUniformVec3(const std::string& name) const {
	GLfloat values[3];
	glGetnUniformfv(static_cast<unsigned>(mId), getUniformLocation(name), 3 * sizeof(float), values);
	return reinterpret_cast<IKIGAI::MATH::Vector3f&>(values);
}

IKIGAI::MATH::Vector4f IKIGAI::RENDER::ShaderGl::getUniformVec4(const std::string& name) const {
	GLfloat values[4];
	glGetnUniformfv(static_cast<unsigned>(mId), getUniformLocation(name), 4 * sizeof(float), values);
	return reinterpret_cast<IKIGAI::MATH::Vector4f&>(values);
}

IKIGAI::MATH::Matrix3f IKIGAI::RENDER::ShaderGl::getUniformMat3(const std::string& name) const {
	GLfloat values[16];
	glGetnUniformfv(static_cast<unsigned>(mId), getUniformLocation(name), 9 * sizeof(float), values);
	return reinterpret_cast<IKIGAI::MATH::Matrix3f&>(values);
}

IKIGAI::MATH::Matrix4f IKIGAI::RENDER::ShaderGl::getUniformMat4(const std::string& name) const {
	GLfloat values[16];
	glGetnUniformfv(static_cast<unsigned>(mId), getUniformLocation(name), 16 * sizeof(float), values);
	return reinterpret_cast<IKIGAI::MATH::Matrix4f&>(values);
}

int IKIGAI::RENDER::ShaderGl::getUniformInt(const std::string& name) const {
	int value;
	glGetUniformiv(static_cast<unsigned>(mId), getUniformLocation(name), &value);
	return value;
}

float IKIGAI::RENDER::ShaderGl::getUniformFloat(const std::string& name) const {
	float value;
	glGetUniformfv(static_cast<unsigned>(mId), getUniformLocation(name), &value);
	return value;
}

int IKIGAI::RENDER::ShaderGl::getUniformLocation(const std::string& name) const {
	if (uniformLocationCache.contains(name)) {
		return uniformLocationCache.at(name);
	}
	const int location = glGetUniformLocation(static_cast<unsigned>(mId), name.c_str());
	if (location == -1) {
		LOG_ERROR << ("Uniform: '" + name + "' doesn't exist\n");
		return location;
	}
	uniformLocationCache[name] = location;
	return location;
}
#endif
