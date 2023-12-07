#pragma once

#include <memory>
#include <string>
#include <vector>
#include <span>

#include "componentManager.h"
#include "components/transform.h"
#include <scriptModule/scriptInterpreter.h>
#include <utilsModule/idObject.h>
#include <utilsModule/event.h>
#include <utilsModule/pointers/objPtr.h>
#include "components/scriptComponent.h"
#include <rttr/registration_friend>

namespace IKIGAI::SCENE_SYSTEM {
	class Scene;
}

namespace IKIGAI::ECS {
	struct ObjectData {
		int id = 0;
		std::string name;
		std::string tag;
		bool isActive = true;
		int parentId = -1;
	};

	class Object: public std::enable_shared_from_this<Object> {
	public:
		using Id = ObjectId<Object>;
		using ObjPtr = std::shared_ptr<Object>;

		Object(Id actorID, const std::string& name, const std::string& tag);
		~Object();

		[[nodiscard]] const std::string& getName() const;
		[[nodiscard]] const std::string& getTag() const;

		void setName(const std::string& name);
		void setTag(const std::string& tag);
		void setActive(bool val);

		[[nodiscard]] bool getIsSelfActive() const;
		[[nodiscard]] bool getIsActive() const;

		void setID(Id id);
		[[nodiscard]] Id getID() const;

		void setParent(ObjPtr p_parent);

		void detachFromParent();

		[[nodiscard]] bool hasParent() const;

		[[nodiscard]] ObjPtr getParent() const;

		[[nodiscard]] Id getParentID() const;

		std::span<ObjPtr> getChildren();

		void markAsDestroy();

		bool isAlive() const;

		//TODO: move to systems?
		void onStart();
		void onEnable();
		void onDisable();
		void onDestroy();

		//TODO: remove when add all systems
		void onUpdate(std::chrono::duration<double> dt);
		void onFixedUpdate(std::chrono::duration<double> dt);
		void onLateUpdate(std::chrono::duration<double> dt);

		template<typename T, typename ...Args>
		inline UTILS::WeakPtr<T> addComponent(Args&& ...args) {
			static_assert(std::is_base_of_v<Component, T>, "T should derive from Component");

			if (auto found = getComponent<T>(); !found) {
				auto instance = T(*this, args...);
				if (getIsActive()) {
					//TODO: call systems methods
					instance.onAwake();
					instance.onEnable();
					instance.onStart();
				}
				ComponentManager::GetInstance().addComponent<T>(getID(), instance);

				auto component = getComponent<T>();
				componentAddedEvent.run(component.get());
				//TODO: move it to ScriptComponent::onStart()
				if constexpr (std::is_same<T, ScriptComponent>::value) {
					ScriptComponent::createdEvent.run(component.get());
				}

				//auto id = setModelEvent.add(std::bind(&MaterialRenderer::updateMaterialList, this));
				//setMaterialEventId = std::shared_ptr<EVENT::EventListener>(new EVENT::EventListener(id), [this](auto* e) {
				//	this->obj->setModelEvent.removeListener(*e);
				//	});
				return component;
			}
			else {
				return found;
			}
		}
		
		template<typename T>
		inline bool removeComponent() {
			static_assert(std::is_base_of_v<Component, T>, "T should derive from Component");
			static_assert(!std::is_same_v<TransformComponent, T>, "You can't remove a Transform from an actor");

			auto result = getComponent<T>();
			if (!result) {
				return false;
			}
			//TODO: move it to ScriptComponent::onStart()
			if constexpr (std::is_same<T, ScriptComponent>::value) {
				ScriptComponent::destroyedEvent.run(result.get());
			}
			componentRemovedEvent.run(result.get());
			ComponentManager::GetInstance().removeComponent<T>(getID());
			return true;
		}
		
		template<typename T>
		inline UTILS::WeakPtr<T> getComponent() const {
			static_assert(std::is_base_of_v<Component, T>, "T should derive from Component");

			if (ComponentManager::GetInstance().checkComponent<T>(getID())) {
				return ComponentManager::GetInstance().getComponent<T>(getID());
			}
			
			return nullptr;
		}

		template<typename T>
		inline UTILS::WeakPtr<Component> getComponentBase() const {
			static_assert(std::is_base_of_v<Component, T>, "T should derive from Component");

			if (ComponentManager::GetInstance().checkComponent<T>(getID())) {
				return ComponentManager::GetInstance().getComponentBase<T>(getID());
			}

			return nullptr;
		}

		auto getComponents() const {
			return ComponentManager::GetInstance().getComponents(id);
		}

		[[nodiscard]] UTILS::WeakPtr<TransformComponent> getTransform() const;

		ObjectData getObjectData();

	private:
		void recursiveActiveUpdate();

	public:
		EVENT::Event<object_ptr<Component>>	componentAddedEvent;
		EVENT::Event<object_ptr<Component>>	componentRemovedEvent;
		EVENT::Event<object_ptr<Component>>	componentChangedEvent;

		static EVENT::Event<Object&>				destroyedEvent;
		static EVENT::Event<Object&>				createdEvent;
		static EVENT::Event<Object&, Object&>		attachEvent;
		static EVENT::Event<Object&>				dettachEvent;

		//TODO: think how do it better
		EVENT::Event<>				setModelEvent;

		//for serealization
		int getIDInt();
		void setIDInt(int _id);
		int getParentId();
		void setParentId(int _id);
	protected:
		Id id;
		std::string name;
		std::string tag;
		
		bool	isActive = true;
		bool	isDestroyed = false;
		bool	isReady = false;

		std::weak_ptr<Object> parent;
		std::vector<ObjPtr> children;
	public:
		UTILS::WeakPtr<TransformComponent> transform;
	};
};
