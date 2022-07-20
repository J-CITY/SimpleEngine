module;
#include <cmath>
#include <stdexcept>

module glmath:Vector3;

using namespace KUMA;
using namespace KUMA::MATHGL;

const Vector3 Vector3::One(1.0f, 1.0f, 1.0f);
const Vector3 Vector3::Zero(0.0f, 0.0f, 0.0f);
const Vector3 Vector3::Forward(0.0f, 0.0f, 1.0f);
const Vector3 Vector3::Right(1.0f, 0.0f, 0.0f);
const Vector3 Vector3::Up(0.0f, 1.0f, 0.0f);


Vector3::Vector3(float x, float y, float z) : x(x), y(y), z(z) {
}
Vector3::Vector3(const Vector3& toCopy) : x(toCopy.x), y(toCopy.y), z(toCopy.z) {
}
//Vector3::Vector3(const Vector4& toCopy) : x(toCopy.x), y(toCopy.y), z(toCopy.z) {
//}

Vector3 Vector3::operator-() const {
	return operator*(-1);
}

Vector3 Vector3::operator=(const Vector3& other) {
	this->x = other.x;
	this->y = other.y;
	this->z = other.z;

	return *this;
}

Vector3 Vector3::operator+(const Vector3& other) const {
	return Add(*this, other);
}

Vector3& Vector3::operator+=(const Vector3& other) {
	*this = Add(*this, other);
	return *this;
}

Vector3 Vector3::operator-(const Vector3& other) const {
	return Substract(*this, other);
}

Vector3& Vector3::operator-=(const Vector3& other) {
	*this = Substract(*this, other);
	return *this;
}

Vector3 Vector3::operator*(float scalar) const {
	return Multiply(*this, scalar);
}

Vector3 Vector3::operator*(const Vector3& vec) const {
	return Vector3(x * vec.x, y * vec.y, z * vec.z);
}

Vector3& Vector3::operator*=(float scalar) {
	*this = Multiply(*this, scalar);
	return *this;
}

Vector3 Vector3::operator/(float scalar) const {
	return Divide(*this, scalar);
}

Vector3& Vector3::operator/=(float scalar) {
	*this = Divide(*this, scalar);
	return *this;
}

bool Vector3::operator==(const Vector3& other) {
	return
		this->x == other.x &&
		this->y == other.y &&
		this->z == other.z;
}

bool Vector3::operator!=(const Vector3& other) {
	return !operator==(other);
}

Vector3 Vector3::Add(const Vector3& left, const Vector3& right) {
	return Vector3
	(
		left.x + right.x,
		left.y + right.y,
		left.z + right.z
	);
}

Vector3 Vector3::Substract(const Vector3& left, const Vector3& right) {
	return Vector3
	(
		left.x - right.x,
		left.y - right.y,
		left.z - right.z
	);
}

Vector3 Vector3::Multiply(const Vector3& target, float scalar) {
	return Vector3
	(
		target.x * scalar,
		target.y * scalar,
		target.z * scalar
	);
}

Vector3 Vector3::Divide(const Vector3& left, float scalar) {
	Vector3 result(left);

	if (scalar == 0)
		throw std::logic_error("Division by 0");

	result.x /= scalar;
	result.y /= scalar;
	result.z /= scalar;

	return result;
}

float Vector3::Length(const Vector3& target) {
	return std::sqrt(target.x * target.x + target.y * target.y + target.z * target.z);
}

float Vector3::LengthSqrt(const Vector3& target) {
	return (target.x * target.x + target.y * target.y + target.z * target.z);
}

float Vector3::Dot(const Vector3& left, const Vector3& right) {
	return left.x * right.x + left.y * right.y + left.z * right.z;
}

float Vector3::Distance(const Vector3& left, const Vector3& right) {
	return std::sqrt
	(
		(left.x - right.x) * (left.x - right.x) +
		(left.y - right.y) * (left.y - right.y) +
		(left.z - right.z) * (left.z - right.z)
	);
}

Vector3 Vector3::Cross(const Vector3& left, const Vector3& right) {
	return Vector3
	(
		left.y * right.z - left.z * right.y,
		left.z * right.x - left.x * right.z,
		left.x * right.y - left.y * right.x
	);
}

Vector3 Vector3::Normalize(const Vector3& target) {
	float length = Length(target);

	if (length > 0.0f) {
		float targetLength = 1.0f / length;

		return Vector3
		(
			target.x * targetLength,
			target.y * targetLength,
			target.z * targetLength
		);
	}
	else {
		return Vector3::Zero;
	}
}

Vector3 Vector3::Lerp(const Vector3& start, const Vector3& end, float alpha) {
	return (start + (end - start) * alpha);
}

Vector3 Vector3::Mix(const Vector3& x, const Vector3& y, float alpha) {
	return x * (1.0f-alpha) + y * alpha;
}

float Vector3::AngleBetween(const Vector3& from, const Vector3& to) {
	float lengthProduct = Length(from) * Length(to);

	if (lengthProduct > 0.0f) {
		float fractionResult = Dot(from, to) / lengthProduct;

		if (fractionResult >= -1.0f && fractionResult <= 1.0f)
			return acosf(fractionResult);
	}

	return 0.0f;
}

void Vector3::clear() {
	x = y = z = 0.0f;
}

float& Vector3::operator[](unsigned i) {
	if (i == 0) {
		return x;
	}
	if (i == 1) {
		return y;
	}
	return z;

}
float Vector3::operator[](unsigned i) const {
	if (i == 0) return x;
	if (i == 1) return y;
	return z;
}

void Vector3::addScaledVector(const Vector3& vector, float scale) {
	x += vector.x * scale;
	y += vector.y * scale;
	z += vector.z * scale;
}
float Vector3::dot(const Vector3& vector) const {
	return Dot(*this, vector);
}
Vector3 Vector3::componentProduct(const Vector3& vector) const {
	return Vector3(x * vector.x, y * vector.y, z * vector.z);
}
Vector3 Vector3::cross(const Vector3& vector) const {
	return Cross(*this, vector);
}
float Vector3::magnitude() const {
	return sqrt(x * x + y * y + z * z);
}
float Vector3::squareMagnitude() const {
	return x * x + y * y + z * z;
}
void Vector3::normalise() {
	float l = magnitude();
	if (l > 0) {
		(*this) *= 1.f / l;
	}
}
void Vector3::componentProductUpdate(const Vector3& vector) {
	x *= vector.x;
	y *= vector.y;
	z *= vector.z;
}

Vector3 KUMA::MATHGL::operator*(const float f, const Vector3& V) {
	return V * f;
}

Vector3 KUMA::MATHGL::operator/(const float f, const Vector3& V) {
	return V / f;
}

