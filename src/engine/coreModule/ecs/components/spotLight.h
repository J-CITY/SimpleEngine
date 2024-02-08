#pragma once
#include "lightComponent.h"

namespace IKIGAI::ECS { class Object; }

namespace IKIGAI::ECS {
	class SpotLight : public LightComponent {
	public:
		SpotLight(UTILS::Ref<ECS::Object> obj);
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

		float distance = 0.0f;
		float orthoBoxSize = 3000.0f;

		static constexpr float angle = TO_RADIANS(90.0f);
		float aspect = 1.0f;

		float maxDistance = 1000.0f;

		MATHGL::Vector3 Direction;
		
		MATHGL::Matrix4 projectionMat;
		MATHGL::Matrix4 BiasedProjectionMatrix;
		MATHGL::Matrix4 Transform;
	};
}