#include "matrix4.h"

#include <string>

using namespace KUMA;
using namespace KUMA::MATHGL;

#define PI 3.14159265359f
#define EPSILON 0.00001f

const Matrix4 Matrix4::Identity = Matrix4(1.f, 0.f, 0.f, 0.f,
	0.f, 1.f, 0.f, 0.f,
	0.f, 0.f, 1.f, 0.f,
	0.f, 0.f, 0.f, 1.f);

Matrix4& Matrix4::operator=(const Matrix4& other) {
	this->data = other.data;
	return *this;
}

bool Matrix4::operator==(const Matrix4& other) {
	return AreEquals(*this, other);
}

Matrix4 Matrix4::operator+(const Matrix4& other) const {
	return Add(*this, other);
}

Matrix4& Matrix4::operator+=(const Matrix4& other) {
	*this = Add(*this, other);
	return *this;
}

Matrix4 Matrix4::operator-(float scalar) const {
	return Subtract(*this, scalar);
}

Matrix4 Matrix4::operator-(const Matrix4& other) const {
	return Subtract(*this, other);
}

Matrix4& Matrix4::operator-=(const Matrix4& other) {
	*this = Subtract(*this, other);
	return *this;
}

Matrix4 Matrix4::operator*(float scalar) const {
	return Multiply(*this, scalar);
}

Matrix4& Matrix4::operator*=(float scalar) {
	*this = Multiply(*this, scalar);
	return *this;
}

Vector4 Matrix4::operator*(const Vector4& vector) const {
	return Multiply(*this, vector);
}

Matrix4 Matrix4::operator*(const Matrix4& other) const {
	return Multiply(*this, other);
}

Matrix4& Matrix4::operator*=(const Matrix4& other) {
	*this = Multiply(*this, other);
	return *this;
}

Matrix4 Matrix4::operator/(float scalar) const {
	return Divide(*this, scalar);
}

Matrix4& Matrix4::operator/=(float scalar) {
	*this = Divide(*this, scalar);
	return *this;
}

Matrix4 Matrix4::operator/(const Matrix4& other) const {
	return Divide(*this, other);
}

Matrix4& Matrix4::operator/=(const Matrix4& other) {
	*this = Divide(*this, other);
	return *this;
}
float& Matrix4::operator()(uint8_t row, uint8_t column) {
	if (row >= 4 || column >= 4)
		throw std::out_of_range(
			"Invalid index : " + std::to_string(row) + "," + std::to_string(column) + " is out of range");
	return data[4 * row + column];
}

void Matrix4::setRow(uint8_t row, const Vector4& vec) {
	if (row >= 4)
		throw std::out_of_range(
			"Invalid index : " + std::to_string(row) + " is out of range");

	data[4 * row] = vec.x;
	data[4 * row+1] = vec.y;
	data[4 * row+2] = vec.z;
	data[4 * row+3] = vec.w;
}

void Matrix4::setCol(uint8_t col, const Vector4& vec) {
	if (col >= 4)
		throw std::out_of_range(
			"Invalid index : " + std::to_string(col) + " is out of range");

	data[col] = vec.x;
	data[col + 4] = vec.y;
	data[col+  8] = vec.z;
	data[col + 12] = vec.w;
}

Vector4 Matrix4::getRow(uint8_t row) {
	if (row >= 4)
		throw std::out_of_range(
			"Invalid index : " + std::to_string(row) + " is out of range");

	return Vector4(data[4 * row],
		data[4 * row + 1],
		data[4 * row + 2],
		data[4 * row + 3]);
}

Vector4 Matrix4::getCol(uint8_t col) {
	if (col >= 4)
		throw std::out_of_range(
			"Invalid index : " + std::to_string(col) + " is out of range");

	return Vector4(data[col],
		data[col + 4],
		data[col + 8],
		data[col + 12]);
}

//Vector4 Matrix4::operator()(uint8_t row) {
//	if (row >= 4)
//		throw std::out_of_range(
//			"Invalid index : " + std::to_string(row));
//	return Vector4(data[4 * row], data[4 * row+1], data[4 * row+2], data[4 * row+3]);
//}

Matrix4 Matrix4::Scaling(const Vector3& scale) {
	return Matrix4(scale.x, 0, 0, 0,
		0, scale.y, 0, 0,
		0, 0, scale.z, 0,
		0, 0, 0, 1);
}

