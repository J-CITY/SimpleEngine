#include "camera.h"

using namespace IKIGAI;
using namespace IKIGAI::RENDER;

Camera::Camera() :
	projectionMode(ProjectionMode::PERSPECTIVE),
	fov(45.0f),
	size(5.0f),
	_near(0.1f),
	_far(100.f),
	frustumGeometryCulling(false),
	frustumLightCulling(false) {
}

void Camera::cacheMatrices(unsigned p_windowWidth, unsigned p_windowHeight, const MATHGL::Vector3& p_position, const MATHGL::Quaternion& p_rotation) {
	cacheProjectionMatrix(p_windowWidth, p_windowHeight);
	cacheViewMatrix(p_position, p_rotation);
	cacheFrustum(viewMatrix, projectionMatrix);
}

void Camera::cacheProjectionMatrix(unsigned p_windowWidth, unsigned p_windowHeight) {
	projectionMatrix = calculateProjectionMatrix(p_windowWidth, p_windowHeight);
}

void Camera::cacheViewMatrix(const MATHGL::Vector3& p_position, const MATHGL::Quaternion& p_rotation) {
	viewMatrix = calculateViewMatrix(p_position, p_rotation);
}

void Camera::cacheFrustum(const MATHGL::Matrix4& p_view, const MATHGL::Matrix4& p_projection) {
	frustum.make(p_projection * p_view);
}

float Camera::getFov() const {
	return fov;
}

float Camera::getSize() const {
	return size;
}

float Camera::getNear() const {
	return _near;
}

float Camera::getFar() const {
	return _far;
}

const IKIGAI::MATHGL::Matrix4& Camera::getProjectionMatrix() const {
	return projectionMatrix;
}

const IKIGAI::MATHGL::Matrix4& Camera::getViewMatrix() const {
	return viewMatrix;
}

const Frustum& Camera::getFrustum() const {
	return frustum;
}

bool Camera::isFrustumGeometryCulling() const {
	return frustumGeometryCulling;
}

bool Camera::isFrustumLightCulling() const {
	return frustumLightCulling;
}

Camera::ProjectionMode Camera::getProjectionMode() const {
	return projectionMode;
}

void Camera::setFov(float p_value) {
	fov = p_value;
}

void Camera::setSize(float p_value) {
	size = p_value;
}

void Camera::setNear(float p_value) {
	_near = p_value;
}

void Camera::setFar(float p_value) {
	_far = p_value;
}

void Camera::setFrustumGeometryCulling(bool p_enable) {
	frustumGeometryCulling = p_enable;
}

void Camera::setFrustumLightCulling(bool p_enable) {
	frustumLightCulling = p_enable;
}

void Camera::setProjectionMode(ProjectionMode p_projectionMode) {
	projectionMode = p_projectionMode;
}

void Camera::setView(MATHGL::Matrix4& in) {
	viewMatrix = in;
}

IKIGAI::MATHGL::Matrix4 Camera::calculateProjectionMatrix(uint16_t p_windowWidth, uint16_t p_windowHeight) const {
	
	const auto ratio = p_windowWidth / static_cast<float>(p_windowHeight);

	switch (projectionMode) {
	case ProjectionMode::ORTHOGRAPHIC:
		return MATHGL::Matrix4::CreateOrthographic(size, ratio, _near, _far);

	case ProjectionMode::PERSPECTIVE:
		return MATHGL::Matrix4::CreatePerspective(fov, ratio, _near, _far);

	default:
		return MATHGL::Matrix4::Identity;
	}
}

IKIGAI::MATHGL::Vector3 Camera::calculateViewVector(const MATHGL::Vector3& p_position, const MATHGL::Quaternion& p_rotation) const {
	const auto forward = p_rotation * MATHGL::Vector3::Forward;
	return p_position.x + forward.x, p_position.y + forward.y, p_position.z + forward.z;
}

IKIGAI::MATHGL::Matrix4 Camera::calculateViewMatrix(const MATHGL::Vector3& p_position, const MATHGL::Quaternion& p_rotation) const {
	const auto up = p_rotation * MATHGL::Vector3::Up;
	const auto forward = p_rotation * MATHGL::Vector3::Forward;

	return MATHGL::Matrix4::CreateView(
		p_position.x, p_position.y, p_position.z,												// Position
		p_position.x + forward.x, p_position.y + forward.y, p_position.z + forward.z,			// LookAt (Position + Forward)
		up.x, up.y, up.z																		// Up Vector
	);
}