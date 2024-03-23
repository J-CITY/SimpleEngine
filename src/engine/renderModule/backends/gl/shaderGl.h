#pragma once
#include "mathModule/math.h"
#include "renderModule/backends/interface/resourceStruct.h"

#ifdef OPENGL_BACKEND
#include <coreModule/graphicsWrapper.hpp>
#include <fstream>
#include <optional>
#include <sstream>
#include <unordered_map>

#include "coreModule/glmWrapper.hpp"

#include "../interface/shaderInterface.h"
#include "../interface/reflectionStructs.h"

namespace IKIGAI
{
	namespace RENDER
	{
		struct UniformInfo;
	}
}

namespace IKIGAI::RENDER
{
    class ShaderGl : public IKIGAI::RENDER::ShaderInterface {
    public:
        unsigned int ID = 0;
        // constructor generates the shader on the fly
        // ------------------------------------------------------------------------

        explicit ShaderGl(const ShaderResource& res);
        bool checkBinarySupport();
        void compile(std::string vertexCode, std::string fragmentCode,
            std::string geometryCode, std::string tessControlCode,
            std::string tessEvalCode, std::string computeCode);

        ShaderGl(std::optional<std::string> vertexPath = std::nullopt, std::optional<std::string> fragmentPath = std::nullopt,
                 std::optional<std::string> geometryPath = std::nullopt, std::optional<std::string> tessControlPath = std::nullopt,
                 std::optional<std::string> tessEvalPath = std::nullopt, std::optional<std::string> computePath = std::nullopt);

        //ShaderGl(std::string computePath);

        ~ShaderGl() override;
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
        void setMat4(const std::string& name, const IKIGAI::MATH::Matrix4f& mat) const;
        void setVec2(const std::string& name, const IKIGAI::MATH::Vector2f& vec2);
        void setVec3(const std::string& name, const IKIGAI::MATH::Vector3f& vec3);
        void setVec4(const std::string& name, const IKIGAI::MATH::Vector4f& vec4);
        void setMat4(const std::string& name, const IKIGAI::MATH::Matrix4f& mat4);
        void setMat3(const std::string& name, const IKIGAI::MATH::Matrix3f& mat3);

        IKIGAI::MATH::Vector2f getUniformVec2(const std::string& name);
        IKIGAI::MATH::Vector3f getUniformVec3(const std::string& name);
        IKIGAI::MATH::Vector4f getUniformVec4(const std::string& name);
        IKIGAI::MATH::Matrix4f getUniformMat4(const std::string& name);
        int getUniformInt(const std::string& name);
        float getUniformFloat(const std::string& name);
        int getUniformLocation(const std::string& name);


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

        const std::unordered_map<std::string, IKIGAI::RENDER::UniformInform>& getUniformsInfo() const override {
            return mUniforms;
        }
    private:
        std::array<std::string, 6> read(std::optional<std::string> vertexPath, std::optional<std::string> fragmentPath,
            std::optional<std::string> geometryPath, std::optional<std::string> tessControlPath,
            std::optional<std::string> tessEvalPath, std::optional<std::string> computePath);
        void getReflection();

        void checkCompileErrors(GLuint shader, std::string type);

        std::unordered_map<std::string, IKIGAI::RENDER::UniformInform> mUniforms;
        std::unordered_map<std::string, int> uniformLocationCache;
    };
}

#endif
