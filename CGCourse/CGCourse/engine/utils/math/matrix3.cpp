module;
#include <cmath>
#include <stdexcept>
#include <string>

module glmath:Matrix3;

using namespace KUMA;
using namespace KUMA::MATHGL;

Matrix3::Matrix3() {
	//for (auto& d : data) {
	//	d = 0.0f;
	//}

	//for (auto i = 0; i < data.size(); i++) {
	//	data[i] = 0.0f;
	//}

	data = Identity.data;
}
Matrix3::Matrix3(float val) {
	//for (auto& d : data) {
	//	d = val;
	//}
	for (auto i = 0; i < data.size(); i++) {
		data[i] = val;
	}
}

Matrix3::Matrix3(float element1, float element2, float element3,
	float element4, float element5, float element6,
	float element7, float element8, float element9) {
	data[0] = element1;
	data[1] = element2;
	data[2] = element3;
	data[3] = element4;
	data[4] = element5;
	data[5] = element6;
	data[6] = element7;
	data[7] = element8;
	data[8] = element9;
}

Matrix3::Matrix3(Vector3 a, Vector3 b, Vector3 c) {
	data[0] = a.x;
	data[1] = a.y;
	data[2] = a.z;
	data[3] = b.x;
	data[4] = b.y;
	data[5] = b.z;
	data[6] = c.x;
	data[7] = c.y;
	data[8] = c.z;
}

Matrix3::Matrix3(Array<float, 9>&& in) {
	data = in;
};

Matrix3::Matrix3(const Matrix3& in) {
	*this = in;
}

Vector3 Matrix3::transform(const Vector3& vector) const {
	return (*this) * vector;
}

void Matrix3::setInertiaTensorCoeffs(float ix, float iy, float iz,
	float ixy, float ixz, float iyz) {
	data[0] = ix;
	data[1] = data[3] = -ixy;
	data[2] = data[6] = -ixz;
	data[4] = iy;
	data[5] = data[7] = -iyz;
	data[8] = iz;
}

void Matrix3::setBlockInertiaTensor(const Vector3& halfSizes, float mass) {
	Vector3 squares = halfSizes.componentProduct(halfSizes);
	setInertiaTensorCoeffs(0.3f * mass * (squares.y + squares.z),
		0.3f * mass * (squares.x + squares.z),
		0.3f * mass * (squares.x + squares.y));
}

void Matrix3::setInverse(const Matrix3& m) {
	float t4 = m.data[0] * m.data[4];
	float t6 = m.data[0] * m.data[5];
	float t8 = m.data[1] * m.data[3];
	float t10 = m.data[2] * m.data[3];
	float t12 = m.data[1] * m.data[6];
	float t14 = m.data[2] * m.data[6];

	// Calculate the determinant
	float t16 = (t4 * m.data[8] - t6 * m.data[7] - t8 * m.data[8] +
		t10 * m.data[7] + t12 * m.data[5] - t14 * m.data[4]);

	// Make sure the determinant is non-zero.
	if (t16 == 0.0f) {
		return;
	}
	float t17 = 1 / t16;

	data[0] = (m.data[4] * m.data[8] - m.data[5] * m.data[7]) * t17;
	data[1] = -(m.data[1] * m.data[8] - m.data[2] * m.data[7]) * t17;
	data[2] = (m.data[1] * m.data[5] - m.data[2] * m.data[4]) * t17;
	data[3] = -(m.data[3] * m.data[8] - m.data[5] * m.data[6]) * t17;
	data[4] = (m.data[0] * m.data[8] - t14) * t17;
	data[5] = -(t6 - t10) * t17;
	data[6] = (m.data[3] * m.data[7] - m.data[4] * m.data[6]) * t17;
	data[7] = -(m.data[0] * m.data[7] - t12) * t17;
	data[8] = (t4 - t8) * t17;
}
void Matrix3::setComponents(const Vector3& compOne, const Vector3& compTwo,
	const Vector3& compThree) {
	data[0] = compOne.x;
	data[1] = compTwo.x;
	data[2] = compThree.x;
	data[3] = compOne.y;
	data[4] = compTwo.y;
	data[5] = compThree.y;
	data[6] = compOne.z;
	data[7] = compTwo.z;
	data[8] = compThree.z;

}

