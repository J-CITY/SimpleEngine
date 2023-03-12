#pragma once

#include <string>
#include <unordered_map>
#include <vector>
#include "../../render/backends/interface/uniformTypes.h"

import logger;
import glmath;

constexpr int MAX_BONES = 100;

namespace KUMA {
	namespace RESOURCES {
		class ShaderLoader;
		
		class Shader {
			friend class ShaderLoader;
		public:
			Shader(const std::string path, uint32_t id){};
			~Shader(){};
			//uint32_t getId() const;
			//std::vector<RENDER::UniformInfo>& getUniforms();
			//std::string getPath();
			//void bind() const;
			//void unbind() const;
			//void setUniformInt(const std::string& name, int value);
			//void setUniformFloat(const std::string& name, float value);
			//void setUniformVec2(const std::string& name, const MATHGL::Vector2f& vec2);
			//void setUniformVec3(const std::string& name, const MATHGL::Vector3& vec3);
			//void setUniformVec4(const std::string& name, const MATHGL::Vector4& vec4);
			//void setUniformMat4(const std::string& name, const MATHGL::Matrix4& mat4);
			//void setUniformMat3(const std::string& name, const MATHGL::Matrix3& mat3);
			//int getUniformInt(const std::string& name);
			//float getUniformFloat(const std::string& name);
			//MATHGL::Vector2f getUniformVec2(const std::string& name);
			//MATHGL::Vector3 getUniformVec3(const std::string& name);
			//MATHGL::Vector4 getUniformVec4(const std::string& name);
			//MATHGL::Matrix4 getUniformMat4(const std::string& name);
			//const RENDER::UniformInfo* getUniformInfo(const std::string& name) const;
			//void queryUniforms();
			//unsigned int getUniformLocation(const std::string& name);

			static bool isEngineUBOMember(const std::string& uniformName);
			static bool isEngineUniformMember(const std::string& uniformName);
		//private:
			//uint32_t id;
			std::string path;
			//std::vector<RENDER::UniformInfo> uniforms;
			//std::unordered_map<std::string, int> uniformLocationCache;
		};
	}
};


