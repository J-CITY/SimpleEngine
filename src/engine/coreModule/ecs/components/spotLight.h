#pragma once
#include "lightComponent.h"
#include "mathModule/math.h"

namespace IKIGAI::ECS { class Object; }

namespace IKIGAI::ECS {
	class SpotLight : public LightComponent {
	public:
		struct Descriptor : public Component::Descriptor {
			std::string Type;
			MATH::Vector3f Color;
			float Intensity = 0.0f;
			float Constant = 0.0f;
			float Linear = 0.0f;
			float Quadratic = 0.0f;
			float Cutoff = 0.0f;
			float OuterCutoff = 0.0f;

			template<class Context>
			constexpr static auto serde(Context& context, Descriptor& value) {
				using Self = Descriptor;
				using namespace serde::attribute;
				serde::serde_struct(context, value)
					.field(&Self::Type, "SpotLightType")
					.field(&Self::Intensity, "Intensity")
					.field(&Self::Constant, "Constant")
					.field(&Self::Linear, "Linear")
					.field(&Self::Quadratic, "Quadratic")
					.field(&Self::Cutoff, "Cutoff")
					.field(&Self::OuterCutoff, "OuterCutoff");
			}
		};
		SpotLight(UTILS::Ref<ECS::Object> obj);
		SpotLight(UTILS::Ref<ECS::Object> obj, const Descriptor& descriptor);
		SpotLight(UTILS::Ref<ECS::Object> obj, const Component::Descriptor& descriptor) :
			SpotLight(obj, static_cast<const Descriptor&>(descriptor)) {
		};
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

		static constexpr float angle = MATH::TO_RADIANS(90.0f);
		float aspect = 1.0f;

		float maxDistance = 1000.0f;

		MATH::Vector3f Direction;
		MATH::Matrix4f projectionMat;
		MATH::Matrix4f BiasedProjectionMatrix;
		MATH::Matrix4f Transform;
	public:
		static auto GetMembers() {
			return std::tuple{
			};
		}
	};

	template <>
	inline std::string ECS::GetType<SpotLight>() {
		return "class IKIGAI::ECS::SpotLight";
	}

	template <>
	inline std::string IKIGAI::ECS::GetComponentName<SpotLight>() {
		return "SpotLight";
	}
}
