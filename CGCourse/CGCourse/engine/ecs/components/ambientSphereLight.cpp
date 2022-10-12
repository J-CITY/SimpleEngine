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
