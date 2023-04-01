module;
#include <cmath>
#include <stdexcept>
#include <array>

//#include "../../utils/refl.hpp"

module glmath:Quaternion;

import :Constants;
import :Matrix4;
import :Matrix3;

using namespace KUMA;
using namespace KUMA::MATHGL;

const Quaternion Quaternion::Identity = Quaternion(0.0f, 0.0f, 0.0f, 1.0f);

Quaternion::Quaternion() = default;
Quaternion::Quaternion(float in) : w(in) {}
Quaternion::Quaternion(float x, float y, float z, float w) : x(x), y(y), z(z), w(w) {};
Quaternion::Quaternion(const Quaternion& in) : x(in.x), y(in.y), z(in.z), w(in.w) {}

Quaternion::Quaternion(const Matrix3& rotationMatrix) {
	float trace = rotationMatrix.data[0] + rotationMatrix.data[4] + rotationMatrix.data[8];
	if (trace > 0.0f) {
		// I changed M_EPSILON to 0
		float s = 0.5f / sqrt(trace + 1.0f);
		w = 0.25f / s;
		x = (rotationMatrix.data[7] - rotationMatrix.data[5]) * s;
		y = (rotationMatrix.data[2] - rotationMatrix.data[6]) * s;
		z = (rotationMatrix.data[3] - rotationMatrix.data[1]) * s;
	}
	else {
		if (rotationMatrix.data[0] > rotationMatrix.data[4] && rotationMatrix.data[0] > rotationMatrix.data[8]) {
			float s = 2.0f * sqrt(1.0f + rotationMatrix.data[0] - rotationMatrix.data[4] - rotationMatrix.data[8]);
			w = (rotationMatrix.data[7] - rotationMatrix.data[5]) / s;
			x = 0.25f * s;
			y = (rotationMatrix.data[1] + rotationMatrix.data[3]) / s;
			z = (rotationMatrix.data[2] + rotationMatrix.data[6]) / s;
		}
		else if (rotationMatrix.data[4] > rotationMatrix.data[8]) {
			float s = 2.0f * sqrt(1.0f + rotationMatrix.data[4] - rotationMatrix.data[0] - rotationMatrix.data[8]);
			w = (rotationMatrix.data[2] - rotationMatrix.data[6]) / s;
			x = (rotationMatrix.data[1] + rotationMatrix.data[3]) / s;
			y = 0.25f * s;
			z = (rotationMatrix.data[5] + rotationMatrix.data[7]) / s;
		}
		else {
			float s = 2.0f * sqrt(1.0f + rotationMatrix.data[8] - rotationMatrix.data[0] - rotationMatrix.data[4]);
			w = (rotationMatrix.data[3] - rotationMatrix.data[1]) / s;
			x = (rotationMatrix.data[2] + rotationMatrix.data[6]) / s;
			y = (rotationMatrix.data[5] + rotationMatrix.data[7]) / s;
			z = 0.25f * s;
		}
	}
}