Matrix4 Matrix4::Translation(const Vector3& translation) {
	return Matrix4(1, 0, 0, translation.x,
		0, 1, 0, translation.y,
		0, 0, 1, translation.z,
		0, 0, 0, 1);
}

bool Matrix4::AreEquals(const Matrix4& left, const Matrix4& right) {
	return memcmp(&left, &right, 16 * sizeof(float)) == 0;
}

Matrix4 Matrix4::Add(const Matrix4& left, float scalar) {
	Matrix4 result(left);
	for (int8_t i = 0; i < 16; i++)
		result.data[i] += scalar;
	return result;
}

Matrix4 Matrix4::Add(const Matrix4& left, const Matrix4& right) {
	Matrix4 result(left);
	for (int8_t i = 0; i < 16; i++)
		result.data[i] += right.data[i];
	return result;
}

Matrix4 Matrix4::Subtract(const Matrix4& left, float scalar) {
	Matrix4 result(left);
	for (int8_t i = 0; i < 16; ++i)
		result.data[i] -= scalar;
	return result;
}

Matrix4 Matrix4::Subtract(const Matrix4& left, const Matrix4& right) {
	Matrix4 result(left);
	for (int8_t i = 0; i < 16; ++i)
		result.data[i] -= right.data[i];
	return result;
}

Matrix4 Matrix4::Multiply(const Matrix4& left, float scalar) {
	Matrix4 result(left);
	for (int8_t i = 0; i < 16; ++i)
		result.data[i] *= scalar;
	return result;
}

Vector4 Matrix4::Multiply(const Matrix4& matrix, const Vector4& vector) {
	Vector4 multiply;

	multiply.x = ((matrix.data[0] * vector.x) + (matrix.data[1] * vector.y) + (matrix.data[2]
		* vector.z) + (matrix.data[3] * vector.w));
	multiply.y = ((matrix.data[4] * vector.x) + (matrix.data[5] * vector.y) + (matrix.data[6]
		* vector.z) + (matrix.data[7] * vector.w));
	multiply.z = ((matrix.data[8] * vector.x) + (matrix.data[9] * vector.y) + (matrix.data[10]
		* vector.z) + (matrix.data[11] * vector.w));
	multiply.w = ((matrix.data[12] * vector.x) + (matrix.data[13] * vector.y) + (matrix.data[
		14] * vector.z) + (matrix.data[15] * vector.w));
	return multiply;
}

Matrix4 Matrix4::Multiply(const Matrix4& left, const Matrix4& right) {
	return Matrix4(
		((left.data[0] * right.data[0]) + (left.data[1] * right.data[4]) + (left.data[
			2] * right.data[8]) + (left.data[3] * right.data[12])),
		((left.data[0] * right.data[1]) + (left.data[1] * right.data[5]) + (left.data[
			2] * right.data[9]) + (left.data[3] * right.data[13])),
				((left.data[0] * right.data[2]) + (left.data[1] * right.data[6]) + (left.data[
					2] * right.data[10]) + (left.data[3] * right.data[14])),
				((left.data[0] * right.data[3]) + (left.data[1] * right.data[7]) + (left.data[
					2] * right.data[11]) + (left.data[3] * right.data[15])),

						((left.data[4] * right.data[0]) + (left.data[5] * right.data[4]) + (left.data[
							6] * right.data[8]) + (left.data[7] * right.data[12])),
						((left.data[4] * right.data[1]) + (left.data[5] * right.data[5]) + (left.data[
							6] * right.data[9]) + (left.data[7] * right.data[13])),
								((left.data[4] * right.data[2]) + (left.data[5] * right.data[6]) + (left.data[
									6] * right.data[10]) + (left.data[7] * right.data[14])),
								((left.data[4] * right.data[3]) + (left.data[5] * right.data[7]) + (left.data[
									6] * right.data[11]) + (left.data[7] * right.data[15])),

										((left.data[8] * right.data[0]) + (left.data[9] * right.data[4]) + (left.data[
											10] * right.data[8]) + (left.data[11] * right.data[12])),
										((left.data[8] * right.data[1]) + (left.data[9] * right.data[5]) + (left.data[
											10] * right.data[9]) + (left.data[11] * right.data[13])),
												((left.data[8] * right.data[2]) + (left.data[9] * right.data[6]) + (left.data[
													10] * right.data[10]) + (left.data[11] * right.data[14])),
												((left.data[8] * right.data[3]) + (left.data[9] * right.data[7]) + (left.data[
													10] * right.data[11]) + (left.data[11] * right.data[15])),

														((left.data[12] * right.data[0]) + (left.data[13] * right.data[4]) + (left.
															data[14] * right.data[8]) + (left.data[15] * right.data[12])),
														((left.data[12] * right.data[1]) + (left.data[13] * right.data[5]) + (left.
															data[14] * right.data[9]) + (left.data[15] * right.data[13])),
														((left.data[12] * right.data[2]) + (left.data[13] * right.data[6]) + (left.
															data[14] * right.data[10]) + (left.data[15] * right.data[14])),
														((left.data[12] * right.data[3]) + (left.data[13] * right.data[7]) + (left.
															data[14] * right.data[11]) + (left.data[15] * right.data[15])));
}

