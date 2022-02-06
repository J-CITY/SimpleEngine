#pragma once

#include <memory>
#include <string>
#include <vector>


#include "ComponentManager.h"
#include "components/transform.h"
#include "../scripting/ScriptInterpreter.h"
#include "../utils/idObject.h"
#include "../utils/event.h"

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

		std::vector<std::shared_ptr<Object>>& getChildren();

		void markAsDestroy();

    	bool isAlive() const;

		void onStart();

		void onEnable();

		void onDisable();

		void onDestroy();

		void onUpdate(float dt);

		void onFixedUpdate(float dt);

		void onLateUpdate(float dt);

		//TODO: refactor to addComponent
		std::shared_ptr<Script> addScript(const std::string& name);
    	//TODO: удалять и из off контейнеров, тк объект может быть выключен
		bool removeScript(std::shared_ptr<Script> script);
		bool removeScript(const std::string& name);
		std::shared_ptr<Script> getScript(const std::string& name);
		std::unordered_map<std::string, std::shared_ptr<Script>>& getScripts();
    	
    	template<typename T, typename ...Args>
		inline std::shared_ptr<T> addComponent(Args&& ...args) {
			static_assert(std::is_base_of<Component, T>::value, "T should derive from Component");
    		static_assert(!std::is_same<Script, T>::value, "Use addScript()");

			if (auto found = getComponent<T>(); !found) {
				components.insert(components.begin(), std::make_shared<T>(*this, args...));
				std::shared_ptr<T> instance = std::dynamic_pointer_cast<T>(components.front());
				componentAddedEvent.run(instance);
				if (getIsActive()) {
					instance->onEnable();
					instance->onStart();
				}
				ComponentManager::getInstance()->addComponent<T>(getID(), instance);
				return instance;
			}
			else {
				return found;
			}
		}

		template<typename T>
		inline bool removeComponent() {
			static_assert(std::is_base_of<Component, T>::value, "T should derive from Component");
			static_assert(!std::is_same<TransformComponent, T>::value, "You can't remove a Transform from an actor");

			std::shared_ptr<T> result(nullptr);

			for (auto it = components.begin(); it != components.end(); ++it) {
				result = std::dynamic_pointer_cast<T>(*it);
				if (result) {
					componentRemovedEvent.run(result);
					components.erase(it);
					ComponentManager::getInstance()->removeComponents<T>(getID());
					return true;
				}
			}

			return false;
		}

		template<>
		inline bool removeComponent<Script>() {
			if (ComponentManager::getInstance()->scriptComponents.count(id)) {
				ComponentManager::getInstance()->removeComponents<Script>(getID());
				return true;
			}
			return false;
		}

		template<typename T>
		inline std::shared_ptr<T> getComponent() const {
			static_assert(std::is_base_of<Component, T>::value, "T should derive from Component");
			static_assert(!std::is_same<Script, T>::value, "Use getScripts()");
			
			std::shared_ptr<T> result(nullptr);
			for (auto it = components.begin(); it != components.end(); ++it) {
				result = std::dynamic_pointer_cast<T>(*it);
				if (result) {
					return result;
				}
			}
			return nullptr;
		}
    	
    	std::vector<std::shared_ptr<Component>>& getComponents();

		virtual void onDeserialize(nlohmann::json& j) override;
		virtual void onSerialize(nlohmann::json& j) override;
    	
    private:
        void recursiveActiveUpdate();

    public:
        EVENT::Event<std::shared_ptr<Component>>	componentAddedEvent;
		EVENT::Event<std::shared_ptr<Component>>	componentRemovedEvent;

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
        std::vector<std::shared_ptr<Component>> components;
    public:
        std::shared_ptr<TransformComponent> transform;
    };
};
