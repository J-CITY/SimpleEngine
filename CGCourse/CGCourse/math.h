#pragma once
#include <array>
#include <ostream>
#include <vector>
#include <cmath>

namespace Mathgl {

	# define M_PI           3.14159265358979323846

	template <size_t M, size_t N>
	class Matrix {
	protected:
		std::array<std::array<float, N>, M> data;

	public:
		
		Matrix(): arr() {
			for (int i = 0; i < M; ++i) {
				for (int j = 0; j < N; ++j) {
					data[i][j] = 0;
				}
			}
		}

		Matrix(float f): arr() {
			if (M != N) {
				throw std::invalid_argument("Matrix dimension must be the same");
			}
			Matrix<M, N>();

			for (int i = 0; i < M; ++i) {
				data[i][i] = f;
			}
		}

		Matrix(std::initializer_list<std::initializer_list<float>> init) : arr() {
			if (init.size() != M) {
				throw std::invalid_argument("Matrix dimension must be the same");;
			}
			for (int i = 0; i < M; ++i) {
				if ((init_list.begin() + i)->size() != N)
					throw;

				for (int j = 0; j < N; ++j)
					data[i][j] = *((init.begin() + i)->begin() + j);
			}
		}
		
		std::array<float, N>& operator[](const int index) { return data[index]; }
		const std::array<float, N>& operator[](const int index) const { return data[index]; }
	};


	template <int M, int N>
	inline Matrix<M, N> operator+(const Matrix<M, N>& M1, Matrix<M, N>& M2) {
		Matrix<M, N> out;

		for (int i = 0; i < M; ++i) {
			for (int j = 0; j < N; ++j) {
				out[i][j] = M1[i][j] + M2[i][j];
			}
		}

		return out;
	}

	template <int M, int N>
	inline Matrix<M, N> operator-(const Matrix<M, N>& M1, const Matrix<M, N>& M2) {
		Matrix<M, N> out;

		for (int i = 0; i < M; ++i) {
			for (int j = 0; j < N; ++j) {
				out[i][j] = M1[i][j] - M2[i][j];
			}
		}
		return out;
	}

	template <int M, int N>
	inline Matrix<M, N> operator*(const float f, const Matrix<M, N>& mat) {
		Matrix<M, N> out;

		for (int i = 0; i < M; ++i) {
			for (int j = 0; j < N; ++j) {
				out[i][j] = mat[i][j] * f;
			}
		}
		return out;
	}

	template <int M, int N>
	inline Matrix<M, N> operator*(const Matrix<M, N>& mat, const float f) {
		return operator*(f, mat);
	}

	template <int M, int N>
	inline Matrix<N, M> operator*(const Matrix<N, M>& M1, const Matrix<N, M>& M2) {
		Matrix<M, N> out;

		for (int i = 0; i < M; ++i) {
			for (int j = 0; j < N; ++j) {
				for (int k = 0; k < M; ++k) {
					out[i][j] += M1[i][k] * M2[k][j];
				}
			}
		}
		return out;
	}

	template <int M, int N>
	inline Matrix<N, M> T(const Matrix<M, N>& mat) {
		return transpose(mat);
	}
	
	template <int M, int N>
	inline Matrix<N, M> transpose(const Matrix<M, N>& mat) {
		Matrix<N, M> transposed;

		for (int i = 0; i < N; ++i) {
			for (int j = 0; j < M; ++j) {
				transposed[i][j] = mat[j][i];
			}
		}
		return transposed;
	}

	template <int M, int N>
	inline std::ostream& operator<<(std::ostream& out, const Matrix<M, N>& mat) {
		for (int i = 0; i < M; ++i) {
			for (int j = 0; j < N; ++j) {
				out << mat[i][j] << " ";
			}
			out << '\n';
		}
		return out;
	}

	template <int M, int N>
	inline const float* value_ptr(const Matrix<M, N>& mat) {
		return &(mat[0][0]);
	}

	struct Vector2 {
		float x = 0.0f, y = 0.0f;
		
		Vector2() {
		}

		Vector2(const float f): x(f), y(f) {
		}

		Vector2(const float x, const float y): x(x), y(y) {
		}

		float& operator[](const int index) { return *(&x + index); }
		const float& operator[](const int index) const { return *(&x + index); }
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

