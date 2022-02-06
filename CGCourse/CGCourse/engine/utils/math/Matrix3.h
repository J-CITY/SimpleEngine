#pragma once

#include <array>
#include "Vector3.h"
namespace KUMA {
	namespace MATHGL {
		struct Matrix3 {
			static const Matrix3 Identity;
			std::array<float, 9> data;

			Matrix3() {
				for (auto& d : data) {
					d = 0.0f;
				}

				data = Identity.data;
			}
			Matrix3(float val) {
				for (auto& d : data) {
					d = val;
				}
			}

			Matrix3(float element1, float element2, float element3,
				float element4, float element5, float element6,
				float element7, float element8, float element9) {
				data[0] = element1;
				data[1] = element2;
				data[2] = element3;
				data[3] = element4;
				data[4] = element5;
				data[5] = element6;
				data[6] = element7;
				data[7] = element8;
				data[8] = element9;
			}

			Matrix3(Vector3 a, Vector3 b, Vector3 c) {
				data[0] = a.x;
				data[1] = a.y;
				data[2] = a.z;
				data[3] = b.x;
				data[4] = b.y;
				data[5] = b.z;
				data[6] = c.x;
				data[7] = c.y;
				data[8] = c.z;
			}

			
			Matrix3(std::array<float, 9>&& in) {
				data = in;
			}

			Matrix3(const Matrix3& in) {
				*this = in;
			}

			Matrix3& operator=(const Matrix3& other);
			bool operator==(const Matrix3& other);
			Matrix3 operator+(const Matrix3& other) const;
			Matrix3& operator+=(const Matrix3& other);

			Matrix3 operator-(const Matrix3& other) const;

			Matrix3& operator-=(const Matrix3& other);

			Matrix3 operator*(float scalar) const;

			Matrix3& operator*=(float scalar);
			Vector3 operator*(const Vector3& vector) const;
			Matrix3 operator*(const Matrix3& other) const;
			Matrix3& operator*=(const Matrix3& other);
			Matrix3 operator/(float scalar) const;
			Matrix3& operator/=(float scalar);
			Matrix3 operator/(const Matrix3& other) const;
			Matrix3& operator/=(const Matrix3& other);
			float& operator()(unsigned row, unsigned column);
			static bool AreEquals(const Matrix3& left, const Matrix3& right);
			static Matrix3 Add(const Matrix3& left, float scalar);
			static Matrix3 Add(const Matrix3& left, const Matrix3& right);
			static Matrix3 Subtract(const Matrix3& left, float scalar);
			static Matrix3 Subtract(const Matrix3& left, const Matrix3& right);
			static Matrix3 Multiply(const Matrix3& left, float scalar);
			static Vector3 Multiply(const Matrix3& matrix, const Vector3& vector);
			static Matrix3 Multiply(const Matrix3& left, const Matrix3& right);
			static Matrix3 Divide(const Matrix3& left, float scalar);
			static Matrix3 Divide(const Matrix3& left, const Matrix3& right);
			static bool IsIdentity(const Matrix3& matrix);
			static float Determinant(const Matrix3& matrix);
			static Matrix3 Transpose(const Matrix3& matrix);
			static Matrix3 Cofactor(const Matrix3& matrix);
			static Matrix3 Minor(const Matrix3& matrix);
			static Matrix3 Adjoint(const Matrix3& other);
			static Matrix3 Inverse(const Matrix3& matrix);
			static Matrix3 Rotation(float rotation);
			static Matrix3 Rotate(const Matrix3& matrix, float rotation);
			static Vector3 GetRow(const Matrix3& matrix, unsigned row);
			static Vector3 GetColumn(const Matrix3& matrix, unsigned column);
		};
	}
}
