export module glmath:Constants;

export constexpr float PI = 3.14159265359f;
export constexpr float EPSILON = 0.00001f;

export template<typename T>
constexpr auto TO_RADIANS(const T& value) {
	return value * PI / 180.f;
}

export template<typename T>
constexpr auto TO_DEGREES(const T& value) {
	return value * 180.f / PI;
}


