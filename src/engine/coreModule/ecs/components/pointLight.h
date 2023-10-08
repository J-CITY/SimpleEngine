#pragma once
#include "lightComponent.h"

import glmath;

namespace IKIGAI::ECS { class Object; }

namespace IKIGAI::ECS{
	class PointLight : public LightComponent {
	public:
		PointLight(Ref<ECS::Object> obj);
		float getConstant() const;
		float getLinear() const;
		float getQuadratic() const;
		void setConstant(float constant);
		void setLinear(float linear);
		void setQuadratic(float quadratic);

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
		
		float Radius = 0.0f;
		MATHGL::Matrix4 shadowProjectionMat;
		std::array<MATHGL::Matrix4, 6> ProjectionMatrices;


		//std::shared_ptr<RESOURCES::CubeMap> DepthMap;

		int depthMapTextureId = 0;

		float zFar = 0.0f;
	};
}