	inline Vector2 operator*(const Matrix<2, 2>& M, const Vector2& V) {
		Vector2 out;

		for (int i = 0; i < 2; ++i) {
			for (int j = 0; j < 2; ++j) {
				out[j] += M[i][j] * V[j];
			}
		}

		return out;
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

	struct Vector3 {
		float x = 0.0f, y = 0.0f, z = 0.0f;


		Vector3() {
		}

		Vector3(const float f): x(f), y(f), z(f) {
		}

		Vector3(const float x, const float y, const float z): x(x), y(y), z(z) {
		}

		inline float& operator[](const int index) { return *(&x + index); }
		inline const float& operator[](const int index) const { return *(&x + index); }
	};


	inline bool operator==(const Vector3& V1, const Vector3& V2) {
		return V1.x == V2.x && V1.y == V2.y && V1.z == V2.z;
	}

	inline bool operator!=(const Vector3& V1, const Vector3& V2) {
		return !operator==(V1, V2);
	}

	inline Vector3 operator-(const Vector3& V) {
		return Vector3(-V.x, -V.y, -V.z);
	}

	inline void operator+=(Vector3& V1, const Vector3& V2) {
		for (int i = 0; i < 3; ++i)
			V1[i] += V2[i];
	}

	inline void operator-=(Vector3& V1, const Vector3& V2) {
		operator+=(V1, -V2);
	}

	inline Vector3 operator+(const Vector3& V1, const Vector3& V2) {
		return Vector3(V1.x + V2.x, V1.y + V2.y, V1.z + V2.z);
	}

	inline Vector3 operator-(const Vector3& V1, const Vector3& V2) {
		return Vector3(V1.x - V2.x, V1.y - V2.y, V1.z - V2.z);
	}

	inline Vector3 operator*(const Vector3& V, const float f) {
		return Vector3(V.x * f, V.y * f, V.z * f);
	}

	inline Vector3 operator*(const Vector3& V1, const Vector3& V2) {
		return Vector3(V1.x * V2.x, V1.y * V2.y, V1.z * V2.z);
	}

	inline Vector3 operator*(const float f, const Vector3& V) {
		return operator*(V, f);
	}

	inline Vector3 operator/(const Vector3& V, const float f) {
		return Vector3(V.x / f, V.y / f, V.z / f);
	}

	inline Vector3 operator/(const float f, const Vector3& V) {
		return operator/(V, f);
	}

	inline float dot(const Vector3& V1, const Vector3& V2) {
		return V1.x * V2.x + V1.y * V2.y + V1.z * V2.z;
	}

	inline Vector3 cross(const Vector3& v1, const Vector3& v2) {
		Vector3 out;
		out.x = v1.y * v2.z - v1.z * v2.y;
		out.y = v1.z * v2.x - v1.x * v2.z;
		out.z = v1.x * v2.y - v1.y * v2.x;

		return out;
	}

	inline float length(const Vector3& V) {
		return sqrt(pow(V.x, 2) + pow(V.y, 2) + pow(V.z, 2));
	}

	inline Vector3 normalize(const Vector3& V) {
		return V / length(V);
	}

	inline Vector3 operator*(const Matrix<3, 3>& M, const Vector3& V) {
		Vector3 out;

		for (int i = 0; i < 3; ++i)
			for (int j = 0; j < 3; ++j)
				out[j] += M[i][j] * V[j];

		return out;
	}

	inline std::ostream& operator<<(std::ostream& out, const Vector3& V) {
		out << "( ";
		for (int i = 0; i < 3; ++i) {
			out << V[i];
			if (i != 2)
				out << ", ";
		}
		out << " )";

		return out;
	}

	struct Vector4 {
		float x = 0.0f, y = 0.0f, z = 0.0f, w = 0.0f;

		Vector4() {

		}

		Vector4(const float f): x(f), y(f), z(f), w(f) {

		}

		Vector4(const float x, const float y, const float z, const float w): x(x), y(y), z(z), w(w) {
		}

		inline float& operator[](const int index) { return *(&x + index); }
		inline const float& operator[](const int index) const { return *(&x + index); }
	};


	inline bool operator==(const Vector4& V1, const Vector4& V2) {
		return V1.x == V2.x && V1.y == V2.y && V1.z == V2.z && V1.w == V2.w;
	}

	inline bool operator!=(const Vector4& V1, const Vector4& V2) {
		return !operator==(V1, V2);
	}

	inline Vector4 operator-(const Vector4& V) {
		return Vector4(-V.x, -V.y, -V.z, -V.w);
	}

	inline void operator+=(Vector4& V1, const Vector4& V2) {
		for (int i = 0; i < 4; ++i)
			V1[i] += V2[i];
	}

	inline void operator-=(Vector4& V1, const Vector4& V2) {
		operator+=(V1, -V2);
	}

	inline Vector4 operator+(const Vector4& V1, const Vector4& V2) {
		return Vector4(V1.x + V2.x, V1.y + V2.y, V1.z + V2.z, V1.w + V2.w);
	}

	inline Vector4 operator-(const Vector4& V1, const Vector4& V2) {
		return Vector4(V1.x - V2.x, V1.y - V2.y, V1.z - V2.z, V1.w - V2.w);
	}

	inline Vector4 operator*(const Vector4& V1, const Vector4& V2) {
		return Vector4(V1.x * V2.x, V1.y * V2.y, V1.z * V2.z, V1.w * V2.w);
	}

	inline Vector4 operator*(const Vector4& V, const float f) {
		return Vector4(V.x * f, V.y * f, V.z * f, V.w * f);
	}

	inline Vector4 operator*(const float f, const Vector4& V) {
		return operator*(V, f);
	}

	inline Vector4 operator/(const Vector4& V, const float f) {
		return Vector4(V.x / f, V.y / f, V.z / f, V.w / f);
	}

	inline Vector4 operator/(const float f, const Vector4& V) {
		return operator/(V, f);
	}

	inline float dot(const Vector4& V1, const Vector4& V2) {
		return V1.x * V2.x + V1.y * V2.y + V1.z * V2.z + V1.w * V2.w;
	}

	inline float length(const Vector4& V) {
		return sqrt(pow(V.x, 2) + pow(V.y, 2) + pow(V.z, 2) + pow(V.w, 2));
	}

	inline Vector4 normalize(const Vector4& V) {
		return V / length(V);
	}

	inline Vector4 operator*(const Matrix<4, 4>& M, const Vector4& V) {
		Vector4 out;

		for (int i = 0; i < 4; ++i)
			for (int j = 0; j < 4; ++j)
				out[i] += M[i][j] * V[j];

		return out;
	}

	inline std::ostream& operator<<(std::ostream& out, const Vector4& V) {
		out << "( ";
		for (int i = 0; i < 4; ++i) {
			out << V[i];
			if (i != 3)
				out << ", ";
		}
		out << " )";

		return out;
	}

	using Mat4 = Matrix<4, 4>;
	using Mat3 = Matrix<3, 3>;
	using Mat2 = Matrix<2, 2>;

	using Vec2 = Vector2;
	using Vec3 = Vector3;
	using Vec4 = Vector4;

	Mat4 Eye4() {
		auto res = Mat4();
		res[0][0] = 0.0f;
		res[1][1] = 0.0f;
		res[2][2] = 0.0f;
		res[3][3] = 0.0f;
	}

	Mat3 Eye3() {
		auto res = Mat3();
		res[0][0] = 0.0f;
		res[1][1] = 0.0f;
		res[2][2] = 0.0f;
	}
	
	float radians(float degree) {
		return degree * M_PI / 180.0f;
	}

	inline float cos(float angle) {
		return cosf(angle);
	}

	inline float sin(float angle) {
		return sinf(angle);
	}

	inline float tan(float angle) {
		return tanf(angle);
	}

	inline Matrix<4, 4> ortho(float right, float top, float near, float far) {
		Matrix<4, 4> mat;

		mat[0][0] = 1.f / right;
		mat[1][1] = 1.f / top;
		mat[2][2] = -2 / (far - near);
		mat[2][3] = (far + near) / (near - far);
		mat[3][3] = 1;

		return mat;
	}

	inline Matrix<4, 4> perspective(float fov, float ratio, float near, float far) {
		float right = tan(fov / 2.f) * near;
		float top = right / ratio;

		Matrix<4, 4> mat;
		mat[0][0] = near / right;
		mat[1][1] = near / top;
		mat[2][2] = -(far + near) / (far - near);
		mat[2][3] = -(2 * far * near) / (far - near);
		mat[3][2] = -1;

		return mat;
	}


	inline Matrix<4, 4> lookAt(const Vector3& eye, const Vector3& target, const Vector3& worldUp) {
		Vector3 camera_front = normalize(eye - target);
		Vector3 camera_right = normalize(cross(normalize(worldUp), camera_front));
		Vector3 camera_up = cross(camera_front, camera_right);

		Matrix<4, 4> camera_translation(1.f);
		camera_translation[0][3] = -eye.x;
		camera_translation[1][3] = -eye.y;
		camera_translation[2][3] = -eye.z;

		Matrix<4, 4> camera_rotation(1.f);
		for (int i = 0; i < 3; ++i)
			camera_rotation[i][0] = camera_right[i];
		for (int i = 0; i < 3; ++i)
			camera_rotation[i][1] = camera_up[i];
		for (int i = 0; i < 3; ++i)
			camera_rotation[i][2] = camera_front[i];

		return transpose(camera_rotation) * camera_translation;
	}

	inline Matrix<4, 4> scale(const Matrix<4, 4>& mat, const Vector3& vec) {
		Matrix<4, 4> scale_mat(1.f);
		for (int i = 0; i < 3; ++i)
			scale_mat[i][i] = vec[i];

		return mat * scale_mat;

	}

	inline Matrix<4, 4> translate(const Matrix<4, 4>& mat, const Vector3& trans_vec) {
		Matrix<4, 4> trans_mat(1.f);
		for (int i = 0; i < 3; ++i)
			trans_mat[i][3] = trans_vec[i];

		return mat * trans_mat;
	}

	inline Matrix<4, 4> rotate(const Matrix<4, 4>& mat, const Vector3& vec, const float angle) {
		Vector3 R = normalize(vec);

		Matrix<4, 4> rot_mat;
		float a = radians(angle);
		float c = cos(a);
		float s = sin(a);

		rot_mat = {
		{ c + R.x * R.x * (1 - c), R.x * R.y * (1 - c) - R.z * s, R.x * R.z * (1 - c) + R.y * s, 0 },
			{ R.y * R.x * (1 - c) + R.z * s, c + R.y * R.y * (1 - c), R.y * R.z * (1 - c) - R.x * s, 0 },
			{ R.z * R.x * (1 - c) - R.y * s, R.z * R.y * (1 - c) + R.x * s, c + R.z * R.z * (1 - c), 0 },
			{ 0, 0, 0, 1 }
		};

		return mat * rot_mat;
	}
}


