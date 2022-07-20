export;
#pragma once
#include <cmath>
#include <stdexcept>
export module glmath:Quaternion;

import :Vector3;

export namespace KUMA::MATHGL {
	struct Matrix3;
	struct Matrix4;

	struct Quaternion {
		float x = 0.0f;
		float y = 0.0f;
		float z = 0.0f;
		float w = 0.0f;

		static const Quaternion Identity;

		Quaternion();
		Quaternion(float in);
		Quaternion(float x, float y, float z, float w);
		Quaternion(const Quaternion& in);
		Quaternion(const Matrix3& rotationMatrix);
		Quaternion(const Matrix4& rotationMatrix);
		Quaternion(const Vector3& euler);

		void normalise();
		void addScaledVector(const Vector3& vector, float scale);

		bool operator==(const Quaternion& otherQuat) const;
		bool operator!=(const Quaternion& otherQuat) const;
		Quaternion operator+(const Quaternion& otherQuat) const;
		Quaternion& operator+=(const Quaternion& otherQuat);
		Quaternion operator-(const Quaternion& otherQuat) const;
		Quaternion& operator-=(const Quaternion& otherQuat);
		float operator|(const Quaternion& otherQuat) const;
		Quaternion& operator*=(const float scale);
		Quaternion operator*(const float scale) const;
		Quaternion operator*(const Quaternion& otherQuat) const;
		Quaternion& operator*=(const Quaternion& otherQuat);
		Vector3 operator*(const Vector3& toMultiply) const;
		Matrix3 operator*(const Matrix3& multiply) const;
		Quaternion& operator/=(const float scale);
		Quaternion operator/(const float scale) const;

		static Quaternion LookAt(const Vector3& forward, const Vector3& up);
		static Matrix4 ToMatrix4(const Quaternion& target);
		static Matrix3 ToMatrix3(const Quaternion& target);
		static Quaternion Normalize(const Quaternion& target);
		static float Length(const Quaternion& target);
		static float LengthSquare(const Quaternion& target);
		static bool IsNormalized(const Quaternion& target);
		static Vector3 RotatePoint(const Vector3& point, const Quaternion& quaternion);
		static Vector3 RotatePoint(const Vector3& point, const Quaternion& quaternion, const Vector3& pivot);
		static float Dot(const Quaternion& V1, const Quaternion& V2);
		static Quaternion Slerp(Quaternion& x, Quaternion& y, float a);
	};
	
	Quaternion operator*(const float f, const Quaternion& V);
	Quaternion operator/(const float f, const Quaternion& V);
}
