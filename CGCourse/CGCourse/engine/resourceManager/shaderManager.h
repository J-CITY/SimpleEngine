#pragma once

#include <sstream>
#include <string>
#include <GL/glew.h>
#include <fstream>
#include <functional>
#include "resourceManager.h"
#include "../utils/debug/logger.h"
#include "resource/shader.h"

namespace KUMA {
	namespace RESOURCES {
		class ShaderLoader : public ResourceManager<Shader> {
		public:
			static std::shared_ptr<Shader> Create(const std::string& filePath) {
				FILE_PATH = filePath;

				std::array<std::string, 3> source = ParseShader(filePath);

				uint32_t programID = CreateProgram(source[0], source[1], source[2]);

				if (programID) {
					return std::make_shared<Shader>(filePath, programID);
				}
				return nullptr;
			}

			static std::shared_ptr<Shader> CreateFromSource(const std::string& vertexShader, const std::string& fragmentShader, const std::string& geometryShader="") {
				uint32_t programID = CreateProgram(vertexShader, fragmentShader, geometryShader);

				if (programID) {
					return std::make_shared<Shader>("", programID);
				}
				return nullptr;
			}

			static void	Recompile(Shader& shader, const std::string& filePath) {
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
			static void Destroy(std::shared_ptr<Shader> shader) {
				if (shader) {
					shader.reset();
				}
			}
		public:
			virtual std::shared_ptr<Shader> createResource(const std::string& path) override {
				std::string realPath = getRealPath(path);
				auto shader = ShaderLoader::Create(realPath);
				if (shader) {
					shader->path = path;
				}
				return shader;
			}

			virtual void destroyResource(std::shared_ptr<Shader> res) override {
				ShaderLoader::Destroy(res);
			}
		private:
			static std::array<std::string, 3> ParseShader(const std::string& filePath) {
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
					auto includePath = line.substr(pos+1);
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
			static uint32_t CreateProgram(const std::string& vertexShader, const std::string& fragmentShader, const std::string& geometryShader) {
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

			static uint32_t CompileShader(uint32_t p_type, const std::string& p_source) {
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


			static std::string FILE_PATH;
		};

	}
}
