#pragma once
#include <string>

#include "lightComponent.h"
//#include "../../../DiffuseLightRT.h"

namespace IKIGAI::ECS { class Object; }

namespace IKIGAI::ECS {
	class DirectionalLight : public LightComponent {
	public:

		constexpr static size_t TextureCount = 3;
		DirectionalLight(UTILS::Ref<ECS::Object> obj);

		void setDistance(float d) { distance = d; }
		float getDistance() { return distance; }

		std::array<MATHGL::Matrix4, 3> ProjectionMatrices;
		std::array<MATHGL::Matrix4, 3> BiasedProjectionMatrices;

		
		float distance = 0.0f;
		float orthoBoxSize = 100.0f;

		float zNear = 0.0f;
		float zFar = 0.0f;
		MATHGL::Vector3 Direction = MATHGL::Vector3(0.5f, 1.0f, 1.0f);
		std::array<float, TextureCount> Projections = {15.0f, 150.0f, 1500.0f};
		
		MATHGL::Matrix4 shadowProjectionMat;
		MATHGL::Matrix4 lightView;
		MATHGL::Matrix4 lightSpaceMatrix;
		size_t depthMapTextureID = 0;
	};
}