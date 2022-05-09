#pragma once
#include "../resourceManager/resource/shader.h"
#include "../resourceManager/resource/texture.h"
#include "../utils/debug/logger.h"
//TODO: move to resources

#include <variant>

#include "../resourceManager/serializerInterface.h"

namespace KUMA {
	namespace RENDER {
		class Material: public RESOURCES::Serializable {
		public:
			void setShader(std::shared_ptr<RESOURCES::Shader> _shader);
			void fillUniform();
			void bind(std::shared_ptr<RESOURCES::Shader> _shader, std::shared_ptr<RESOURCES::Texture> emptyTexture, bool useTextures);
			void bind(std::shared_ptr<RESOURCES::Texture> _texture, bool useTextures);
			void unbind();

			template<typename T>
			inline void set(const std::string key, const T& val) {
				if (hasShader()) {
					if (uniformsData.find(key) != uniformsData.end())
						uniformsData[key] = val;
				}
				else {
					LOG_ERROR("Material Set failed: No attached shader");
				}
			}

			template<typename T>
			inline const T& get(const std::string key) {
				if (uniformsData.find(key) == uniformsData.end())
					return T();
				else
					return std::get<T>(uniformsData.at(key));
			}

			std::shared_ptr<RESOURCES::Shader> getShader();
			bool hasShader() const;

			void setBlendable(bool p_blendable);

			void setBackfaceCulling(bool p_backfaceCulling);

			void setFrontfaceCulling(bool p_frontfaceCulling);
			void setDepthTest(bool p_depthTest);

			void setDepthWriting(bool p_depthWriting);

			void setColorWriting(bool p_colorWriting);

			void setGPUInstances(int p_instances);

			bool isBlendable() const;

			bool hasBackfaceCulling() const;

			bool hasFrontfaceCulling() const;

			bool hasDepthTest() const;

			bool hasDepthWriting() const;

			bool hasColorWriting() const;

			int getGPUInstances() const;

			uint8_t generateStateMask() const;

			std::map<std::string, ShaderUniform>& getUniformsData();

			std::string path;

			virtual void onDeserialize(nlohmann::json& j) override;
			virtual void onSerialize(nlohmann::json& j) override;

			float Displacement = 0.0f;

			std::shared_ptr<RESOURCES::Texture> albedoMap;
		public:
			std::shared_ptr<RESOURCES::Shader> shader;
			std::map<std::string, ShaderUniform> uniformsData;

			bool blendable = false;
			bool backfaceCulling = true;
			bool frontfaceCulling = false;
			bool depthTest = true;
			bool depthWriting = true;
			bool colorWriting = true;
			int gpuInstances = 1;

			bool isDeferred = false;
		};
	}
}
