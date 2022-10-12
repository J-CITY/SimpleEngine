#pragma once

#include "component.h"
#include "../../render/camera.h"

namespace KUMA::ECS { class Object; }

namespace KUMA::ECS {
	class CameraComponent : public Component {
	public:
		int blurAmount = 0;
		float exposure = 1.0f;
		
		CameraComponent(Ref<ECS::Object> obj);
		~CameraComponent() = default;
		void ResizeRenderTexture(size_t w, size_t h);
		void setFov(float value);
		void setSize(float value);
		void setNear(float value);
		void setFar(float value);
		void setFrustumGeometryCulling(bool enable);
		void setFrustumLightCulling(bool enable);
		void setProjectionMode(RENDER::Camera::ProjectionMode projectionMode);
		float getFov() const;
		float getSize() const;
		float getNear() const;
		float getFar() const;
		bool isFrustumGeometryCulling() const;
		bool isFrustumLightCulling() const;
		RENDER::Camera::ProjectionMode getProjectionMode() const;
		RENDER::Camera& getCamera();

		virtual void onDeserialize(nlohmann::json& j) override {
			camera.setFar(j["data"]["far"]);
			camera.setFov(j["data"]["fov"]);
			camera.setSize(j["data"]["size"]);
			camera.setNear(j["data"]["near"]);
			camera.setFrustumGeometryCulling(j["data"]["geometryCulling"]);
			camera.setFrustumLightCulling(j["data"]["lightCulling"]);
			camera.setProjectionMode(j["data"]["mode"]);
		}
		virtual void onSerialize(nlohmann::json& j) override {
			j["data"]["far"] = camera.getFar();
			j["data"]["fov"] = camera.getFov();
			j["data"]["size"] = camera.getSize();
			j["data"]["near"] = camera.getNear();
			j["data"]["geometryCulling"] = camera.isFrustumGeometryCulling();
			j["data"]["lightCulling"] = camera.isFrustumLightCulling();
			j["data"]["mode"] = camera.getProjectionMode();
		}
	private:
		RENDER::Camera camera;
	};
}