Quaternion::Quaternion(const Matrix4& rotationMatrix) {
	float halfSquare;

	// Check diagonal (trace)
	const float trace = rotationMatrix.data[0] + rotationMatrix.data[5] + rotationMatrix.data[10];

	if (trace > 0.0f) {
		const float InvSquare = 1 / sqrt(trace + 1.f);
		w = 0.5f * (1.f / InvSquare);
		halfSquare = 0.5f * InvSquare;

		x = (rotationMatrix.data[6] - rotationMatrix.data[9]) * halfSquare;
		y = (rotationMatrix.data[8] - rotationMatrix.data[2]) * halfSquare;
		z = (rotationMatrix.data[1] - rotationMatrix.data[4]) * halfSquare;
	}
	else {
		// diagonal is negative
		int i = 0;

		if (rotationMatrix.data[5] > rotationMatrix.data[0])
			i = 1;

		if (rotationMatrix.data[10] > rotationMatrix.data[0] || rotationMatrix.data[10] > rotationMatrix.data[5])
			i = 2;

		static const int next[3] = {1, 2, 0};
		const int j = next[i];
		const int k = next[j];

		halfSquare = rotationMatrix.data[i * 5] - rotationMatrix.data[j * 5] - rotationMatrix.data[k * 5] + 1.0f;

		const float InvSquare = 1 / sqrt(trace + 1.f);

		float qt[4];
		qt[i] = 0.5f * (1.f / InvSquare);

		halfSquare = 0.5f * InvSquare;
		//if i is 0, j is 1 and k is 2
		if (i == 0) {
			qt[3] = (rotationMatrix.data[6] - rotationMatrix.data[9]) * halfSquare;
			qt[j] = (rotationMatrix.data[1] + rotationMatrix.data[4]) * halfSquare;
			qt[k] = (rotationMatrix.data[2] + rotationMatrix.data[8]) * halfSquare;
		}
		//if i is 1, j is 2 and k is 0
		else if (i == 1) {
			qt[3] = (rotationMatrix.data[8] - rotationMatrix.data[2]) * halfSquare;
			qt[j] = (rotationMatrix.data[6] + rotationMatrix.data[9]) * halfSquare;
			qt[k] = (rotationMatrix.data[4] + rotationMatrix.data[1]) * halfSquare;
		}
		//if i is 2, j is 0 and k is 1
		else {
			qt[3] = (rotationMatrix.data[1] - rotationMatrix.data[4]) * halfSquare;
			qt[j] = (rotationMatrix.data[8] + rotationMatrix.data[2]) * halfSquare;
			qt[k] = (rotationMatrix.data[9] + rotationMatrix.data[6]) * halfSquare;
		}
		x = qt[0];
		y = qt[1];
		z = qt[2];
		w = qt[3];
	}
}
Quaternion::Quaternion(const Vector3& euler) {
	/* Degree to radians then times 0.5f = 0.0087f */
	float yaw = TO_RADIANS(euler.z) * 0.5f;
	float pitch = TO_RADIANS(euler.y) * 0.5f;
	float roll = TO_RADIANS(euler.x) * 0.5f;

	float cy = cos(yaw);
	float sy = sin(yaw);
	float cp = cos(pitch);
	float sp = sin(pitch);
	float cr = cos(roll);
	float sr = sin(roll);

	x = sr * cp * cy - cr * sp * sy;
	y = cr * sp * cy + sr * cp * sy;
	z = cr * cp * sy - sr * sp * cy;
	w = cr * cp * cy + sr * sp * sy;
}

Quaternion Quaternion::Normalize(const Quaternion& target) {
	return target / Length(target);
}

Matrix3 Quaternion::ToMatrix3(const Quaternion& target) {
	if (!IsNormalized(target))
		throw std::logic_error("Cannot convert non-normalized quaternions to Matrix4");

	float y2 = target.y * target.y;
	float wz = target.w * target.z;
	float x2 = target.x * target.x;
	float z2 = target.z * target.z;
	float xz = target.x * target.z;
	float yz = target.y * target.z;
	float xy = target.x * target.y;
	float wy = target.w * target.y;
	float wx = target.w * target.x;

	Matrix3 converted;
	converted.data[0] = 1.0f - (2 * y2) - (2 * z2);
	converted.data[1] = (2 * xy) - (2 * wz);
	converted.data[2] = (2 * xz) + (2 * wy);
	converted.data[3] = (2 * xy) + (2 * wz);
	converted.data[4] = 1.0f - (2 * x2) - (2 * z2);
	converted.data[5] = (2 * yz) - (2 * wx);
	converted.data[6] = (2 * xz) - (2 * wy);
	converted.data[7] = (2 * yz) + (2 * wx);
	converted.data[8] = 1.0f - (2 * x2) - (2 * y2);
	return converted;
}

