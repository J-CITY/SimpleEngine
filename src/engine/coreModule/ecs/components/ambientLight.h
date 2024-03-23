#pragma once

#include "directionalLight.h"

namespace IKIGAI::ECS { class Object; }

namespace IKIGAI::ECS {
	class AmbientLight: public LightComponent {
	public:
		struct Descriptor: public Component::Descriptor {
			std::string Type;
			MATH::Vector3f Color;
			MATH::Vector3f Size;
			float Intensity = 0.0f;

			template<class Context>
			constexpr static auto serde(Context& context, Descriptor& value) {
				using Self = Descriptor;
				using namespace serde::attribute;
				serde::serde_struct(context, value)
					.field(&Self::Type, "AmbientLightType")
					.field(&Self::Color, "Color", default_{MATH::Vector3f(1.0f, 1.0f, 1.0f)})
					.field(&Self::Size, "Size", default_{MATH::Vector3f(1.0f, 1.0f, 1.0f)})
					.field(&Self::Intensity, "Intensity");
			}
		};

		AmbientLight(UTILS::Ref<ECS::Object> _obj);
		AmbientLight(UTILS::Ref<ECS::Object> _obj, const Descriptor& descriptor);
		AmbientLight(UTILS::Ref<ECS::Object> obj, const Component::Descriptor& descriptor) :
			AmbientLight(obj, static_cast<const Descriptor&>(descriptor)) {
		};
		MATH::Vector3f getSize() const;
		void setSize(const MATH::Vector3f& val);

	public:
		static auto GetMembers() {
			return std::tuple{
			};
		}
		
	};
	template <>
	inline std::string IKIGAI::ECS::GetType<AmbientLight>() {
		return "class IKIGAI::ECS::AmbientLight";
	}

	template <>
	inline std::string IKIGAI::ECS::GetComponentName<AmbientLight>() {
		return "AmbientLight";
	}
}
