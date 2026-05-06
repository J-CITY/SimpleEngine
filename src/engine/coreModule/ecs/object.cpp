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
#include "utilsModule/visitorHelper.h"
#include "utilsModule/log/loggerDefine.h"

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

template<typename T>
void WriteComponentDescriptorImpl(std::vector<ComponentsDescriptorType>& components, const ECS::Object* obj) {
	if (auto comp = obj->getComponent<T>()) {
		components.push_back(comp->getDescriptor());
	}
}

template<template<typename...> class Container, typename...ComponentType>
void WriteComponentDescriptor(std::vector<ComponentsDescriptorType>& components, const ECS::Object* obj, Container<ComponentType...> opt) {
	(WriteComponentDescriptorImpl<ComponentType>(components, obj), ...);
}

void WriteComponentDescriptor(std::vector<ComponentsDescriptorType>& components, const ECS::Object* obj) {
	WriteComponentDescriptor(components, obj, ECS::ComponentsTypeProviderType{});
}

Object::Descriptor Object::getDescriptor() const {
	Descriptor d;
	d.Name = getName();
	d.Tag = getTag();
	d.Id = getIDInt();
	d.ParentId = getParentId();
	d.IsActive = mIsActive;
	WriteComponentDescriptor(d.Components, this);
	return d;
}

Object::Object(ECS2::Entity actorID, const std::string& name, const std::string& tag) :
	mEntity(actorID),
	mName(name),
	mTag(tag) {
	mTransform = addComponent<TransformComponent>();
	createdEvent.run(*this);
}

template<class Desc, typename T>
void AddComponentImpl(const Desc& data, ECS::Object* obj) {
	if (data.Type == ECS::GetType<T>()) {
		auto c = obj->addComponent<T>(static_cast<const ComponentBase::Descriptor&>(data));
	}
}

template<class Desc, template<typename...> class Container, typename...ComponentType>
void AddComponent(const Desc& data, ECS::Object* obj, Container<ComponentType...> opt) {
	(AddComponentImpl<Desc, ComponentType>(data, obj), ...);
}

template<class Desc>
void AddComponent(const Desc& data, ECS::Object* obj) {
	AddComponent(data, obj, ECS::ComponentsTypeProviderType{});
}

//TODO: set mParent? when call this
Object::Object(const Descriptor& _descriptor): mEntity(ECS2::Entity::ID(_descriptor.Id)), mName(_descriptor.Name), mTag(_descriptor.Tag) {
	setActive(_descriptor.IsActive);
	RESOURCES::ServiceManager::Get<ECS2::World>().registerEntity(mEntity);
	for (auto& component : _descriptor.Components) {
		std::visit(overloaded{[this](auto& arg) {AddComponent(arg, this);}}, component);
		mTransform = getComponent<TransformComponent>();
	}
}

Object::~Object() {
	if (mIsActive) {
		onDisable();
	}
	if (mIsReady) {
		onDestroy();
	}

	destroyedEvent.run(*this);

	std::vector<std::shared_ptr<Object>> toDetach = mChildren;
	for (auto child : toDetach) {
		child->detachFromParent();
	}
	toDetach.clear();

	detachFromParent();
	for (auto& [key, val] : mComponents) {
		if (val) {
			componentRemovedEvent.run(val.get());
		}
	}
	RESOURCES::ServiceManager::Get<ECS2::World>().destroyEntity(mEntity);
	//ComponentManager::GetInstance().entityDestroyed(getID());
}

const std::string& Object::getName() const {
	return mName;
}

const std::string& Object::getTag() const {
	return mTag;
}

void Object::setName(const std::string& val) {
	mName = val;
}

void Object::setTag(const std::string& val) {
	mTag = val;
}

void Object::setActive(bool val) {
	if (val != mIsActive) {
		mIsActive = val;
		recursiveActiveUpdate();
	}
}

bool Object::getIsSelfActive() const {
	return mIsActive;
}

bool Object::getIsActive() const {
	auto p = mParent.lock();
	return mIsActive && (p ? p->getIsActive() : true);
}

void Object::setID(ECS2::Entity val) {
	mEntity = val;
}

ECS2::Entity Object::getID() const {
	return mEntity;
}

void Object::setParent(std::shared_ptr<Object> _mParent) {
	detachFromParent();

	mParent = _mParent;
	getComponent<TransformComponent>()->setParent(*_mParent->getComponent<TransformComponent>().get());

	_mParent->mChildren.push_back(shared_from_this());
	attachEvent.run(*this, *_mParent);
}

void Object::setParentInPos(std::shared_ptr<Object> _mParent, int pos) {
	if (pos > _mParent->mChildren.size()) {
		//TODO: problem
		return;
	}
	detachFromParent();

	mParent = _mParent;
	getComponent<TransformComponent>()->setParent(*_mParent->getComponent<TransformComponent>().get());

	_mParent->mChildren.insert(_mParent->mChildren.begin() + pos, shared_from_this());
	attachEvent.run(*this, *_mParent);
}