Matrix4 Matrix4::Divide(const Matrix4& left, float scalar) {
	Matrix4 result(left);
	for (float& element : result.data)
		element /= scalar;

	return result;
}

Matrix4 Matrix4::Divide(const Matrix4& left, const Matrix4& right) {
	Matrix4 leftCopy(left);
	return leftCopy * Inverse(right);
}

bool Matrix4::IsIdentity(const Matrix4& matrix) {
	return Identity.data == matrix.data;
}

float Matrix4::GetMinor(float minor0, float minor1, float minor2, float minor3, float minor4, float minor5, float minor6, float minor7, float minor8) {
	return minor0 * (minor4 * minor8 - minor5 * minor7)
		- minor1 * (minor3 * minor8 - minor5 * minor6)
		+ minor2 * (minor3 * minor7 - minor4 * minor6);
}

float Matrix4::Determinant(const Matrix4& matrix) {
	return matrix.data[0] * GetMinor(matrix.data[5], matrix.data[9], matrix.data[13], matrix.data[6], matrix.data[10], matrix.data[14],
		matrix.data[7], matrix.data[11], matrix.data[15])
		- matrix.data[4] * GetMinor(matrix.data[1], matrix.data[9], matrix.data[13], matrix.data[2], matrix.data[10], matrix.data[14],
			matrix.data[3], matrix.data[11], matrix.data[15])
		+ matrix.data[8] * GetMinor(matrix.data[1], matrix.data[5], matrix.data[13], matrix.data[2], matrix.data[6], matrix.data[14],
			matrix.data[3], matrix.data[7], matrix.data[15])
		- matrix.data[12] * GetMinor(matrix.data[1], matrix.data[5], matrix.data[9], matrix.data[2], matrix.data[6], matrix.data[10],
			matrix.data[3], matrix.data[7], matrix.data[11]);
}

Matrix4 Matrix4::Transpose(const Matrix4& matrix) {
	Matrix4 TransposedMatrix(matrix);

	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			TransposedMatrix.data[4 * j + i] = matrix.data[4 * i + j];
		}
	}
	return TransposedMatrix;
}


