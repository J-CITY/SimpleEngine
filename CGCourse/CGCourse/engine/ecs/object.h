#pragma once

#include <memory>
#include <string>
#include <vector>
#include <span>

#include "ComponentManager.h"
#include "components/transform.h"
#include "../scripting/scriptInterpreter.h"
#include "../utils/idObject.h"
#include "../utils/event.h"
#include "../utils/pointers/objPtr.h"

//TODO: можно убрать компоненты и хратить только в ECS

namespace KUMA::SCENE_SYSTEM {
	class Scene;
}

namespace KUMA::ECS {
    class Object: public std::enable_shared_from_this<Object>, public RESOURCES::Serializable {
    public:
		Object(ObjectId<Object> actorID, const std::string& name, const std::string& tag);
		~Object();
    	
		const std::string& getName() const;
		const std::string& getTag() const;

		void setName(const std::string& name);
		void setTag(const std::string& tag);
		void setActive(bool val);
		bool getIsSelfActive() const;
		bool getIsActive() const;
		void setID(ObjectId<Object> id);

    	ObjectId<Object> getID() const;

		void setParent(std::shared_ptr<Object> p_parent);

		void detachFromParent();

		bool hasParent() const;

		std::shared_ptr<Object> getParent() const;

		ObjectId<Object> getParentID() const;

		std::span<std::shared_ptr<Object>> getChildren();

		void markAsDestroy();

    	bool isAlive() const;

		void onStart();

		void onEnable();

		void onDisable();

		void onDestroy();

		void onUpdate(std::chrono::duration<double> dt);

		void onFixedUpdate(std::chrono::duration<double> dt);

		void onLateUpdate(std::chrono::duration<double> dt);

		//TODO: refactor to addComponent
		//std::shared_ptr<ScriptComponent> addScript(const std::string& name);
		//bool removeScript(std::shared_ptr<ScriptComponent> script);
		//bool removeScript(const std::string& name);
		//std::shared_ptr<ScriptComponent> getScript(const std::string& name);
		//std::unordered_map<std::string, std::shared_ptr<ScriptComponent>>& getScripts();
    	
    	template<typename T, typename ...Args>
		inline Ref<T> addComponent(Args&& ...args) {
			static_assert(std::is_base_of<Component, T>::value, "T should derive from Component");
    		//static_assert(!std::is_same<ScriptComponent, T>::value, "Use addScript()");

			if (auto& found = getComponent<T>(); !found) {
				auto instance = T(*this, args...);
				if (std::is_same<T, ScriptComponent>::value) {
					ScriptComponent::createdEvent.run(object_ptr(dynamic_cast<ScriptComponent*>(&instance)));
				}
				componentAddedEvent.run(&instance);
				if (getIsActive()) {
					instance.onAwake();
					instance.onEnable();
					instance.onStart();
				}
				ComponentManager::getInstance()->addComponent<T>(getID(), std::move(instance));
				//components.push_back(getComponent<T>().value());
				return getComponent<T>().value();
			}
			else {
				return found.value();
			}
		}

		//template<typename ...Args>
		//inline Ref<ScriptComponent> addComponent<ScriptComponent>(Args&& ...args) {
		//	if (auto found = getComponent<ScriptComponent>(); !found) {
		//		auto instance = ScriptComponent(*this, args...);
		//		ScriptComponent::createdEvent.run(&instance);
		//		componentAddedEvent.run(&instance);
		//		if (getIsActive()) {
		//			instance.onAwake();
		//			instance.onEnable();
		//			instance.onStart();
		//		}
		//		ComponentManager::getInstance()->addComponent<ScriptComponent>(getID(), instance);
		//		//components.push_back(getComponent<ScriptComponent>().value());
		//		return getComponent<ScriptComponent>().value();
		//	}
		//	else {
		//		return found.value();
		//	}
		//}

