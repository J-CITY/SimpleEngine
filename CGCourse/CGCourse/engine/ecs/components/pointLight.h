#pragma once
#include "lightComponent.h"
#include "../../render/buffers/depthBuffer.h"

import glmath;

namespace KUMA::ECS { class Object; }

namespace KUMA::ECS{
	class PointLight : public LightComponent {
	public:
		PointLight(const ECS::Object& obj);
		float getConstant() const;
		float getLinear() const;
		float getQuadratic() const;
		void setConstant(float constant);
		void setLinear(float linear);
		void setQuadratic(float quadratic);

		virtual void onDeserialize(nlohmann::json& j) override {
			LightComponent::onDeserialize(j);
			data.constant = j["data"]["constant"];
			data.linear = j["data"]["linear"];
			data.quadratic = j["data"]["quadratic"];
		}
		virtual void onSerialize(nlohmann::json& j) override {
			LightComponent::onSerialize(j);
			j["data"]["constant"] = data.constant;
			j["data"]["linear"] = data.linear;
			j["data"]["quadratic"] = data.quadratic;
		}

		std::vector<MATHGL::Vector3> DirectionTable =
		{
			 MATHGL::Vector3::Normalize(MATHGL::Vector3(1.0f, 0.0001f, 0.0001f)),
			 MATHGL::Vector3::Normalize(MATHGL::Vector3(-1.0f, 0.0001f, 0.0001f)),
			 MATHGL::Vector3::Normalize(MATHGL::Vector3(0.0001f,    1.0f, 0.0001f)),
			 MATHGL::Vector3::Normalize(MATHGL::Vector3(0.0001f,   -1.0f, 0.0001f)),
			 MATHGL::Vector3::Normalize(MATHGL::Vector3(0.0001f, 0.0001f,    1.0f)),
			 MATHGL::Vector3::Normalize(MATHGL::Vector3(0.0001f, 0.0001f,   -1.0f)),
		};

		std::vector<MATHGL::Vector3> UpTable =
		{
			 MATHGL::Vector3::Vector3(0.0f, -1.0f,  0.0f),
			 MATHGL::Vector3::Vector3(0.0f, -1.0f,  0.0f),
			 MATHGL::Vector3::Vector3(0.0f,  0.0f,  1.0f),
			 MATHGL::Vector3::Vector3(0.0f,  0.0f, -1.0f),
			 MATHGL::Vector3::Vector3(0.0f, -1.0f,  0.0f),
			 MATHGL::Vector3::Vector3(0.0f, -1.0f,  0.0f),
		};
		
		MATHGL::Matrix4 PointLight::GetMatrix(size_t index, const MATHGL::Vector3& position) const {
			auto Projection = MATHGL::Matrix4::CreatePerspective(TO_RADIANS(90.0f), 1.0f, 0.1f, this->Radius);
			auto directionNorm = DirectionTable[index];
			auto View = MATHGL::Matrix4::CreateView(
				position,
				position + directionNorm,
				UpTable[index]
			);
			return Projection * View;
		}

		
		float Radius = 0.0f;
		MATHGL::Matrix4 shadowProjectionMat;
		std::array<MATHGL::Matrix4, 6> ProjectionMatrices;


		std::shared_ptr<RESOURCES::CubeMap> DepthMap;

		int depthMapTextureId = 0;

		float zFar = 0.0f;
	};
}