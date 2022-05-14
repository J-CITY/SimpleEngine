#include "shaderManager.h"

using namespace KUMA;
using namespace KUMA::RESOURCES;

std::string KUMA::RESOURCES::ShaderLoader::FILE_PATH = "";

std::shared_ptr<Shader> ShaderLoader::createResource(const std::string& path) {
	return CreateFromFile(path);
}

std::shared_ptr<Shader> ShaderLoader::CreateFromFile(const std::string& path) {
	std::string realPath = getRealPath(path);
	auto shader = Create(realPath);
	if (shader) {
		shader->path = path;
	}
	return shader;
}

void ShaderLoader::destroyResource(std::shared_ptr<Shader> res) {
	Destroy(res);
}


void ShaderLoader::Destroy(std::shared_ptr<Shader> res) {
	if (res) {
		res.reset();
	}
}

std::array<std::string, 3> ShaderLoader::ParseShader(const std::string& filePath) {
	std::ifstream stream(filePath);
	if (!stream) {
		LOG_ERROR("Can not open file " + filePath);
		return {"", "", ""};
	}
	enum class ShaderType { NONE = -1, VERTEX = 0, FRAGMENT = 1, GEOMETRY };

	std::string line;
	std::stringstream ss[3];
	ShaderType type = ShaderType::NONE;

	std::function<void(std::stringstream& ss, std::string&, const std::string&)> makeIncludePath;
	std::function<void(std::stringstream&, std::string&)> readInclude;
	readInclude = [&makeIncludePath](std::stringstream& ss, const std::string& filePath) {
		std::ifstream stream(filePath);
		if (!stream) {
			LOG_ERROR("Can not open file " + filePath);
			return;
		}
		std::string line;
		while (std::getline(stream, line)) {
			if (line.find("#include") != std::string::npos) {
				makeIncludePath(ss, line, filePath);
			}
			else {
				ss << line << '\n';
			}
		}
	};
	makeIncludePath = [&readInclude](std::stringstream& ss, std::string& line, const std::string& filePath) {
		std::size_t pos = line.find("\"");
		auto includePath = line.substr(pos + 1);
		if (includePath.empty()) {
			LOG_ERROR("Shader: include path is empty");
			return;
		}
		includePath = includePath.substr(0, includePath.size() - 1);
		includePath = std::filesystem::path(filePath).parent_path().string() +
			(includePath[0] == '/' || includePath[0] == '\\' ? "" : "/") + includePath;
		readInclude(ss, includePath);
	};

	while (std::getline(stream, line)) {
		if (line.find("#shader") != std::string::npos) {
			if (line.find("vertex") != std::string::npos)			type = ShaderType::VERTEX;
			else if (line.find("fragment") != std::string::npos)	type = ShaderType::FRAGMENT;
			else if (line.find("geometry") != std::string::npos)	type = ShaderType::GEOMETRY;
		}
		else if (line.find("#include") != std::string::npos) {
			makeIncludePath(ss[static_cast<int>(type)], line, filePath);
		}
		else if (type != ShaderType::NONE) {
			ss[static_cast<int>(type)] << line << '\n';
		}
	}

	return {
		ss[static_cast<int>(ShaderType::VERTEX)].str(),
		ss[static_cast<int>(ShaderType::FRAGMENT)].str(),
		ss[static_cast<int>(ShaderType::GEOMETRY)].str()
	};
}

uint32_t ShaderLoader::CreateProgram(const std::string& vertexShader, const std::string& fragmentShader, const std::string& geometryShader) {
	const uint32_t program = glCreateProgram();

	const uint32_t vs = CompileShader(GL_VERTEX_SHADER, vertexShader);
	const uint32_t fs = CompileShader(GL_FRAGMENT_SHADER, fragmentShader);
	uint32_t gs = 0;
	if (!geometryShader.empty())
		gs = CompileShader(GL_GEOMETRY_SHADER, geometryShader);

	if (vs == 0 || fs == 0)
		return 0;

	glAttachShader(program, vs);
	glAttachShader(program, fs);
	if (gs != 0)
		glAttachShader(program, gs);
	glLinkProgram(program);

	GLint linkStatus;
	glGetProgramiv(program, GL_LINK_STATUS, &linkStatus);

	if (linkStatus == GL_FALSE) {
		GLint maxLength;
		glGetProgramiv(program, GL_INFO_LOG_LENGTH, &maxLength);

		std::string errorLog(maxLength, ' ');
		glGetProgramInfoLog(program, maxLength, &maxLength, errorLog.data());

		LOG_ERROR("[LINK] \"" + FILE_PATH + "\":\n" + errorLog);

		glDeleteProgram(program);

		return 0;
	}

	glValidateProgram(program);
	glDeleteShader(vs);
	glDeleteShader(fs);
	if (gs != 0)
		glDeleteShader(gs);

	return program;
}

uint32_t ShaderLoader::CompileShader(uint32_t p_type, const std::string& p_source) {
	const uint32_t id = glCreateShader(p_type);

	const char* src = p_source.c_str();

	glShaderSource(id, 1, &src, nullptr);

	glCompileShader(id);

	GLint compileStatus;
	glGetShaderiv(id, GL_COMPILE_STATUS, &compileStatus);

	if (compileStatus == GL_FALSE) {
		GLint maxLength;
		glGetShaderiv(id, GL_INFO_LOG_LENGTH, &maxLength);

		std::string errorLog(maxLength, ' ');
		glGetShaderInfoLog(id, maxLength, &maxLength, errorLog.data());

		std::string shaderTypeString = "VERTEX SHADER";
		if (p_type == GL_FRAGMENT_SHADER) shaderTypeString = "FRAGMENT SHADER";
		if (p_type == GL_GEOMETRY_SHADER) shaderTypeString = "GEOMETRY SHADER";
		std::string errorHeader = "[" + shaderTypeString + "] \"";
		LOG_ERROR(errorHeader + FILE_PATH + "\":\n" + errorLog);

		glDeleteShader(id);

		return 0;
	}

	return id;
}

std::shared_ptr<Shader> ShaderLoader::Create(const std::string& filePath) {
	FILE_PATH = filePath;

	std::array<std::string, 3> source = ParseShader(filePath);

	uint32_t programID = CreateProgram(source[0], source[1], source[2]);

	if (programID) {
		return std::make_shared<Shader>(filePath, programID);
	}
	return nullptr;
}

std::shared_ptr<Shader> ShaderLoader::CreateFromSource(const std::string& vertexShader, const std::string& fragmentShader, const std::string& geometryShader) {
	uint32_t programID = CreateProgram(vertexShader, fragmentShader, geometryShader);

	if (programID) {
		return std::make_shared<Shader>("", programID);
	}
	return nullptr;
}

void	ShaderLoader::Recompile(Shader& shader, const std::string& filePath) {
	FILE_PATH = filePath;

	std::array<std::string, 3> source = ParseShader(filePath);

	/* Create the new program */
	uint32_t newProgram = CreateProgram(source[0], source[1], source[2]);

	if (newProgram) {
		/* Pointer to the shaderID (const data member, tricks to access it) */
		std::uint32_t* shaderID = reinterpret_cast<uint32_t*>(&shader) + offsetof(Shader, id);

		/* Deletes the previous program */
		glDeleteProgram(*shaderID);

		/* Store the new program in the shader */
		*shaderID = newProgram;

		shader.queryUniforms();

		LOG_INFO("[COMPILE] \"" + FILE_PATH + "\": Success!");
	}
	else {
		LOG_INFO("[COMPILE] \"" + FILE_PATH + "\": Failed! Previous shader version keept");
	}
}
