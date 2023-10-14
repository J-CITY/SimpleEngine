#include "object.h"

#include <algorithm>
#include <execution>


#include "componentManager.h"

#include "components/ambientLight.h"
#include "components/ambientSphereLight.h"
#include "components/directionalLight.h"
#include "components/inputComponent.h"
#include "components/pointLight.h"
#include "components/spotLight.h"
#include "../gui/components/spriteComponent.h"
#include "sceneModule/sceneManager.h"

using namespace IKIGAI;
using namespace IKIGAI::ECS;


EVENT::Event<Object&> Object::destroyedEvent;
EVENT::Event<Object&> Object::createdEvent;
EVENT::Event<Object&, Object&> Object::attachEvent;
EVENT::Event<Object&> Object::dettachEvent;

//Object::Object() : id(-1) {
//	transform = addComponent<TransformComponent>();
//	//createdEvent.run(*this);
//}

Object::Object(ObjectId<Object> actorID, const std::string& name, const std::string& tag) :
	id(actorID),
	name(name),
	tag(tag)//,
	/*transform(addComponent<TransformComponent>())*/ {
	transform = addComponent<TransformComponent>();
	createdEvent.run(*this);
}

Object::~Object() {
	if (isActive) {
		onDisable();
	}
	if (isReady) {
		onDestroy();
	}

	destroyedEvent.run(*this);

	std::vector<std::shared_ptr<Object>> toDetach = children;
	for (auto child : toDetach) {
		child->detachFromParent();
	}
	toDetach.clear();

	detachFromParent();
	auto components = ComponentManager::GetInstance().getComponents(id);
	std::for_each(components.begin(), components.end(), [&](UTILS::WeakPtr<Component> p_component) {
		componentRemovedEvent.run(p_component.get());
	});
	ComponentManager::GetInstance().entityDestroyed(getID());
}

const std::string& Object::getName() const {
	return name;
}

const std::string& Object::getTag() const {
	return tag;
}

void Object::setName(const std::string& val) {
	name = val;
}

void Object::setTag(const std::string& val) {
	tag = val;
}

void Object::setActive(bool val) {
	if (val != isActive) {
		isActive = val;
		recursiveActiveUpdate();
	}
}

bool Object::getIsSelfActive() const {
	return isActive;
}

bool Object::getIsActive() const {
	auto p = parent.lock();
	return isActive && (p ? p->getIsActive() : true);
}

void Object::setID(ObjectId<Object> val) {
	id = val;
}

ObjectId<Object> Object::getID() const {
	return id;
}

void Object::setParent(std::shared_ptr<Object> _parent) {
	detachFromParent();

	parent = _parent;
	getComponent<TransformComponent>()->setParent(*_parent->getComponent<TransformComponent>().get());

	_parent->children.push_back(shared_from_this());
	attachEvent.run(*this, *_parent);
}

void Object::detachFromParent() {
	dettachEvent.run(*this);

	if (auto p = parent.lock()) {
		p->children.erase(std::remove_if(p->children.begin(), p->children.end(), [this](std::shared_ptr<Object> e) {
			return e.get() == this;
		}));
	}
}

bool Object::hasParent() const {
	return parent.lock() != nullptr;
}

std::shared_ptr<Object> Object::getParent() const {
	return parent.lock();
}

ObjectId<Object> Object::getParentID() const {
	if (auto p = parent.lock()) {
		return p->getID();
	}
	return ObjectId<Object>(0);
}

std::span<std::shared_ptr<Object>> Object::getChildren() {
	return children;
}

void Object::markAsDestroy() {
	isDestroyed = true;
	for (auto child : children) {
		child->markAsDestroy();
	}
}

bool Object::isAlive() const {
	return !isDestroyed;
}

void Object::onStart() {
	isReady = true;
	auto components = ComponentManager::GetInstance().getComponents(id);
	std::for_each(components.begin(), components.end(), [](auto element) { element->onStart(); });
}

void Object::onEnable() {
	auto components = ComponentManager::GetInstance().getComponents(id);
	std::for_each(components.begin(), components.end(), [](auto element) { element->onEnable(); });
	ComponentManager::GetInstance().enable(id);
}

