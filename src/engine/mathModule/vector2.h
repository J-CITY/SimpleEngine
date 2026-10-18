#pragma once
// Lightweight canonical Vector2 math. Include math.h when serde is needed.
#include <cmath>
#include <stdexcept>

namespace IKIGAI::MATH {
constexpr int math_max(int a) {
	return a;
}
template <typename... Args> constexpr int math_max(int a, Args... args) {
	int b = math_max(args...);
	return a > b ? a : b;
}

template <template <typename> class TT, typename T, int... Indexes> class Swizzle {
	T v[math_max(Indexes...) + 1];

public:
	TT<T>& operator=(const TT<T>& rhs) {
		int indexes[] = { Indexes... }; // unpack
		for (int i = 0; i < sizeof...(Indexes); i++) {
			v[indexes[i]] = rhs[i]; // access pack element
		}
		return *reinterpret_cast<TT<T>*>(this);
	}
	operator TT<T>() const {
		// unpack
		return TT<T>(v[Indexes]...);
	}
};

template <typename T> struct Vector2 {
	union {
		T data[2];
		struct {
			T x, y;
		};
		struct {
			T width, heights;
		};
		Swizzle<Vector2, T, 0, 0> xx;
		Swizzle<Vector2, T, 0, 0> rr;
		Swizzle<Vector2, T, 0, 1> xy;
		Swizzle<Vector2, T, 0, 1> rg;
		Swizzle<Vector2, T, 1, 0> yx;
		Swizzle<Vector2, T, 1, 0> gr;
		Swizzle<Vector2, T, 1, 1> yy;
		Swizzle<Vector2, T, 1, 1> gg;
	};
	Vector2<T>()
		: x(0)
		, y(0) {};
	Vector2<T>(T _v)
		: x(_v)
		, y(_v) {};
	Vector2<T>(T _x, T _y)
		: x(_x)
		, y(_y) {};
	operator T*() {
		return data;
	};
	operator const T*() const {
		return static_cast<const T*>(data);
	};

	T getX() {
		return x;
	}
	T getY() {
		return y;
	}
	void setX(T v) {
		x = v;
	}
	void setY(T v) {
		y = v;
	}

	T& operator[](const int index) {
		switch (index) {
		case 0:
			return x;
		case 1:
			return y;
		default:
			break;
		}
		throw std::out_of_range("Vector2: out of range");
	}
	const T& operator[](const int index) const {
		switch (index) {
		case 0:
			return x;
		case 1:
			return y;
		default:
			break;
		}
		throw std::out_of_range("Vector2: out of range");
	}

	Vector2& operator=(const Vector2& right) {
		if (this == &right) {
			return *this;
		}
		x = right.x;
		y = right.y;
		return *this;
	}

	inline bool operator==(const Vector2& V2) const {
		return this->x == V2.x && this->y == V2.y;
	}

	inline bool operator!=(const Vector2& V2) const {
		return !operator==(V2);
	}

	inline Vector2 operator-() const {
		return Vector2(-this->x, -this->y);
	}

	inline void operator+=(const Vector2& V2) {
		for (int i = 0; i < 2; ++i) {
			(*this)[i] += V2[i];
		}
	}

	inline void operator-=(const Vector2& V2) {
		for (int i = 0; i < 2; ++i) {
			(*this)[i] -= V2[i];
		}
	}

	inline Vector2 operator+(const Vector2& V2) const {
		return Vector2(this->x + V2.x, this->y + V2.y);
	}

	inline Vector2 operator-(const Vector2& V2) const {
		return Vector2(this->x - V2.x, this->y - V2.y);
	}

	inline Vector2 operator*(const Vector2& V2) const {
		return Vector2(this->x * V2.x, this->y * V2.y);
	}

	inline Vector2 operator*(float f) const {
		return Vector2(this->x * f, this->y * f);
	}

	inline Vector2& operator*=(float scalar) {
		x *= scalar;
		y *= scalar;
		return *this;
	}

	inline Vector2 operator/(float f) const {
		return Vector2(this->x / f, this->y / f);
	}

	inline float dot(const Vector2& V2) const {
		return this->x * V2.x + this->y * V2.y;
	}

	inline float length() const {
		return std::sqrt(std::pow(x, 2) + std::pow(y, 2));
	}

	inline Vector2 normalize() const {
		return *this / length();
	}

	void clear() {
		x = y = 0;
	}

	// Serialization is defined by math.h; this header has no serde dependency.
	template <class Context> constexpr static auto serde(Context& context, Vector2& value);
};

template <class T> Vector2<T> operator*(const float f, const Vector2<T>& V) {
	return Vector2<T>(V.x * f, V.y * f);
}

using Vector2i = Vector2<int>;
using Vector2u = Vector2<unsigned>;
using Vector2f = Vector2<float>;
using Vector2d = Vector2<double>;

}
