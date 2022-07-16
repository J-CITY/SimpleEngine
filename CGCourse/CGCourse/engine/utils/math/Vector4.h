#pragma once

#include <cmath>
#include <ostream>

#include "vector3.h"

import glmath;

namespace KUMA {
	namespace MATHGL {
		struct Matrix4;
		
		struct Vector4 {
			float x = 0.0f, y = 0.0f, z = 0.0f, w = 0.0f;

			Vector4() = default;
			Vector4(const float f) : x(f), y(f), z(f), w(f) {}
			Vector4(const Vector3& vec, const float f) : x(vec.x), y(vec.y), z(vec.z), w(f) {}
			Vector4(const float x, const float y, const float z, const float w) : x(x), y(y), z(z), w(w) {}

			float& operator[](const unsigned index);
			const float& operator[](const unsigned index) const;
			Vector4& operator=(const Vector4& right);

			float dot(const Vector4& V2);
			float length();
			Vector4 normalize();

			static float Dot(const Vector4& V1, const Vector4& V2);
			static float Length(const Vector4& V);
			static Vector4 Normalize(const Vector4& V);
		};


		inline bool operator==(const Vector4& V1, const Vector4& V2) {
			return V1.x == V2.x && V1.y == V2.y && V1.z == V2.z && V1.w == V2.w;
		}

		inline bool operator!=(const Vector4& V1, const Vector4& V2) {
			return !operator==(V1, V2);
		}

		inline Vector4 operator-(const Vector4& V) {
			return Vector4(-V.x, -V.y, -V.z, -V.w);
		}

		inline void operator+=(Vector4& V1, const Vector4& V2) {
			for (int i = 0; i < 4; ++i) {
				V1[i] += V2[i];
			}
		}

		inline void operator-=(Vector4& V1, const Vector4& V2) {
			operator+=(V1, -V2);
		}

		inline Vector4 operator+(const Vector4& V1, const Vector4& V2) {
			return Vector4(V1.x + V2.x, V1.y + V2.y, V1.z + V2.z, V1.w + V2.w);
		}

		inline Vector4 operator-(const Vector4& V1, const Vector4& V2) {
			return Vector4(V1.x - V2.x, V1.y - V2.y, V1.z - V2.z, V1.w - V2.w);
		}

		inline Vector4 operator*(const Vector4& V1, const Vector4& V2) {
			return Vector4(V1.x * V2.x, V1.y * V2.y, V1.z * V2.z, V1.w * V2.w);
		}

		inline Vector4 operator*(const Vector4& V, const float f) {
			return Vector4(V.x * f, V.y * f, V.z * f, V.w * f);
		}

		inline Vector4 operator*(const float f, const Vector4& V) {
			return operator*(V, f);
		}

		inline Vector4 operator/(const Vector4& V, const float f) {
			return Vector4(V.x / f, V.y / f, V.z / f, V.w / f);
		}

		inline Vector4 operator/(const float f, const Vector4& V) {
			return operator/(V, f);
		}

		inline std::ostream& operator<<(std::ostream& out, const Vector4& V) {
			out << "( ";
			for (int i = 0; i < 4; ++i) {
				out << V[i];
				if (i != 3)
					out << ", ";
			}
			out << " )";

			return out;
		}
	}
}
