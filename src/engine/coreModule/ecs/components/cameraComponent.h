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
		[[nodiscard]] Descriptor getDescriptor() const;
	public:
		static auto GetMembers() {
			return std::tuple{
				IKIGAI::UTILS::MakeMemberInfo("Far", &CameraComponent::getFar, &CameraComponent::setFar,
				UTILS::Meta_t{
					{UTILS::MetaParam::FLAGS, UTILS::MetaInfo::USE_IN_EDITOR_COMPONENT_INSPECTOR},
				{UTILS::MetaParam::EDIT_RANGE, MATH::Vector2f{0.0f, 10000.0f}},
				{UTILS::MetaParam::EDIT_STEP, 1.0f},
				{UTILS::MetaParam::EDIT_WIDGET, UTILS::WidgetType::DRAG_FLOAT},
				}),
				IKIGAI::UTILS::MakeMemberInfo("Fov", &CameraComponent::getFov, &CameraComponent::setFov,
				UTILS::Meta_t{
					{UTILS::MetaParam::FLAGS, UTILS::MetaInfo::USE_IN_EDITOR_COMPONENT_INSPECTOR},
				{UTILS::MetaParam::EDIT_RANGE, MATH::Vector2f{0.0f, 200.0f}},
				{UTILS::MetaParam::EDIT_STEP, 1.0f},
				{UTILS::MetaParam::EDIT_WIDGET, UTILS::WidgetType::DRAG_FLOAT},
				}),
				//IKIGAI::UTILS::MakeMemberInfo("Size", &CameraComponent::getSize, &CameraComponent::setSize,
				//UTILS::Meta_t{
				//	{UTILS::MetaParam::FLAGS, UTILS::MetaInfo::USE_IN_EDITOR_COMPONENT_INSPECTOR},
				//{UTILS::MetaParam::EDIT_WIDGET, UTILS::WidgetType::DRAG_COLOR_2},
				//}),
				IKIGAI::UTILS::MakeMemberInfo("Near", &CameraComponent::getNear, &CameraComponent::setNear,
				UTILS::Meta_t{
					{UTILS::MetaParam::FLAGS, UTILS::MetaInfo::USE_IN_EDITOR_COMPONENT_INSPECTOR},
				{UTILS::MetaParam::EDIT_RANGE, MATH::Vector2f{0.0f, 1000.0f}},
				{UTILS::MetaParam::EDIT_STEP, 1.0f},
				{UTILS::MetaParam::EDIT_WIDGET, UTILS::WidgetType::DRAG_FLOAT},
				}),
				IKIGAI::UTILS::MakeMemberInfo("ProjectionMode", &CameraComponent::getProjectionMode, &CameraComponent::setProjectionMode,
				UTILS::Meta_t{
					{UTILS::MetaParam::FLAGS, UTILS::MetaInfo::USE_IN_EDITOR_COMPONENT_INSPECTOR},
				{UTILS::MetaParam::EDIT_WIDGET, UTILS::WidgetType::COMBO},
				}),
				IKIGAI::UTILS::MakeMemberInfo("FrustumCulling", &CameraComponent::isFrustumGeometryCulling, &CameraComponent::setFrustumGeometryCulling,
				UTILS::Meta_t{
					{UTILS::MetaParam::FLAGS, UTILS::MetaInfo::USE_IN_EDITOR_COMPONENT_INSPECTOR},
				{UTILS::MetaParam::EDIT_WIDGET, UTILS::WidgetType::BOOL},
				}),
				IKIGAI::UTILS::MakeMemberInfo("FrustumLight", &CameraComponent::isFrustumLightCulling, &CameraComponent::setFrustumLightCulling,
				UTILS::Meta_t{
					{UTILS::MetaParam::FLAGS, UTILS::MetaInfo::USE_IN_EDITOR_COMPONENT_INSPECTOR},
				{UTILS::MetaParam::EDIT_WIDGET, UTILS::WidgetType::BOOL},
				}),
				IKIGAI::UTILS::MakeMemberInfo("FrustumCullingBVH", &CameraComponent::isFrustumGeometryBVHCulling, &CameraComponent::setFrustumGeometryBVHCulling,
				UTILS::Meta_t{
					{UTILS::MetaParam::FLAGS, UTILS::MetaInfo::USE_IN_EDITOR_COMPONENT_INSPECTOR},
				{UTILS::MetaParam::EDIT_WIDGET, UTILS::WidgetType::BOOL},
				})
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
		[[nodiscard]] Descriptor getDescriptor() const;
		
		float EyeDistance = 0.1f;
		float FocusDistance = 10.0f;
	public:
		static auto GetMembers() {
			return std::tuple{
				IKIGAI::UTILS::MakeMemberInfoLambda<VrCameraComponent, float>("Far",
				[](VrCameraComponent& obj) { return obj.getFar(); },
				[](VrCameraComponent& obj, float data) { obj.setFar(data); },
				UTILS::Meta_t{
					{UTILS::MetaParam::FLAGS, UTILS::MetaInfo::USE_IN_EDITOR_COMPONENT_INSPECTOR},
					{UTILS::MetaParam::EDIT_RANGE, MATH::Vector2f{0.0f, 10000.0f}},
					{UTILS::MetaParam::EDIT_STEP, 1.0f},
					{UTILS::MetaParam::EDIT_WIDGET, UTILS::WidgetType::DRAG_FLOAT},
				}),
				IKIGAI::UTILS::MakeMemberInfoLambda<VrCameraComponent, float>("Fov", 
				[](VrCameraComponent& obj) { return obj.getFov(); },
				[](VrCameraComponent& obj, float data) { obj.setFov(data); },
				UTILS::Meta_t{
					{UTILS::MetaParam::FLAGS, UTILS::MetaInfo::USE_IN_EDITOR_COMPONENT_INSPECTOR},
					{UTILS::MetaParam::EDIT_RANGE, MATH::Vector2f{0.0f, 200.0f}},
					{UTILS::MetaParam::EDIT_STEP, 1.0f},
					{UTILS::MetaParam::EDIT_WIDGET, UTILS::WidgetType::DRAG_FLOAT},
				}),
				//IKIGAI::UTILS::MakeMemberInfo("Size", &CameraComponent::getSize, &CameraComponent::setSize,
				//UTILS::Meta_t{
				//	{UTILS::MetaParam::FLAGS, UTILS::MetaInfo::USE_IN_EDITOR_COMPONENT_INSPECTOR},
				//{UTILS::MetaParam::EDIT_WIDGET, UTILS::WidgetType::DRAG_COLOR_2},
				//}),
				IKIGAI::UTILS::MakeMemberInfoLambda<VrCameraComponent, float>("Near",
				[](VrCameraComponent& obj) { return obj.getNear(); },
				[](VrCameraComponent& obj, float data) { obj.setNear(data); },
				UTILS::Meta_t{
					{UTILS::MetaParam::FLAGS, UTILS::MetaInfo::USE_IN_EDITOR_COMPONENT_INSPECTOR},
					{UTILS::MetaParam::EDIT_RANGE, MATH::Vector2f{0.0f, 1000.0f}},
					{UTILS::MetaParam::EDIT_STEP, 1.0f},
					{UTILS::MetaParam::EDIT_WIDGET, UTILS::WidgetType::DRAG_FLOAT},
				}),
				IKIGAI::UTILS::MakeMemberInfoLambda<VrCameraComponent, RENDER::Camera::ProjectionMode>("ProjectionMode",
				[](VrCameraComponent& obj) { return obj.getProjectionMode(); },
				[](VrCameraComponent& obj, RENDER::Camera::ProjectionMode data) { obj.setProjectionMode(data); },
				UTILS::Meta_t{
					{UTILS::MetaParam::FLAGS, UTILS::MetaInfo::USE_IN_EDITOR_COMPONENT_INSPECTOR},
					{UTILS::MetaParam::EDIT_WIDGET, UTILS::WidgetType::COMBO},
				}),
				IKIGAI::UTILS::MakeMemberInfoLambda<VrCameraComponent, bool>("FrustumCulling",
				[](VrCameraComponent& obj) { return obj.isFrustumGeometryCulling(); },
				[](VrCameraComponent& obj, bool data) { obj.setFrustumGeometryCulling(data); },
				UTILS::Meta_t{
					{UTILS::MetaParam::FLAGS, UTILS::MetaInfo::USE_IN_EDITOR_COMPONENT_INSPECTOR},
					{UTILS::MetaParam::EDIT_WIDGET, UTILS::WidgetType::BOOL},
				}),
				IKIGAI::UTILS::MakeMemberInfoLambda<VrCameraComponent, bool>("FrustumLight",
				[](VrCameraComponent& obj) { return obj.isFrustumLightCulling(); },
				[](VrCameraComponent& obj, bool data) { obj.setFrustumLightCulling(data); },
				UTILS::Meta_t{
					{UTILS::MetaParam::FLAGS, UTILS::MetaInfo::USE_IN_EDITOR_COMPONENT_INSPECTOR},
					{UTILS::MetaParam::EDIT_WIDGET, UTILS::WidgetType::BOOL},
				}),
				IKIGAI::UTILS::MakeMemberInfoLambda<VrCameraComponent, bool>("FrustumCullingBVH", 
				[](VrCameraComponent& obj) { return obj.isFrustumGeometryBVHCulling(); },
				[](VrCameraComponent& obj, bool data) { obj.setFrustumGeometryBVHCulling(data); },
				UTILS::Meta_t{
					{UTILS::MetaParam::FLAGS, UTILS::MetaInfo::USE_IN_EDITOR_COMPONENT_INSPECTOR},
					{UTILS::MetaParam::EDIT_WIDGET, UTILS::WidgetType::BOOL},
				})
			};
		}
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