#pragma once
#include "lightComponent.h"
#include "../../render/buffers/depthBuffer.h"

namespace KUMA::ECS { class Object; }

namespace KUMA::ECS {
	class SpotLight : public LightComponent {
	public:
		SpotLight(const ECS::Object& obj);
		float getConstant() const;
		float getLinear() const;
		float getQuadratic() const;
		float getCutoff() const;
		float getOuterCutoff() const;
		void setConstant(float constant);
		void setLinear(float linear);
		void setQuadratic(float quadratic);
		void setCutoff(float cutoff);
		void setOuterCutoff(float outerCutoff);

		virtual void onDeserialize(nlohmann::json& j) override {
			LightComponent::onDeserialize(j);
			data.constant = j["data"]["constant"];
			data.linear = j["data"]["linear"];
			data.quadratic = j["data"]["quadratic"];
			data.cutoff = j["data"]["cutoff"];
			data.outerCutoff = j["data"]["outerCutoff"];
		}
		virtual void onSerialize(nlohmann::json& j) override {
			LightComponent::onSerialize(j);
			j["data"]["constant"] = data.constant;
			j["data"]["linear"] = data.linear;
			j["data"]["quadratic"] = data.quadratic;
			j["data"]["cutoff"] = data.cutoff;
			j["data"]["outerCutoff"] = data.outerCutoff;
		}

		MATHGL::Matrix4 SpotLight::GetMatrix(const MATHGL::Vector3& position) const {
			auto Projection = MATHGL::Matrix4::CreatePerspective(TO_RADIANS(2.0f * angle), 1.0f, 1.1f, this->maxDistance);
			auto normalizedDirection = MATHGL::Vector3::Normalize(MATHGL::Vector3(
				this->Direction.x + 0.0001f,
				this->Direction.y,
				this->Direction.z + 0.0001f
			));
			auto View = MATHGL::Matrix4::CreateView(
				position,
				position + normalizedDirection,
				MATHGL::Vector3(0.0f, 1.0f, 0.0f)
			);
			return Projection * View;
		}


		float distance = 0.0f;
		float orthoBoxSize = 3000.0f;

		const float angle = TO_RADIANS(90.0f);
		float aspect = 1.0f;

		float maxDistance = 1000.0f;

		MATHGL::Vector3 Direction;
		
		MATHGL::Matrix4 projectionMat;
		MATHGL::Matrix4 BiasedProjectionMatrix;
		MATHGL::Matrix4 Transform;
	};
}