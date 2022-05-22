#include "pointLight.h"
#include "../engine/ecs/object.h"

using namespace KUMA;
using namespace KUMA::ECS;

PointLight::PointLight(const ECS::Object& obj) : LightComponent(obj) {
	__NAME__ = "PointLight";
	data.type = RENDER::Light::Type::POINT;
	//pointLightShadowFBO.setupFrameBuffer(shadowRes, shadowRes, true);

	//shadowProjectionMat = MATHGL::Matrix4::CreatePerspective(ang, aspect, zNear, zFar);

	DepthMap = std::make_shared<RESOURCES::CubeMap>();
	
	auto depthTextureSize = 1024;
	DepthMap->LoadDepth(depthTextureSize, depthTextureSize);
	
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
