#include "light.h"

using namespace KUMA;
using namespace KUMA::RENDER;

Light::Light(ECS::Transform& tranform, Type type) : transform(tranform), type(type) {}

uint32_t pack(uint8_t c0, uint8_t c1, uint8_t c2, uint8_t c3) {
	return (c0 << 24) | (c1 << 16) | (c2 << 8) | c3;
}

uint32_t Pack(const KUMA::MATHGL::Vector3& vec) {
	return pack(static_cast<uint8_t>(vec.x * 255.f), static_cast<uint8_t>(vec.y * 255.f), static_cast<uint8_t>(vec.z * 255.f), 0);
}


LightOGL Light::generateOGLStruct() const {
	LightOGL result;

	auto position = transform.getWorldPosition();
	result.pos[0] = position.x;
	result.pos[1] = position.y;
	result.pos[2] = position.z;

	auto forward = transform.getWorldForward();
	result.forward[0] = forward.x;
	result.forward[1] = forward.y;
	result.forward[2] = forward.z;

	result.color[0] = color.x;
	result.color[1] = color.y;
	result.color[2] = color.z;

	result.type = static_cast<int>(type) - 1;
	result.cutoff = cutoff;
	result.outerCutoff = outerCutoff;

	result.constant = constant;
	result.linear = linear;
	result.quadratic = quadratic;
	result.intensity = intensity;

	/*
	auto position = transform.getWorldPosition();
	result.data[0] = position.x;
	result.data[1] = position.y;
	result.data[2] = position.z;

	auto forward = transform.getWorldForward();
	result.data[4] = forward.x;
	result.data[5] = forward.y;
	result.data[6] = forward.z;

	result.data[8] = static_cast<float>(Pack(color));

	result.data[12] = static_cast<float>(type);
	result.data[13] = cutoff;
	result.data[14] = outerCutoff;

	result.data[3] = constant;
	result.data[7] = linear;
	result.data[11] = quadratic;
	result.data[15] = intensity;
	 */

	return result;
}

float calculateLuminosity(float constant, float linear, float quadratic, float intensity, float distance) {
	auto attenuation = (constant + linear * distance + quadratic * (distance * distance));
	return (1.0f / attenuation) * std::abs(intensity);
}

float calculatePointLightRadius(float constant, float linear, float quadratic, float intensity) {
	constexpr float threshold = 1 / 255.0f;
	constexpr float step = 1.0f;

	float distance = 0.0f;

	if (calculateLuminosity(constant, linear, quadratic, intensity, 1000.0f) > threshold) {
		return std::numeric_limits<float>::infinity();
	}
	else if (calculateLuminosity(constant, linear, quadratic, intensity, 20.0f) < threshold) {
		distance = 0.0f;
	}
	else if (calculateLuminosity(constant, linear, quadratic, intensity, 750.0f) > threshold) {
		distance = 750.0f;
	}
	else if (calculateLuminosity(constant, linear, quadratic, intensity, 50.0f) < threshold) {
		distance = 20.0f + step;
	}
	else if (calculateLuminosity(constant, linear, quadratic, intensity, 100.0f) < threshold) {
		distance = 50.0f + step;
	}
	else if (calculateLuminosity(constant, linear, quadratic, intensity, 500.0f) > threshold) {
		distance = 500.0f;
	}
	else if (calculateLuminosity(constant, linear, quadratic, intensity, 250.0f) > threshold) {
		distance = 250.0f;
	}
	while (true) {
		if (calculateLuminosity(constant, linear, quadratic, intensity, distance) < threshold) {
			return distance;
		}
		else {
			distance += step;
		}
	}
}

float calculateAmbientBoxLightRadius(const KUMA::MATHGL::Vector3& pos, const KUMA::MATHGL::Vector3& size) {
	return KUMA::MATHGL::Vector3::Distance(pos, pos + size);
}

float Light::getEffectRange() const {
	switch (type) {
	case Type::POINT:
	case Type::SPOT:			return calculatePointLightRadius(constant, linear, quadratic, intensity);
	case Type::AMBIENT_BOX:		return calculateAmbientBoxLightRadius(transform.getWorldPosition(), {constant, linear, quadratic});
	case Type::AMBIENT_SPHERE:	return constant;
	}

	return std::numeric_limits<float>::infinity();
}

const ECS::Transform& Light::getTransform() const {
	return transform;
}
