export module glmath:Vector4;

import :Vector3;

export namespace IKIGAI::MATHGL {
	struct Vector4 {
		float x = 0.0f, y = 0.0f, z = 0.0f, w = 0.0f;

		Vector4();
		Vector4(const float f);
		Vector4(const Vector3& vec, const float f);
		Vector4(const float x, const float y, const float z, const float w);

		float& operator[](const unsigned index);
		const float& operator[](const unsigned index) const;
		Vector4& operator=(const Vector4& right);

		float dot(const Vector4& V2);
		float length();
		Vector4 normalize();

		bool operator==(const Vector4& V2);
		bool operator!=(const Vector4& V2);
		Vector4 operator-();
		void operator+=(const Vector4& V2);
		void operator-=(const Vector4& V2);
		Vector4 operator+(const Vector4& V2);
		Vector4 operator-(const Vector4& V2);
		Vector4 operator*(const Vector4& V2);
		Vector4 operator*(const float f) const;
		Vector4 operator/(const float f) const;

		static float Dot(const Vector4& V1, const Vector4& V2);
		static float Length(const Vector4& V);
		static Vector4 Normalize(const Vector4& V);
	};

	Vector4 operator*(const float f, const Vector4& V);
	Vector4 operator/(const float f, const Vector4& V);
}
