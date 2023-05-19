#include "transform.h"
#include "../../../resourceManager/ServiceManager.h"
#include "../../../window/window.h"

using namespace KUMA::ECS;
using namespace KUMA::MATHGL;

//Transform Notifier

TransformNotifier::NotificationHandlerID TransformNotifier::addNotificationHandler(NotificationHandler notificationHandler) {
	NotificationHandlerID handlerID = availableHandlerID++;
	notificationHandlers.emplace(handlerID, notificationHandler);
	return handlerID;
}

void TransformNotifier::notifyChildren(Notification p_notification) {
	if (!notificationHandlers.empty())
		for (auto const& [id, handler] : notificationHandlers)
			handler(p_notification);
}

bool TransformNotifier::removeNotificationHandler(const NotificationHandlerID& p_notificationHandlerID) {
	return notificationHandlers.erase(p_notificationHandlerID) != 0;
}

//Transform Component

TransformComponent::TransformComponent(Ref<ECS::Object> obj, Vector3 localPosition,
	Quaternion localRotation, Vector3 localScale): Component(obj) {
	__NAME__ = "Transform";
	transform = std::make_shared<Transform>();
	transform->generateMatrices(localPosition, localRotation, localScale);
}

void TransformComponent::setParent(TransformComponent& parent) {
	transform->setParent(parent.getTransform());
}

bool TransformComponent::removeParent() {
	return transform->removeParent();
}

bool TransformComponent::hasParent() const {
	return transform->hasParent();
}

void TransformComponent::setLocalPosition(Vector3 newPosition) {
	transform->setLocalPosition(newPosition);
}

void TransformComponent::setLocalRotation(Quaternion newRotation) {
	transform->setLocalRotation(newRotation);
}

void TransformComponent::setLocalScale(Vector3 newScale) {
	transform->setLocalScale(newScale);
}

void TransformComponent::translateLocal(const Vector3 & translation) {
	transform->translateLocal(translation);
}

void TransformComponent::rotateLocal(const Quaternion & p_rotation) {
	transform->rotateLocal(p_rotation);
}

void TransformComponent::scaleLocal(const Vector3 & p_scale) {
	transform->scaleLocal(p_scale);
}

const Vector3 & TransformComponent::getLocalPosition() const {
	return transform->getLocalPosition();
}

const Quaternion & TransformComponent::getLocalRotation() const {
	return transform->getLocalRotation();
}

const Vector3 & TransformComponent::getLocalScale() const {
	return transform->getLocalScale();
}

const Vector3 & TransformComponent::getWorldPosition() const {
	return transform->getWorldPosition();
}

const Quaternion& TransformComponent::getWorldRotation() const {
	return transform->getWorldRotation();
}

const Vector3 & TransformComponent::getWorldScale() const {
	return transform->getWorldScale();
}

const Matrix4 & TransformComponent::getLocalMatrix() const {
	return transform->getLocalMatrix();
}

const Matrix4 & TransformComponent::getWorldMatrix() const {
	return transform->getWorldMatrix();
}

Transform& TransformComponent::getTransform() {
	return *transform;
}

Vector3 TransformComponent::getWorldForward() const {
	return transform->getWorldForward();
}

Vector3 TransformComponent::getWorldUp() const {
	return transform->getWorldUp();
}

Vector3 TransformComponent::getWorldRight() const {
	return transform->getWorldRight();
}

Vector3 TransformComponent::getLocalForward() const {
	return transform->getLocalForward();
}

Vector3 TransformComponent::getLocalUp() const {
	return transform->getLocalUp();
}

Vector3 TransformComponent::getLocalRight() const {
	return transform->getLocalRight();
}

//Transform

Transform::Transform(Vector3 localPosition, Quaternion localRotation, Vector3 localScale) :
	notificationHandlerID(-1),
	parent(nullptr) {
	generateMatrices(localPosition, localRotation, localScale);
}

Transform::~Transform() {
	notifier.notifyChildren(TransformNotifier::Notification::DESTROYED);
}

