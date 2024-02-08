#include "lightComponent.h"
#include "../object.h"

using namespace IKIGAI::ECS;

LightComponent::LightComponent(UTILS::Ref<ECS::Object> obj): Component(obj),
	data(obj->getID()) {
}

const IKIGAI::RENDER::Light& LightComponent::getData() const {
	return data;
}

const IKIGAI::MATHGL::Vector3& LightComponent::getColor() const {
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
