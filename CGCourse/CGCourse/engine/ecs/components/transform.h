#pragma once

#include "component.h"
import glmath;
#include <functional>

#include "../../resourceManager/serializerInterface.h"

//#include "../../utils/refl.hpp"

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
		void setLocalAnchor(KUMA::MATHGL::Vector2f p_new);
		void setLocalPivot(KUMA::MATHGL::Vector2f p_new);
		void setLocalSize(KUMA::MATHGL::Vector2f p_new);
		void translateLocal(const KUMA::MATHGL::Vector3& p_translation);
		void rotateLocal(const KUMA::MATHGL::Quaternion& p_rotation);
		void scaleLocal(const KUMA::MATHGL::Vector3& p_scale);
		const KUMA::MATHGL::Vector3& getLocalPosition() const;
		const KUMA::MATHGL::Quaternion& getLocalRotation() const;
		const KUMA::MATHGL::Vector3& getLocalScale() const;
		const KUMA::MATHGL::Vector2f& getLocalAnchor() const;
		const KUMA::MATHGL::Vector2f& getLocalPivot() const;
		const KUMA::MATHGL::Vector2f& getLocalSize() const;
		const KUMA::MATHGL::Vector3& getWorldPosition() const;
		const KUMA::MATHGL::Quaternion& getWorldRotation() const;
		const KUMA::MATHGL::Vector3& getWorldScale() const;
		const KUMA::MATHGL::Matrix4& getLocalMatrix() const;
		const MATHGL::Matrix4& getWorldMatrix() const;
		const MATHGL::Matrix4& getPrevWorldMatrix() const;
		KUMA::MATHGL::Vector3 getWorldForward() const;
		KUMA::MATHGL::Vector3 getWorldUp() const;
		KUMA::MATHGL::Vector3 getWorldRight() const;
		KUMA::MATHGL::Vector3 getLocalForward() const;
		KUMA::MATHGL::Vector3 getLocalUp() const;
		KUMA::MATHGL::Vector3 getLocalRight() const;
		void setPrevWorldMatrix(MATHGL::Matrix4 m);

		void turnOnAnchorPivot() {
			use2d = true;
		}
		void turnOffAnchorPivot() {
			use2d = false;
		}

		bool isAnchorPivotMode() {
			return use2d;
		}

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
		MATHGL::Matrix4 prevWorldMatrix;
		Transform* parent = nullptr;

		//for 2d
		bool use2d = false;
		MATHGL::Vector2f mAnchor;
		MATHGL::Vector2f mPivot;
		MATHGL::Vector2f mSize;
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
			if (transform->getWorldScale().x == transform->getWorldScale().y && transform->getWorldScale().y == transform->getWorldScale().z) {
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
			transform->setLocalPosition(dump);
			RESOURCES::DeserializeQuat(j["data"]["rot"], dumpq);
			transform->setLocalRotation(dumpq);
			RESOURCES::DeserializeVec3(j["data"]["scale"], dump);
			transform->setLocalScale(dump);
		}
		virtual void onSerialize(nlohmann::json& j) override {
			RESOURCES::SerializeVec3(j["data"]["pos"], transform->getLocalPosition());
			RESOURCES::SerializeQuat(j["data"]["rot"], transform->getLocalRotation());
			RESOURCES::SerializeVec3(j["data"]["scale"], transform->getLocalScale());
		}

		float getLocalScaleX() const {
			return transform->getLocalScale().x;
		}
		float getLocalScaleY() const {
			return transform->getLocalScale().y;
		}
		float getLocalScaleZ() const {
			return transform->getLocalScale().z;
		}
		MATHGL::Vector3 getLocalScale_() const {
			return transform->getLocalScale();
		}

		void setLocalScaleX(float val) {
			transform->setLocalScale({ val, getLocalScaleY(), getLocalScaleZ() });
		}
		void setLocalScaleY(float val) {
			transform->setLocalScale({ getLocalScaleX(), val, getLocalScaleZ() });
		}
		void setLocalScaleZ(float val) {
			transform->setLocalScale({ getLocalScaleX(), getLocalScaleY(), val});
		}
		void setLocalScale_(MATHGL::Vector3 val) {
			transform->setLocalScale(val);
		}

		float getLocalPositionX() const {
			return transform->getLocalPosition().x;
		}
		float getLocalPositionY() const {
			return transform->getLocalPosition().y;
		}
		float getLocalPositionZ() const {
			return transform->getLocalPosition().z;
		}
		MATHGL::Vector3 getLocalPosition_() const {
			return transform->getLocalPosition();
		}

		void setLocalPositionX(float val) {
			transform->setLocalPosition({ val, getLocalPositionY(), getLocalPositionZ() });
		}
		void setLocalPositionY(float val) {
			transform->setLocalPosition({ getLocalPositionX(), val, getLocalPositionZ() });
		}
		void setLocalPositionZ(float val) {
			transform->setLocalPosition({ getLocalPositionX(), getLocalPositionY(), val });
		}
		void setLocalPosition_(MATHGL::Vector3 val) {
			transform->setLocalPosition(val);
		}

		//In degree
		float getLocalRotationX() const {
			const auto rot = MATHGL::Quaternion::ToEulerAngles(getLocalRotation());
			return TO_DEGREES(rot.x);
		}
		float getLocalRotationY() const {
			const auto rot = MATHGL::Quaternion::ToEulerAngles(getLocalRotation());
			return TO_DEGREES(rot.y);
		}
		float getLocalRotationZ() const {
			const auto rot = MATHGL::Quaternion::ToEulerAngles(getLocalRotation());
			return TO_DEGREES(rot.z);
		}
		MATHGL::Vector3 getLocalRotationDeg() const {
			auto rot = MATHGL::Quaternion::ToEulerAngles(getLocalRotation());
			rot.x = TO_DEGREES(rot.x);
			rot.y = TO_DEGREES(rot.y);
			rot.z = TO_DEGREES(rot.z);
			return rot;
		}

		void setLocalRotationX(float val) {
			if (val > 180.0f) { val = -180.0f + (val - 180.0f); }
			else if (val < -180.0f) { val = 180.0f + (val + 180.0f); }
			const auto rot = MATHGL::Quaternion::ToEulerAngles(getLocalRotation());
			MATHGL::Vector3 rotVec = MATHGL::Vector3{ val, TO_DEGREES(rot.y), TO_DEGREES(rot.z) };
			setLocalRotation(MATHGL::Quaternion({ rotVec }));
		}
		void setLocalRotationY(float val) {
			if (val > 89.0f) { val = 89.0f; }
			else if (val < -89.0f) { val = -89.0f; }
			const auto rot = MATHGL::Quaternion::ToEulerAngles(getLocalRotation());
			MATHGL::Vector3 rotVec = MATHGL::Vector3{ TO_DEGREES(rot.x), val, TO_DEGREES(rot.z) };
			setLocalRotation(MATHGL::Quaternion({ rotVec }));
		}
		void setLocalRotationZ(float val) {
			if (val > 180.0f) { val = -180.0f + (val - 180.0f); }
			else if (val < -180.0f) { val = 180.0f + (val + 180.0f); }
			const auto rot = MATHGL::Quaternion::ToEulerAngles(getLocalRotation());
			MATHGL::Vector3 rotVec = MATHGL::Vector3{ TO_DEGREES(rot.x), TO_DEGREES(rot.y), val };
			setLocalRotation(MATHGL::Quaternion({ rotVec }));
		}
		void setLocalRotationDeg(MATHGL::Vector3 val) {
			if (val.x > 180.0f) { val.x = -180.0f + (val.x - 180.0f); }
			else if (val.x < -180.0f) { val.x = 180.0f + (val.x + 180.0f); }
			if (val.z > 180.0f) { val.z = -180.0f + (val.z - 180.0f); }
			else if (val.z < -180.0f) { val.z = 180.0f + (val.z + 180.0f); }
			if (val.y > 89.0f) { val.y = 89.0f; }
			else if (val.y < -89.0f) { val.y = -89.0f; }
			setLocalRotation(MATHGL::Quaternion({ val }));
		}

	private:
		std::shared_ptr<Transform> transform;
	};
}

struct MetaInfo {
	constexpr static const char* FLAGS = "FLAGS";

	enum Flags : uint32_t {
		SERIALIZABLE = 1 << 0,
		USE_IN_EDITOR = 1 << 1,
		USE_IN_ANIMATION = 1 << 2,
		USE_IN_COMPONENT_INSPECTOR = 1 << 3,
	};
};

struct EditorMetaInfo {
	constexpr static const char* EDIT_RANGE = "EDIT_RANGE";
	constexpr static const char* EDIT_WIDGET = "EDIT_WIDGET";
	constexpr static const char* EDIT_STEP = "EDIT_STEP";

	enum Flags : uint32_t {
		SERIALIZABLE = 1 << 0,
		USE_IN_EDITOR = 1 << 1,
	};

	enum WidgetType : uint32_t {
		DRAG_INT,
		DRAG_FLOAT,
		DRAG_FLOAT_3,
		DRAG_FLOAT_4,
		DRAG_COLOR_3,
		DRAG_COLOR_4,
		COMBO
	};
};

struct Pair {
	float first = 0.0f;
	float second = 0.0f;
};

//REFL_AUTO(
//	type(KUMA::ECS::Transform),
//	field(localRotation),
//	field(localRotation),
//	field(localScale)
//)
