#pragma once
#include <cmath>
#include <ostream>

struct Vector2 {
	float x = 0.0f, y = 0.0f;

	Vector2() = default;
	Vector2(const float f) : x(f), y(f) {}
	Vector2(const float x, const float y) : x(x), y(y) {}

	float& operator[](const int index) { return *(&x + index); }
	const float& operator[](const int index) const { return *(&x + index); }

	Vector2& operator=(const Vector2& right) {
		if (this == &right) {
			return *this;
		}
		x = right.x;
		y = right.y;
		return *this;
	}
};


inline bool operator==(const Vector2& V1, const Vector2& V2) {
	return V1.x == V2.x && V1.y == V2.y;
}

inline bool operator!=(const Vector2& V1, const Vector2& V2) {
	return !operator==(V1, V2);
}

inline Vector2 operator-(const Vector2& V) {
	return Vector2(-V.x, -V.y);
}

inline void operator+=(Vector2& V1, const Vector2& V2) {
	for (int i = 0; i < 2; ++i)
		V1[i] += V2[i];
}

inline void operator-=(Vector2& V1, const Vector2& V2) {
	operator+=(V1, -V2);
}

inline Vector2 operator+(const Vector2& V1, const Vector2& V2) {
	return Vector2(V1.x + V2.x, V1.y + V2.y);
}

inline Vector2 operator-(const Vector2& V1, const Vector2& V2) {
	return Vector2(V1.x - V2.x, V1.y - V2.y);
}

inline Vector2 operator*(const Vector2& V1, const Vector2& V2) {
	return Vector2(V1.x * V2.x, V1.y * V2.y);
}

inline Vector2 operator*(const Vector2& V, const float f) {
	return Vector2(V.x * f, V.y * f);
}

inline Vector2 operator*(const float f, const Vector2& V) {
	return operator*(V, f);
}

inline Vector2 operator/(const Vector2& V, const float f) {
	return Vector2(V.x / f, V.y / f);
}

inline Vector2 operator/(const float f, const Vector2& V) {
	return operator/(V, f);
}

inline float dot(const Vector2& V1, const Vector2& V2) {
	return V1.x * V2.x + V1.y * V2.y;
}

inline float length(const Vector2& V) {
	return sqrt(pow(V.x, 2) + pow(V.y, 2));
}

inline Vector2 normalize(const Vector2& V) {
	return V / length(V);
}

inline std::ostream& operator<<(std::ostream& out, const Vector2& V) {
	out << "( ";
	for (int i = 0; i < 2; ++i) {
		out << V[i];
		if (i != 1)
			out << ", ";
	}
	out << " )";

	return out;
}