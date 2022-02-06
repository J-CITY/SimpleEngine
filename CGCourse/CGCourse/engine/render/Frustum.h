#pragma once

#include <array>
#include "../ecs/components/transform.h"
#include "../utils/math/Matrix4.h"
#include "objects/BoundingSphere.h"

namespace KUMA {
	namespace RENDER {
		class Frustum {
		public:
			void make(const MATHGL::Matrix4& viewProjection);
			bool pointInFrustum(float x, float y, float z) const;
			bool sphereInFrustum(float x, float y, float z, float radius) const;
			bool cubeInFrustum(float x, float y, float z, float size) const;
			bool boundingSphereInFrustum(const BoundingSphere& boundingSphere, 
				const ECS::Transform& transform) const;
			std::array<float, 4> getNearPlane() const;
			std::array<float, 4> getFarPlane() const;
		private:
			float data[6][4];
		};
	}
}