void Transform::notificationHandler(TransformNotifier::Notification p_notification) {
	switch (p_notification) {
	case TransformNotifier::Notification::CHANGED:
		updateWorldMatrix();
		break;

	case TransformNotifier::Notification::DESTROYED:
		parent = nullptr;
		generateMatrices(worldPosition, worldRotation, worldScale);
		updateWorldMatrix();
		break;
	}
}

void Transform::setParent(Transform& p_parent) {
	parent = &p_parent;

	notificationHandlerID = parent->notifier.addNotificationHandler(std::bind(&Transform::notificationHandler, this, std::placeholders::_1));

	updateWorldMatrix();
}

bool Transform::removeParent() {
	if (parent != nullptr) {
		parent->notifier.removeNotificationHandler(notificationHandlerID);
		parent = nullptr;
		updateWorldMatrix();

		return true;
	}

	return false;
}

bool Transform::hasParent() const {
	return parent != nullptr;
}


void Transform::generateMatrices(Vector3 position, Quaternion rotation, Vector3 scale) {
	if (use2d) {
		auto screenSz = RESOURCES::ServiceManager::Get<WINDOW_SYSTEM::Window>().getSize();
		auto parentSize = hasParent() ? parent->mSize : MATHGL::Vector2f(screenSz.x, screenSz.y);

		localMatrix = MATHGL::Matrix4();
		localMatrix *= MATHGL::Matrix4::Translation(MATHGL::Vector3(parentSize.x * mAnchor.x, parentSize.y * mAnchor.y, 0.0f));
		localMatrix *= MATHGL::Matrix4::Translation(MATHGL::Vector3(mSize.x * -mPivot.x * localScale.x,
			mSize.y * -mPivot.y * localScale.y, 0.0f));
		localMatrix *= MATHGL::Matrix4::Translation(position);
		localMatrix *= MATHGL::Matrix4::Translation({ mSize.x * mPivot.x * localScale.x, mSize.y * mPivot.y * localScale.y, 0.0f });
		localMatrix *= MATHGL::Quaternion::ToMatrix4(MATHGL::Quaternion::Normalize(localRotation));
		localMatrix *= MATHGL::Matrix4::Translation({ mSize.x * -mPivot.x * localScale.x, mSize.y * -mPivot.y * localScale.y, 0.0f });
		localMatrix *= MATHGL::Matrix4::Scaling(localScale/* * MATHGL::Vector3(mSize.x, mSize.y, 1.0f)*/);
	}
	else {
		localMatrix = Matrix4::Translation(position) * Quaternion::ToMatrix4(Quaternion::Normalize(rotation)) * Matrix4::Scaling(scale);
	}
	localPosition = position;
	localRotation = rotation;
	localScale = scale;
	updateWorldMatrix();
}

void Transform::updateWorldMatrix() {
	prevWorldMatrix = worldMatrix;
	worldMatrix = hasParent() ? parent->worldMatrix * localMatrix : localMatrix;
	preDecomposeWorldMatrix();
	notifier.notifyChildren(TransformNotifier::Notification::CHANGED);
}

void Transform::setLocalPosition(Vector3 newPosition) {
	generateMatrices(newPosition, localRotation, localScale);
}

void Transform::setLocalRotation(Quaternion p_newRotation) {
	generateMatrices(localPosition, p_newRotation, localScale);
}

void Transform::setLocalScale(Vector3 p_newScale) {
	generateMatrices(localPosition, localRotation, p_newScale);
}

void Transform::setLocalAnchor(Vector2f p_new) {
	mAnchor = p_new;
	generateMatrices(localPosition, localRotation, localScale);
}
void Transform::setLocalPivot(Vector2f p_new) {
	mPivot = p_new;
	generateMatrices(localPosition, localRotation, localScale);
}
void Transform::setLocalSize(Vector2f p_new) {
	mSize = p_new;
	generateMatrices(localPosition, localRotation, localScale);
}

void Transform::translateLocal(const Vector3& p_translation) {
	setLocalPosition(localPosition + p_translation);
}

