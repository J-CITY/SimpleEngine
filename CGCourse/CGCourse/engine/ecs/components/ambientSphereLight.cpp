#include "ambientSphereLight.h"

using namespace KUMA::ECS;

AmbientSphereLight::AmbientSphereLight(Ref<ECS::Object> obj): LightComponent(obj) {
	__NAME__ = "AmbientSphereLight";
	data.type = RENDER::Light::Type::AMBIENT_SPHERE;
	data.color = { 0.1f, 0.1f, 0.1f };
	data.constant = 1.0f;
}

float AmbientSphereLight::getRadius() const {
	return data.quadratic;
}

void AmbientSphereLight::setRadius(float r) {
	data.constant = r;
}

#include <rttr/registration>

RTTR_REGISTRATION
{
	rttr::registration::class_<KUMA::ECS::AmbientSphereLight>("AmbientSphereLight")
	(
		rttr::metadata(MetaInfo::FLAGS, MetaInfo::SERIALIZABLE | MetaInfo::USE_IN_EDITOR)
	)
	.property("Color", &KUMA::ECS::AmbientSphereLight::getColor, &KUMA::ECS::AmbientSphereLight::setColor)
	(
		rttr::metadata(MetaInfo::FLAGS, MetaInfo::SERIALIZABLE | MetaInfo::USE_IN_EDITOR | MetaInfo::USE_IN_COMPONENT_INSPECTOR),
		rttr::metadata(EditorMetaInfo::EDIT_RANGE, Pair { 0.0f, 10000.0f }),
		rttr::metadata(EditorMetaInfo::EDIT_STEP, 0.1f),
		rttr::metadata(EditorMetaInfo::EDIT_WIDGET, EditorMetaInfo::WidgetType::DRAG_COLOR_3)
	)
	.property("Intensity", &KUMA::ECS::AmbientSphereLight::getIntensity, &KUMA::ECS::AmbientSphereLight::setIntensity)
	(
		rttr::metadata(MetaInfo::FLAGS, MetaInfo::SERIALIZABLE | MetaInfo::USE_IN_EDITOR | MetaInfo::USE_IN_COMPONENT_INSPECTOR),
		rttr::metadata(EditorMetaInfo::EDIT_RANGE, Pair { 0.0f, 1.0f }),
		rttr::metadata(EditorMetaInfo::EDIT_STEP, 0.1f),
		rttr::metadata(EditorMetaInfo::EDIT_WIDGET, EditorMetaInfo::WidgetType::DRAG_FLOAT)
	)
	.property("Radius", &KUMA::ECS::AmbientSphereLight::getRadius, &KUMA::ECS::AmbientSphereLight::setRadius)
	(
		rttr::metadata(MetaInfo::FLAGS, MetaInfo::SERIALIZABLE | MetaInfo::USE_IN_EDITOR | MetaInfo::USE_IN_COMPONENT_INSPECTOR),
		rttr::metadata(EditorMetaInfo::EDIT_RANGE, Pair { 0.0f, 1.0f }),
		rttr::metadata(EditorMetaInfo::EDIT_STEP, 0.1f),
		rttr::metadata(EditorMetaInfo::EDIT_WIDGET, EditorMetaInfo::WidgetType::DRAG_FLOAT)
	);
}
