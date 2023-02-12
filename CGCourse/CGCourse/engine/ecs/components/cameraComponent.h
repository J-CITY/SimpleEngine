#pragma once

#include "component.h"
#include "../../render/camera.h"
#include <opencv2/aruco.hpp>
#include <opencv2/opencv.hpp>
namespace KUMA
{
	namespace RESOURCES
	{
		class Texture;
	}
}

namespace KUMA::ECS { class Object; }

import glmath;

namespace KUMA::ECS {
	class CameraComponent : public Component {
	public:
		int blurAmount = 0;
		float exposure = 1.0f;
		
		CameraComponent(Ref<ECS::Object> obj);
		~CameraComponent() override = default;
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

	class VrCameraComponent : public Component {
	public:
		int blurAmount = 0;
		float exposure = 1.0f;

		std::shared_ptr<RESOURCES::Texture> leftTexture;
		std::shared_ptr<RESOURCES::Texture> rightTexture;

		VrCameraComponent(Ref<ECS::Object> obj);;
		~VrCameraComponent() override = default;

		void onUpdate(std::chrono::duration<double> dt) override;
		void updateEyes();
		//private:
		std::shared_ptr<ECS::Object> left;
		std::shared_ptr<ECS::Object> right;

		RENDER::Camera camera;
		
		float EyeDistance = 0.1f;
		float FocusDistance = 10.0f;
	};

	class ArCameraComponent : public Component {
	public:
		std::shared_ptr<RESOURCES::Texture> cameraTexture;

		cv::VideoCapture cap;
		cv::Mat frame;
		cv::aruco::Dictionary dictionary;
		cv::aruco::ArucoDetector markerDetector;
		cv::Mat cameraMatrix;
		cv::Mat distCoeffs;

		bool markerFind = false;

		MATHGL::Matrix4 view;

		ArCameraComponent(Ref<ECS::Object> obj);;
		~ArCameraComponent() override = default;

		void onUpdate(std::chrono::duration<double> dt) override;
		//private:
	};
}