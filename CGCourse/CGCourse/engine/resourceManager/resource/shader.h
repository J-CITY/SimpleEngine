#pragma once

#include <string>
#include <memory>
#include <unordered_map>
#include <vector>
#include <GL/glew.h>

#include "texture.h"
#include "../../render/buffers/uniformTypes.h"
import logger;

import glmath;

#define MAX_BONES 100

namespace KUMA {
	namespace RESOURCES {
		class ShaderLoader;
		
		class Shader {
			friend class ShaderLoader;
		public:
			Shader(const std::string path, uint32_t id) : path(path), id(id) {
				queryUniforms();
			}
			
			~Shader() {
				glDeleteProgram(id);
			}

			uint32_t getId() const {
				return id;
			}

			std::vector<RENDER::UniformInfo>& getUniforms() {
				return uniforms;
			}

			std::string getPath() {
				return path;
			}

			void bind() const {
				glUseProgram(id);
			}
			void unbind() const {
				glUseProgram(0);
			}
			void setUniformInt(const std::string& name, int value) {
				glUniform1i(getUniformLocation(name), value);
			}

			void setUniformFloat(const std::string& name, float value) {
				glUniform1f(getUniformLocation(name), value);
			}

			void setUniformVec2(const std::string& name, const MATHGL::Vector2f& vec2) {
				glUniform2f(getUniformLocation(name), vec2.x, vec2.y);
			}

			void setUniformVec3(const std::string& name, const MATHGL::Vector3& vec3) {
				glUniform3f(getUniformLocation(name), vec3.x, vec3.y, vec3.z);
			}

			void setUniformVec4(const std::string& name, const MATHGL::Vector4& vec4) {
				glUniform4f(getUniformLocation(name), vec4.x, vec4.y, vec4.z, vec4.w);
			}

			void setUniformMat4(const std::string& name, const MATHGL::Matrix4& mat4) {
				glUniformMatrix4fv(getUniformLocation(name), 1, GL_TRUE, mat4.data._values);
			}

			void setUniformMat3(const std::string& name, const MATHGL::Matrix3& mat3) {
				glUniformMatrix3fv(getUniformLocation(name), 1, GL_TRUE, mat3.data._values);
			}

			int getUniformInt(const std::string& name) {
				int value;
				glGetUniformiv(id, getUniformLocation(name), &value);
				return value;
			}

			float getUniformFloat(const std::string& name) {
				float value;
				glGetUniformfv(id, getUniformLocation(name), &value);
				return value;
			}

			MATHGL::Vector2f getUniformVec2(const std::string& name) {
				GLfloat values[2];
				glGetnUniformfv(id, getUniformLocation(name), 2 * sizeof(float), values);
				return reinterpret_cast<MATHGL::Vector2f&>(values);
			}

			MATHGL::Vector3 getUniformVec3(const std::string& name) {
				GLfloat values[3];
				glGetnUniformfv(id, getUniformLocation(name), 3 * sizeof(float), values);
				return reinterpret_cast<MATHGL::Vector3&>(values);
			}

			MATHGL::Vector4 getUniformVec4(const std::string& name) {
				GLfloat values[4];
				glGetnUniformfv(id, getUniformLocation(name), 4 * sizeof(float), values);
				return reinterpret_cast<MATHGL::Vector4&>(values);
			}

			MATHGL::Matrix4 getUniformMat4(const std::string& name) {
				GLfloat values[16];
				glGetnUniformfv(id, getUniformLocation(name), 16 * sizeof(float), values);
				return reinterpret_cast<MATHGL::Matrix4&>(values);
			}

			const RENDER::UniformInfo* getUniformInfo(const std::string& name) const {
				auto found = std::find_if(uniforms.begin(), uniforms.end(), [&name](const RENDER::UniformInfo& in) {
					return name == in.name;
				});

				if (found != uniforms.end())
					return &*found;
				else
					return nullptr;
			}

			void queryUniforms() {
				GLint numActiveUniforms = 0;
				uniforms.clear();
				glGetProgramiv(id, GL_ACTIVE_UNIFORMS, &numActiveUniforms);
				std::vector<GLchar> nameData(256);
				for (int unif = 0; unif < numActiveUniforms; ++unif) {
					GLint arraySize = 0;
					GLenum type = 0;
					GLsizei actualLength = 0;
					glGetActiveUniform(id, unif, static_cast<GLsizei>(nameData.size()), &actualLength, &arraySize, &type, &nameData[0]);
					std::string name(static_cast<char*>(nameData.data()), actualLength);

					//auto p =name.find("[");
					//if (p != std::string::npos) {
					//	name = name.substr(0, p);
					//}

					if (!isEngineUBOMember(name) && !isEngineUniformMember(name)) {
						RENDER::ShaderUniform defaultValue;

						switch (static_cast<RENDER::UniformType>(type)) {
						case RENDER::UniformType::UNIFORM_BOOL:			defaultValue = static_cast<bool>(getUniformInt(name));					break;
						case RENDER::UniformType::UNIFORM_INT:			defaultValue = (getUniformInt(name));						break;
						case RENDER::UniformType::UNIFORM_FLOAT:		defaultValue = (getUniformFloat(name));					break;
						case RENDER::UniformType::UNIFORM_FLOAT_VEC2:	defaultValue = (getUniformVec2(name));		break;
						case RENDER::UniformType::UNIFORM_FLOAT_VEC3:	defaultValue = (getUniformVec3(name));		break;
						case RENDER::UniformType::UNIFORM_FLOAT_VEC4:	defaultValue = (getUniformVec4(name));		break;
						case RENDER::UniformType::UNIFORM_SAMPLER_2D: {
							std::shared_ptr<RESOURCES::Texture> t;
							defaultValue = t;
							break;
						}
						}

						//if (defaultValue) {
							uniforms.push_back({
								static_cast<RENDER::UniformType>(type),
								name,
								getUniformLocation(nameData.data()),
								defaultValue
							});
						//}
					}
				}


				//bones
				//getUniformLocation("u_UseBone");
				//setUniformInt("u_UseBone", 0);
				//for (int i = 0; i < MAX_BONES; i ++) {
				//	getUniformLocation("u_FinalBonesMatrices["+ std::to_string(i) +"]");
				//}
			}

			static bool isEngineUBOMember(const std::string& uniformName) {
				return uniformName.rfind("ubo_", 0) == 0;
			}

			static bool isEngineUniformMember(const std::string& uniformName) {
				return uniformName.rfind("u_engine_", 0) == 0;
			}
			
			unsigned int getUniformLocation(const std::string& name) {
				if (uniformLocationCache.find(name) != uniformLocationCache.end())
					return uniformLocationCache.at(name);

				const int location = glGetUniformLocation(id, name.c_str());

				if (location == -1) {
					//LOG_WARNING("Uniform: '" + name + "' doesn't exist\n");
				}
				uniformLocationCache[name] = location;

				return location;
			}

		private:
			uint32_t id;
			std::string path;
			std::vector<RENDER::UniformInfo> uniforms;
			std::unordered_map<std::string, int> uniformLocationCache;
		};
	}
};


