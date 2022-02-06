#pragma once
#include <string>

#include "lightComponent.h"
#include "../../../DiffuseLightRT.h"
#include "../../render/buffers/depthBuffer.h"

namespace KUMA::ECS { class Object; }

namespace KUMA::ECS {
	class DirectionalLight : public LightComponent {
	public:

		constexpr static size_t TextureCount = 3;
		DirectionalLight(const ECS::Object& obj);

		virtual void onDeserialize(nlohmann::json& j) override {
			LightComponent::onDeserialize(j);
		}
		virtual void onSerialize(nlohmann::json& j) override {
			LightComponent::onSerialize(j);
		}


		std::array<MATHGL::Matrix4, 3> ProjectionMatrices;
		std::array<MATHGL::Matrix4, 3> BiasedProjectionMatrices;

		
		float distance = 0.0f;
		float orthoBoxSize = 3000.0f;

		float zNear = 0.0f;
		float zFar = 0.0f;
		MATHGL::Vector3 Direction = MATHGL::Vector3(0.5f, 1.0f, 1.0f);
		std::array<float, TextureCount> Projections = {15.0f, 150.0f, 1500.0f};
		
		MATHGL::Matrix4 shadowProjectionMat;
		MATHGL::Matrix4 lightView;
		MATHGL::Matrix4 lightSpaceMatrix;
		size_t depthMapTextureID = 0;
		MATHGL::Matrix4 GetMatrix(const KUMA::MATHGL::Vector3& center, size_t index) const;
	};
}