Matrix4 Matrix4::Inverse(const Matrix4& matrix) {
	const float determinant = Determinant(matrix);
	if (determinant == 0)
		throw std::logic_error("Division by 0");

	const float cof0 = GetMinor(matrix.data[5], matrix.data[9], matrix.data[13], matrix.data[6], matrix.data[10], matrix.data[14],
		matrix.data[7], matrix.data[11], matrix.data[15]);
	const float cof1 = GetMinor(matrix.data[1], matrix.data[9], matrix.data[13], matrix.data[2], matrix.data[10], matrix.data[14],
		matrix.data[3], matrix.data[11], matrix.data[15]);
	const float cof2 = GetMinor(matrix.data[1], matrix.data[5], matrix.data[13], matrix.data[2], matrix.data[6], matrix.data[14],
		matrix.data[3], matrix.data[7], matrix.data[15]);
	const float cof3 = GetMinor(matrix.data[1], matrix.data[5], matrix.data[9], matrix.data[2], matrix.data[6], matrix.data[10],
		matrix.data[3], matrix.data[7], matrix.data[11]);

	const float det = matrix.data[0] * cof0 - matrix.data[4] * cof1 + matrix.data[8] * cof2 - matrix.data[12] * cof3;
	if (fabs(det) <= EPSILON)
		return Identity;
	const float cof4 = GetMinor(matrix.data[4], matrix.data[8], matrix.data[12], matrix.data[6], matrix.data[10], matrix.data[14],
		matrix.data[7], matrix.data[11], matrix.data[15]);
	const float cof5 = GetMinor(matrix.data[0], matrix.data[8], matrix.data[12], matrix.data[2], matrix.data[10], matrix.data[14],
		matrix.data[3], matrix.data[11], matrix.data[15]);
	const float cof6 = GetMinor(matrix.data[0], matrix.data[4], matrix.data[12], matrix.data[2], matrix.data[6], matrix.data[14],
		matrix.data[3], matrix.data[7], matrix.data[15]);
	const float cof7 = GetMinor(matrix.data[0], matrix.data[4], matrix.data[8], matrix.data[2], matrix.data[6], matrix.data[10],
		matrix.data[3], matrix.data[7], matrix.data[11]);

	const float cof8 = GetMinor(matrix.data[4], matrix.data[8], matrix.data[12], matrix.data[5], matrix.data[9], matrix.data[13],
		matrix.data[7], matrix.data[11], matrix.data[15]);
	const float cof9 = GetMinor(matrix.data[0], matrix.data[8], matrix.data[12], matrix.data[1], matrix.data[9], matrix.data[13],
		matrix.data[3], matrix.data[11], matrix.data[15]);
	const float cof10 = GetMinor(matrix.data[0], matrix.data[4], matrix.data[12], matrix.data[1], matrix.data[5], matrix.data[13],
		matrix.data[3], matrix.data[7], matrix.data[15]);
	const float cof11 = GetMinor(matrix.data[0], matrix.data[4], matrix.data[8], matrix.data[1], matrix.data[5], matrix.data[9],
		matrix.data[3], matrix.data[7], matrix.data[11]);

	const float cof12 = GetMinor(matrix.data[4], matrix.data[8], matrix.data[12], matrix.data[5], matrix.data[9], matrix.data[13],
		matrix.data[6], matrix.data[10], matrix.data[14]);
	const float cof13 = GetMinor(matrix.data[0], matrix.data[8], matrix.data[12], matrix.data[1], matrix.data[9], matrix.data[13],
		matrix.data[2], matrix.data[10], matrix.data[14]);
	const float cof14 = GetMinor(matrix.data[0], matrix.data[4], matrix.data[12], matrix.data[1], matrix.data[5], matrix.data[13],
		matrix.data[2], matrix.data[6], matrix.data[14]);
	const float cof15 = GetMinor(matrix.data[0], matrix.data[4], matrix.data[8], matrix.data[1], matrix.data[5], matrix.data[9],
		matrix.data[2], matrix.data[6], matrix.data[10]);

	const float detInv = 1.0f / det;
	Matrix4 inverse;

	inverse.data[0] = detInv * cof0;
	inverse.data[4] = -detInv * cof4;
	inverse.data[8] = detInv * cof8;
	inverse.data[12] = -detInv * cof12;
	inverse.data[1] = -detInv * cof1;
	inverse.data[5] = detInv * cof5;
	inverse.data[9] = -detInv * cof9;
	inverse.data[13] = detInv * cof13;
	inverse.data[2] = detInv * cof2;
	inverse.data[6] = -detInv * cof6;
	inverse.data[10] = detInv * cof10;
	inverse.data[14] = -detInv * cof14;
	inverse.data[3] = -detInv * cof3;
	inverse.data[7] = detInv * cof7;
	inverse.data[11] = -detInv * cof11;
	inverse.data[15] = detInv * cof15;

	return inverse;
}

Matrix4 Matrix4::CreatePerspective(const float fov, const float aspectRatio, const float zNear, const float zFar) {
	const float tangent = tanf(fov / 2.0f * PI / 180.0f);
	const float height = zNear * tangent;
	const float width = height * aspectRatio;

	return CreateFrustum(-width, width, -height, height, zNear, zFar);
}

