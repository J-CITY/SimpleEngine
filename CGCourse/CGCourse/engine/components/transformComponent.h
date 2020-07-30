#pragma once
#include <SFML/Graphics/Transform.hpp>

#include "component.h"

namespace SE {

	class TransformComponent: public Component {
	public:
		TransformComponent();
		TransformComponent(const Mathgl::Vec3& position, const Mathgl::Mat4& rotation, const Mathgl::Vec3& scale);
		~TransformComponent() = default;

		TransformComponent& rotate(const Mathgl::Vec3& axis, float angle);
		TransformComponent& scale(float scale);
		TransformComponent& scale(const Mathgl::Vec3& scale);
		TransformComponent& translate(const Mathgl::Vec3& position);

		TransformComponent& setPosition(const Mathgl::Vec3& position);
		TransformComponent& setScale(const Mathgl::Vec3& scale);
		TransformComponent& setRotation(const Mathgl::Mat4& rotation);
		TransformComponent& setRotation(const Mathgl::Vec3& axis, float w);

		Mathgl::Vec3 getPosition(void) const;
		Mathgl::Vec3 getScale(void) const;
		Mathgl::Mat4 getRotation(void) const;
		Mathgl::Mat4 getTransformMatrix(void) const;
		Mathgl::Vec4 getDirection(void) const;

	private:
		Mathgl::Vec3 _position;
		Mathgl::Mat4 _rotation;
		Mathgl::Vec3 _scale;
	};

}