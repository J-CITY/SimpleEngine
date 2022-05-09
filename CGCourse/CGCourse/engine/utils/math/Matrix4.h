#pragma once

#include <array>
#include "vector4.h"

namespace KUMA {
	namespace MATHGL {
		struct Vector3;
		struct Vector4;

		struct Matrix4 {
			std::array<float, 16> data;

			Matrix4() {
				data = Identity.data;
			}
			Matrix4(float val) {
				//for (auto& d : data) {
				//	d = val;
				//}
				data = {val, 0.f, 0.f, 0.f,
					0.f, val, 0.f, 0.f,
					0.f, 0.f, val, 0.f,
					0.f, 0.f, 0.f, val
				};
			}

			Matrix4(float element1, float element2, float element3, float element4,
				float element5, float element6, float element7, float element8,
				float element9, float element10, float element11, float element12,
				float element13, float element14, float element15, float element16) {
				data[0] = element1;
				data[1] = element2;
				data[2] = element3;
				data[3] = element4;
				data[4] = element5;
				data[5] = element6;
				data[6] = element7;
				data[7] = element8;
				data[8] = element9;
				data[9] = element10;
				data[10] = element11;
				data[11] = element12;
				data[12] = element13;
				data[13] = element14;
				data[14] = element15;
				data[15] = element16;
			}

			Matrix4(std::array<float, 16>&& in) {
				data = in;
			}

			Matrix4(const Matrix4& in) {
				*this = in;
			}
			~Matrix4() = default;

			static Matrix4 MakeBiasMatrix() {
				Matrix4 Result(
					0.5f, 0.0f, 0.0f, 0.0f,
					0.0f, 0.5f, 0.0f, 0.0f,
					0.0f, 0.0f, 0.5f, 0.0f,
					0.5f, 0.5f, 0.5f, 1.0f
				);
				return Result;
			}

			Vector3 transformInverseDirection(const Vector3& vector) const {
				return Vector3(
					vector.x * data[0] +
					vector.y * data[4] +
					vector.z * data[8],

					vector.x * data[1] +
					vector.y * data[5] +
					vector.z * data[9],

					vector.x * data[2] +
					vector.y * data[6] +
					vector.z * data[10]
				);
			}

			Vector3 transform(const Vector3& vector) const {
				return Vector3(
					vector.x * data[0] +
					vector.y * data[1] +
					vector.z * data[2] + data[3],

					vector.x * data[4] +
					vector.y * data[5] +
					vector.z * data[6] + data[7],

					vector.x * data[8] +
					vector.y * data[9] +
					vector.z * data[10] + data[11]
				);
			}

			Vector3 transformInverse(const Vector3& vector) const {
				Vector3 tmp = vector;
				tmp.x -= data[3];
				tmp.y -= data[7];
				tmp.z -= data[11];
				return Vector3(
					tmp.x * data[0] +
					tmp.y * data[4] +
					tmp.z * data[8],

					tmp.x * data[1] +
					tmp.y * data[5] +
					tmp.z * data[9],

					tmp.x * data[2] +
					tmp.y * data[6] +
					tmp.z * data[10]
				);
			}

			Vector3 transformDirection(const Vector3& vector) const {
				return Vector3(
					vector.x * data[0] +
					vector.y * data[1] +
					vector.z * data[2],

					vector.x * data[4] +
					vector.y * data[5] +
					vector.z * data[6],

					vector.x * data[8] +
					vector.y * data[9] +
					vector.z * data[10]
				);
			}
			Vector3 getAxisVector(int i) const {
				return Vector3(data[i], data[i + 4], data[i + 8]);
			}
			Vector3 operator*(const Vector3& vector) const {
				return Vector3(
					vector.x * data[0] +
					vector.y * data[1] +
					vector.z * data[2] + data[3],

					vector.x * data[4] +
					vector.y * data[5] +
					vector.z * data[6] + data[7],

					vector.x * data[8] +
					vector.y * data[9] +
					vector.z * data[10] + data[11]
				);
			}


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

			void setRow(uint8_t row, const Vector4& vec);
			void setCol(uint8_t row, const Vector4& vec);
			Vector4 getRow(uint8_t row);
			Vector4 getCol(uint8_t row);
			//Vector4 operator()(uint8_t row);

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
	}
}
