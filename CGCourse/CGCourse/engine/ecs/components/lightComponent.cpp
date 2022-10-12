#include "lightComponent.h"
#include "../object.h"

using namespace KUMA::ECS;

LightComponent::LightComponent(Ref<ECS::Object> obj): Component(obj),
	data(obj->getTransform()->getTransform()) {
}

const KUMA::RENDER::Light& LightComponent::getData() const {
	return data;
}

const KUMA::MATHGL::Vector3& LightComponent::getColor() const {
	return data.color;
}

float LightComponent::getIntensity() const {
	return data.intensity;
}

void LightComponent::setColor(const MATHGL::Vector3& color) {
	data.color = color;
}

void LightComponent::setIntensity(float intensity) {
	data.intensity = intensity;
}
