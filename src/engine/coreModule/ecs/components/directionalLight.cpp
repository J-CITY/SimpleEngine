#include "directionalLight.h"
#include "../object.h"

using namespace IKIGAI::ECS;

DirectionalLight::DirectionalLight(UTILS::Ref<ECS::Object> obj) : LightComponent(obj) {
	__NAME__ = "DirectionalLight";
	data.type = RENDER::Light::Type::DIRECTIONAL;
}

DirectionalLight::DirectionalLight(UTILS::Ref<ECS::Object> obj, const Descriptor& descriptor) : LightComponent(obj) {
	data.type = RENDER::Light::Type::DIRECTIONAL;
	data.color = descriptor.Color;
	data.intensity = descriptor.Intensity;
	distance = descriptor.Distance;
}

//#include <rttr/registration>
//
//RTTR_REGISTRATION
//{
//	rttr::registration::class_<IKIGAI::ECS::DirectionalLight>("DirectionalLight")
//	(
//		rttr::metadata(MetaInfo::FLAGS, MetaInfo::SERIALIZABLE)
//	)
//	.property("Color", &IKIGAI::ECS::DirectionalLight::getColor, &IKIGAI::ECS::DirectionalLight::setColor)
//	(
//		rttr::metadata(MetaInfo::FLAGS, MetaInfo::SERIALIZABLE| MetaInfo::USE_IN_EDITOR_COMPONENT_INSPECTOR),
//		rttr::metadata(EditorMetaInfo::EDIT_RANGE, Pair { 0.0f, 10000.0f }),
//		rttr::metadata(EditorMetaInfo::EDIT_STEP, 0.1f),
//		rttr::metadata(EditorMetaInfo::EDIT_WIDGET, EditorMetaInfo::WidgetType::DRAG_COLOR_3)
//	)
//	.property("Intensity", &IKIGAI::ECS::DirectionalLight::getIntensity, &IKIGAI::ECS::DirectionalLight::setIntensity)
//	(
//		rttr::metadata(MetaInfo::FLAGS, MetaInfo::SERIALIZABLE| MetaInfo::USE_IN_EDITOR_COMPONENT_INSPECTOR),
//		rttr::metadata(EditorMetaInfo::EDIT_RANGE, Pair { 0.0f, 1.0f }),
//		rttr::metadata(EditorMetaInfo::EDIT_STEP, 0.1f),
//		rttr::metadata(EditorMetaInfo::EDIT_WIDGET, EditorMetaInfo::WidgetType::DRAG_FLOAT)
//	)
//	.property("Distance", &IKIGAI::ECS::DirectionalLight::getDistance, &IKIGAI::ECS::DirectionalLight::setDistance)
//	(
//		rttr::metadata(MetaInfo::FLAGS, MetaInfo::SERIALIZABLE| MetaInfo::USE_IN_EDITOR_COMPONENT_INSPECTOR),
//		rttr::metadata(EditorMetaInfo::EDIT_RANGE, Pair { 0.0f, 1000.0f }),
//		rttr::metadata(EditorMetaInfo::EDIT_STEP, 0.1f),
//		rttr::metadata(EditorMetaInfo::EDIT_WIDGET, EditorMetaInfo::WidgetType::DRAG_FLOAT)
//	);
//}
