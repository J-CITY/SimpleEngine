#pragma once

#include "component.h"
import glmath;
#include <functional>
#include <rttr/registration_friend.h>

#include "../../resourceManager/serializerInterface.h"

//#include "../../utils/refl.hpp"

namespace IKIGAI::ECS { class Object; }

namespace IKIGAI::ECS {

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
		Transform(IKIGAI::MATHGL::Vector3 localPosition = IKIGAI::MATHGL::Vector3(0.0f, 0.0f, 0.0f),
			IKIGAI::MATHGL::Quaternion localRotation = IKIGAI::MATHGL::Quaternion::Identity, 
			IKIGAI::MATHGL::Vector3 localScale = IKIGAI::MATHGL::Vector3(1.0f, 1.0f, 1.0f));

		~Transform();
		void notificationHandler(TransformNotifier::Notification notification);
		void setParent(Transform& parent);
		bool removeParent();
		bool hasParent() const;

		void generateMatrices(IKIGAI::MATHGL::Vector3 position, 
			IKIGAI::MATHGL::Quaternion rotation, 
			IKIGAI::MATHGL::Vector3 scale);

		void updateWorldMatrix();
		void setLocalPosition(IKIGAI::MATHGL::Vector3 p_newPosition);
		void setLocalRotation(IKIGAI::MATHGL::Quaternion p_newRotation);
		void setLocalScale(IKIGAI::MATHGL::Vector3 p_newScale);
		void setLocalAnchor(IKIGAI::MATHGL::Vector2f p_new);
		void setLocalPivot(IKIGAI::MATHGL::Vector2f p_new);
		void setLocalSize(IKIGAI::MATHGL::Vector2f p_new);
		void translateLocal(const IKIGAI::MATHGL::Vector3& p_translation);
		void rotateLocal(const IKIGAI::MATHGL::Quaternion& p_rotation);
		void scaleLocal(const IKIGAI::MATHGL::Vector3& p_scale);
		const IKIGAI::MATHGL::Vector3& getLocalPosition() const;
		const IKIGAI::MATHGL::Quaternion& getLocalRotation() const;
		const IKIGAI::MATHGL::Vector3& getLocalScale() const;
		const IKIGAI::MATHGL::Vector2f& getLocalAnchor() const;
		const IKIGAI::MATHGL::Vector2f& getLocalPivot() const;
		const IKIGAI::MATHGL::Vector2f& getLocalSize() const;
		const IKIGAI::MATHGL::Vector3& getWorldPosition() const;
		const IKIGAI::MATHGL::Quaternion& getWorldRotation() const;
		const IKIGAI::MATHGL::Vector3& getWorldScale() const;
		const IKIGAI::MATHGL::Matrix4& getLocalMatrix() const;
		const MATHGL::Matrix4& getWorldMatrix() const;
		const MATHGL::Matrix4& getPrevWorldMatrix() const;
		IKIGAI::MATHGL::Vector3 getWorldForward() const;
		IKIGAI::MATHGL::Vector3 getWorldUp() const;
		IKIGAI::MATHGL::Vector3 getWorldRight() const;
		IKIGAI::MATHGL::Vector3 getLocalForward() const;
		IKIGAI::MATHGL::Vector3 getLocalUp() const;
		IKIGAI::MATHGL::Vector3 getLocalRight() const;
		void setPrevWorldMatrix(MATHGL::Matrix4 m);

		void turnOnAnchorPivot();
		void turnOffAnchorPivot();
		bool isAnchorPivotMode() const;

		TransformNotifier notifier;
		TransformNotifier::NotificationHandlerID notificationHandlerID;
	private:
		void preDecomposeWorldMatrix();
		void preDecomposeLocalMatrix();

		IKIGAI::MATHGL::Vector3 localPosition;
		IKIGAI::MATHGL::Quaternion localRotation;
		IKIGAI::MATHGL::Vector3 localScale;
		IKIGAI::MATHGL::Vector3 worldPosition;
		IKIGAI::MATHGL::Quaternion worldRotation;
		IKIGAI::MATHGL::Vector3 worldScale;
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
		RTTR_REGISTRATION_FRIEND
	public:
		TransformComponent(Ref<ECS::Object> obj,
			IKIGAI::MATHGL::Vector3 localPosition = IKIGAI::MATHGL::Vector3(0.0f, 0.0f, 0.0f),
			MATHGL::Quaternion localRotation = MATHGL::Quaternion::Identity, 
			IKIGAI::MATHGL::Vector3 localScale = IKIGAI::MATHGL::Vector3(1.0f, 1.0f, 1.0f));

