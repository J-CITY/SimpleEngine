#pragma once
#include <cstdint>

#include "frustum.h"
import glmath;

namespace KUMA {
	namespace RENDER {
		class Camera {
		public:
			enum class ProjectionMode {
				ORTHOGRAPHIC,
				PERSPECTIVE
			};
			NLOHMANN_JSON_SERIALIZE_ENUM(Camera::ProjectionMode, {
				{Camera::ProjectionMode::ORTHOGRAPHIC, "ORTHOGRAPHIC"},
				{Camera::ProjectionMode::PERSPECTIVE, "PERSPECTIVE"}
			})
			
			Camera();
			void cacheMatrices(unsigned int winWidth, unsigned int winHeight, 
				const MATHGL::Vector3& position, const MATHGL::Quaternion& rotation);
			void cacheProjectionMatrix(unsigned int winWidth, unsigned int winHeight);
			void cacheViewMatrix(const MATHGL::Vector3& position, const MATHGL::Quaternion& rotation);
			void cacheFrustum(const MATHGL::Matrix4& view, const MATHGL::Matrix4& projection);

			float getFov() const;
			float getSize() const;
			float getNear() const;
			float getFar() const;
			const MATHGL::Matrix4& getProjectionMatrix() const;
			const MATHGL::Matrix4& getViewMatrix() const;
			const Frustum& getFrustum() const;
			bool isFrustumGeometryCulling() const;
			bool isFrustumLightCulling() const;
			ProjectionMode getProjectionMode() const;
			void setFov(float p_value);
			void setSize(float p_value);
			void setNear(float p_value);
			void setFar(float p_value);
			void setFrustumGeometryCulling(bool p_enable);
			void setFrustumLightCulling(bool p_enable);
			void setProjectionMode(ProjectionMode p_projectionMode);

		//private:
			MATHGL::Matrix4 calculateProjectionMatrix(uint16_t p_windowWidth, uint16_t p_windowHeight) const;
			MATHGL::Matrix4 calculateViewMatrix(const MATHGL::Vector3& p_position, const MATHGL::Quaternion& p_rotation) const;

		private:
			Frustum frustum;
			MATHGL::Matrix4 viewMatrix;
			MATHGL::Matrix4 projectionMatrix;
			ProjectionMode  projectionMode;

			float fov;
			float size;
			float _near;
			float _far;

			bool frustumGeometryCulling;
			bool frustumLightCulling;
		};
	}
}
