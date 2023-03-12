#pragma once

#ifdef OPENGL_BACKEND
#include <variant>
#include <memory>

#include "shaderGl.h"
#include "textureGl.h"
#include "uniformBufferGl.h"
#include "../../../resourceManager/serializerInterface.h"
#include "../../../utils/loader.h"
#include "../interface/materialInterface.h"
import glmath;

namespace KUMA {
	namespace RENDER {
		class UniformBuffer;

		class MaterialGl: public MaterialInterface, public RESOURCES::Serializable {
		public:
			using UniformData = std::variant<float, int, bool,
				MATHGL::Vector2f, MATHGL::Vector3, MATHGL::Vector4,
				std::shared_ptr<TextureGl>, std::vector<unsigned char>>;

			struct Uniform {
				std::string name;
				KUMA::RENDER::UNIFORM_TYPE type;
				unsigned location;
				UniformData defaultValue;
			};

			using UniformType = std::variant<UniformBufferGl<std::vector<unsigned char>>, Uniform>;
		public:
			std::shared_ptr<ShaderGl> mShader;
			std::unordered_map<std::string, UniformData> mUniformData;
			std::unordered_map<std::string, UniformType> mUniforms;

			bool mBlendable = false;
			bool mBackfaceCulling = true;
			bool mFrontfaceCulling = false;
			bool mDepthTest = true;
			bool mDepthWriting = true;
			bool mColorWriting = true;
			int mGpuInstances = 1;
			bool mIsDeferred = false;

			bool mIsCastShadow = true;
			bool mIsBakedShadow = false;

			void generateUniformsData();
		public:
			void fillUniformsWithShader(std::shared_ptr<ShaderGl> shader, std::shared_ptr<TextureInterface> defaultTexture, bool useTextures);


			void setShader(std::shared_ptr<ShaderGl> shader);
			void fillUniforms(std::shared_ptr<TextureInterface> defaultTexture, bool useTextures);
			void bind(std::shared_ptr<TextureInterface> defaultTexture, bool useTextures) override;
			void unbind() override;

			bool hasShader() const override {
				return mShader != nullptr;
			}

			unsigned getGPUInstances() const override {
				return mGpuInstances;
			}

			bool isDeferred() const override {
				return mIsDeferred;
			}

			bool isBlendable() const override {
				return mBlendable;
			}

			bool isCastShadow() const override {
				return mIsCastShadow;
			}

			bool isShadowBaked() const override {
				return mIsBakedShadow;
			}

			bool isBackfaceCulling() const override {
				return mBackfaceCulling;
			}

			bool isFrontfaceCulling() const override {
				return mFrontfaceCulling;
			}

			bool isDepthTest() const override {
				return mDepthTest;
			}

			bool isDepthWriting() const override {
				return mDepthWriting;
			}

			bool isColorWriting() const override {
				return mColorWriting;
			}

			std::shared_ptr<ShaderInterface> getShader() const override {
				return mShader;
			}

			void set(const std::string& name, UniformData data) {
				//TODO: add check for uniform type
				if (mUniformData.contains(name)) mUniformData.at(name) = data;
			}

			void setIsDeferred(bool v, std::shared_ptr<ShaderInterface> shader) override {
				mIsDeferred = v;

				if (mIsDeferred) {
					setShader(std::dynamic_pointer_cast<ShaderGl>(shader));
				}
				else {
					mUniforms.clear();
					mUniformData.clear();
					mShader = std::dynamic_pointer_cast<ShaderGl>(shader);
				}
			}



			//void set(const std::string& name, UniformData data, size_t offset) {
			//	//TODO: add check for uniform type and check that uniform is Uniform Buffer
			//	auto& data = std::get<std::vector<unsigned char>>(mUniformData.at(name));
			//	
			//}

			//struct Check
			//{
			//	MATHGL::Vector2f        u_TextureTiling;
			//	MATHGL::Vector2f        u_TextureOffset;
			//	MATHGL::Vector4        u_Diffuse;
			//	MATHGL::Vector3        u_Specular;
			//	float       u_Shininess;
			//	float       u_HeightScale;
			//	bool         u_EnableNormalMapping;
			//};
			void set(const std::string& name, const std::string& memberName, UniformData data);

			UniformData get(const std::string& name) {
				//TODO: add check for uniform type
				return mUniformData.at(name);
			}

			UniformData get(const std::string& name, const std::string& memberName) {
				const auto& udinfo = mShader->getUniformsInfo();
				auto cnt = udinfo.count(name);
				const auto& uniformInfo = udinfo.at(name);
				for (const auto& member : uniformInfo.members) {
					if (member.name == memberName) {
						auto startPtr = std::get<std::vector<unsigned char>>(mUniformData.at(name)).data() + member.offset;
						switch (member.type) {
						case UNIFORM_TYPE::VEC4:  { MATHGL::Vector4 res;  memcpy(std::addressof(res), startPtr, member.size); return res; }
						case UNIFORM_TYPE::VEC3:  { MATHGL::Vector3 res;  memcpy(std::addressof(res), startPtr, member.size); return res; }
						case UNIFORM_TYPE::VEC2:  { MATHGL::Vector2 res;  memcpy(std::addressof(res), startPtr, member.size); return res; }
						case UNIFORM_TYPE::INT:   { int res;  memcpy(std::addressof(res), startPtr, member.size); return res; }
						case UNIFORM_TYPE::FLOAT: { float res;  memcpy(std::addressof(res), startPtr, member.size); return res; }
						case UNIFORM_TYPE::BOOL:  { bool res;  memcpy(std::addressof(res), startPtr, member.size); return res; }
						}
						break;
					}
				}
				throw;
				return 0;
			}

			bool isEngineUBOMember(const std::string& uniformName) {
				return uniformName.rfind("EngineUBO", 0) == 0 || uniformName.rfind("Engine", 0) == 0;
			}

			bool isEngineUniform(const std::string& uniformName) {
				return uniformName.rfind("engine_", 0) == 0;
			}

			virtual void onSerialize(nlohmann::json& j) override {
				
			}

			bool trySetSimpleMember(const std::string& k, const nlohmann::json& v, std::optional<std::string> subname = std::nullopt);

			virtual void onDeserialize(nlohmann::json& j) override {
				auto vertexPath = j["shaderVertex"].get<std::string>();
				auto fragmentPath = j["shaderFragment"].get<std::string>();
				//TODO: get shader from resource system
				setShader(std::make_shared<ShaderGl>(vertexPath, fragmentPath));

				mBlendable = j.value("blendable", false);
				mBackfaceCulling = j.value("backfaceCulling", true);
				mFrontfaceCulling = j.value("frontfaceCulling", false);
				mDepthTest = j.value("depthTest", true);
				mDepthWriting = j.value("depthWriting", true);
				mColorWriting = j.value("colorWriting", true);
				mGpuInstances = j.value("gpuInstances", 1);
				mIsDeferred = j.value("isDeferred", false);

				if (j.count("uniforms")) {
					for (auto& [k, v] : j["uniforms"].items()) {
						if (!trySetSimpleMember(k, v)) {
							//uniform buffer
							for (auto& [name, data] : v.items()) {
								trySetSimpleMember(k, data, name);
							}
						}
					}
				}
			}

			uint8_t generateStateMask() const;
		};
	}
}

#endif