Vector3 Matrix3::transformTranspose(const Vector3& vector) const {
	return Vector3(
		vector.x * data[0] + vector.y * data[3] + vector.z * data[6],
		vector.x * data[1] + vector.y * data[4] + vector.z * data[7],
		vector.x * data[2] + vector.y * data[5] + vector.z * data[8]
	);
}

void Matrix3::setSkewSymmetric(const Vector3 vector) {
	data[0] = data[4] = data[8] = 0;
	data[1] = -vector.z;
	data[2] = vector.y;
	data[3] = vector.z;
	data[5] = -vector.x;
	data[6] = -vector.y;
	data[7] = vector.x;
}

Matrix3 Matrix3::transpose() const {
	return Transpose(*this);
}

Matrix3 Matrix3::inverse() const {
	return Inverse(*this);
}
Matrix3 Matrix3::LinearInterpolate(const Matrix3& a, const Matrix3& b, float prop) {
	Matrix3 result;
	for (unsigned i = 0; i < 9; i++) {
		result.data[i] = a.data[i] * (1 - prop) + b.data[i] * prop;
	}
	return result;
}

const Matrix3 Matrix3::Identity = Matrix3(1.0f, 0.0f, 0.0f,
	0.0f, 1.0f, 0.0f,
	0.0f, 0.0f, 1.0f);


Matrix3& Matrix3::operator=(const Matrix3& other) {
	this->data = other.data;
	return *this;
}

bool Matrix3::operator==(const Matrix3& other) {
	return AreEquals(*this, other);
}

Matrix3 Matrix3::operator+(const Matrix3& other) const {
	return Add(*this, other);
}

Matrix3& Matrix3::operator+=(const Matrix3& other) {
	*this = Add(*this, other);
	return *this;
}

Matrix3 Matrix3::operator-(const Matrix3& other) const {
	return Subtract(*this, other);
}

Matrix3& Matrix3::operator-=(const Matrix3& other) {
	*this = Subtract(*this, other);
	return *this;
}

Matrix3 Matrix3::operator*(float scalar) const {
	return Multiply(*this, scalar);
}

Matrix3& Matrix3::operator*=(float scalar) {
	*this = Multiply(*this, scalar);
	return *this;
}

Vector3 Matrix3::operator*(const Vector3& vector) const {
	return Multiply(*this, vector);
}

Matrix3 Matrix3::operator*(const Matrix3& other) const {
	return Multiply(*this, other);
}

Matrix3& Matrix3::operator*=(const Matrix3& other) {
	*this = Multiply(*this, other);
	return *this;
}

Matrix3 Matrix3::operator/(float scalar) const {
	return Divide(*this, scalar);
}

Matrix3& Matrix3::operator/=(float scalar) {
	*this = Divide(*this, scalar);
	return *this;
}

Matrix3 Matrix3::operator/(const Matrix3& other) const {
	return Divide(*this, other);
}

Matrix3& Matrix3::operator/=(const Matrix3& other) {
	*this = Divide(*this, other);
	return *this;
}

float& Matrix3::operator()(unsigned row, unsigned column) {
	if (row >= 3 || column >= 3)
		throw std::out_of_range(
			"Invalid index : " + std::to_string(row) + "," + std::to_string(column) + " is out of range");
	return data[3 * row + column];
}

bool Matrix3::AreEquals(const Matrix3& left, const Matrix3& right) {
	return memcmp(&left, &right, 9 * sizeof(float)) == 0;
}

Matrix3 Matrix3::Add(const Matrix3& left, float scalar) {
	Matrix3 result(left);
	for (uint8_t i = 0; i < 9; ++i)
		result.data[i] += scalar;
	return result;
}

Matrix3 Matrix3::Add(const Matrix3& left, const Matrix3& right) {
	Matrix3 result(left);
	for (uint8_t i = 0; i < 9; ++i)
		result.data[i] += right.data[i];
	return result;
}

Matrix3 Matrix3::Subtract(const Matrix3& left, float scalar) {
	Matrix3 result(left);
	//for (float& element : result.data) {
	//	element -= scalar;
	//}
	for (uint8_t i = 0; i < 9; ++i)
		result.data[i] -= scalar;
	return result;
}

