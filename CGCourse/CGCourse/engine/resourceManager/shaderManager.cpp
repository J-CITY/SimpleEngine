#include "shaderManager.h"

#include "ServiceManager.h"

using namespace KUMA;
using namespace KUMA::RESOURCES;

std::string KUMA::RESOURCES::ShaderLoader::FILE_PATH = "";

ResourcePtr<Shader> ShaderLoader::createResource(const std::string& path) {
	return CreateFromFile(path);
}

ResourcePtr<Shader> ShaderLoader::CreateFromFile(const std::string& path) {
	std::string realPath = getRealPath(path);
	auto shader = Create(realPath);
	if (shader) {
		shader->path = path;
	}
	return shader;
}

int ln = 1;
std::array<std::string, 5> ShaderLoader::ParseShader(const std::string& filePath) {
	std::ifstream stream(filePath);
	if (!stream) {
		LOG_ERROR("Can not open file " + filePath);
		return {"", "", ""};
	}
	enum class ShaderType { NONE = -1, VERTEX = 0, FRAGMENT = 1, GEOMETRY = 2, TESS_CONTROL = 3, TESS_EVALUATION = 4 };

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
				//std::cout << ln << line << std::endl; ln++;
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
			if (line.find("vertex") != std::string::npos) { type = ShaderType::VERTEX; ln = 1; }
			else if (line.find("fragment") != std::string::npos) { type = ShaderType::FRAGMENT; ln = 1; }
			else if (line.find("geometry") != std::string::npos) { type = ShaderType::GEOMETRY; ln = 1; }
			else if (line.find("tessControl") != std::string::npos) { type = ShaderType::TESS_CONTROL; ln = 1; }
			else if (line.find("tessEvaluation") != std::string::npos) { type = ShaderType::TESS_EVALUATION; ln = 1; }
		}
		else if (line.find("#include") != std::string::npos) {
			makeIncludePath(ss[static_cast<int>(type)], line, filePath);
		}
		else if (type != ShaderType::NONE) {
			ss[static_cast<int>(type)] << line << '\n';
			//std::cout << ln << line << std::endl; ln++;
		}
	}

	return {
		ss[static_cast<int>(ShaderType::VERTEX)].str(),
		ss[static_cast<int>(ShaderType::FRAGMENT)].str(),
		ss[static_cast<int>(ShaderType::GEOMETRY)].str()
	};
}

uint32_t ShaderLoader::CreateProgram(const std::string& vertexShader, const std::string& fragmentShader, 
	const std::string& geometryShader, const std::string& tessCompShader, const std::string& tessEvoluationShader) {
	const uint32_t program = glCreateProgram();

	const uint32_t vs = CompileShader(GL_VERTEX_SHADER, vertexShader);
	const uint32_t fs = CompileShader(GL_FRAGMENT_SHADER, fragmentShader);
	uint32_t gs = 0;
	uint32_t tcs = 0;
	uint32_t tes = 0;
	if (!geometryShader.empty()) {
		gs = CompileShader(GL_GEOMETRY_SHADER, geometryShader);
	}
	if (!tessCompShader.empty()) {
		tcs = CompileShader(GL_TESS_CONTROL_SHADER, geometryShader);
	}
	if (!tessEvoluationShader.empty()) {
		tes = CompileShader(GL_TESS_EVALUATION_SHADER, geometryShader);
	}
	if (vs == 0 || fs == 0)
		return 0;

	glAttachShader(program, vs);
	glAttachShader(program, fs);
	if (gs != 0)
		glAttachShader(program, gs);
	if (tcs != 0)
		glAttachShader(program, tcs);
	if (tes != 0)
		glAttachShader(program, tes);
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
	if (tcs != 0)
		glDeleteShader(tcs);
	if (tes != 0)
		glDeleteShader(tes);

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

ResourcePtr<Shader> ShaderLoader::Create(const std::string& filePath) {
	FILE_PATH = filePath;

	std::array<std::string, 5> source = ParseShader(filePath);

	uint32_t programID = CreateProgram(source[0], source[1], source[2], source[3], source[4]);

	if (programID) {
		return ResourcePtr<Shader>(new Shader(filePath, programID), [](Shader* m) {
			ServiceManager::Get<ShaderLoader>().unloadResource<ShaderLoader>(m->path);
		});
	}
	return nullptr;
}

ResourcePtr<Shader> ShaderLoader::CreateFromSource(const std::string& vertexShader, const std::string& fragmentShader,
	const std::string& geometryShader, const std::string& tessCompShader, const std::string& tessEvoluationShader) {
	uint32_t programID = CreateProgram(vertexShader, fragmentShader, geometryShader, tessCompShader, tessEvoluationShader);

	if (programID) {
		return ResourcePtr<Shader>(new Shader("", programID), [](Shader* m) {
			ServiceManager::Get<ShaderLoader>().unloadResource<ShaderLoader>(m->path);
		});
	}
	return nullptr;
}

void ShaderLoader::Recompile(Shader& shader, const std::string& filePath) {
	FILE_PATH = filePath;

	std::array<std::string, 5> source = ParseShader(filePath);

	/* Create the new program */
	uint32_t newProgram = CreateProgram(source[0], source[1], source[2], source[3], source[4]);

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