		void setParent(TransformComponent& p_parent);
		bool removeParent();
		bool hasParent() const;
		void setLocalPosition(IKIGAI::MATHGL::Vector3 p_newPosition);

		void setLocalRotation(MATHGL::Quaternion p_newRotation);

		void setLocalScale(IKIGAI::MATHGL::Vector3 p_newScale);

		void translateLocal(const IKIGAI::MATHGL::Vector3& p_translation);

		void rotateLocal(const MATHGL::Quaternion& p_rotation);

		void scaleLocal(const IKIGAI::MATHGL::Vector3& p_scale);
		const IKIGAI::MATHGL::Vector3& getLocalPosition() const;

		const MATHGL::Quaternion& getLocalRotation() const;

		const IKIGAI::MATHGL::Vector3& getLocalScale() const;

		const IKIGAI::MATHGL::Vector3& getWorldPosition() const;

		const MATHGL::Quaternion& getWorldRotation() const;
		const IKIGAI::MATHGL::Vector3& getWorldScale() const;

		const MATHGL::Matrix4& getLocalMatrix() const;
		const MATHGL::Matrix4& getWorldMatrix() const;
		Transform& getTransform();
		IKIGAI::MATHGL::Vector3 getWorldForward() const;
		IKIGAI::MATHGL::Vector3 getWorldUp() const;

		IKIGAI::MATHGL::Vector3 getWorldRight() const;
		IKIGAI::MATHGL::Vector3 getLocalForward() const;
		IKIGAI::MATHGL::Vector3 getLocalUp() const;

		IKIGAI::MATHGL::Vector3 getLocalRight() const;

		MATHGL::Matrix3 GetNormalMatrix(const MATHGL::Matrix4& model) const;

		[[nodiscard]] MATHGL::Vector3 getLocalRotationDeg() const;
		void setLocalRotationDeg(MATHGL::Vector3 val);
	private:
		[[nodiscard]] float getLocalScaleX() const;
		[[nodiscard]] float getLocalScaleY() const;
		[[nodiscard]] float getLocalScaleZ() const;
		[[nodiscard]] MATHGL::Vector3 getLocalScale_() const;

		void setLocalScaleX(float val);
		void setLocalScaleY(float val);
		void setLocalScaleZ(float val);
		void setLocalScale_(MATHGL::Vector3 val);

		[[nodiscard]] float getLocalPositionX() const;
		[[nodiscard]] float getLocalPositionY() const;
		[[nodiscard]] float getLocalPositionZ() const;
		[[nodiscard]] MATHGL::Vector3 getLocalPosition_() const;

		void setLocalPositionX(float val);
		void setLocalPositionY(float val);
		void setLocalPositionZ(float val);
		void setLocalPosition_(MATHGL::Vector3 val);

		//In degree
		[[nodiscard]] float getLocalRotationX() const;
		[[nodiscard]] float getLocalRotationY() const;
		[[nodiscard]] float getLocalRotationZ() const;

		void setLocalRotationX(float val);
		void setLocalRotationY(float val);
		void setLocalRotationZ(float val);

	private:
		std::shared_ptr<Transform> transform;
	};
}

struct MetaInfo {
	constexpr static const char* FLAGS = "FLAGS";
	constexpr static const char* DEFAULT = "DEFAULT";

	enum Flags : uint32_t {
		NONE = 0,
		SERIALIZABLE = 1 << 0,
		USE_IN_EDITOR_ANIMATION = 1 << 1,
		USE_IN_EDITOR_COMPONENT_INSPECTOR = 1 << 2,
		OPTIONAL_PARAM = 1 << 3,
	};
};

struct EditorMetaInfo {
	constexpr static const char* EDIT_RANGE = "EDIT_RANGE";
	constexpr static const char* EDIT_WIDGET = "EDIT_WIDGET";
	constexpr static const char* EDIT_STEP = "EDIT_STEP";
	constexpr static const char* FILE_EXTENSION = "FILE_EXTENSION";

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
		COMBO,
		BOOL,
		STRING,
		STRING_WITH_FILE_CHOOSE,
		STRINGS_ARRAY,
		OPT_STRING,

		MATERIAL,
		MODEL_LOD
	};
};

struct Pair {
	float first = 0.0f;
	float second = 0.0f;
};

//REFL_AUTO(
//	type(IKIGAI::ECS::Transform),
//	field(localRotation),
//	field(localRotation),
//	field(localScale)
//)