Matrix4 Quaternion::ToMatrix4(const Quaternion& target) {
	if (!IsNormalized(target))
		throw std::logic_error("Cannot convert non-normalized quaternions to Matrix4");

	float y2 = target.y * target.y;	float wz = target.w * target.z;	float x2 = target.x * target.x;
	float z2 = target.z * target.z;	float xz = target.x * target.z;	float yz = target.y * target.z;
	float xy = target.x * target.y;	float wy = target.w * target.y;	float wx = target.w * target.x;

	Matrix4 converted = Matrix4::Identity;
	converted.data[0] = 1.0f - (2 * y2) - (2 * z2);		converted.data[1] = (2 * xy) - (2 * wz);				converted.data[2] = (2 * xz) + (2 * wy);			 converted.data[3] = 0;
	converted.data[4] = (2 * xy) + (2 * wz);				converted.data[5] = 1.0f - (2 * x2) - (2 * z2);		converted.data[6] = (2 * yz) - (2 * wx);			 converted.data[7] = 0;
	converted.data[8] = (2 * xz) - (2 * wy);				converted.data[9] = (2 * yz) + (2 * wx);			converted.data[10] = 1.0f - (2 * x2) - (2 * y2); converted.data[11] = 0;
	converted.data[12] = 0;								converted.data[13] = 0;								converted.data[14] = 0;							 converted.data[15] = 1;
	return converted;
}
bool Quaternion::IsNormalized(const Quaternion& target) {
	return abs(Length(target) - 1.0f) < 0.0001f;
}

Vector3 Quaternion::RotatePoint(const Vector3& point, const Quaternion& quaternion) {
	Vector3 Q(quaternion.x, quaternion.y, quaternion.z);
	Vector3 T = Vector3::Cross(Q, point) * 2.0f;

	return point + (T * quaternion.w) + Vector3::Cross(Q, T);
}

Vector3 Quaternion::RotatePoint(const Vector3& point, const Quaternion& quaternion, const Vector3& pivot) {
	Vector3 toRotate = point - pivot;
	return RotatePoint(toRotate, quaternion);
}


float Quaternion::Length(const Quaternion& target) {
	return sqrtf(LengthSquare(target));
}

float Quaternion::LengthSquare(const Quaternion& target) {
	return target.x * target.x + target.y * target.y + target.z * target.z + target.w * target.w;
}


bool Quaternion::operator==(const Quaternion& otherQuat) const {
	return x == otherQuat.x && y == otherQuat.x && z == otherQuat.z && w == otherQuat.w;
}

bool Quaternion::operator!=(const Quaternion& otherQuat) const {
	return x != otherQuat.x || y != otherQuat.x || z != otherQuat.z || w != otherQuat.w;
}

Quaternion Quaternion::operator+(const Quaternion& otherQuat) const {
	return Quaternion(x + otherQuat.x, y + otherQuat.y,
		z + otherQuat.z, w + otherQuat.w);
}

Quaternion& Quaternion::operator+=(const Quaternion& otherQuat) {
	x += otherQuat.x;
	y += otherQuat.x;
	z += otherQuat.z;
	w += otherQuat.w;
	return *this;
}

Quaternion Quaternion::operator-(const Quaternion& otherQuat) const {
	return Quaternion(x - otherQuat.x, y - otherQuat.x,
		z - otherQuat.z, w - otherQuat.w);
}

Quaternion& Quaternion::operator-=(const Quaternion& otherQuat) {
	x -= otherQuat.x;
	y -= otherQuat.x;
	z -= otherQuat.z;
	w -= otherQuat.w;
	return *this;
}

float Quaternion::operator|(const Quaternion& otherQuat) const {
	return (x * otherQuat.x + y * otherQuat.x + z *
		otherQuat.z + w * otherQuat.w);
}

Quaternion Quaternion::operator*(const float scale) const {
	Quaternion result(*this);
	result.x *= scale;
	result.y *= scale;
	result.z *= scale;
	result.w *= scale;

	return result;
}

Quaternion& Quaternion::operator*=(const float scale) {
	x *= scale;
	y *= scale;
	z *= scale;
	w *= scale;

	return *this;
}

Quaternion Quaternion::operator*(const Quaternion& otherQuat) const {
	return Quaternion
	(
		x * otherQuat.w + y * otherQuat.z - z * otherQuat.y + w * otherQuat.x,
		-x * otherQuat.z + y * otherQuat.w + z * otherQuat.x + w * otherQuat.y,
		x * otherQuat.y - y * otherQuat.x + z * otherQuat.w + w * otherQuat.z,
		-x * otherQuat.x - y * otherQuat.y - z * otherQuat.z + w * otherQuat.w
	);
}

