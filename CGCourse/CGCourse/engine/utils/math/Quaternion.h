#pragma once
#include "Matrix4.h"
#include "Vector3.h"

namespace SE {
	namespace Mathgl {
		struct Quaternion {
			float x = 0.0f;
			float y = 0.0f;
			float z = 0.0f;
			float w = 0.0f;

			//static const Quaternion Identity;

			Quaternion() = default;
			Quaternion(float in) : w(in) {}
			Quaternion(float x, float y, float z, float w): x(x), y(y), z(z), w(w) {};
			Quaternion(const Quaternion& in): x(in.x), y(in.y), z(in.z), w(in.w) {}

			Quaternion(const Matrix3& p_rotationMatrix) {
				float trace = p_rotationMatrix.data[0] + p_rotationMatrix.data[4] + p_rotationMatrix.data[8];
				if (trace > 0.0f) {
					// I changed M_EPSILON to 0
					float s = 0.5f / sqrt(trace + 1.0f);
					w = 0.25f / s;
					x = (p_rotationMatrix.data[7] - p_rotationMatrix.data[5]) * s;
					y = (p_rotationMatrix.data[2] - p_rotationMatrix.data[6]) * s;
					z = (p_rotationMatrix.data[3] - p_rotationMatrix.data[1]) * s;
				}
				else {
					if (p_rotationMatrix.data[0] > p_rotationMatrix.data[4] && p_rotationMatrix.data[0] > p_rotationMatrix.data[8]) {
						float s = 2.0f * sqrt(1.0f + p_rotationMatrix.data[0] - p_rotationMatrix.data[4] - p_rotationMatrix.data[8]);
						w = (p_rotationMatrix.data[7] - p_rotationMatrix.data[5]) / s;
						x = 0.25f * s;
						y = (p_rotationMatrix.data[1] + p_rotationMatrix.data[3]) / s;
						z = (p_rotationMatrix.data[2] + p_rotationMatrix.data[6]) / s;
					}
					else if (p_rotationMatrix.data[4] > p_rotationMatrix.data[8]) {
						float s = 2.0f * sqrt(1.0f + p_rotationMatrix.data[4] - p_rotationMatrix.data[0] - p_rotationMatrix.data[8]);
						w = (p_rotationMatrix.data[2] - p_rotationMatrix.data[6]) / s;
						x = (p_rotationMatrix.data[1] + p_rotationMatrix.data[3]) / s;
						y = 0.25f * s;
						z = (p_rotationMatrix.data[5] + p_rotationMatrix.data[7]) / s;
					}
					else {
						float s = 2.0f * sqrt(1.0f + p_rotationMatrix.data[8] - p_rotationMatrix.data[0] - p_rotationMatrix.data[4]);
						w = (p_rotationMatrix.data[3] - p_rotationMatrix.data[1]) / s;
						x = (p_rotationMatrix.data[2] + p_rotationMatrix.data[6]) / s;
						y = (p_rotationMatrix.data[5] + p_rotationMatrix.data[7]) / s;
						z = 0.25f * s;
					}
				}
			}

			Quaternion(const Matrix4& p_rotationMatrix) {
				float halfSquare;

				// Check diagonal (trace)
				const float trace = p_rotationMatrix.data[0] + p_rotationMatrix.data[5] + p_rotationMatrix.data[10];

				if (trace > 0.0f) {
					const float InvSquare = 1 / sqrt(trace + 1.f);
					w = 0.5f * (1.f / InvSquare);
					halfSquare = 0.5f * InvSquare;

					x = (p_rotationMatrix.data[6] - p_rotationMatrix.data[9]) * halfSquare;
					y = (p_rotationMatrix.data[8] - p_rotationMatrix.data[2]) * halfSquare;
					z = (p_rotationMatrix.data[1] - p_rotationMatrix.data[4]) * halfSquare;
				}
				else {
					// diagonal is negative
					int i = 0;

					if (p_rotationMatrix.data[5] > p_rotationMatrix.data[0])
						i = 1;

					if (p_rotationMatrix.data[10] > p_rotationMatrix.data[0] || p_rotationMatrix.data[10] > p_rotationMatrix.data[5])
						i = 2;

					static const int next[3] = {1, 2, 0};
					const int j = next[i];
					const int k = next[j];

					halfSquare = p_rotationMatrix.data[i * 5] - p_rotationMatrix.data[j * 5] - p_rotationMatrix.data[k * 5] + 1.0f;

					const float InvSquare = 1 / sqrt(trace + 1.f);

					float qt[4];
					qt[i] = 0.5f * (1.f / InvSquare);

					halfSquare = 0.5f * InvSquare;
					//if i is 0, j is 1 and k is 2
					if (i == 0) {
						qt[3] = (p_rotationMatrix.data[6] - p_rotationMatrix.data[9]) * halfSquare;
						qt[j] = (p_rotationMatrix.data[1] + p_rotationMatrix.data[4]) * halfSquare;
						qt[k] = (p_rotationMatrix.data[2] + p_rotationMatrix.data[8]) * halfSquare;
					}
					//if i is 1, j is 2 and k is 0
					else if (i == 1) {
						qt[3] = (p_rotationMatrix.data[8] - p_rotationMatrix.data[2]) * halfSquare;
						qt[j] = (p_rotationMatrix.data[6] + p_rotationMatrix.data[9]) * halfSquare;
						qt[k] = (p_rotationMatrix.data[4] + p_rotationMatrix.data[1]) * halfSquare;
					}
					//if i is 2, j is 0 and k is 1
					else {
						qt[3] = (p_rotationMatrix.data[1] - p_rotationMatrix.data[4]) * halfSquare;
						qt[j] = (p_rotationMatrix.data[8] + p_rotationMatrix.data[2]) * halfSquare;
						qt[k] = (p_rotationMatrix.data[9] + p_rotationMatrix.data[6]) * halfSquare;
					}
					x = qt[0];
					y = qt[1];
					z = qt[2];
					w = qt[3];
				}
			}
			Quaternion(const Vector3& p_euler) {
				/* Degree to radians then times 0.5f = 0.0087f */
				float yaw = TO_RADIANS(p_euler.z) * 0.5f;
				float pitch = TO_RADIANS(p_euler.y) * 0.5f;
				float roll = TO_RADIANS(p_euler.x) * 0.5f;

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

			static Quaternion LookAt(const Vector3& p_forward, const Vector3& p_up) {
				auto vector = Vector3::Normalize(p_forward);
				auto vector2 = Vector3::Normalize(Vector3::Cross(p_up, vector));
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

		};
	}
}
