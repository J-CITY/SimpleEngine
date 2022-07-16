export;

#include <cmath>
#include <stdexcept>

export module glmath:Vector2;

export namespace KUMA::MATHGL {
	template<typename T = float>
	struct Vector2 {
		T x = 0.0f, y = 0.0f;

		Vector2() {
			static_assert(std::is_arithmetic<T>::value, "T is not number");
		};
		Vector2(const T f) : x(f), y(f) {
			Vector2::Vector2();
		}
		Vector2(const T x, const T y) : x(x), y(y) {
			Vector2::Vector2();
		}

		T& operator[](const int index) {
			if (index > 1) {
				throw std::logic_error("Index > 3");
			}
			switch (index) {
			case 0: return x;
			case 1: return y;
			}
		}
		const T& operator[](const int index) const {
			if (index > 1) {
				throw std::logic_error("Index > 3");
			}
			switch (index) {
			case 0: return x;
			case 1: return y;
			}
		}

		Vector2& operator=(const Vector2& right) {
			if (this == &right) {
				return *this;
			}
			x = right.x;
			y = right.y;
			return *this;
		}

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
	};

	using Vector2i = Vector2<int>;
	using Vector2u = Vector2<unsigned>;
	using Vector2f = Vector2<float>;
}