Quaternion& Quaternion::operator*=(const Quaternion& otherQuat) {
	Quaternion temp(
		x * otherQuat.w + y * otherQuat.z - z * otherQuat.x +
		w * otherQuat.x,
		-x * otherQuat.z + y * otherQuat.w + z * otherQuat.x +
		w * otherQuat.x,
		x * otherQuat.x - y * otherQuat.x + z * otherQuat.w -
		w * otherQuat.z,
		-x * otherQuat.x - y * otherQuat.x - z * otherQuat.z +
		w * otherQuat.w);

	x = temp.x;
	y = temp.y;
	z = temp.z;
	w = temp.w;

	return *this;
}

Vector3 Quaternion::operator*(const Vector3& toMultiply) const {
	const float num = x * 2.0f;
	const float num2 = y * 2.0f;
	const float num3 = z * 2.0f;
	const float num4 = x * num;
	const float num5 = y * num2;
	const float num6 = z * num3;
	const float num7 = x * num2;
	const float num8 = x * num3;
	const float num9 = y * num3;
	const float num10 = w * num;
	const float num11 = w * num2;
	const float num12 = w * num3;
	Vector3 result;
	result.x = (1.f - (num5 + num6)) * toMultiply.x + (num7 - num12) * toMultiply.y + (num8 + num11) *
		toMultiply.z;
	result.y = (num7 + num12) * toMultiply.x + (1.f - (num4 + num6)) * toMultiply.y + (num9 - num10) *
		toMultiply.z;
	result.z = (num8 - num11) * toMultiply.x + (num9 + num10) * toMultiply.y + (1.f - (num4 + num5)) *
		toMultiply.z;
	return result;
}

Matrix3 Quaternion::operator*(const Matrix3& multiply) const {
	return (ToMatrix3(*this) * multiply);
}

Quaternion& Quaternion::operator/=(const float scale) {
	const float reciprocate = 1.0f / scale;
	x *= reciprocate;
	y *= reciprocate;
	z *= reciprocate;
	w *= reciprocate;

	return *this;
}

Quaternion Quaternion::operator/(const float scale) const {
	Quaternion temp(*this);
	const float reciprocate = 1.0f / scale;
	temp.x *= reciprocate;
	temp.y *= reciprocate;
	temp.z *= reciprocate;
	temp.w *= reciprocate;

	return temp;
}

Quaternion Quaternion::LookAt(const Vector3& forward, const Vector3& up) {
	auto vector = Vector3::Normalize(forward);
	auto vector2 = Vector3::Normalize(Vector3::Cross(up, vector));
	auto vector3 = Vector3::Cross(vector, vector2);
	auto m00 = vector2.x;
	auto m01 = vector2.y;
	auto m02 = vector2.z;
	auto m10 = vector3.x;
	auto m11 = vector3.y;
	auto m12 = vector3.z;
	auto m20 = vector.x;
	auto m21 = vector.y;
	auto m22 = vector.z;


	float num8 = (m00 + m11) + m22;
	auto quaternion = Quaternion::Identity;
	if (num8 > 0.f) {
		auto num = sqrt(num8 + 1.f);
		quaternion.w = num * 0.5f;
		num = 0.5f / num;
		quaternion.x = (m12 - m21) * num;
		quaternion.y = (m20 - m02) * num;
		quaternion.z = (m01 - m10) * num;
		return quaternion;
	}
	if ((m00 >= m11) && (m00 >= m22)) {
		auto num7 = sqrt(((1.f + m00) - m11) - m22);
		auto num4 = 0.5f / num7;
		quaternion.x = 0.5f * num7;
		quaternion.y = (m01 + m10) * num4;
		quaternion.z = (m02 + m20) * num4;
		quaternion.w = (m12 - m21) * num4;
		return quaternion;
	}
	if (m11 > m22) {
		auto num6 = sqrt(((1.f + m11) - m00) - m22);
		auto num3 = 0.5f / num6;
		quaternion.x = (m10 + m01) * num3;
		quaternion.y = 0.5f * num6;
		quaternion.z = (m21 + m12) * num3;
		quaternion.w = (m20 - m02) * num3;
		return quaternion;
	}
	auto num5 = sqrt(((1.f + m22) - m00) - m11);
	auto num2 = 0.5f / num5;
	quaternion.x = (m20 + m02) * num2;
	quaternion.y = (m21 + m12) * num2;
	quaternion.z = 0.5f * num5;
	quaternion.w = (m01 - m10) * num2;
	return quaternion;
}

