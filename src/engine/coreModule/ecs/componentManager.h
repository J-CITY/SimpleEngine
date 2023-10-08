#pragma once
#include <map>
#include <memory>
#include <unordered_map>

#include "componentArray.h"
#include "componentArrayInterface.h"
#include "componentTypes.h"
#include "systemManager.h"

#include <utilsModule/pointers/objPtr.h>

#include "components/physicsComponent.h"
#include "components/AmbientLight.h"
#include "components/AmbientSphereLight.h"
#include "components/audioComponent.h"
#include "components/cameraComponent.h"
#include "components/directionalLight.h"
#include "components/inputComponent.h"
#include "components/logicComponent.h"
#include "components/modelRenderer.h"
#include "components/materialRenderer.h"
#include "components/pointLight.h"
#include "components/scriptComponent.h"
#include "components/skeletal.h"
#include "components/spotLight.h"

namespace IKIGAI {
	namespace SCENE_SYSTEM {
		class Scene;
		class Object;
	}
}

namespace IKIGAI::ECS {
	class ComponentManager : public UTILS::SingletonService<ComponentManager> {
		FRIEND_SINGLETON_SERVICE(ComponentManager)
		std::unique_ptr<SystemManager> systemManager;
		ComponentManager();

		std::unordered_map<Entity, Signature> signatures;

	public:
		using ComponentType = ObjectId<ComponentManager>;

	private:
		ComponentType nextComponentType = ObjectIdGenerator<ComponentManager>::generateId();

		std::unordered_map<const char*, ComponentType> componentTypes;
		std::unordered_map<const char*, std::shared_ptr<ComponentArrayInterface>> componentArrays;
		std::unordered_map<const char*, std::shared_ptr<ComponentArrayInterface>> componentArraysOff;
	public:

		template<typename T>
		void registerComponent() {
			const char* typeName = typeid(T).name();

			assert(!componentTypes.count(typeName) && "Registering component type more than once.");

			// Add this component type to the component type map
			componentTypes.insert({ typeName, nextComponentType });

			// Create a ComponentArray pointer and add it to the component arrays map
			componentArrays.insert({ typeName, std::make_shared<ComponentArray<T>>() });
			componentArraysOff.insert({ typeName, std::make_shared<ComponentArray<T>>() });
			
			nextComponentType = ObjectIdGenerator<ComponentManager>::generateId();
		}

		template<typename T>
		ComponentType getComponentType() {
			const char* typeName = typeid(T).name();
			assert(componentTypes.count(typeName) && "Component not registered before use.");
			return componentTypes.at(typeName);
		}
		
		void setSignature(Entity entity, Signature signature);

		Signature getSignature(Entity entity);

		// System methods
		template<typename T>
		std::shared_ptr<T> registerSystem() {
			static_assert(std::is_base_of_v<System, T>, "Must inherit from class Component");
			return systemManager->registerSystem<T>();
		}

		template<typename T>
		void setSystemSignature(Signature signature) {
			static_assert(std::is_base_of_v<System, T>, "Must inherit from class Component");
			systemManager->setSignature<T>(signature);
		}

		SystemManager& getSystemManager();

		template<typename T>
		void addComponent(Entity entity, T& component) {
			static_assert(std::is_base_of_v<Component, T>, "Must inherit from class Component");
			getComponentArray<T>()->insertData(entity, component);

			auto signature = getSignature(entity);
			signature.set(static_cast<int>(getComponentType<T>()), true);
			setSignature(entity, signature);

			systemManager->entitySignatureChanged(entity, signature);
		}

		template<typename T>
		void removeComponent(Entity entity) {
			static_assert(std::is_base_of_v<Component, T>, "Must inherit from class Component");
			getComponentArray<T>()->removeData(entity);

			auto signature = getSignature(entity);
			signature.set(static_cast<int>(getComponentType<T>()), false);
			setSignature(entity, signature);

			systemManager->entitySignatureChanged(entity, signature);
		}

		template<typename T>
		bool checkComponent(Entity entity) {
			static_assert(std::is_base_of_v<Component, T>, "Must inherit from class Component");
			return getComponentArray<T>()->count(entity);
		}

		template<typename T>
		Ref<T> getComponentRef(Entity entity) {
			static_assert(std::is_base_of_v<Component, T>, "Must inherit from class Component");
			return getComponentArray<T>()->getData(entity);
		}

		template<typename T>
		UTILS::WeakPtr<T> getComponent(Entity entity) {
			static_assert(std::is_base_of_v<Component, T>, "Must inherit from class Component");
			return getComponentArray<T>()->getDataPtr(entity);
		}

		template<typename T>
		UTILS::WeakPtr<Component> getComponentBase(Entity entity) {
			static_assert(std::is_base_of_v<Component, T>, "Must inherit from class Component");
			return getComponentArray<T>()->getDataBasePtr(entity);
		}
	private:
		template <typename T>
		void tryGetComponent(std::vector<UTILS::WeakPtr<Component>>& res, Entity entity) {
			auto arr = getComponentArray<T>();
			if (arr->count(entity)) {
				res.push_back(arr->getDataBasePtr(entity));
			}
		}

		template<template<typename...> class Container, typename...ComponentType>
		std::vector<UTILS::WeakPtr<Component>> tryGetComponents(Entity entity, Container<ComponentType...> opt) {
			std::vector<UTILS::WeakPtr<Component>> res;
			(tryGetComponent<ComponentType>(res, entity), ...);
			return res;
		}
	public:
		std::vector<UTILS::WeakPtr<Component>> getComponents(Entity entity) {
			return tryGetComponents(entity, ComponentsTypeProviderType{});
		}

		void entityDestroyed(Entity entity);

		template<typename T>
		std::shared_ptr<ECS::ComponentArray<T>> getComponentArray() {
			const char* typeName = typeid(T).name();
			assert(componentTypes.count(typeName) && "Component not registered before use.");
			return std::static_pointer_cast<ComponentArray<T>>(componentArrays[typeName]);
		}

		template<typename T>
		ECS::ComponentArray<T>& getComponentArrayRef() {
			const char* typeName = typeid(T).name();
			assert(componentTypes.count(typeName) && "Component not registered before use.");
			return *std::static_pointer_cast<ComponentArray<T>>(componentArrays[typeName]);
		}

		template<typename T>
		std::shared_ptr<ComponentArray<T>> getComponentArrayOff() {
			const char* typeName = typeid(T).name();
			assert(componentTypes.count(typeName) && "Component not registered before use.");
			return std::static_pointer_cast<ComponentArray<T>>(componentArraysOff[typeName]);
		}

	private:
		template<typename T>
		void enabledComponentImpl(Entity id) {
			if (getComponentArrayOff<T>()->count(id)) {
				auto data = getComponentArrayOff<T>()->removeData(id);
				getComponentArray<T>()->insertData(id, data);
			}
		}

		template<template<typename...> class Container, typename...ComponentType>
		void enabledComponent(Entity id, Container<ComponentType...> opt) {
			(enabledComponentImpl<ComponentType>(id), ...);
		}

	public:
		void enable(Entity id);
		void disable(Entity id);
	private:
		template<typename T>
		void disableComponentImpl(Entity id) {
			if (getComponentArray<T>()->count(id)) {
				auto data = getComponentArray<T>()->removeData(id);
				getComponentArrayOff<T>()->insertData(id, data);
			}
		}

		template<template<typename...> class Container, typename...ComponentType>
		void disableComponent(Entity id, Container<ComponentType...> opt) {
			(disableComponentImpl<ComponentType>(id), ...);
		}
	};
}
