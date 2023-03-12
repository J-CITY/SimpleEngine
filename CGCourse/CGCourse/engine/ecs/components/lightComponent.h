#pragma once

#include <glm/glm.hpp>

#include "component.h"
#include "../../../game/TextureAtlas.h"
#include "../../render/light.h"
import glmath;

namespace KUMA::ECS { class Object; }

namespace KUMA::ECS {
	class LightComponent : public Component
	{
	public:
		LightComponent(Ref<ECS::Object> obj);
		const RENDER::Light& getData() const;
		const MATHGL::Vector3& getColor() const;
		float getIntensity() const;
		void setColor(const MATHGL::Vector3& color);
		void setIntensity(float intensity);
		virtual void onDeserialize(nlohmann::json& j) override {
			MATHGL::Vector3 dump;
			RESOURCES::DeserializeVec3(j["color"], dump);
			data.color = dump;
			data.intensity = j["intensity"];
		}
		virtual void onSerialize(nlohmann::json& j) override {
			j["data"]["intensity"] = data.intensity;
			RESOURCES::SerializeVec3(j["data"]["color"], data.color);
		}

		//unsigned int shadowRes = 1024;
		//unsigned int depthMapTextureID = 0;
		//
		//float strength = 1.0f;
		//float zNear = 1.0f;
		//float zFar = 2000.0f;
		//int depthMap;

		std::shared_ptr<RESOURCES::Texture> shadowMap;

		glm::mat4 shadowProjectionMatGLM = glm::mat4(0.0);
		glm::mat4 lookAtPerFace[6];

		MATHGL::Matrix4 Transform;
	protected:
		RENDER::Light data;
	};
}