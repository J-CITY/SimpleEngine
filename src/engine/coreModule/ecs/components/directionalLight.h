#pragma once
#include <string>

#include "lightComponent.h"
//#include "../../../DiffuseLightRT.h"

namespace IKIGAI::ECS { class Object; }

namespace IKIGAI::ECS {
	class DirectionalLight : public LightComponent {
	public:
		struct Descriptor : public Component::Descriptor {
			std::string Type;
			MATH::Vector3f Color;
			float Distance;
			float Intensity;

			template<class Context>
			constexpr static auto serde(Context& context, Descriptor& value) {
				using Self = Descriptor;
				using namespace serde::attribute;
				serde::serde_struct(context, value)
					.field(&Self::Type, "DirectionalLightType")
					.field(&Self::Color, "Color", default_{MATH::Vector3f(1.0f, 1.0f, 1.0f)})
					.field(&Self::Distance, "Distance")
					.field(&Self::Intensity, "Intensity");
			}
		};
		constexpr static size_t TextureCount = 3;
		DirectionalLight(UTILS::Ref<ECS::Object> obj);
		DirectionalLight(UTILS::Ref<ECS::Object> obj, const Descriptor& descriptor);
		DirectionalLight(UTILS::Ref<ECS::Object> obj, const Component::Descriptor& descriptor) :
			DirectionalLight(obj, static_cast<const Descriptor&>(descriptor)) {
		};

		void setDistance(float d) { distance = d; }
		float getDistance() const { return distance; }

		//TODO: Think about it
		float distance = 0.0f;
		float orthoBoxSize = 100.0f;

		//TODO: delete it
		std::array<MATH::Matrix4f, 3> ProjectionMatrices;
		std::array<MATH::Matrix4f, 3> BiasedProjectionMatrices;
		float zNear = 0.0f;
		float zFar = 0.0f;
		MATH::Vector3f Direction = MATH::Vector3f(0.5f, 1.0f, 1.0f);
		std::array<float, TextureCount> Projections = {15.0f, 150.0f, 1500.0f};
		MATH::Matrix4f shadowProjectionMat;
		MATH::Matrix4f lightView;
		MATH::Matrix4f lightSpaceMatrix;
		size_t depthMapTextureID = 0;
		[[nodiscard]] Descriptor getDescriptor() const;
	public:
		static auto GetMembers() {
			return std::tuple{
				IKIGAI::UTILS::MakeMemberInfoLambda<DirectionalLight, const MATH::Vector3f&>("Color",
				[](DirectionalLight& obj) { return obj.getColor(); },
				[](DirectionalLight& obj, const MATH::Vector3f& data) { obj.setColor(data); },
				UTILS::Meta_t{
					{UTILS::MetaParam::FLAGS, UTILS::MetaInfo::USE_IN_EDITOR_COMPONENT_INSPECTOR},
				{UTILS::MetaParam::EDIT_WIDGET, UTILS::WidgetType::DRAG_COLOR_3},
				}),
				IKIGAI::UTILS::MakeMemberInfoLambda<DirectionalLight, float>("Intensity",
				[](DirectionalLight& obj) { return obj.getIntensity(); },
				[](DirectionalLight& obj, float data) { return obj.setIntensity(data); },
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
	inline std::string IKIGAI::ECS::GetType<DirectionalLight>() {
		return "class IKIGAI::ECS::DirectionalLight";
	}

	template <>
	inline std::string IKIGAI::ECS::GetComponentName<DirectionalLight>() {
		return "DirectionalLight";
	}
}