void Transform::rotateLocal(const Quaternion& p_rotation) {
	setLocalRotation(localRotation * p_rotation);
}

void Transform::scaleLocal(const Vector3& p_scale) {
	setLocalScale(Vector3(
		localScale.x * p_scale.x,
		localScale.y * p_scale.y,
		localScale.z * p_scale.z
	));
}

const Vector3& Transform::getLocalPosition() const {
	return localPosition;
}

const Quaternion& Transform::getLocalRotation() const {
	return localRotation;
}

const Vector3& Transform::getLocalScale() const {
	return localScale;
}

const Vector2f& Transform::getLocalAnchor() const {
	return mAnchor;
}
const Vector2f& Transform::getLocalPivot() const {
	return mPivot;
}
const Vector2f& Transform::getLocalSize() const {
	return mSize;
}

const Vector3& Transform::getWorldPosition() const {
	return worldPosition;
}

const Quaternion& Transform::getWorldRotation() const {
	return worldRotation;
}

const Vector3& Transform::getWorldScale() const {
	return worldScale;
}

const Matrix4& Transform::getLocalMatrix() const {
	return localMatrix;
}

const Matrix4& Transform::getWorldMatrix() const {
	return worldMatrix;
}

const Matrix4& Transform::getPrevWorldMatrix() const {
	return prevWorldMatrix;
}

Vector3 Transform::getWorldForward() const {
	return worldRotation * Vector3::Forward;
}

Vector3 Transform::getWorldUp() const {
	return worldRotation * Vector3::Up;
}

Vector3 Transform::getWorldRight() const {
	return worldRotation * Vector3::Right;
}

Vector3 Transform::getLocalForward() const {
	return localRotation * Vector3::Forward;
}

Vector3 Transform::getLocalUp() const {
	return localRotation * Vector3::Up;
}

Vector3 Transform::getLocalRight() const {
	return localRotation * Vector3::Right;
}

void Transform::setPrevWorldMatrix(MATHGL::Matrix4 m) {
	prevWorldMatrix = m;
}

void Transform::preDecomposeWorldMatrix() {
	worldPosition.x = worldMatrix(0, 3);
	worldPosition.y = worldMatrix(1, 3);
	worldPosition.z = worldMatrix(2, 3);

	Vector3 columns[3] =
	{
		{ worldMatrix(0, 0), worldMatrix(1, 0), worldMatrix(2, 0)},
		{ worldMatrix(0, 1), worldMatrix(1, 1), worldMatrix(2, 1)},
		{ worldMatrix(0, 2), worldMatrix(1, 2), worldMatrix(2, 2)},
	};

	worldScale.x = Vector3::Length(columns[0]);
	worldScale.y = Vector3::Length(columns[1]);
	worldScale.z = Vector3::Length(columns[2]);

	if (worldScale.x) {
		columns[0] /= worldScale.x;
	}
	if (worldScale.y) {
		columns[1] /= worldScale.y;
	}
	if (worldScale.z) {
		columns[2] /= worldScale.z;
	}

	Matrix3 rotationMatrix
	(
		columns[0].x, columns[1].x, columns[2].x,
		columns[0].y, columns[1].y, columns[2].y,
		columns[0].z, columns[1].z, columns[2].z
	);

	worldRotation = Quaternion(rotationMatrix);
}

void Transform::preDecomposeLocalMatrix() {
	localPosition.x = localMatrix(0, 3);
	localPosition.y = localMatrix(1, 3);
	localPosition.z = localMatrix(2, 3);

	Vector3 columns[3] =
	{
		{ localMatrix(0, 0), localMatrix(1, 0), localMatrix(2, 0)},
		{ localMatrix(0, 1), localMatrix(1, 1), localMatrix(2, 1)},
		{ localMatrix(0, 2), localMatrix(1, 2), localMatrix(2, 2)},
	};

	localScale.x = Vector3::Length(columns[0]);
	localScale.y = Vector3::Length(columns[1]);
	localScale.z = Vector3::Length(columns[2]);

	if (localScale.x) {
		columns[0] /= localScale.x;
	}
	if (localScale.y) {
		columns[1] /= localScale.y;
	}
	if (localScale.z) {
		columns[2] /= localScale.z;
	}

	Matrix3 rotationMatrix(
		columns[0].x, columns[1].x, columns[2].x,
		columns[0].y, columns[1].y, columns[2].y,
		columns[0].z, columns[1].z, columns[2].z
	);

	localRotation = Quaternion(rotationMatrix);
}