Matrix3 Matrix3::Subtract(const Matrix3& left, const Matrix3& right) {
	Matrix3 result(left);
	for (uint8_t i = 0; i < 9; ++i)
		result.data[i] -= right.data[i];
	return result;
}

Matrix3 Matrix3::Multiply(const Matrix3& left, float scalar) {
	Matrix3 result(left);
	//for (float& element : result.data) {
	//	element *= scalar;
	//}
	for (uint8_t i = 0; i < 9; ++i)
		result.data[i] *= scalar;
	return result;
}

Vector3 Matrix3::Multiply(const Matrix3& matrix, const Vector3& vector) {
	Vector3 result;
	result.x = ((matrix.data[0] * vector.x) + (matrix.data[1] * vector.y) + (matrix.data[2] *
		vector.z));
	result.y = ((matrix.data[3] * vector.x) + (matrix.data[4] * vector.y) + (matrix.data[5] *
		vector.z));
	result.z = ((matrix.data[6] * vector.x) + (matrix.data[7] * vector.y) + (matrix.data[8] *
		vector.z));

	return result;
}

Matrix3 Matrix3::Multiply(const Matrix3& left, const Matrix3& right) {
	return Matrix3(
		(left.data[0] * right.data[0]) + (left.data[1] * right.data[3]) + (left.data[2
		] * right.data[6]),
		(left.data[0] * right.data[1]) + (left.data[1] * right.data[4]) + (left.data[2
		] * right.data[7]),
		(left.data[0] * right.data[2]) + (left.data[1] * right.data[5]) + (left.data[2
		] * right.data[8]),

		(left.data[3] * right.data[0]) + (left.data[4] * right.data[3]) + (left.data[5
		] * right.data[6]),
		(left.data[3] * right.data[1]) + (left.data[4] * right.data[4]) + (left.data[5
		] * right.data[7]),
		(left.data[3] * right.data[2]) + (left.data[4] * right.data[5]) + (left.data[5
		] * right.data[8]),

		(left.data[6] * right.data[0]) + (left.data[7] * right.data[3]) + (left.data[8
		] * right.data[6]),
		(left.data[6] * right.data[1]) + (left.data[7] * right.data[4]) + (left.data[8
		] * right.data[7]),
		(left.data[6] * right.data[2]) + (left.data[7] * right.data[5]) + (left.data[8
		] * right.data[8]));
}

Matrix3 Matrix3::Divide(const Matrix3& left, float scalar) {
	Matrix3 result(left);
	//for (float& element : result.data) {
	//	element /= scalar;
	//}
	for (uint8_t i = 0; i < 9; ++i)
		result.data[i] /= scalar;
	return result;
}

Matrix3 Matrix3::Divide(const Matrix3& left, const Matrix3& right) {
	return left * Inverse(right);
}

bool Matrix3::IsIdentity(const Matrix3& matrix) {
	return false;// Identity.data == matrix.data;
}

float Matrix3::Determinant(const Matrix3& matrix) {
	return matrix.data[0] * (matrix.data[4] * matrix.data[8] - matrix.data[5] * matrix.data[7])
		- matrix.data[3] * (matrix.data[1] * matrix.data[8] - matrix.data[2] * matrix.data[7])
		+ matrix.data[6] * (matrix.data[1] * matrix.data[5] - matrix.data[2] * matrix.data[4]);
}

Matrix3 Matrix3::Transpose(const Matrix3& matrix) {
	Matrix3 result;

	result.data[0] = matrix.data[0];
	result.data[1] = matrix.data[3];
	result.data[2] = matrix.data[6];

	result.data[3] = matrix.data[1];
	result.data[4] = matrix.data[4];
	result.data[5] = matrix.data[7];

	result.data[6] = matrix.data[2];
	result.data[7] = matrix.data[5];
	result.data[8] = matrix.data[8];

	return result;
}

