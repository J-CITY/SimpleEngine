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
		[[nodiscard]] Descriptor getDescriptor() const;
	public:
		static auto GetMembers() {
			return std::tuple{
				IKIGAI::UTILS::MakeMemberInfoLambda<AmbientLight, const MATH::Vector3f&>("Color",
				[](AmbientLight& obj) { return obj.getColor(); },
				[](AmbientLight& obj, const MATH::Vector3f& data) { obj.setColor(data); },
				UTILS::Meta_t{
					{UTILS::MetaParam::FLAGS, UTILS::MetaInfo::USE_IN_EDITOR_COMPONENT_INSPECTOR},
				{UTILS::MetaParam::EDIT_WIDGET, UTILS::WidgetType::DRAG_COLOR_3},
				}),
				IKIGAI::UTILS::MakeMemberInfoLambda<AmbientLight, float>("Intensity",
				[](AmbientLight& obj) { return obj.getIntensity(); },
				[](AmbientLight& obj, float data) { return obj.setIntensity(data); },
				UTILS::Meta_t{
					{UTILS::MetaParam::FLAGS, UTILS::MetaInfo::USE_IN_EDITOR_COMPONENT_INSPECTOR},
				{UTILS::MetaParam::EDIT_RANGE, MATH::Vector2f{0.0f, 1.0f}},
				{UTILS::MetaParam::EDIT_STEP, 0.1f},
				{UTILS::MetaParam::EDIT_WIDGET, UTILS::WidgetType::DRAG_FLOAT},
				})
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
