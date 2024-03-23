#include "lightComponent.h"
#include "../object.h"

using namespace IKIGAI::ECS;

LightComponent::LightComponent(UTILS::Ref<ECS::Object> obj): Component(obj),
	data(obj->getID()) {
}

const IKIGAI::RENDER::Light& LightComponent::getData() const {
	return data;
}

const IKIGAI::MATH::Vector3f& LightComponent::getColor() const {
	return data.color;
}

float LightComponent::getIntensity() const {
	return data.intensity;
}

void LightComponent::setColor(const MATH::Vector3f& color) {
	data.color = color;
}

void LightComponent::setIntensity(float intensity) {
	data.intensity = intensity;
}
