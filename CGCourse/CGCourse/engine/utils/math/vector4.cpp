#include "vector4.h"

using namespace KUMA;
using namespace KUMA::MATHGL;

Vector4& Vector4::operator=(const Vector4& right) {
	if (this == &right) {
		return *this;
	}
	x = right.x;
	y = right.y;
	z = right.z;
	w = right.w;
	return *this;
}

float& Vector4::operator[](const unsigned index) {
	if (index > 3) {
		throw std::logic_error("Index > 3");
	}
	switch (index) {
	case 0: return x;
	case 1: return y;
	case 2: return z;
	case 3: return w;
	}
}
const float& Vector4::operator[](const unsigned index) const {
	if (index > 3) {
		throw std::logic_error("Index > 3");
	}
	switch (index) {
	case 0: return x;
	case 1: return y;
	case 2: return z;
	case 3: return w;
	}
}

float Vector4::dot(const Vector4& V) {
	return Dot(*this, V);
}

float Vector4::length() {
	return Length(*this);
}

Vector4 Vector4::normalize() {
	return Normalize(*this);
}

float Vector4::Dot(const Vector4& V1, const Vector4& V2) {
	return V1.x * V2.x + V1.y * V2.y + V1.z * V2.z + V1.w * V2.w;
}

float Vector4::Length(const Vector4& V) {
	return sqrt(pow(V.x, 2) + pow(V.y, 2) + pow(V.z, 2) + pow(V.w, 2));
}

Vector4 Vector4::Normalize(const Vector4& V) {
	return V / Length(V);
}
