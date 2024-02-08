#include "ambientLight.h"

using namespace IKIGAI::ECS;

AmbientLight::AmbientLight(UTILS::Ref<ECS::Object> obj): LightComponent(obj) {
	__NAME__ = "AmbientLight";
	
	data.type = RENDER::Light::Type::AMBIENT_BOX;
	data.color = { 0.1f, 0.1f, 0.1f };
	data.constant = 1.0f;
	data.linear = 1.0f;
	data.quadratic = 1.0f;
}

IKIGAI::MATHGL::Vector3 AmbientLight::getSize() const {
	return { data.constant, data.linear, data.quadratic };
}

void AmbientLight::setSize(const MATHGL::Vector3& val) {
	data.constant = val.x;
	data.linear = val.y;
	data.quadratic = val.z;
}

#include <rttr/registration>

RTTR_REGISTRATION
{
	rttr::registration::class_<IKIGAI::ECS::AmbientLight>("AmbientLight")
	(
		rttr::metadata(MetaInfo::FLAGS, MetaInfo::SERIALIZABLE)
	)
	.property("Color", &IKIGAI::ECS::AmbientLight::getColor, &IKIGAI::ECS::AmbientLight::setColor)
	(
		rttr::metadata(MetaInfo::FLAGS, MetaInfo::SERIALIZABLE | MetaInfo::USE_IN_EDITOR_COMPONENT_INSPECTOR),
		rttr::metadata(EditorMetaInfo::EDIT_RANGE, Pair { 0.0f, 10000.0f }),
		rttr::metadata(EditorMetaInfo::EDIT_STEP, 0.1f),
		rttr::metadata(EditorMetaInfo::EDIT_WIDGET, EditorMetaInfo::WidgetType::DRAG_COLOR_3)
	)
	.property("Intensity", &IKIGAI::ECS::AmbientLight::getIntensity, &IKIGAI::ECS::AmbientLight::setIntensity)
	(
		rttr::metadata(MetaInfo::FLAGS, MetaInfo::SERIALIZABLE | MetaInfo::USE_IN_EDITOR_COMPONENT_INSPECTOR),
		rttr::metadata(EditorMetaInfo::EDIT_RANGE, Pair { 0.0f, 1.0f }),
		rttr::metadata(EditorMetaInfo::EDIT_STEP, 0.1f),
		rttr::metadata(EditorMetaInfo::EDIT_WIDGET, EditorMetaInfo::WidgetType::DRAG_FLOAT)
	);
}