		template<typename T>
		inline bool removeComponent() {
			static_assert(std::is_base_of<Component, T>::value, "T should derive from Component");
			static_assert(!std::is_same<TransformComponent, T>::value, "You can't remove a Transform from an actor");

			auto result = getComponent<T>();
			if (!result) {
				return false;
			}
			if (std::is_same<T, ScriptComponent>::value) {
				ScriptComponent::destroyedEvent.run(dynamic_cast<ScriptComponent*>(result.value().getPtr().get()));
			}
			componentRemovedEvent.run(result.value().getPtr());
			ComponentManager::getInstance()->removeComponent<T>(getID());

			//T* result = nullptr;
			//for (auto it = components.begin(); it != components.end(); ++it) {
			//	result = dynamic_cast<T>(&(*it).get());
			//	if (result != nullptr) {
			//		componentRemovedEvent.run(*result);
			//		components.erase(it);
			//		ComponentManager::getInstance()->removeComponent<T>(getID());
			//		return true;
			//	}
			//}
			return true;
		}

		//template<>
		//inline bool removeComponent<ScriptComponent>() {
		//
		//	auto result = getComponent<ScriptComponent>();
		//	if (!result) {
		//		return false;
		//	}
		//	ScriptComponent::destroyedEvent.run(result.value().getPtr());
		//	componentRemovedEvent.run(result.value().getPtr());
		//	ComponentManager::getInstance()->removeComponent<ScriptComponent>(getID());
		//
		//	//ScriptComponent* result = nullptr;
		//	//for (auto it = components.begin(); it != components.end(); ++it) {
		//	//	result = dynamic_cast<ScriptComponent*>(&(*it).get());
		//	//	if (result != nullptr) {
		//	//		componentRemovedEvent.run(*result);
		//	//		ScriptComponent::destroyedEvent.run(*result);
		//	//		components.erase(it);
		//	//		ComponentManager::getInstance()->removeComponent<ScriptComponent>(getID());
		//	//		return true;
		//	//	}
		//	//}
		//	return true;
		//}

		//template<>
		//inline bool removeComponent<ScriptComponent>() {
		//	if (ComponentManager::getInstance()->scriptComponents.count(id)) {
		//		ComponentManager::getInstance()->removeComponents<ScriptComponent>(getID());
		//		return true;
		//	}
		//	return false;
		//}

		template<typename T>
		inline std::optional<Ref<T>> getComponent() const {
			static_assert(std::is_base_of<Component, T>::value, "T should derive from Component");
			//static_assert(!std::is_same<ScriptComponent, T>::value, "Use getScripts()");

			if (ComponentManager::getInstance()->checkComponent<T>(getID())) {
				return ComponentManager::getInstance()->getComponent<T>(getID());
			}
			return std::nullopt;
		}
    	
    	//std::vector<std::reference_wrapper<Component>>& getComponents();

		virtual void onDeserialize(nlohmann::json& j) override;
		virtual void onSerialize(nlohmann::json& j) override;


		Ref<TransformComponent> getTransform() const {
			return getComponent<TransformComponent>().value();
		}
    private:
        void recursiveActiveUpdate();

    public:
        EVENT::Event<object_ptr<Component>>	componentAddedEvent;
		EVENT::Event<object_ptr<Component>>	componentRemovedEvent;

        static EVENT::Event<Object&>				destroyedEvent;
        static EVENT::Event<Object&>				createdEvent;
        static EVENT::Event<Object&, Object&>		attachEvent;
        static EVENT::Event<Object&>				dettachEvent;
    protected:
        ObjectId<Object> id;
        std::string		name;
        std::string		tag;
    	
        bool	isActive = true;
        bool	isDestroyed = false;
        bool	isReady = false;

    	std::weak_ptr<Object> parent;
        std::vector<std::shared_ptr<Object>> children;
        //std::vector<std::reference_wrapper<Component>> components;
    public:
        //Ref<TransformComponent> transform;
    };
};
