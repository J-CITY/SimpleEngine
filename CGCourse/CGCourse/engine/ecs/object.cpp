#include "object.h"

#include <algorithm>


#include "ComponentManager.h"

#include "components/ambientLight.h"
#include "components/ambientSphereLight.h"
#include "components/directionalLight.h"
#include "components/inputComponent.h"
#include "components/pointLight.h"
#include "components/spotLight.h"

using namespace KUMA;
using namespace KUMA::ECS;


EVENT::Event<Object&> Object::destroyedEvent;
EVENT::Event<Object&> Object::createdEvent;
EVENT::Event<Object&, Object&> Object::attachEvent;
EVENT::Event<Object&> Object::dettachEvent;

Object::Object(ObjectId<Object> actorID, const std::string& name, const std::string& tag) :
	id(actorID),
	name(name),
	tag(tag),
	transform(addComponent<TransformComponent>()) {
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

	std::for_each(components.begin(), components.end(), [&](std::shared_ptr<Component> p_component) {
		componentRemovedEvent.run(p_component);
	});
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
	transform->setParent(*_parent->transform);

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

	transform->removeParent();
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

std::vector<std::shared_ptr<Object>>& Object::getChildren() {
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
	std::for_each(components.begin(), components.end(), [](auto element) { element->onStart(); });
}

void Object::onEnable() {
	std::for_each(components.begin(), components.end(), [](auto element) { element->onEnable(); });
	ComponentManager::getInstance()->enable(id);
}

void Object::onDisable() {
	std::for_each(components.begin(), components.end(), [](auto element) { element->onDisable(); });
	ComponentManager::getInstance()->disable(id);
}

void Object::onDestroy() {
	std::for_each(components.begin(), components.end(), [](auto element) { element->onDestroy(); });
}

void Object::onUpdate(float dt) {
	if (getIsActive()) {
		std::for_each(components.begin(), components.end(), [&](auto element) { element->onUpdate(dt); });
		std::for_each(ComponentManager::getInstance()->scriptComponents[id].begin(), 
			ComponentManager::getInstance()->scriptComponents[id].end(), [&](auto element) { element.second->onUpdate(dt); });
	}
}

void Object::onFixedUpdate(float dt) {
	if (getIsActive()) {
		std::for_each(components.begin(), components.end(), [&](auto element) { element->onFixedUpdate(dt); });
	}
}

void Object::onLateUpdate(float dt) {
	if (getIsActive()) {
		std::for_each(components.begin(), components.end(), [&](auto element) { element->onLateUpdate(dt); });
	}
}

std::vector<std::shared_ptr<Component>>& Object::getComponents() {
	return components;
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

void Object::onDeserialize(nlohmann::json& j) {
	name = j["name"];
	tag = j["tag"];
	id = ObjectId<Object>(j["id"]);
	isActive = j["isActive"];

	for (auto& c : j["components"]) {
		if (c["type"] == "Transform") {
			auto _c = addComponent<TransformComponent>();
			_c->onDeserialize(c);
		}
		else if (c["type"] == "SpotLight") {
			auto _c = addComponent<SpotLight>();
			_c->onDeserialize(c);
		}
		else if (c["type"] == "Script") {
			auto _c = addScript(c["data"]["name"]);
			_c->onDeserialize(c);
		}
		else if (c["type"] == "PointLight") {
			auto _c = addComponent<PointLight>();
			_c->onDeserialize(c);
		}
		else if (c["type"] == "ModelRenderer") {
			auto _c = addComponent<ModelRenderer>();
			_c->onDeserialize(c);
		}
		else if (c["type"] == "MaterialRenderer") {
			auto _c = addComponent<MaterialRenderer>();
			_c->onDeserialize(c);
		}
		else if (c["type"] == "Input") {
			auto _c = addComponent<InputComponent>([](float){});
			_c->onDeserialize(c);
		}
		else if (c["type"] == "DirectionalLight") {
			auto _c = addComponent<DirectionalLight>();
			_c->onDeserialize(c);
		}
		else if (c["type"] == "Camera") {
			auto _c = addComponent<CameraComponent>();
			_c->onDeserialize(c);
		}
		else if (c["type"] == "AmbientSphereLight") {
			auto _c = addComponent<AmbientSphereLight>();
			_c->onDeserialize(c);
		}
		else if (c["type"] == "AmbientLight") {
			auto _c = addComponent<AmbientLight>();
			_c->onDeserialize(c);
		}
	}
}
void Object::onSerialize(nlohmann::json& j) {
	nlohmann::json oj;
	oj["name"] = name;
	oj["tag"] = tag;
	oj["id"] = id.operator int();
	oj["isActive"] = isActive;
	auto _parent = parent.lock();
	oj["parent"] = _parent ? _parent->getID().operator int() : -1;

	for (auto& c : components) {
		nlohmann::json cj;
		cj["type"] = c->getName();
		c->onSerialize(cj);
		oj["components"].push_back(cj);
	}
	j.push_back(oj);
}

//TODO: refactor to addComponent
std::shared_ptr<Script> Object::addScript(const std::string& name) {
	auto newInstance = std::make_shared<Script>(*this, name);
	ComponentManager::getInstance()->addComponent<Script>(getID(), newInstance);
	if (getIsActive()) {
		newInstance->onAwake();
		newInstance->onEnable();
		newInstance->onStart();
	}
	return newInstance;
}

//TODO: удалять и из off контейнеров, тк объект может быть выключен
bool Object::removeScript(std::shared_ptr<Script> script) {
	bool found = false;
	for (auto& [name, behaviour] : ComponentManager::getInstance()->scriptComponents[id]) {
		if (behaviour.get() == script.get()) {
			found = true;
			break;
		}
	}

	if (found) {
		return removeScript(script->name);
	}
	return false;
}

bool Object::removeScript(const std::string& name) {
	auto found = getScript(name);
	if (found) {
		ComponentManager::getInstance()->scriptComponents[id].erase(name);
		return true;
	}
	return false;
}

std::shared_ptr<Script> Object::getScript(const std::string& name) {
	auto result = ComponentManager::getInstance()->scriptComponents[id].find(name);
	if (result != ComponentManager::getInstance()->scriptComponents[id].end())
		return result->second;
	else
		return nullptr;
}

std::unordered_map<std::string, std::shared_ptr<Script>>& Object::getScripts() {
	return ComponentManager::getInstance()->scriptComponents[id];
}