void Object::detachFromParent() {
	dettachEvent.run(*this);

	if (auto p = mParent.lock()) {
		p->mChildren.erase(std::remove_if(p->mChildren.begin(), p->mChildren.end(), [this](std::shared_ptr<Object> e) {
			return e.get() == this;
		}));
	}
}

bool Object::hasParent() const {
	return mParent.lock() != nullptr;
}

std::shared_ptr<Object> Object::getParent() const {
	return mParent.lock();
}

ECS2::Entity Object::getParentID() const {
	if (auto p = mParent.lock()) {
		return p->getID();
	}
	return ECS2::Entity(ECS2::Entity::ID(0));
}

std::span<std::shared_ptr<Object>> Object::getChildren() {
	return mChildren;
}

void Object::markAsDestroy() {
	mIsDestroyed = true;
	for (auto child : mChildren) {
		child->markAsDestroy();
	}
}

bool Object::isAlive() const {
	return !mIsDestroyed;
}

void Object::onStart() {
	mIsReady = true;
	for (auto& [key, val] : mComponents) {
		if (val) {
			val.get()->onStart();
		}
	}
	//auto components = ComponentManager::GetInstance().getComponents(mEntity);
	//std::for_each(components.begin(), components.end(), [](auto element) { element->onStart(); });
}

void Object::onEnable() {
	//auto components = ComponentManager::GetInstance().getComponents(mEntity);
	//std::for_each(components.begin(), components.end(), [](auto element) { element->onEnable(); });
	//ComponentManager::GetInstance().enable(mEntity);
	for (auto& [key, val] : mComponents) {
		if (val) {
			val.get()->onEnable();
		}
	}
}

void Object::onDisable() {
	//auto components = ComponentManager::GetInstance().getComponents(mEntity);
	//std::for_each(components.begin(), components.end(), [](auto element) { element->onDisable(); });
	//ComponentManager::GetInstance().disable(mEntity);
	for (auto& [key, val] : mComponents) {
		if (val) {
			val.get()->onDisable();
		}
	}
}

void Object::onDestroy() {
	//auto components = ComponentManager::GetInstance().getComponents(mEntity);
	//std::for_each(components.begin(), components.end(), [](auto element) { element->onDestroy(); });
	for (auto& [key, val] : mComponents) {
		if (val) {
			val.get()->onDestroy();
		}
	}
}

void Object::onUpdate(std::chrono::duration<double> dt) {
	if (getIsActive()) {
		//auto components = ComponentManager::GetInstance().getComponents(mEntity);
		//std::for_each(components.begin(), components.end(), [&](auto element) { element->onUpdate(dt); });
		for (auto& [key, val] : mComponents) {
			if (val) {
				val.get()->onUpdate(dt);
			}
		}
	}
}

void Object::onFixedUpdate(std::chrono::duration<double> dt) {
	if (getIsActive()) {
		//auto components = ComponentManager::GetInstance().getComponents(mEntity);
		//std::for_each(components.begin(), components.end(), [&](auto element) { element->onFixedUpdate(dt); });
		for (auto& [key, val] : mComponents) {
			if (val) {
				val.get()->onFixedUpdate(dt);
			}
		}
	}
}

void Object::onLateUpdate(std::chrono::duration<double> dt) {
	if (getIsActive()) {
		//auto components = ComponentManager::GetInstance().getComponents(mEntity);
		//std::for_each(components.begin(), components.end(), [&](auto element) { element->onLateUpdate(dt); });
		for (auto& [key, val] : mComponents) {
			if (val) {
				val.get()->onLateUpdate(dt);
			}
		}
	}
}

void Object::recursiveActiveUpdate() {
	bool mIsActive = getIsActive();

	if (mIsActive) {
		onEnable();
		if (!mIsReady) {
			onStart();
		}
	}

	if (!mIsActive) {
		onDisable();
	}

	for (auto child : mChildren) {
		child->recursiveActiveUpdate();
	}
}

UTILS::WeakPtr<TransformComponent> Object::getTransform() const {
	return mTransform;
}

int Object::getIDInt() const { return static_cast<int>(mEntity.getUniqueId()); }

void Object::setIDInt(int id) { mEntity = ECS2::Entity(ECS2::Entity::ID(id)); }

int Object::getParentId() const {
	auto _p = mParent.lock();
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
	auto _p = scene.findObjectByID(ECS2::Entity::ID(_id));
	if (_p) {
		LOG_ERROR << ("Object::setParentId: can not find actor with id: " + std::to_string(_id));
	}
	setParent(_p);
}
