export module glmath:Vector3;


export namespace KUMA::MATHGL {
	struct Vector3 {
		static const Vector3 One;
		static const Vector3 Zero;
		static const Vector3 Forward;
		static const Vector3 Right;
		static const Vector3 Up;

		float x = 0.0f;
		float y = 0.0f;
		float z = 0.0f;

		Vector3(float p_x = 0.0f, float p_y = 0.0f, float p_z = 0.0f);
		Vector3(const Vector3& p_toCopy);
		Vector3(Vector3&& toMove) noexcept = default;

		void clear();
		void addScaledVector(const Vector3& vector, float scale);
		float dot(const Vector3& vector) const;
		Vector3 componentProduct(const Vector3& vector) const;
		Vector3 cross(const Vector3& vector) const;
		float magnitude() const;
		float squareMagnitude() const;
		void normalise();
		void componentProductUpdate(const Vector3& vector);

		Vector3 operator-() const;
		Vector3 operator=(const Vector3& p_other);
		Vector3 operator+(const Vector3& p_other) const;
		Vector3& operator+=(const Vector3& p_other);
		Vector3 operator-(const Vector3& p_other) const;
		Vector3& operator-=(const Vector3& p_other);
		Vector3 operator*(float p_scalar) const;
		Vector3 operator*(const Vector3& vec) const;
		Vector3& operator*=(float p_scalar);
		Vector3 operator/(float p_scalar) const;
		Vector3& operator/=(float p_scalar);
		//bool operator==(const Vector3& p_other);
		//bool operator!=(const Vector3& p_other);
		float& operator[](unsigned i);
		float operator[](unsigned i) const;

		static Vector3 Add(const Vector3& p_left, const Vector3& p_right);
		static Vector3 Substract(const Vector3& p_left, const Vector3& p_right);
		static Vector3 Multiply(const Vector3& p_target, float p_scalar);
		static Vector3 Divide(const Vector3& p_left, float p_scalar);

		static float Length(const Vector3& p_target);
		static float LengthSqrt(const Vector3& p_target);
		static float Dot(const Vector3& p_left, const Vector3& p_right);
		static float Distance(const Vector3& p_left, const Vector3& p_right);
		static Vector3 Cross(const Vector3& p_left, const Vector3& p_right);
		static Vector3 Normalize(const Vector3& p_target);
		static Vector3 Lerp(const Vector3& p_start, const Vector3& p_end, float p_alpha);
		static Vector3 Mix(const Vector3& x, const Vector3& y, float p_alpha);
		static float AngleBetween(const Vector3& p_from, const Vector3& p_to);
	};

	Vector3 operator*(const float f, const Vector3& V);
	Vector3 operator/(const float f, const Vector3& V);

	bool operator==(const Vector3& left, const Vector3& right);
	bool operator!=(const Vector3& left, const Vector3& right);
}
