export;
#include <stdexcept>

export module glmath:Matrix4;

import :Vector3;
import :Vector4;
import customarray;

export namespace IKIGAI::MATHGL {
	struct Matrix4 {
		Array<float, 16> data;

		Matrix4();
		Matrix4(float val);
		Matrix4(float element1, float element2, float element3, float element4,
			float element5, float element6, float element7, float element8,
			float element9, float element10, float element11, float element12,
			float element13, float element14, float element15, float element16);
		Matrix4(Array<float, 16>&& in);
		Matrix4(const Matrix4& in);
		~Matrix4() = default;

		float* getData();

		Vector3 transformInverseDirection(const Vector3& vector) const;
		Vector3 transform(const Vector3& vector) const;
		Vector3 transformInverse(const Vector3& vector) const;
		Vector3 transformDirection(const Vector3& vector) const;
		Vector3 getAxisVector(int i) const;
		void setRow(uint8_t row, const Vector4& vec);
		void setCol(uint8_t row, const Vector4& vec);
		Vector4 getRow(uint8_t row);
		Vector4 getCol(uint8_t row);

		Vector3 operator*(const Vector3& vector) const;
		Matrix4& operator=(const Matrix4& other);
		bool operator==(const Matrix4& other);
		Matrix4 operator+(const Matrix4& other) const;
		Matrix4& operator+=(const Matrix4& other);
		Matrix4 operator-(float scalar) const;
		Matrix4 operator-(const Matrix4& other) const;
		Matrix4& operator-=(const Matrix4& other);
		Matrix4 operator*(float scalar) const;
		Matrix4& operator*=(float scalar);
		Vector4 operator*(const Vector4& vector) const;
		Matrix4 operator*(const Matrix4& other) const;
		Matrix4& operator*=(const Matrix4& other);
		Matrix4 operator/(float scalar) const;
		Matrix4& operator/=(float scalar);
		Matrix4 operator/(const Matrix4& other) const;
		Matrix4& operator/=(const Matrix4& other);
		float& operator()(uint8_t row, uint8_t column);

		static Matrix4 MakeBiasMatrix();
		static Matrix4 Scaling(const Vector3& scale);
		static Matrix4 Translation(const Vector3& translation);
		static bool AreEquals(const Matrix4& left, const Matrix4& right);
		static Matrix4 Add(const Matrix4& left, float scalar);
		static Matrix4 Add(const Matrix4& left, const Matrix4& right);
		static Matrix4 Subtract(const Matrix4& left, float scalar);
		static Matrix4 Subtract(const Matrix4& left, const Matrix4& right);
		static Matrix4 Multiply(const Matrix4& left, float scalar);
		static Vector4 Multiply(const Matrix4& matrix, const Vector4& vector);
		static Matrix4 Multiply(const Matrix4& left, const Matrix4& right);
		static Matrix4 Divide(const Matrix4& left, float scalar);
		static Matrix4 Divide(const Matrix4& left, const Matrix4& right);
		static bool IsIdentity(const Matrix4& matrix);
		static float GetMinor(float minor0, float minor1, float minor2,
			float minor3, float minor4, float minor5,
			float minor6, float minor7, float minor8);
		static float Determinant(const Matrix4& matrix);
		static Matrix4 Transpose(const Matrix4& matrix);
		static Matrix4 Inverse(const Matrix4& matrix);
		static Matrix4 CreatePerspective(const float fov, const float aspectRatio, const float zNear, const float zFar);
		static Matrix4 CreateOrthographic(const float left, const float right, const float bottom, const float top, const float zNear, const float zFar);
		static Matrix4 CreateOrthographic(const float size, const float aspectRatio, const float zNear, const float zFar);
		static Matrix4 CreateView(const float eyeX, const float eyeY, const float eyeZ, const float lookX, const float lookY, const float lookZ, const float upX, const float upY, const float upZ);
		static Matrix4 CreateView(Vector3 eye, Vector3 look, Vector3 up);
		static Matrix4 CreateFrustum(const float left, const float right, const float bottom, const float top, const float zNear, const float zFar);

		static const Matrix4 Identity;
	};

	Matrix4 operator*(const float f, const Matrix4& V);
	Matrix4 operator/(const float f, const Matrix4& V);
}
