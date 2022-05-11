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

			Vector3 transform(const Vector3& vector) const {
				return (*this) * vector;
			}

			void setInertiaTensorCoeffs(float ix, float iy, float iz,
				float ixy = 0, float ixz = 0, float iyz = 0) {
				data[0] = ix;
				data[1] = data[3] = -ixy;
				data[2] = data[6] = -ixz;
				data[4] = iy;
				data[5] = data[7] = -iyz;
				data[8] = iz;
			}

			/**
			 * Sets the value of the matrix as an inertia tensor of
			 * a rectangular block aligned with the body's coordinate
			 * system with the given axis half-sizes and mass.
			 */
			void setBlockInertiaTensor(const Vector3& halfSizes, float mass) {
				Vector3 squares = halfSizes.componentProduct(halfSizes);
				setInertiaTensorCoeffs(0.3f * mass * (squares.y + squares.z),
					0.3f * mass * (squares.x + squares.z),
					0.3f * mass * (squares.x + squares.y));
			}

			void setInverse(const Matrix3& m) {
				float t4 = m.data[0] * m.data[4];
				float t6 = m.data[0] * m.data[5];
				float t8 = m.data[1] * m.data[3];
				float t10 = m.data[2] * m.data[3];
				float t12 = m.data[1] * m.data[6];
				float t14 = m.data[2] * m.data[6];

				// Calculate the determinant
				float t16 = (t4 * m.data[8] - t6 * m.data[7] - t8 * m.data[8] +
					t10 * m.data[7] + t12 * m.data[5] - t14 * m.data[4]);

				// Make sure the determinant is non-zero.
				if (t16 == 0.0f) {
					return;
				}
				float t17 = 1 / t16;

				data[0] = (m.data[4] * m.data[8] - m.data[5] * m.data[7]) * t17;
				data[1] = -(m.data[1] * m.data[8] - m.data[2] * m.data[7]) * t17;
				data[2] = (m.data[1] * m.data[5] - m.data[2] * m.data[4]) * t17;
				data[3] = -(m.data[3] * m.data[8] - m.data[5] * m.data[6]) * t17;
				data[4] = (m.data[0] * m.data[8] - t14) * t17;
				data[5] = -(t6 - t10) * t17;
				data[6] = (m.data[3] * m.data[7] - m.data[4] * m.data[6]) * t17;
				data[7] = -(m.data[0] * m.data[7] - t12) * t17;
				data[8] = (t4 - t8) * t17;
			}
			void setComponents(const Vector3& compOne, const Vector3& compTwo,
				const Vector3& compThree) {
				data[0] = compOne.x;
				data[1] = compTwo.x;
				data[2] = compThree.x;
				data[3] = compOne.y;
				data[4] = compTwo.y;
				data[5] = compThree.y;
				data[6] = compOne.z;
				data[7] = compTwo.z;
				data[8] = compThree.z;

			}

			Vector3 transformTranspose(const Vector3& vector) const {
				return Vector3(
					vector.x * data[0] + vector.y * data[3] + vector.z * data[6],
					vector.x * data[1] + vector.y * data[4] + vector.z * data[7],
					vector.x * data[2] + vector.y * data[5] + vector.z * data[8]
				);
			}

			void setSkewSymmetric(const Vector3 vector) {
				data[0] = data[4] = data[8] = 0;
				data[1] = -vector.z;
				data[2] = vector.y;
				data[3] = vector.z;
				data[5] = -vector.x;
				data[6] = -vector.y;
				data[7] = vector.x;
			}

			Matrix3 transpose() const {
				return Transpose(*this);
			}

			Matrix3 inverse() const {
				return Inverse(*this);
			}

			//static Matrix3 linearInterpolate(const Matrix3& a, const Matrix3& b, float prop) {
			//	Matrix3 result;
			//	for (unsigned i = 0; i < 9; i++) {
			//		result.data[i] = a.data[i] * (1 - prop) + b.data[i] * prop;
			//	}
			//	return result;
			//}

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
