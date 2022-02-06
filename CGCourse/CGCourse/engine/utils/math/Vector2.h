#pragma once
#include <cmath>
#include <ostream>

namespace KUMA {
	namespace MATHGL {
		struct Vector2 {
			float x = 0.0f, y = 0.0f;

			Vector2() = default;
			Vector2(const float f) : x(f), y(f) {}
			Vector2(const float x, const float y) : x(x), y(y) {}

			float& operator[](const int index);
			const float& operator[](const int index) const;
			Vector2& operator=(const Vector2& right);

			inline bool operator==(const Vector2& V2) {
				return this->x == V2.x && this->y == V2.y;
			}

			inline bool operator!=(const Vector2& V2) {
				return !operator==(V2);
			}

			inline Vector2 operator-() const {
				return Vector2(-this->x, -this->y);
			}

			inline void operator+=(const Vector2& V2) {
				for (int i = 0; i < 2; ++i)
					(*this)[i] += V2[i];
			}

			inline void operator-=(const Vector2& V2) {
				for (int i = 0; i < 2; ++i)
					(*this)[i] -= V2[i];
			}

			inline Vector2 operator+(const Vector2& V2) const {
				return Vector2(this->x + V2.x, this->y + V2.y);
			}

			inline Vector2 operator-(const Vector2& V2) const {
				return Vector2(this->x - V2.x, this->y - V2.y);
			}

			inline Vector2 operator*(const Vector2& V2) {
				return Vector2(this->x * V2.x, this->y * V2.y);
			}

			inline Vector2 operator*(const float f) {
				return Vector2(this->x * f, this->y * f);
			}

			inline Vector2 operator/(const float f) {
				return Vector2(this->x / f, this->y / f);
			}

			inline float dot(const Vector2& V2) {
				return this->x * V2.x + this->y * V2.y;
			}

			inline float length() {
				return sqrt(pow(x, 2) + pow(y, 2));
			}

			inline Vector2 normalize() {
				return *this / length();
			}

			inline std::ostream& operator<<(std::ostream& out) {
				out << "( ";
				for (int i = 0; i < 2; ++i) {
					out << (*this)[i];
					if (i != 1)
						out << ", ";
				}
				out << " )";

				return out;
			}
		};

	}
}