Matrix3 Matrix3::Cofactor(const Matrix3& matrix) {
	return Matrix3(
		((matrix.data[4] * matrix.data[8]) - (matrix.data[5] * matrix.data[7])), //0
		-((matrix.data[3] * matrix.data[8]) - (matrix.data[5] * matrix.data[6])), //1
		((matrix.data[3] * matrix.data[7]) - (matrix.data[4] * matrix.data[6])), //2
		-((matrix.data[1] * matrix.data[8]) - (matrix.data[2] * matrix.data[7])), //3
		((matrix.data[0] * matrix.data[8]) - (matrix.data[2] * matrix.data[6])), //4
		-((matrix.data[0] * matrix.data[7]) - (matrix.data[1] * matrix.data[6])), //5
		((matrix.data[1] * matrix.data[5]) - (matrix.data[2] * matrix.data[4])), //6
		-((matrix.data[0] * matrix.data[5]) - (matrix.data[2] * matrix.data[3])), //7 
		((matrix.data[0] * matrix.data[4]) - (matrix.data[1] * matrix.data[3]))); //8
}

Matrix3 Matrix3::Minor(const Matrix3& matrix) {
	return Matrix3(
		((matrix.data[4] * matrix.data[8]) - (matrix.data[5] * matrix.data[7])), //0
		((matrix.data[3] * matrix.data[8]) - (matrix.data[5] * matrix.data[6])), //1
		((matrix.data[3] * matrix.data[7]) - (matrix.data[4] * matrix.data[6])), //2
		((matrix.data[1] * matrix.data[8]) - (matrix.data[2] * matrix.data[7])), //3
		((matrix.data[0] * matrix.data[8]) - (matrix.data[2] * matrix.data[6])), //4
		((matrix.data[0] * matrix.data[7]) - (matrix.data[1] * matrix.data[6])), //5
		((matrix.data[1] * matrix.data[5]) - (matrix.data[2] * matrix.data[4])), //6
		((matrix.data[0] * matrix.data[5]) - (matrix.data[2] * matrix.data[3])), //7 
		((matrix.data[0] * matrix.data[4]) - (matrix.data[1] * matrix.data[3]))); //8
}

Matrix3 Matrix3::Adjoint(const Matrix3& other) {
	return Transpose(Cofactor(other));
}

Matrix3 Matrix3::Inverse(const Matrix3& matrix) {
	const float determinant = Determinant(matrix);
	if (determinant == 0)
		throw std::logic_error("Division by 0");

	return Adjoint(matrix) / determinant;
}

//Matrix3 Matrix3::Translation(const FVector2& translation) {
//	return Matrix3(1, 0, translation.x,
//		0, 1, translation.y,
//		0, 0, 1);
//}

//Matrix3 Matrix3::Translate(const Matrix3& matrix, const FVector2& translation) {
//	return matrix * Translation(translation);
//}

Matrix3 Matrix3::Rotation(float rotation) {
	return Matrix3(std::cos(rotation), -std::sin(rotation), 0,
		std::sin(rotation), std::cos(rotation), 0,
		0, 0, 1);
}

Matrix3 Matrix3::Rotate(const Matrix3& matrix, float rotation) {
	return matrix * Rotation(rotation);
}

//Matrix3 Matrix3::Scaling(const Vector2& scale) {
//	return Matrix3(scale.x, 0, 0,
//		0, scale.y, 0,
//		0, 0, 1);
//}

//Matrix3 Matrix3::Scale(const Matrix3& matrix, const FVector2& scale) {
//	return matrix * Scaling(scale);
//}

Vector3 Matrix3::GetRow(const Matrix3& matrix, unsigned row) {
	if (row >= 3)
		throw std::out_of_range("Invalid index : " + std::to_string(row) + " is out of range");

	return Vector3(matrix.data[row * 3], matrix.data[row * 3 + 1], matrix.data[row * 3 + 2]);
}

Vector3 Matrix3::GetColumn(const Matrix3& matrix, unsigned column) {
	if (column >= 3)
		throw std::out_of_range("Invalid index : " + std::to_string(column) + " is out of range");

	return Vector3(matrix.data[column + 6], matrix.data[column + 3], matrix.data[column]);
}

Matrix3 KUMA::MATHGL::operator*(const float f, const Matrix3& V) {
	return V * f;
}

Matrix3 KUMA::MATHGL::operator/(const float f, const Matrix3& V) {
	return V / f;
}
