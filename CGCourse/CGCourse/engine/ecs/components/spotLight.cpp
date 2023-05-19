#include "spotLight.h"
#include <string>

using namespace KUMA;
using namespace KUMA::ECS;

SpotLight::SpotLight(Ref<ECS::Object> obj) : LightComponent(obj) {
	__NAME__ = "SpotLight";
	data.type = RENDER::Light::Type::SPOT;
	//bool initFBOFLag3 = dirShadowFBO.setupFrameBuffer(shadowRes, shadowRes, false);
}

float SpotLight::getConstant() const{
	return data.constant;
}

float SpotLight::getLinear() const {
	return data.linear;
}

float SpotLight::getQuadratic() const {
	return data.quadratic;
}

float SpotLight::getCutoff() const {
	return data.cutoff;
}

float SpotLight::getOuterCutoff() const {
	return data.outerCutoff;
}

void SpotLight::setConstant(float constant) {
	data.constant = constant;
}

void SpotLight::setLinear(float linear) {
	data.linear = linear;
}

void SpotLight::setQuadratic(float quadratic) {
	data.quadratic = quadratic;
}

void SpotLight::setCutoff(float cutoff) {
	data.cutoff = cutoff;
}

void SpotLight::setOuterCutoff(float outerCutoff) {
	data.outerCutoff = outerCutoff;
}


#include <rttr/registration>

RTTR_REGISTRATION
{
	rttr::registration::class_<KUMA::ECS::SpotLight>("SpotLight")
	(
		rttr::metadata(MetaInfo::FLAGS, MetaInfo::SERIALIZABLE | MetaInfo::USE_IN_EDITOR)
	)
	.property("Color", &KUMA::ECS::SpotLight::getColor, &KUMA::ECS::SpotLight::setColor)
	(
		rttr::metadata(MetaInfo::FLAGS, MetaInfo::SERIALIZABLE | MetaInfo::USE_IN_EDITOR | MetaInfo::USE_IN_COMPONENT_INSPECTOR),
		rttr::metadata(EditorMetaInfo::EDIT_RANGE, Pair { 0.0f, 10000.0f }),
		rttr::metadata(EditorMetaInfo::EDIT_STEP, 0.1f),
		rttr::metadata(EditorMetaInfo::EDIT_WIDGET, EditorMetaInfo::WidgetType::DRAG_COLOR_3)
	)
	.property("Intensity", &KUMA::ECS::SpotLight::getIntensity, &KUMA::ECS::SpotLight::setIntensity)
	(
		rttr::metadata(MetaInfo::FLAGS, MetaInfo::SERIALIZABLE | MetaInfo::USE_IN_EDITOR | MetaInfo::USE_IN_COMPONENT_INSPECTOR),
		rttr::metadata(EditorMetaInfo::EDIT_RANGE, Pair { 0.0f, 1.0f }),
		rttr::metadata(EditorMetaInfo::EDIT_STEP, 0.1f),
		rttr::metadata(EditorMetaInfo::EDIT_WIDGET, EditorMetaInfo::WidgetType::DRAG_FLOAT)
	)
	.property("Linear", &KUMA::ECS::SpotLight::getLinear, &KUMA::ECS::SpotLight::setLinear)
	(
		rttr::metadata(MetaInfo::FLAGS, MetaInfo::SERIALIZABLE | MetaInfo::USE_IN_EDITOR | MetaInfo::USE_IN_COMPONENT_INSPECTOR),
		rttr::metadata(EditorMetaInfo::EDIT_RANGE, Pair { 0.0f, 1.0f }),
		rttr::metadata(EditorMetaInfo::EDIT_STEP, 0.1f),
		rttr::metadata(EditorMetaInfo::EDIT_WIDGET, EditorMetaInfo::WidgetType::DRAG_FLOAT)
	)
	.property("Quadratic", &KUMA::ECS::SpotLight::getQuadratic, &KUMA::ECS::SpotLight::setQuadratic)
	(
		rttr::metadata(MetaInfo::FLAGS, MetaInfo::SERIALIZABLE | MetaInfo::USE_IN_EDITOR | MetaInfo::USE_IN_COMPONENT_INSPECTOR),
		rttr::metadata(EditorMetaInfo::EDIT_RANGE, Pair { 0.0f, 1.0f }),
		rttr::metadata(EditorMetaInfo::EDIT_STEP, 0.1f),
		rttr::metadata(EditorMetaInfo::EDIT_WIDGET, EditorMetaInfo::WidgetType::DRAG_FLOAT)
	)
	.property("Constant", &KUMA::ECS::SpotLight::getConstant, &KUMA::ECS::SpotLight::setConstant)
	(
		rttr::metadata(MetaInfo::FLAGS, MetaInfo::SERIALIZABLE | MetaInfo::USE_IN_EDITOR | MetaInfo::USE_IN_COMPONENT_INSPECTOR),
		rttr::metadata(EditorMetaInfo::EDIT_RANGE, Pair { 0.0f, 1.0f }),
		rttr::metadata(EditorMetaInfo::EDIT_STEP, 0.1f),
		rttr::metadata(EditorMetaInfo::EDIT_WIDGET, EditorMetaInfo::WidgetType::DRAG_FLOAT)
	)
	.property("Cutoff", &KUMA::ECS::SpotLight::getCutoff, &KUMA::ECS::SpotLight::setCutoff)
	(
		rttr::metadata(MetaInfo::FLAGS, MetaInfo::SERIALIZABLE | MetaInfo::USE_IN_EDITOR | MetaInfo::USE_IN_COMPONENT_INSPECTOR),
		rttr::metadata(EditorMetaInfo::EDIT_RANGE, Pair { 0.0f, 1.0f }),
		rttr::metadata(EditorMetaInfo::EDIT_STEP, 0.1f),
		rttr::metadata(EditorMetaInfo::EDIT_WIDGET, EditorMetaInfo::WidgetType::DRAG_FLOAT)
	)
	.property("OuterCutoff", &KUMA::ECS::SpotLight::getOuterCutoff, &KUMA::ECS::SpotLight::setOuterCutoff)
	(
		rttr::metadata(MetaInfo::FLAGS, MetaInfo::SERIALIZABLE | MetaInfo::USE_IN_EDITOR | MetaInfo::USE_IN_COMPONENT_INSPECTOR),
		rttr::metadata(EditorMetaInfo::EDIT_RANGE, Pair { 0.0f, 1.0f }),
		rttr::metadata(EditorMetaInfo::EDIT_STEP, 0.1f),
		rttr::metadata(EditorMetaInfo::EDIT_WIDGET, EditorMetaInfo::WidgetType::DRAG_FLOAT)
	);
}
