#pragma once

#include "lightComponent.h"

namespace IKIGAI::ECS { class Object; }

namespace IKIGAI::ECS {
	class AmbientSphereLight : public LightComponent {
	public:
		struct Descriptor : public Component::Descriptor {
			std::string Type;
			MATH::Vector3f Color;
			float Radius = 0.0f;
			float Intensity;

			template<class Context>
			constexpr static auto serde(Context& context, Descriptor& value) {
				using Self = Descriptor;
				using namespace serde::attribute;
				serde::serde_struct(context, value)
					.field(&Self::Type, "AmbientSphereLightType")
					.field(&Self::Color, "Color", default_{MATH::Vector3f(1.0f, 1.0f, 1.0f)})
					.field(&Self::Radius, "Radius", default_{1.0f})
					.field(&Self::Intensity, "Intensity");
			}
		};
		AmbientSphereLight(UTILS::Ref<ECS::Object> _obj);
		AmbientSphereLight(UTILS::Ref<ECS::Object> _obj, const Descriptor& descriptor);
		AmbientSphereLight(UTILS::Ref<ECS::Object> obj, const Component::Descriptor& descriptor) :
			AmbientSphereLight(obj, static_cast<const Descriptor&>(descriptor)) {
		};

		float getRadius() const;
		void setRadius(float p_radius);

	public:
		static auto GetMembers() {
			return std::tuple{
			};
		}
		
	};

	template <>
	inline std::string IKIGAI::ECS::GetType<AmbientSphereLight>() {
		return "class IKIGAI::ECS::AmbientSphereLight";
	}
	template <>
	inline std::string IKIGAI::ECS::GetComponentName<AmbientSphereLight>() {
		return "AmbientSphereLight";
	}
}
