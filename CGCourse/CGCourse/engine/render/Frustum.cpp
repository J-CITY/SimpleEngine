#include "frustum.h"

using namespace KUMA;
using namespace KUMA::RENDER;

enum FrustumSide {
	RIGHT = 0,
	LEFT,
	BOTTOM,
	TOP,
	BACK,
	FRONT
};

enum PlaneData {
	A = 0,
	B,
	C,
	D
};

void normalizePlane(float frustum[6][4], int side) {
	float magnitude = (float)sqrt(frustum[side][A] * frustum[side][A] +
		frustum[side][B] * frustum[side][B] +
		frustum[side][C] * frustum[side][C]);
	frustum[side][A] /= magnitude;
	frustum[side][B] /= magnitude;
	frustum[side][C] /= magnitude;
	frustum[side][D] /= magnitude;
}


void Frustum::make(const MATHGL::Matrix4& viewProjection) {
	auto columnMajorViewProjection = MATHGL::Matrix4::Transpose(viewProjection);
	const auto& clip = columnMajorViewProjection.data;

	data[RIGHT][A] = clip[3] - clip[0];
	data[RIGHT][B] = clip[7] - clip[4];
	data[RIGHT][C] = clip[11] - clip[8];
	data[RIGHT][D] = clip[15] - clip[12];

	normalizePlane(data, RIGHT);

	data[LEFT][A] = clip[3] + clip[0];
	data[LEFT][B] = clip[7] + clip[4];
	data[LEFT][C] = clip[11] + clip[8];
	data[LEFT][D] = clip[15] + clip[12];

	normalizePlane(data, LEFT);

	data[BOTTOM][A] = clip[3] + clip[1];
	data[BOTTOM][B] = clip[7] + clip[5];
	data[BOTTOM][C] = clip[11] + clip[9];
	data[BOTTOM][D] = clip[15] + clip[13];

	normalizePlane(data, BOTTOM);

	data[TOP][A] = clip[3] - clip[1];
	data[TOP][B] = clip[7] - clip[5];
	data[TOP][C] = clip[11] - clip[9];
	data[TOP][D] = clip[15] - clip[13];

	normalizePlane(data, TOP);

	data[BACK][A] = clip[3] - clip[2];
	data[BACK][B] = clip[7] - clip[6];
	data[BACK][C] = clip[11] - clip[10];
	data[BACK][D] = clip[15] - clip[14];

	normalizePlane(data, BACK);

	data[FRONT][A] = clip[3] + clip[2];
	data[FRONT][B] = clip[7] + clip[6];
	data[FRONT][C] = clip[11] + clip[10];
	data[FRONT][D] = clip[15] + clip[14];

	normalizePlane(data, FRONT);
}

bool Frustum::pointInFrustum(float x, float y, float z) const {
	for (int i = 0; i < 6; i++) {
		if (data[i][A] * x + data[i][B] * y + data[i][C] * z + data[i][D] <= 0) {
			return false;
		}
	}
	return true;
}


bool Frustum::sphereInFrustum(float x, float y, float z, float radius) const {
	for (int i = 0; i < 6; i++) {
		if (data[i][A] * x + data[i][B] * y + data[i][C] * z + data[i][D] <= -radius) {
			return false;
		}
	}
	return true;
}

bool Frustum::cubeInFrustum(float x, float y, float z, float size) const {
	for (int i = 0; i < 6; i++) {
		if (data[i][A] * (x - size) + data[i][B] * (y - size) + data[i][C] * (z - size) + data[i][D] > 0)
			continue;
		if (data[i][A] * (x + size) + data[i][B] * (y - size) + data[i][C] * (z - size) + data[i][D] > 0)
			continue;
		if (data[i][A] * (x - size) + data[i][B] * (y + size) + data[i][C] * (z - size) + data[i][D] > 0)
			continue;
		if (data[i][A] * (x + size) + data[i][B] * (y + size) + data[i][C] * (z - size) + data[i][D] > 0)
			continue;
		if (data[i][A] * (x - size) + data[i][B] * (y - size) + data[i][C] * (z + size) + data[i][D] > 0)
			continue;
		if (data[i][A] * (x + size) + data[i][B] * (y - size) + data[i][C] * (z + size) + data[i][D] > 0)
			continue;
		if (data[i][A] * (x - size) + data[i][B] * (y + size) + data[i][C] * (z + size) + data[i][D] > 0)
			continue;
		if (data[i][A] * (x + size) + data[i][B] * (y + size) + data[i][C] * (z + size) + data[i][D] > 0)
			continue;
		return false;
	}
	return true;
}

bool Frustum::boundingSphereInFrustum(const BoundingSphere& boundingSphere, const ECS::Transform& transform) const {
	const auto& position = transform.getWorldPosition();
	const auto& rotation = transform.getWorldRotation();
	const auto& scale = transform.getWorldScale();

	float maxScale = std::max(std::max(std::max(scale.x, scale.y), scale.z), 0.0f);
	float scaledRadius = boundingSphere.radius * maxScale;
	auto sphereOffset = MATHGL::Quaternion::RotatePoint(boundingSphere.position, rotation) * maxScale;

	MATHGL::Vector3 worldCenter = position + sphereOffset;

	return sphereInFrustum(worldCenter.x, worldCenter.y, worldCenter.z, scaledRadius);
}

std::array<float, 4> Frustum::getNearPlane() const {
	return {data[FRONT][0], data[FRONT][1], data[FRONT][2], data[FRONT][3]};
}

std::array<float, 4> Frustum::getFarPlane() const {
	return {data[BACK][0], data[BACK][1], data[BACK][2], data[BACK][3]};
}
