#include "DiffuseLightRT.h"

KUMA::MATHGL::Vector3 Reflect(const KUMA::MATHGL::Vector3& v, const KUMA::MATHGL::Vector3& n) {
	return v - n * Dot(v, n) * 2.0f;
}
