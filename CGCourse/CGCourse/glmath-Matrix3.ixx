export;
#include <cmath>
#include <stdexcept>
#include <array>
export module glmath:Matrix3;

import :Vector3;
import customarray;

export namespace KUMA::MATHGL {
	struct Matrix3 {
		static const Matrix3 Identity;
		Array<float, 9> data;
		//std::array<float, 9> data;

		Matrix3();
		Matrix3(float val);
		Matrix3(float element1, float element2, float element3,
			float element4, float element5, float element6,
			float element7, float element8, float element9);
		Matrix3(Vector3 a, Vector3 b, Vector3 c);

		Matrix3(Array<float, 9>&& in) {
			data = in;
		};
		Matrix3(const Matrix3& in);

		Vector3 transform(const Vector3& vector) const;
		void setInertiaTensorCoeffs(float ix, float iy, float iz,
			float ixy = 0, float ixz = 0, float iyz = 0);
		void setBlockInertiaTensor(const Vector3& halfSizes, float mass);
		void setInverse(const Matrix3& m);
		void setComponents(const Vector3& compOne, const Vector3& compTwo,
			const Vector3& compThree);
		Vector3 transformTranspose(const Vector3& vector) const;
		void setSkewSymmetric(const Vector3 vector);
		Matrix3 transpose() const;
		Matrix3 inverse() const;
		static Matrix3 LinearInterpolate(const Matrix3& a, const Matrix3& b, float prop);

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
