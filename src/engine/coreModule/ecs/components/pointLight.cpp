#include "pointLight.h"
#include "../object.h"

using namespace IKIGAI;
using namespace IKIGAI::ECS;

PointLight::PointLight(UTILS::Ref<ECS::Object> obj) : LightComponent(obj) {
	__NAME__ = "PointLight";
	data.type = RENDER::Light::Type::POINT;
	//pointLightShadowFBO.setupFrameBuffer(shadowRes, shadowRes, true);

	//shadowProjectionMat = MATHGL::Matrix4::CreatePerspective(ang, aspect, zNear, zFar);

	//DepthMap = std::make_shared<RESOURCES::CubeMap>();
	
	//auto depthTextureSize = 1024;
	//DepthMap->LoadDepth(depthTextureSize, depthTextureSize);
	
	//lookAtPerFace.resize(6);
	//lookAtPerFace[0] = MATHGL::Matrix4::CreateView(lightPos, lightPos + MATHGL::Vector3(1.0, 0.0, 0.0), MATHGL::Vector3(0.0, -1.0, 0.0));
	//lookAtPerFace[1] = MATHGL::Matrix4::CreateView(lightPos, lightPos + MATHGL::Vector3(-1.0, 0.0, 0.0), MATHGL::Vector3(0.0, -1.0, 0.0));
	//lookAtPerFace[2] = MATHGL::Matrix4::CreateView(lightPos, lightPos + MATHGL::Vector3(0.0, 1.0, 0.0), MATHGL::Vector3(0.0, 0.0, 1.0));
	//lookAtPerFace[3] = MATHGL::Matrix4::CreateView(lightPos, lightPos + MATHGL::Vector3(0.0, -1.0, 0.0), MATHGL::Vector3(0.0, 0.0, -1.0));
	//lookAtPerFace[4] = MATHGL::Matrix4::CreateView(lightPos, lightPos + MATHGL::Vector3(0.0, 0.0, 1.0), MATHGL::Vector3(0.0, -1.0, 0.0));
	//lookAtPerFace[5] = MATHGL::Matrix4::CreateView(lightPos, lightPos + MATHGL::Vector3(0.0, 0.0, -1.0), MATHGL::Vector3(0.0, -1.0, 0.0));
}

float PointLight::getConstant() const {
	return data.constant;
}

float PointLight::getLinear() const {
	return data.linear;
}

float PointLight::getQuadratic() const {
	return data.quadratic;
}

void PointLight::setConstant(float constant) {
	data.constant = constant;
}

void PointLight::setLinear(float linear) {
	data.linear = linear;
}

void PointLight::setQuadratic(float quadratic) {
	data.quadratic = quadratic;
}

#include <rttr/registration>

RTTR_REGISTRATION
{
	rttr::registration::class_<IKIGAI::ECS::PointLight>("PointLight")
	(
		rttr::metadata(MetaInfo::FLAGS, MetaInfo::SERIALIZABLE)
	)
	.property("Color", &IKIGAI::ECS::PointLight::getColor, &IKIGAI::ECS::PointLight::setColor)
	(
		rttr::metadata(MetaInfo::FLAGS, MetaInfo::SERIALIZABLE| MetaInfo::USE_IN_EDITOR_COMPONENT_INSPECTOR),
		rttr::metadata(EditorMetaInfo::EDIT_RANGE, Pair { 0.0f, 10000.0f }),
		rttr::metadata(EditorMetaInfo::EDIT_STEP, 0.1f),
		rttr::metadata(EditorMetaInfo::EDIT_WIDGET, EditorMetaInfo::WidgetType::DRAG_COLOR_3)
	)
	.property("Intensity", &IKIGAI::ECS::PointLight::getIntensity, &IKIGAI::ECS::PointLight::setIntensity)
	(
		rttr::metadata(MetaInfo::FLAGS, MetaInfo::SERIALIZABLE| MetaInfo::USE_IN_EDITOR_COMPONENT_INSPECTOR),
		rttr::metadata(EditorMetaInfo::EDIT_RANGE, Pair { 0.0f, 1.0f }),
		rttr::metadata(EditorMetaInfo::EDIT_STEP, 0.1f),
		rttr::metadata(EditorMetaInfo::EDIT_WIDGET, EditorMetaInfo::WidgetType::DRAG_FLOAT)
	)
	.property("Linear", &IKIGAI::ECS::PointLight::getLinear, &IKIGAI::ECS::PointLight::setLinear)
	(
		rttr::metadata(MetaInfo::FLAGS, MetaInfo::SERIALIZABLE| MetaInfo::USE_IN_EDITOR_COMPONENT_INSPECTOR),
		rttr::metadata(EditorMetaInfo::EDIT_RANGE, Pair { 0.0f, 1.0f }),
		rttr::metadata(EditorMetaInfo::EDIT_STEP, 0.1f),
		rttr::metadata(EditorMetaInfo::EDIT_WIDGET, EditorMetaInfo::WidgetType::DRAG_FLOAT)
	)
	.property("Quadratic", &IKIGAI::ECS::PointLight::getQuadratic, &IKIGAI::ECS::PointLight::setQuadratic)
	(
		rttr::metadata(MetaInfo::FLAGS, MetaInfo::SERIALIZABLE| MetaInfo::USE_IN_EDITOR_COMPONENT_INSPECTOR),
		rttr::metadata(EditorMetaInfo::EDIT_RANGE, Pair { 0.0f, 1.0f }),
		rttr::metadata(EditorMetaInfo::EDIT_STEP, 0.1f),
		rttr::metadata(EditorMetaInfo::EDIT_WIDGET, EditorMetaInfo::WidgetType::DRAG_FLOAT)
	)
	.property("Constant", &IKIGAI::ECS::PointLight::getConstant, &IKIGAI::ECS::PointLight::setConstant)
	(
		rttr::metadata(MetaInfo::FLAGS, MetaInfo::SERIALIZABLE| MetaInfo::USE_IN_EDITOR_COMPONENT_INSPECTOR),
		rttr::metadata(EditorMetaInfo::EDIT_RANGE, Pair { 0.0f, 1.0f }),
		rttr::metadata(EditorMetaInfo::EDIT_STEP, 0.1f),
		rttr::metadata(EditorMetaInfo::EDIT_WIDGET, EditorMetaInfo::WidgetType::DRAG_FLOAT)
	);
}