#include <rttr/registration>

RTTR_REGISTRATION
{
	rttr::registration::class_<KUMA::ECS::TransformComponent>("TransformComponent")
	(
		rttr::metadata(MetaInfo::FLAGS, MetaInfo::SERIALIZABLE | MetaInfo::USE_IN_EDITOR | MetaInfo::USE_IN_ANIMATION)
	)
	.property("LocalScaleX", &KUMA::ECS::TransformComponent::getLocalScaleX, &KUMA::ECS::TransformComponent::setLocalScaleX)
	(
		rttr::metadata(MetaInfo::FLAGS, MetaInfo::SERIALIZABLE | MetaInfo::USE_IN_EDITOR),
		rttr::metadata(EditorMetaInfo::EDIT_RANGE, Pair { 0.0f, 10000.0f }),
		rttr::metadata(EditorMetaInfo::EDIT_STEP, 0.1f)
	)
	.property("LocalScaleY", &KUMA::ECS::TransformComponent::getLocalScaleY, &KUMA::ECS::TransformComponent::setLocalScaleY)
	(
		rttr::metadata(MetaInfo::FLAGS, MetaInfo::SERIALIZABLE | MetaInfo::USE_IN_EDITOR),
		rttr::metadata(EditorMetaInfo::EDIT_RANGE, Pair { 0.0f, 10000.0f }),
		rttr::metadata(EditorMetaInfo::EDIT_STEP, 0.1f)
	)
	.property("LocalScaleZ", &KUMA::ECS::TransformComponent::getLocalScaleZ, &KUMA::ECS::TransformComponent::setLocalScaleZ)
	(
		rttr::metadata(MetaInfo::FLAGS, MetaInfo::SERIALIZABLE | MetaInfo::USE_IN_EDITOR),
		rttr::metadata(EditorMetaInfo::EDIT_RANGE, Pair { 0.0f, 10000.0f }),
		rttr::metadata(EditorMetaInfo::EDIT_STEP, 0.1f)
	)
	.property("LocalScale", &KUMA::ECS::TransformComponent::getLocalScale_, &KUMA::ECS::TransformComponent::setLocalScale_)
	(
		rttr::metadata(MetaInfo::FLAGS, MetaInfo::SERIALIZABLE | MetaInfo::USE_IN_EDITOR | MetaInfo::USE_IN_COMPONENT_INSPECTOR),
		rttr::metadata(EditorMetaInfo::EDIT_RANGE, Pair { 0.0f, 10000.0f }),
		rttr::metadata(EditorMetaInfo::EDIT_STEP, 0.1f),
		rttr::metadata(EditorMetaInfo::EDIT_WIDGET, EditorMetaInfo::WidgetType::DRAG_FLOAT_3)
	)

	.property("LocalRotationX", &KUMA::ECS::TransformComponent::getLocalRotationX, &KUMA::ECS::TransformComponent::setLocalRotationX)
	(
		rttr::metadata(MetaInfo::FLAGS, MetaInfo::SERIALIZABLE | MetaInfo::USE_IN_EDITOR),
		rttr::metadata(EditorMetaInfo::EDIT_RANGE, Pair { 0.0f, 10000.0f }),
		rttr::metadata(EditorMetaInfo::EDIT_STEP, 0.1f)
	)
	.property("LocalRotationY", &KUMA::ECS::TransformComponent::getLocalRotationY, &KUMA::ECS::TransformComponent::setLocalRotationY)
	(
		rttr::metadata(MetaInfo::FLAGS, MetaInfo::SERIALIZABLE | MetaInfo::USE_IN_EDITOR),
		rttr::metadata(EditorMetaInfo::EDIT_RANGE, Pair { 0.0f, 10000.0f }),
		rttr::metadata(EditorMetaInfo::EDIT_STEP, 0.1f)
	)
	.property("LocalRotationZ", &KUMA::ECS::TransformComponent::getLocalRotationZ, &KUMA::ECS::TransformComponent::setLocalRotationZ)
	(
		rttr::metadata(MetaInfo::FLAGS, MetaInfo::SERIALIZABLE | MetaInfo::USE_IN_EDITOR),
		rttr::metadata(EditorMetaInfo::EDIT_RANGE, Pair { 0.0f, 10000.0f }),
		rttr::metadata(EditorMetaInfo::EDIT_STEP, 0.1f)
	)
	.property("LocalRotation", &KUMA::ECS::TransformComponent::getLocalRotationDeg, &KUMA::ECS::TransformComponent::setLocalRotationDeg)
	(
		rttr::metadata(MetaInfo::FLAGS, MetaInfo::SERIALIZABLE | MetaInfo::USE_IN_EDITOR | MetaInfo::USE_IN_COMPONENT_INSPECTOR),
		rttr::metadata(EditorMetaInfo::EDIT_RANGE, Pair { 0.0f, 10000.0f }),
		rttr::metadata(EditorMetaInfo::EDIT_STEP, 0.1f),
		rttr::metadata(EditorMetaInfo::EDIT_WIDGET, EditorMetaInfo::WidgetType::DRAG_FLOAT_3)
	)

	.property("LocalPositionX", &KUMA::ECS::TransformComponent::getLocalPositionX, &KUMA::ECS::TransformComponent::setLocalPositionX)
	(
		rttr::metadata(MetaInfo::FLAGS, MetaInfo::SERIALIZABLE | MetaInfo::USE_IN_EDITOR),
		rttr::metadata(EditorMetaInfo::EDIT_RANGE, Pair { 0.0f, 10000.0f }),
		rttr::metadata(EditorMetaInfo::EDIT_STEP, 0.1f)
	)
	.property("LocalPositionY", &KUMA::ECS::TransformComponent::getLocalPositionY, &KUMA::ECS::TransformComponent::setLocalPositionY)
	(
		rttr::metadata(MetaInfo::FLAGS, MetaInfo::SERIALIZABLE | MetaInfo::USE_IN_EDITOR),
		rttr::metadata(EditorMetaInfo::EDIT_RANGE, Pair { 0.0f, 10000.0f }),
		rttr::metadata(EditorMetaInfo::EDIT_STEP, 0.1f)
	)
	.property("LocalPositionZ", &KUMA::ECS::TransformComponent::getLocalPositionZ, &KUMA::ECS::TransformComponent::setLocalPositionZ)
	(
		rttr::metadata(MetaInfo::FLAGS, MetaInfo::SERIALIZABLE | MetaInfo::USE_IN_EDITOR),
		rttr::metadata(EditorMetaInfo::EDIT_RANGE, Pair { 0.0f, 10000.0f }),
		rttr::metadata(EditorMetaInfo::EDIT_STEP, 0.1f)
	)
	.property("LocalPosition", &KUMA::ECS::TransformComponent::getLocalPosition_, &KUMA::ECS::TransformComponent::setLocalPosition_)
	(
		rttr::metadata(MetaInfo::FLAGS, MetaInfo::SERIALIZABLE | MetaInfo::USE_IN_EDITOR | MetaInfo::USE_IN_COMPONENT_INSPECTOR),
		rttr::metadata(EditorMetaInfo::EDIT_RANGE, Pair { 0.0f, 10000.0f }),
		rttr::metadata(EditorMetaInfo::EDIT_STEP, 0.1f),
		rttr::metadata(EditorMetaInfo::EDIT_WIDGET, EditorMetaInfo::WidgetType::DRAG_FLOAT_3)
	);
}