#pragma once

#include "Matrix3.h"
#include "Matrix4.h"
#include "Vector3.h"
#undef PI

#define PI (4.f * atan(1.f))
#define TO_RADIANS(value) value * PI / 180.f
#define TO_DEGREES(value) value * 180.f / PI

namespace KUMA {
	namespace MATHGL {
		struct Quaternion {
			float x = 0.0f;
			float y = 0.0f;
			float z = 0.0f;
			float w = 0.0f;

			static const Quaternion Identity;

			Quaternion() = default;
			Quaternion(float in) : w(in) {}
			Quaternion(float x, float y, float z, float w): x(x), y(y), z(z), w(w) {};
			Quaternion(const Quaternion& in): x(in.x), y(in.y), z(in.z), w(in.w) {}
			Quaternion(const Matrix3& rotationMatrix);
			Quaternion(const Matrix4& rotationMatrix);
			Quaternion(const Vector3& euler);

			void normalise() {
				*this = Normalize(*this);
			}
			void addScaledVector(const Vector3& vector, float scale) {
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

			static Quaternion LookAt(const Vector3& forward, const Vector3& up);
			static Matrix4 ToMatrix4(const Quaternion& target);
			static Matrix3 ToMatrix3(const Quaternion& target);
			static Quaternion Normalize(const Quaternion& target);
			static float Length(const Quaternion& target);
			static float LengthSquare(const Quaternion& target);
			static bool IsNormalized(const Quaternion& target);
			static Vector3 RotatePoint(const Vector3& point, const Quaternion& quaternion);
			static Vector3 RotatePoint(const Vector3& point, const Quaternion& quaternion, const Vector3& pivot);

			static float Quaternion::Dot(const Quaternion& V1, const Quaternion& V2) {
				return V1.x * V2.x + V1.y * V2.y + V1.z * V2.z + V1.w * V2.w;
			}
			static Quaternion Slerp(Quaternion& x, Quaternion& y, float a) {
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
		};
	}
}
