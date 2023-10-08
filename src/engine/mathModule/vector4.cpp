module;
#include <cmath>
#include <stdexcept>

module glmath:Vector4;

using namespace IKIGAI;
using namespace IKIGAI::MATHGL;

Vector4::Vector4() = default;
Vector4::Vector4(const float f) : x(f), y(f), z(f), w(f) {}
Vector4::Vector4(const Vector3& vec, const float f) : x(vec.x), y(vec.y), z(vec.z), w(f) {}
Vector4::Vector4(const float x, const float y, const float z, const float w) : x(x), y(y), z(z), w(w) {}


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

bool Vector4::operator==(const Vector4& V2) {
	return x == V2.x && y == V2.y && z == V2.z && w == V2.w;
}

bool Vector4::operator!=(const Vector4& V2) {
	return !operator==(V2);
}

Vector4 Vector4::operator-() {
	return Vector4(-x, -y, -z, -w);
}

void Vector4::operator+=(const Vector4& V2) {
	x += V2.x;
	y += V2.y;
	z += V2.z;
	w += V2.w;
}

void Vector4::operator-=(const Vector4& V2) {
	x -= V2.x;
	y -= V2.y;
	z -= V2.z;
	w -= V2.w;
}

Vector4 Vector4::operator+(const Vector4& V2) {
	return Vector4(x + V2.x, y + V2.y, z + V2.z, w + V2.w);
}

Vector4 Vector4::operator-(const Vector4& V2) {
	return Vector4(x - V2.x, y - V2.y, z - V2.z, w - V2.w);
}

Vector4 Vector4::operator*(const Vector4& V2) {
	return Vector4(x * V2.x, y * V2.y, z * V2.z, w * V2.w);
}

Vector4 Vector4::operator*(const float f) const {
	return Vector4(x * f, y * f, z * f, w * f);
}

Vector4 Vector4::operator/(const float f) const {
	return Vector4(x / f, y / f, z / f, w / f);
}

Vector4 IKIGAI::MATHGL::operator*(const float f, const Vector4& V) {
	return V * f;
}

Vector4 IKIGAI::MATHGL::operator/(const float f, const Vector4& V) {
	return V / f;
}


#include <rttr/registration>

RTTR_REGISTRATION
{
	rttr::registration::class_<IKIGAI::MATHGL::Vector4>("Vector4")
	.property("X", &IKIGAI::MATHGL::Vector4::x)
	.property("Y", &IKIGAI::MATHGL::Vector4::y)
	.property("Z", &IKIGAI::MATHGL::Vector4::z)
	.property("W", &IKIGAI::MATHGL::Vector4::w);
}

