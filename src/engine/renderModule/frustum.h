﻿#pragma once
#include <coreModule/ecs/components/transform.h>
import glmath;
#include "objects/boundingSphere.h"

namespace IKIGAI::MATHGL {
	class Matrix4;
}

namespace IKIGAI {
	namespace RENDER {
		class Frustum {
		public:
			void make(const MATHGL::Matrix4& viewProjection);
			[[nodiscard]] bool pointInFrustum(float x, float y, float z) const;
			[[nodiscard]] bool sphereInFrustum(float x, float y, float z, float radius) const;
			[[nodiscard]] bool cubeInFrustum(float x, float y, float z, float size) const;
			[[nodiscard]] bool boundingSphereInFrustum(const BoundingSphere& boundingSphere, const ECS::Transform& transform) const;
			[[nodiscard]] std::array<float, 4> getNearPlane() const;
			[[nodiscard]] std::array<float, 4> getFarPlane() const;
		private:
			float data[6][4] = {};
		};
	}
}