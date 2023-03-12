#pragma once

#ifdef OPENGL_BACKEND
#include <gl/glew.h>
#include <fstream>
#include <optional>
#include <sstream>
#include <unordered_map>
#include <glm/fwd.hpp>

#include "../interface/shaderInterface.h"
#include "../interface/reflectionStructs.h"

import glmath;

namespace KUMA
{
	namespace RENDER
	{
		struct UniformInfo;
	}
}

namespace KUMA::RENDER
{
    class ShaderGl : public KUMA::RENDER::ShaderInterface {
    public:
        unsigned int ID;
        // constructor generates the shader on the fly
        // ------------------------------------------------------------------------
        ShaderGl(std::string vertexPath, std::string fragmentPath,
            std::optional<std::string> geometryPath = std::nullopt, std::optional<std::string> tessControlPath = std::nullopt,
            std::optional<std::string> tessEvalPath = std::nullopt, std::optional<std::string> computePath = std::nullopt);
        // activate the shader
        // ------------------------------------------------------------------------
        void bind() override;
        // utility uniform functions
        // ------------------------------------------------------------------------
        void setBool(const std::string& name, bool value) const;
        void setInt(const std::string& name, int value) const;
        void setFloat(const std::string& name, float value) const;
        void setVec2(const std::string& name, const glm::vec2& value) const;
        void setVec2(const std::string& name, float x, float y) const;
        void setVec3(const std::string& name, const glm::vec3& value) const;
        void setVec3(const std::string& name, float x, float y, float z) const;
        void setVec4(const std::string& name, const glm::vec4& value) const;
        void setVec4(const std::string& name, float x, float y, float z, float w);
        void setMat2(const std::string& name, const glm::mat2& mat) const;
        void setMat3(const std::string& name, const glm::mat3& mat) const;
        void setMat4(const std::string& name, const glm::mat4& mat) const;
        void setMat4(const std::string& name, const KUMA::MATHGL::Matrix4& mat) const;
        void setVec2(const std::string& name, const KUMA::MATHGL::Vector2f& vec2);
        void setVec3(const std::string& name, const KUMA::MATHGL::Vector3& vec3);
        void setVec4(const std::string& name, const KUMA::MATHGL::Vector4& vec4);
        void setMat4(const std::string& name, const KUMA::MATHGL::Matrix4& mat4);
        void setMat3(const std::string& name, const KUMA::MATHGL::Matrix3& mat3);

        KUMA::MATHGL::Vector2f ShaderGl::getUniformVec2(const std::string& name);
        KUMA::MATHGL::Vector3 ShaderGl::getUniformVec3(const std::string& name);
        KUMA::MATHGL::Vector4 ShaderGl::getUniformVec4(const std::string& name);
        KUMA::MATHGL::Matrix4 ShaderGl::getUniformMat4(const std::string& name);
        int ShaderGl::getUniformInt(const std::string& name);
        float ShaderGl::getUniformFloat(const std::string& name);
        unsigned ShaderGl::getUniformLocation(const std::string& name);


        void unbind() override;;
        virtual void setUniform(const UniformBufferInterface& uniform) override {

        }
        virtual void setPushConstant(const PushConstantInterface& uniform) override
        {

        };
        virtual int getId() override
        {
            return ID;
        }

        const std::unordered_map<std::string, KUMA::RENDER::UniformInform>& getUniformsInfo() const override {
            return mUniforms;
        }
    private:
        void getReflection();

        void checkCompileErrors(GLuint shader, std::string type);

        std::unordered_map<std::string, KUMA::RENDER::UniformInform> mUniforms;
        std::unordered_map<std::string, int> uniformLocationCache;
    };
}

#endif
