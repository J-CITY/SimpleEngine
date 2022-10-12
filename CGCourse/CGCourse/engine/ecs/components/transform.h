#pragma once

#include "component.h"
import glmath;
#include <functional>
#include "../../resourceManager/serializerInterface.h"

namespace KUMA::ECS { class Object; }

namespace KUMA::ECS {

	class TransformNotifier {
	public:
		enum class Notification {
			CHANGED,
			DESTROYED
		};

		using NotificationHandler = std::function<void(Notification)>;
		using NotificationHandlerID = int;
		
		NotificationHandlerID addNotificationHandler(NotificationHandler notificationHandler);
		void notifyChildren(Notification notification);
		bool removeNotificationHandler(const NotificationHandlerID& notificationHandlerID);
	private:
		std::unordered_map<NotificationHandlerID, NotificationHandler> notificationHandlers;
		NotificationHandlerID availableHandlerID = 0;
	};
	
	class Transform {
	public:
		Transform(KUMA::MATHGL::Vector3 localPosition = KUMA::MATHGL::Vector3(0.0f, 0.0f, 0.0f),
			KUMA::MATHGL::Quaternion localRotation = KUMA::MATHGL::Quaternion::Identity, 
			KUMA::MATHGL::Vector3 localScale = KUMA::MATHGL::Vector3(1.0f, 1.0f, 1.0f));

		~Transform();
		void notificationHandler(TransformNotifier::Notification notification);
		void setParent(Transform& parent);
		bool removeParent();
		bool hasParent() const;

		void generateMatrices(KUMA::MATHGL::Vector3 position, 
			KUMA::MATHGL::Quaternion rotation, 
			KUMA::MATHGL::Vector3 scale);

		void updateWorldMatrix();
		void setLocalPosition(KUMA::MATHGL::Vector3 p_newPosition);
		void setLocalRotation(KUMA::MATHGL::Quaternion p_newRotation);
		void setLocalScale(KUMA::MATHGL::Vector3 p_newScale);
		void translateLocal(const KUMA::MATHGL::Vector3& p_translation);
		void rotateLocal(const KUMA::MATHGL::Quaternion& p_rotation);
		void scaleLocal(const KUMA::MATHGL::Vector3& p_scale);
		const KUMA::MATHGL::Vector3& getLocalPosition() const;
		const KUMA::MATHGL::Quaternion& getLocalRotation() const;
		const KUMA::MATHGL::Vector3& getLocalScale() const;
		const KUMA::MATHGL::Vector3& getWorldPosition() const;
		const KUMA::MATHGL::Quaternion& getWorldRotation() const;
		const KUMA::MATHGL::Vector3& getWorldScale() const;
		const KUMA::MATHGL::Matrix4& getLocalMatrix() const;
		const MATHGL::Matrix4& getWorldMatrix() const;
		KUMA::MATHGL::Vector3 getWorldForward() const;
		KUMA::MATHGL::Vector3 getWorldUp() const;
		KUMA::MATHGL::Vector3 getWorldRight() const;
		KUMA::MATHGL::Vector3 getLocalForward() const;
		KUMA::MATHGL::Vector3 getLocalUp() const;
		KUMA::MATHGL::Vector3 getLocalRight() const;

		TransformNotifier notifier;
		TransformNotifier::NotificationHandlerID notificationHandlerID;
	private:
		void preDecomposeWorldMatrix();
		void preDecomposeLocalMatrix();

		KUMA::MATHGL::Vector3 localPosition;
		KUMA::MATHGL::Quaternion localRotation;
		KUMA::MATHGL::Vector3 localScale;
		KUMA::MATHGL::Vector3 worldPosition;
		KUMA::MATHGL::Quaternion worldRotation;
		KUMA::MATHGL::Vector3 worldScale;
		MATHGL::Matrix4 localMatrix;
		MATHGL::Matrix4 worldMatrix;
		Transform* parent;
	};
	
	class TransformComponent : public Component {
	public:
		TransformComponent(Ref<ECS::Object> obj,
			KUMA::MATHGL::Vector3 localPosition = KUMA::MATHGL::Vector3(0.0f, 0.0f, 0.0f),
			MATHGL::Quaternion localRotation = MATHGL::Quaternion::Identity, 
			KUMA::MATHGL::Vector3 localScale = KUMA::MATHGL::Vector3(1.0f, 1.0f, 1.0f));

		void setParent(TransformComponent& p_parent);
		bool removeParent();
		bool hasParent() const;
		void setLocalPosition(KUMA::MATHGL::Vector3 p_newPosition);

		void setLocalRotation(MATHGL::Quaternion p_newRotation);

		void setLocalScale(KUMA::MATHGL::Vector3 p_newScale);

		void translateLocal(const KUMA::MATHGL::Vector3& p_translation);

		void rotateLocal(const MATHGL::Quaternion& p_rotation);

		void scaleLocal(const KUMA::MATHGL::Vector3& p_scale);
		const KUMA::MATHGL::Vector3& getLocalPosition() const;

		const MATHGL::Quaternion& getLocalRotation() const;

		const KUMA::MATHGL::Vector3& getLocalScale() const;

		const KUMA::MATHGL::Vector3& getWorldPosition() const;

		const MATHGL::Quaternion& getWorldRotation() const;
		const KUMA::MATHGL::Vector3& getWorldScale() const;

		const MATHGL::Matrix4& getLocalMatrix() const;
		const MATHGL::Matrix4& getWorldMatrix() const;
		Transform& getTransform();
		KUMA::MATHGL::Vector3 getWorldForward() const;
		KUMA::MATHGL::Vector3 getWorldUp() const;

		KUMA::MATHGL::Vector3 getWorldRight() const;
		KUMA::MATHGL::Vector3 getLocalForward() const;
		KUMA::MATHGL::Vector3 getLocalUp() const;

		KUMA::MATHGL::Vector3 getLocalRight() const;

		MATHGL::Matrix3 GetNormalMatrix(const MATHGL::Matrix4& model) const {
			if (transform.getWorldScale().x == this->transform.getWorldScale().y && transform.getWorldScale().y == transform.getWorldScale().z) {
				auto buf = model;
				return MATHGL::Matrix3(
					buf(0, 0), buf(0, 1), buf(0, 2),
					buf(1, 0), buf(1, 1), buf(1, 2),
					buf(2, 0), buf(2, 1), buf(2, 2)
				);
			}
			else {
				auto buf = MATHGL::Matrix4::Transpose(MATHGL::Matrix4::Inverse(model));
				return  MATHGL::Matrix3(
					buf(0, 0), buf(0, 1), buf(0, 2),
					buf(1, 0), buf(1, 1), buf(1, 2),
					buf(2, 0), buf(2, 1), buf(2, 2)
				);
			}
		}

		virtual void onDeserialize(nlohmann::json& j) override {
			MATHGL::Vector3 dump;
			MATHGL::Quaternion dumpq;
			RESOURCES::DeserializeVec3(j["data"]["pos"], dump);
			transform.setLocalPosition(dump);
			RESOURCES::DeserializeQuat(j["data"]["rot"], dumpq);
			transform.setLocalRotation(dumpq);
			RESOURCES::DeserializeVec3(j["data"]["scale"], dump);
			transform.setLocalScale(dump);
		}
		virtual void onSerialize(nlohmann::json& j) override {
			RESOURCES::SerializeVec3(j["data"]["pos"], transform.getLocalPosition());
			RESOURCES::SerializeQuat(j["data"]["rot"], transform.getLocalRotation());
			RESOURCES::SerializeVec3(j["data"]["scale"], transform.getLocalScale());
		}
	private:
		Transform transform;
	};
}
