#pragma once
#include "lightComponent.h"

namespace IKIGAI::ECS { class Object; }

namespace IKIGAI::ECS{
	class PointLight : public LightComponent {
	public:
		struct Descriptor : public Component::Descriptor {
			std::string Type;
			MATH::Vector3f Color;
			float Intensity = 0.0f;
			float Constant = 0.0f;
			float Linear = 0.0f;
			float Quadratic = 0.0f;

			template<class Context>
			constexpr static auto serde(Context& context, Descriptor& value) {
				using Self = Descriptor;
				using namespace serde::attribute;
				serde::serde_struct(context, value)
					.field(&Self::Type, "PointLightType")
					.field(&Self::Color, "Color", default_{MATH::Vector3f(1.0f, 1.0f, 1.0f)})
					.field(&Self::Intensity, "Intensity")
					.field(&Self::Constant, "Constant")
					.field(&Self::Linear, "Linear")
					.field(&Self::Quadratic, "Quadratic");
			}
		};
		PointLight(UTILS::Ref<ECS::Object> _obj);
		PointLight(UTILS::Ref<ECS::Object> _obj, const Descriptor& _descriptor);
		PointLight(UTILS::Ref<ECS::Object> obj, const Component::Descriptor& descriptor) :
			PointLight(obj, static_cast<const Descriptor&>(descriptor)) {
		};
		float getConstant() const;
		float getLinear() const;
		float getQuadratic() const;
		void setConstant(float constant);
		void setLinear(float linear);
		void setQuadratic(float quadratic);

		std::vector<MATH::Vector3f> DirectionTable =
		{
			 MATH::Vector3f::Normalize(MATH::Vector3f(1.0f, 0.0001f, 0.0001f)),
			 MATH::Vector3f::Normalize(MATH::Vector3f(-1.0f, 0.0001f, 0.0001f)),
			 MATH::Vector3f::Normalize(MATH::Vector3f(0.0001f,    1.0f, 0.0001f)),
			 MATH::Vector3f::Normalize(MATH::Vector3f(0.0001f,   -1.0f, 0.0001f)),
			 MATH::Vector3f::Normalize(MATH::Vector3f(0.0001f, 0.0001f,    1.0f)),
			 MATH::Vector3f::Normalize(MATH::Vector3f(0.0001f, 0.0001f,   -1.0f)),
		};

		std::vector<MATH::Vector3f> UpTable =
		{
			 MATH::Vector3f(0.0f, -1.0f,  0.0f),
			 MATH::Vector3f(0.0f, -1.0f,  0.0f),
			 MATH::Vector3f(0.0f,  0.0f,  1.0f),
			 MATH::Vector3f(0.0f,  0.0f, -1.0f),
			 MATH::Vector3f(0.0f, -1.0f,  0.0f),
			 MATH::Vector3f(0.0f, -1.0f,  0.0f),
		};
		
		float Radius = 0.0f;
		MATH::Matrix4f shadowProjectionMat;
		std::array<MATH::Matrix4f, 6> ProjectionMatrices;

		[[nodiscard]] Descriptor getDescriptor() const;

		//std::shared_ptr<RESOURCES::CubeMap> DepthMap;

		int depthMapTextureId = 0;

		float zFar = 0.0f;

	public:
		static auto GetMembers() {
			return std::tuple{
				IKIGAI::UTILS::MakeMemberInfoLambda<PointLight, const MATH::Vector3f&>("Color", 
				[](PointLight& obj) { return obj.getColor(); }, 
				[](PointLight& obj, const MATH::Vector3f& data) { obj.setColor(data); },
				UTILS::Meta_t{
					{UTILS::MetaParam::FLAGS, UTILS::MetaInfo::USE_IN_EDITOR_COMPONENT_INSPECTOR},
				{UTILS::MetaParam::EDIT_WIDGET, UTILS::WidgetType::DRAG_COLOR_3},
				}),
				IKIGAI::UTILS::MakeMemberInfoLambda<PointLight, float>("Intensity",
				[](PointLight& obj){ return obj.getIntensity(); }, 
				[](PointLight& obj, float data) { return obj.setIntensity(data); },
				UTILS::Meta_t{
					{UTILS::MetaParam::FLAGS, UTILS::MetaInfo::USE_IN_EDITOR_COMPONENT_INSPECTOR},
				{UTILS::MetaParam::EDIT_RANGE, MATH::Vector2f{0.0f, 1.0f}},
				{UTILS::MetaParam::EDIT_STEP, 0.1f},
				{UTILS::MetaParam::EDIT_WIDGET, UTILS::WidgetType::DRAG_FLOAT},
				}),
				IKIGAI::UTILS::MakeMemberInfo("Linear", &PointLight::getLinear, &PointLight::setLinear,
				UTILS::Meta_t{
					{UTILS::MetaParam::FLAGS, UTILS::MetaInfo::USE_IN_EDITOR_COMPONENT_INSPECTOR},
				{UTILS::MetaParam::EDIT_RANGE, MATH::Vector2f{0.0f, 1.0f}},
				{UTILS::MetaParam::EDIT_STEP, 0.1f},
				{UTILS::MetaParam::EDIT_WIDGET, UTILS::WidgetType::DRAG_FLOAT},
				}),
				IKIGAI::UTILS::MakeMemberInfo("Quadratic", &PointLight::getQuadratic, &PointLight::setQuadratic,
				UTILS::Meta_t{
					{UTILS::MetaParam::FLAGS, UTILS::MetaInfo::USE_IN_EDITOR_COMPONENT_INSPECTOR},
				{UTILS::MetaParam::EDIT_RANGE, MATH::Vector2f{0.0f, 1.0f}},
				{UTILS::MetaParam::EDIT_STEP, 0.1f},
				{UTILS::MetaParam::EDIT_WIDGET, UTILS::WidgetType::DRAG_FLOAT},
				}),
				IKIGAI::UTILS::MakeMemberInfo("Constant", &PointLight::getConstant, &PointLight::setConstant,
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
	inline std::string ECS::GetType<PointLight>() {
		return "class IKIGAI::ECS::PointLight";
	}

	template <>
	inline std::string IKIGAI::ECS::GetComponentName<PointLight>() {
		return "PointLight";
	}
}
