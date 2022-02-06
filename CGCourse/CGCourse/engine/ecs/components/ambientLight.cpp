#include "ambientLight.h"

using namespace KUMA::ECS;

AmbientLight::AmbientLight(const ECS::Object& obj): LightComponent(obj) {
	__NAME__ = "AmbientLight";
	
	data.type = RENDER::Light::Type::AMBIENT_BOX;
	data.color = { 0.1f, 0.1f, 0.1f };
	data.constant = 1.0f;
	data.linear = 1.0f;
	data.quadratic = 1.0f;
}

KUMA::MATHGL::Vector3 AmbientLight::getSize() const {
	return { data.constant, data.linear, data.quadratic };
}

void AmbientLight::setSize(const MATHGL::Vector3& val) {
	data.constant = val.x;
	data.linear = val.y;
	data.quadratic = val.z;
}