void Quaternion::normalise() {
	*this = Normalize(*this);
}
void Quaternion::addScaledVector(const Vector3& vector, float scale) {
	Quaternion q(
		vector.x * scale,
		vector.y * scale,
		vector.z * scale,
		0
	);
	q *= *this;
	w += q.w * 0.5f;
	x += q.x * 0.5f;
	y += q.y * 0.5f;
	z += q.z * 0.5f;
}

float Quaternion::Dot(const Quaternion& V1, const Quaternion& V2) {
	return V1.x * V2.x + V1.y * V2.y + V1.z * V2.z + V1.w * V2.w;
}
Quaternion Quaternion::Slerp(Quaternion& x, Quaternion& y, float a) {
	Quaternion z = y;

	float cosTheta = Quaternion::Dot(x, y);

	// If cosTheta < 0, the interpolation will take the long way around the sphere.
	// To fix this, one quat must be negated.
	if (cosTheta < 0.0f) {
		z = y * -1.0f;
		cosTheta = -cosTheta;
	}
	auto mix = [](float x, float y, float a) {
		return x * (1.0f - a) + y * a;
	};
	// Perform a linear interpolation when cosTheta is close to 1 to avoid side effect of sin(angle) becoming a zero denominator
	if (cosTheta > 1.0f - std::numeric_limits<float>::epsilon()) {
		// Linear interpolation
		return Quaternion(
			mix(x.x, z.x, a),
			mix(x.y, z.y, a),
			mix(x.z, z.z, a),
			mix(x.w, z.w, a)
		);
	}
	else {
		// Essential Mathematics, page 467
		auto angle = acos(cosTheta);

		auto sin1 = sin((1.0f - a) * angle);
		auto sin2 = sin(a * angle);
		auto s = sin(angle);

		auto xsin1 = x * sin1;
		auto zsin2 = z * sin2;

		auto res = xsin1 + zsin2;
		auto res2 = res / s;

		return ((x * sin((1.0f - a) * angle)) + (z * sin(a * angle))) / sin(angle);
	}
}

Quaternion KUMA::MATHGL::operator*(const float f, const Quaternion& V) {
	return V * f;
}

Quaternion KUMA::MATHGL::operator/(const float f, const Quaternion& V) {
	return V / f;
}

Vector3 Quaternion::ToEulerAngles(Quaternion q) {
	Vector3 angles;

	// roll (x-axis rotation)
	float sinr_cosp = 2 * (q.w * q.x + q.y * q.z);
	float cosr_cosp = 1 - 2 * (q.x * q.x + q.y * q.y);
	angles.x = std::atan2(sinr_cosp, cosr_cosp);

	// pitch (y-axis rotation)
	float sinp = std::sqrt(1 + 2 * (q.w * q.y - q.x * q.z));
	float cosp = std::sqrt(1 - 2 * (q.w * q.y - q.x * q.z));
	angles.y = 2 * std::atan2(sinp, cosp) - PI / 2;

	// yaw (z-axis rotation)
	float siny_cosp = 2 * (q.w * q.z + q.x * q.y);
	float cosy_cosp = 1 - 2 * (q.y * q.y + q.z * q.z);
	angles.z = std::atan2(siny_cosp, cosy_cosp);

	return angles;
}

Quaternion Quaternion::ToQuaternion(Vector3 vec) {
	auto yaw = vec.x;
	auto pitch = vec.y;
	auto roll = vec.z;
	float cr = cos(roll * 0.5);
	float sr = sin(roll * 0.5);
	float cp = cos(pitch * 0.5);
	float sp = sin(pitch * 0.5);
	float cy = cos(yaw * 0.5);
	float sy = sin(yaw * 0.5);

	Quaternion q;
	q.w = cr * cp * cy + sr * sp * sy;
	q.x = sr * cp * cy - cr * sp * sy;
	q.y = cr * sp * cy + sr * cp * sy;
	q.z = cr * cp * sy - sr * sp * cy;

	return q;
}

//REFL_AUTO(
//	type(KUMA::MATHGL::Quaternion),
//	field(x),
//	field(y),
//	field(z),
//	field(w)
//)

