#pragma once

#include "component.h"
#include <renderModule/camera.h>
//#include <opencv2/aruco.hpp>
//#include <opencv2/opencv.hpp>
namespace IKIGAI
{
	namespace RENDER
	{
		class TextureInterface;
	}
}

namespace IKIGAI::ECS { class Object; }


namespace IKIGAI::ECS {
	class CameraComponent : public Component {
	public:
		struct Descriptor : public Component::Descriptor {
			std::string Type;
			float Fov = 0.0f;
			float Size = 0.0f;
			float Near = 0.0f;
			float Far = 0.0f;
			bool GeometryCulling = false;
			bool GeometryBVHCulling = false;
			bool LightCulling = false;
			RENDER::Camera::ProjectionMode Mode = RENDER::Camera::ProjectionMode::PERSPECTIVE;

			template<class Context>
			constexpr static auto serde(Context& context, Descriptor& value) {
				using Self = Descriptor;
				using namespace serde::attribute;
				serde::serde_struct(context, value)
					.field(&Self::Type, "CameraComponentType")
					.field(&Self::Fov, "Fov")
					.field(&Self::Size, "Size")
					.field(&Self::Near, "Near")
					.field(&Self::Far, "Far")
					.field(&Self::GeometryCulling, "GeometryCulling")
					.field(&Self::GeometryBVHCulling, "GeometryBVHCulling")
					.field(&Self::LightCulling, "LightCulling")
					.field(&Self::Mode, "Mode");
			}
		};
		int blurAmount = 0;
		float exposure = 1.0f;
		
		CameraComponent(UTILS::Ref<ECS::Object> _obj);
		CameraComponent(UTILS::Ref<ECS::Object> _obj, const Descriptor& _descriptor);
		CameraComponent(UTILS::Ref<ECS::Object> obj, const Component::Descriptor& descriptor) :
			CameraComponent(obj, static_cast<const Descriptor&>(descriptor)) {
		};
		~CameraComponent() override = default;
		void ResizeRenderTexture(size_t w, size_t h);
		void setFov(float value);
		void setSize(float value);
		void setNear(float value);
		void setFar(float value);
		void setFrustumGeometryCulling(bool enable);
		void setFrustumGeometryBVHCulling(bool enable);
		void setFrustumLightCulling(bool enable);
		void setProjectionMode(RENDER::Camera::ProjectionMode projectionMode);
		float getFov() const;
		float getSize() const;
		float getNear() const;
		float getFar() const;
		bool isFrustumGeometryCulling() const;
		bool isFrustumGeometryBVHCulling() const;
		bool isFrustumLightCulling() const;
		RENDER::Camera::ProjectionMode getProjectionMode() const;
		RENDER::Camera& getCamera();

	public:
		static auto GetMembers() {
			return std::tuple{
			};
		}
		
	protected:
		RENDER::Camera camera;
	};

	class VrCameraComponent : public CameraComponent {
	public:
		struct Descriptor : public Component::Descriptor {
			std::string Type;
			float Fov = 0.0f;
			float Size = 0.0f;
			float Near = 0.0f;
			float Far = 0.0f;
			bool GeometryCulling = false;
			bool GeometryBVHCulling = false;
			bool LightCulling = false;
			RENDER::Camera::ProjectionMode Mode = RENDER::Camera::ProjectionMode::PERSPECTIVE;

			template<class Context>
			constexpr static auto serde(Context& context, Descriptor& value) {
				using Self = Descriptor;
				using namespace serde::attribute;
				serde::serde_struct(context, value)
					.field(&Self::Type, "VrCameraComponentType")
					.field(&Self::Fov, "Fov")
					.field(&Self::Size, "Size")
					.field(&Self::Near, "Near")
					.field(&Self::Far, "Far")
					.field(&Self::GeometryCulling, "GeometryCulling")
					.field(&Self::GeometryBVHCulling, "GeometryBVHCulling")
					.field(&Self::LightCulling, "LightCulling")
					.field(&Self::Mode, "Mode");
			}

		public:
			static auto GetMembers() {
				return std::tuple{
				};
			}
		};
		int blurAmount = 0;
		float exposure = 1.0f;

		std::shared_ptr<RENDER::TextureInterface> leftTexture;
		std::shared_ptr<RENDER::TextureInterface> rightTexture;

		VrCameraComponent(UTILS::Ref<ECS::Object> obj);
		VrCameraComponent(UTILS::Ref<ECS::Object> _obj, const Descriptor& _descriptor);
		VrCameraComponent(UTILS::Ref<ECS::Object> obj, const Component::Descriptor& descriptor) :
			VrCameraComponent(obj, static_cast<const Descriptor&>(descriptor)) {
		};

		std::shared_ptr<IKIGAI::ECS::Object> createObject(const std::string& name);
		~VrCameraComponent() override = default;

		void onUpdate(std::chrono::duration<double> dt) override;
		void updateEyes();
		//private:
		std::shared_ptr<ECS::Object> left;
		std::shared_ptr<ECS::Object> right;

		
		float EyeDistance = 0.1f;
		float FocusDistance = 10.0f;
	};

	template <>
	inline std::string IKIGAI::ECS::GetType<VrCameraComponent>() {
		return "class IKIGAI::ECS::VrCameraComponent";
	}
	template <>
	inline std::string IKIGAI::ECS::GetType<CameraComponent>() {
		return "class IKIGAI::ECS::CameraComponent";
	}

	template <>
	inline std::string IKIGAI::ECS::GetComponentName<VrCameraComponent>() {
		return "VrCameraComponent";
	}
	template <>
	inline std::string IKIGAI::ECS::GetComponentName<CameraComponent>() {
		return "CameraComponent";
	}

	/*class ArCameraComponent : public Component {
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

		ArCameraComponent(UTILS::Ref<ECS::Object> obj);;
		~ArCameraComponent() override = default;

		void onUpdate(std::chrono::duration<double> dt) override;
		//private:
	};*/
}