#include "spotLight.h"
#include <string>

using namespace KUMA;
using namespace KUMA::ECS;

SpotLight::SpotLight(const ECS::Object& obj) : LightComponent(obj) {
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