Matrix4 Matrix4::CreateOrthographic(const float left, const float right, const float bottom, const float top, const float zNear, const float zFar) {
	auto ortho = Matrix4::Identity;

	ortho(0, 0) = 2.0f / (right - left);
	ortho(1, 1) = 2.0f / (top - bottom);
	ortho(2, 2) = -2.0f / (zFar - zNear);
	ortho(0, 3) = -(right + left) / (right - left);
	ortho(1, 3) = -(top + bottom) / (top - bottom);
	ortho(2, 3) = -(zFar + zNear) / (zFar - zNear);
	ortho(3, 3) = 1.0f;

	return ortho;
}

Matrix4 Matrix4::CreateOrthographic(const float size, const float aspectRatio, const float zNear, const float zFar) {
	auto ortho = Matrix4::Identity;

	const auto right = size * aspectRatio;
	const auto left = -right;

	const auto top = size;
	const auto bottom = -top;

	ortho(0, 0) = 2.0f / (right - left);
	ortho(1, 1) = 2.0f / (top - bottom);
	ortho(2, 2) = -2.0f / (zFar - zNear);
	ortho(0, 3) = -(right + left) / (right - left);
	ortho(1, 3) = -(top + bottom) / (top - bottom);
	ortho(2, 3) = -(zFar + zNear) / (zFar - zNear);
	ortho(3, 3) = 1.0f;

	return ortho;
}

Matrix4 Matrix4::CreateView(const float eyeX, const float eyeY, const float eyeZ, const float lookX, const float lookY, const float lookZ, const float upX, const float upY, const float upZ) {
	const Vector3 eye(eyeX, eyeY, eyeZ);
	const Vector3 look(lookX, lookY, lookZ);
	const Vector3 up(upX, upY, upZ);

	const Vector3 forward(eye - look);
	Vector3::Normalize(forward);

	const Vector3 upXForward(Vector3::Cross(up, forward));
	Vector3::Normalize(upXForward);

	const Vector3 v(Vector3::Cross(forward, upXForward));

	Matrix4 View;

	View.data[0] = upXForward.x;
	View.data[1] = upXForward.y;
	View.data[2] = upXForward.z;
	View.data[3] = -Vector3::Dot(eye, upXForward);

	View.data[4] = v.x;
	View.data[5] = v.y;
	View.data[6] = v.z;
	View.data[7] = -Vector3::Dot(eye, v);

	View.data[8] = forward.x;
	View.data[9] = forward.y;
	View.data[10] = forward.z;
	View.data[11] = -Vector3::Dot(eye, forward);

	return View;
}

Matrix4 Matrix4::CreateView(Vector3 eye, Vector3 look, Vector3 up) {
	const Vector3 forward(Vector3::Normalize(eye - look));
	Vector3::Normalize(forward);

	const Vector3 upXForward(Vector3::Normalize(Vector3::Cross(up, forward)));
	Vector3::Normalize(upXForward);

	const Vector3 v(Vector3::Normalize(Vector3::Cross(forward, upXForward)));

	Matrix4 View;

	View.data[0] = upXForward.x;
	View.data[1] = upXForward.y;
	View.data[2] = upXForward.z;
	View.data[3] = -Vector3::Dot(eye, upXForward);

	View.data[4] = v.x;
	View.data[5] = v.y;
	View.data[6] = v.z;
	View.data[7] = -Vector3::Dot(eye, v);

	View.data[8] = forward.x;
	View.data[9] = forward.y;
	View.data[10] = forward.z;
	View.data[11] = -Vector3::Dot(eye, forward);

	return View;
}

Matrix4 Matrix4::CreateFrustum(const float left, const float right, const float bottom, const float top, const float zNear, const float zFar) {
	const float maxView = 2.0f * zNear;
	const float width = right - left;
	const float height = top - bottom;
	const float zRange = zFar - zNear;

	Matrix4 Frustum;

	Frustum.data[0] = maxView / width;
	Frustum.data[5] = maxView / height;
	Frustum.data[2] = (right + left) / width;
	Frustum.data[6] = (top + bottom) / height;
	Frustum.data[10] = (-zFar - zNear) / zRange;
	Frustum.data[14] = -1.0f;
	Frustum.data[11] = (-maxView * zFar) / zRange;
	Frustum.data[15] = 0.0f;

	return Frustum;
}