void Object::onDisable() {
	auto components = ComponentManager::GetInstance().getComponents(id);
	std::for_each(components.begin(), components.end(), [](auto element) { element->onDisable(); });
	ComponentManager::GetInstance().disable(id);
}

void Object::onDestroy() {
	auto components = ComponentManager::GetInstance().getComponents(id);
	std::for_each(components.begin(), components.end(), [](auto element) { element->onDestroy(); });
}

//TODO: remove when add all systems
void Object::onUpdate(std::chrono::duration<double> dt) {
	if (getIsActive()) {
		auto components = ComponentManager::GetInstance().getComponents(id);
		std::for_each(components.begin(), components.end(), [&](auto element) { element->onUpdate(dt); });
		//std::for_each(ComponentManager::GetInstance().scriptComponents[id].begin(),
		//	ComponentManager::GetInstance().scriptComponents[id].end(), [&](auto element) { element.second->onUpdate(dt); });
	}
}

//TODO: remove when add all systems
void Object::onFixedUpdate(std::chrono::duration<double> dt) {
	if (getIsActive()) {
		auto components = ComponentManager::GetInstance().getComponents(id);
		std::for_each(components.begin(), components.end(), [&](auto element) { element->onFixedUpdate(dt); });
	}
}

//TODO: remove when add all systems
void Object::onLateUpdate(std::chrono::duration<double> dt) {
	if (getIsActive()) {
		auto components = ComponentManager::GetInstance().getComponents(id);
		std::for_each(components.begin(), components.end(), [&](auto element) { element->onLateUpdate(dt); });
	}
}

void Object::recursiveActiveUpdate() {
	bool isActive = getIsActive();

	if (isActive) {
		onEnable();
		if (!isReady) {
			onStart();
		}
	}

	if (!isActive) {
		onDisable();
	}

	for (auto child : children) {
		child->recursiveActiveUpdate();
	}
}

UTILS::WeakPtr<TransformComponent> Object::getTransform() const {
	return transform;
}

ObjectData Object::getObjectData() {
	ObjectData res;
	res.parentId = hasParent() ? getParent()->getIDInt() : -1;
	res.name = getName();
	res.id = getIDInt();
	res.isActive = getIsActive();
	res.tag = getTag();
	return res;
}

int Object::getIDInt() { return static_cast<int>(id); }

void Object::setIDInt(int _id) { id = Id(id); }

int Object::getParentId() {
	auto _p = parent.lock();
	if (!_p) {
		return -1;
	}
	return _p->getIDInt();
}

void Object::setParentId(int _id) {
	if (_id < 0) {
		return;
	}
	auto& scene = RESOURCES::ServiceManager::Get<SCENE_SYSTEM::SceneManager>().getCurrentScene();
	auto _p = scene.findObjectByID(Id(_id));
	if (_p) {
		LOG_ERROR("Object::setParentId: can not find actor with id: " + std::to_string(_id));
	}
	setParent(_p);
}

#include <rttr/registration>

RTTR_REGISTRATION
{
	rttr::registration::class_<IKIGAI::ECS::ObjectData>("ObjectData")
	(
		rttr::metadata(MetaInfo::FLAGS, MetaInfo::SERIALIZABLE )
	)
	.property("Name", &IKIGAI::ECS::ObjectData::name)
	(
		rttr::metadata(MetaInfo::FLAGS, MetaInfo::SERIALIZABLE )
	)
	.property("Tag", &IKIGAI::ECS::ObjectData::tag)
	(
		rttr::metadata(MetaInfo::FLAGS, MetaInfo::SERIALIZABLE ),
		rttr::metadata(MetaInfo::DEFAULT, std::string())
	)
	.property("Id", &IKIGAI::ECS::ObjectData::id)
	(
		rttr::metadata(MetaInfo::FLAGS, MetaInfo::SERIALIZABLE )
	)
	.property("IsActive", &IKIGAI::ECS::ObjectData::isActive)
	(
		rttr::metadata(MetaInfo::FLAGS, MetaInfo::SERIALIZABLE ),
		rttr::metadata(MetaInfo::DEFAULT, true)
	)
	.property("Parent", &IKIGAI::ECS::ObjectData::parentId)
	(
		rttr::metadata(MetaInfo::FLAGS, MetaInfo::SERIALIZABLE  | MetaInfo::OPTIONAL_PARAM)
	);
}
