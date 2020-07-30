#include "transformComponent.h"

using namespace SE;
using namespace Mathgl;

TransformComponent::TransformComponent() {
	type = ComponentType::TRANSFORM;
	_position = Mathgl::Vec3();
	_rotation = Mathgl::Mat4();
	_scale = Mathgl::Vec3(1.0f);
}

TransformComponent::TransformComponent(const Vec3& position, const Mat4& rotation, const Vec3& scale) {
	_position = position;
	_rotation = rotation;
	_scale = scale;
}

TransformComponent& TransformComponent::rotate(const Vec3& axis, float angle) {
	_rotation = Mathgl::rotate(_rotation, axis, angle);
	return *this;
}

TransformComponent& TransformComponent::scale(float scale) {
	return setScale(getScale() * scale);
}

TransformComponent& TransformComponent::scale(const Vec3& scale) {
	return setScale(getScale() * scale);
}

TransformComponent& TransformComponent::translate(const Vec3& position) {
	return setPosition(_position + position);
}

TransformComponent& TransformComponent::setPosition(const Vec3& position) {
	_position = position;
	return *this;
}

TransformComponent& TransformComponent::setScale(const Vec3& scale) {
	_scale = scale;
	return *this;
}

TransformComponent& TransformComponent::setRotation(const Vec3& axis, float angle) {
	_rotation = Mathgl::rotate(Eye4(), axis, angle);
	return *this;
}

TransformComponent& TransformComponent::setRotation(const Mat4& rotation) {
	_rotation = rotation;
	return *this;
}

Vec3 TransformComponent::getPosition() const {
	return _position;
}

Vec3 TransformComponent::getScale() const {
	return _scale;
}

Mat4 TransformComponent::getRotation() const {
	return _rotation;
}

Mat4 TransformComponent::getTransformMatrix() const {
	return Mathgl::translate(Eye4(), _position) * _rotation * Mathgl::scale(Eye4(), _scale);
}

Vec4 TransformComponent::getDirection() const {
	return getRotation() * Vec4(0, 0, -1, 0);
}
