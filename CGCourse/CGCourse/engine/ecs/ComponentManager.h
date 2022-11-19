#pragma once
#include <map>
#include <memory>
#include <unordered_map>

//#include "object.h"
#include "componentArrayInterface.h"
#include "systemManager.h"

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
#include "components/ScriptComponent.h"
#include "components/skeletal.h"
#include "components/spotLight.h"

#include "../utils/pointers/objPtr.h"
#include "components/physicsComponent.h"

namespace KUMA {
	namespace SCENE_SYSTEM {
		class Scene;
		class Object;
	}
}

namespace KUMA::ECS {
	template<typename T>
	class ComponentArray: public IComponentArray {
	public:
		void insertData(Entity entity, T component) {
			assert(!entityToIndexInArray.count(entity) && "Component added to same entity more than once.");

			auto newIndex = size;
			entityToIndexInArray[entity] = newIndex;
			indexInArrayToEntity.insert(std::make_pair(newIndex, entity));
			const char* typeName = typeid(T).name();
			if (componentArray.size() > newIndex) {
				componentArray[newIndex] = std::move(component);
			}
			else {
				componentArray.push_back(std::move(component));
			}
			size++;
		}

		T removeData(Entity entity) {
			assert(entityToIndexInArray.count(entity) && "Removing non-existent component.");

			// Copy element at end into deleted element's place to maintain density
			size_t indexOfRemovedEntity = entityToIndexInArray[entity];
			size_t indexOfLastElement = size - 1;
			auto component = std::move(componentArray[indexOfRemovedEntity]);
			componentArray[indexOfRemovedEntity] = std::move(componentArray[indexOfLastElement]);

			// Update map to point to moved spot
			Entity entityOfLastElement = indexInArrayToEntity.at(indexOfLastElement);
			entityToIndexInArray[entityOfLastElement] = indexOfRemovedEntity;
			indexInArrayToEntity.at(indexOfRemovedEntity) = entityOfLastElement;
			entityToIndexInArray.erase(entity);
			indexInArrayToEntity.erase(indexOfLastElement);
			size--;

			return component;
		}

		T& getData(Entity entity) {
			assert(entityToIndexInArray.count(entity) && "Retrieving non-existent component.");
			
			return componentArray[entityToIndexInArray[entity]];
		}

		void entityDestroyed(Entity entity) override {
			if (entityToIndexInArray.count(entity)) {
				removeData(entity);
			}
		}

		bool count(Entity entity) {
			return entityToIndexInArray.count(entity);
		}

		int getSize() const {
			return size;
		}

		using iterator = T*;
		using const_iterator = const T*;

		iterator begin() { return componentArray.data(); }
		const_iterator begin() const { return componentArray.data(); }
		iterator end() { return componentArray.data() + size; }
		const_iterator end() const { return componentArray.data() + size; }

	private:
		std::vector<T> componentArray;
		std::unordered_map<Entity, size_t> entityToIndexInArray;
		std::unordered_map<size_t, Entity> indexInArrayToEntity;
		int size = 0;
	};

	template<typename...> class ComponentsTypeProvider {};
	using ComponentsTypeProviderType = ComponentsTypeProvider<TransformComponent,
		AmbientLight,
		AmbientSphereLight,
		AudioComponent,
		CameraComponent,
		DirectionalLight,
		InputComponent,
		LogicComponent,
		MaterialRenderer,
		ModelRenderer,
		PointLight,
		ScriptComponent,
		Skeletal,
		SpotLight,
		PhysicsComponent
	>;

	class ComponentManager {
		static ComponentManager* instance;

		

		ComponentManager() {
			systemManager = std::make_unique<SystemManager>();
		}
	public:
		std::unique_ptr<SystemManager> systemManager;
		using ComponentType = ObjectId<ComponentManager>;

		static ComponentManager* getInstance() {
			if (!instance)
				instance = new ComponentManager();
			return instance;
		}

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

		std::unordered_map<Entity, Signature> signatures;
		void setSignature(Entity entity, Signature signature) {
			signatures[entity] = signature;
		}

		Signature getSignature(Entity entity) {
			//check
			return signatures[entity];
		}

		// System methods
		template<typename T>
		std::shared_ptr<T> registerSystem() {
			//check T is System
			return systemManager->registerSystem<T>();
		}

		template<typename T>
		void setSystemSignature(Signature signature) {
			//check T is System
			systemManager->setSignature<T>(signature);
		}

		template<typename T>
		void addComponent(Entity entity, T component) {
			//check T is Component
			getComponentArray<T>()->insertData(entity, std::move(component));

			auto signature = getSignature(entity);
			signature.set(static_cast<int>(getComponentType<T>()), true);
			setSignature(entity, signature);

			systemManager->entitySignatureChanged(entity, signature);
		}

		template<typename T>
		void removeComponent(Entity entity) {
			//check T is Component
			getComponentArray<T>()->removeData(entity);

			auto signature = getSignature(entity);
			signature.set(static_cast<int>(getComponentType<T>()), false);
			setSignature(entity, signature);

			systemManager->entitySignatureChanged(entity, signature);
		}

		template<typename T>
		bool checkComponent(Entity entity) {
			//check T is Component
			return getComponentArray<T>()->count(entity);
		}

		template<typename T>
		Ref<T> getComponent(Entity entity) {
			//check T is Component
			return getComponentArray<T>()->getData(entity);
		}

		std::vector<Ref<Component>> getComponents(Entity entity) {
			std::vector<Ref<Component>> res;
			if (getComponentArray<TransformComponent>()->count(entity))
			res.push_back(getComponentArray<TransformComponent>()->getData(entity));
			if (getComponentArray<AmbientLight>()->count(entity))
			res.push_back(getComponentArray<AmbientLight>()->getData(entity));
			if (getComponentArray<AmbientSphereLight>()->count(entity))
			res.push_back(getComponentArray<AmbientSphereLight>()->getData(entity));
			if (getComponentArray<AudioComponent>()->count(entity))
			res.push_back(getComponentArray<AudioComponent>()->getData(entity));
			if (getComponentArray<CameraComponent>()->count(entity))
			res.push_back(getComponentArray<CameraComponent>()->getData(entity));
			if (getComponentArray<DirectionalLight>()->count(entity))
			res.push_back(getComponentArray<DirectionalLight>()->getData(entity));
			if (getComponentArray<InputComponent>()->count(entity))
			res.push_back(getComponentArray<InputComponent>()->getData(entity));
			if (getComponentArray<LogicComponent>()->count(entity))
			res.push_back(getComponentArray<LogicComponent>()->getData(entity));
			if (getComponentArray<MaterialRenderer>()->count(entity))
			res.push_back(getComponentArray<MaterialRenderer>()->getData(entity));
			if (getComponentArray<ModelRenderer>()->count(entity))
			res.push_back(getComponentArray<ModelRenderer>()->getData(entity));
			if (getComponentArray<PointLight>()->count(entity))
			res.push_back(getComponentArray<PointLight>()->getData(entity));
			if (getComponentArray<ScriptComponent>()->count(entity))
			res.push_back(getComponentArray<ScriptComponent>()->getData(entity));
			if (getComponentArray<Skeletal>()->count(entity))
			res.push_back(getComponentArray<Skeletal>()->getData(entity));
			if (getComponentArray<SpotLight>()->count(entity))
			res.push_back(getComponentArray<SpotLight>()->getData(entity));
			return res;
		}

		void entityDestroyed(Entity entity) {
			for (auto const& pair : componentArrays) {
				auto const& components = pair.second;
				components->entityDestroyed(entity);
			}
			systemManager->entityDestroyed(entity);
		}

		template<typename T>
		std::shared_ptr<ECS::ComponentArray<T>> getComponentArray() {
			const char* typeName = typeid(T).name();
			assert(componentTypes.count(typeName) && "Component not registered before use.");
			return std::static_pointer_cast<ComponentArray<T>>(componentArrays[typeName]);
		}

		template<typename T>
		std::shared_ptr<ComponentArray<T>> getComponentArrayOff() {
			const char* typeName = typeid(T).name();
			assert(componentTypes.count(typeName) && "Component not registered before use.");
			return std::static_pointer_cast<ComponentArray<T>>(componentArraysOff[typeName]);
		}
		
		std::unordered_map<const char*, ComponentType> componentTypes;
		std::unordered_map<const char*, std::shared_ptr<IComponentArray>> componentArrays;
		std::unordered_map<const char*, std::shared_ptr<IComponentArray>> componentArraysOff;
		ComponentType nextComponentType = ObjectIdGenerator<ComponentManager>::generateId();


		template<typename T>
		void enabledComponentImpl(Entity id) {
			if (getComponentArrayOff<T>()->count(id)) {
				auto data = getComponentArrayOff<T>()->removeData(id);
				getComponentArray<T>()->insertData(id, std::move(data));
			}
		}

		template<template<typename...> class Container, typename...ComponentType>
		void enabledComponent(Entity id, Container<ComponentType...> opt) {
			(enabledComponentImpl<ComponentType>(id), ...);
		}

		void enable(Entity id) {
			enabledComponent(id, ComponentsTypeProviderType{});
		}

		template<typename T>
		void disableComponentImpl(Entity id) {
			if (getComponentArray<T>()->count(id)) {
				auto data = getComponentArray<T>()->removeData(id);
				getComponentArrayOff<T>()->insertData(id, std::move(data));
			}
		}

		template<template<typename...> class Container, typename...ComponentType>
		void disableComponent(Entity id, Container<ComponentType...> opt) {
			(disableComponentImpl<ComponentType>(id), ...);
		}

		void disable(Entity id) {
			disableComponent(id, ComponentsTypeProviderType{});
		}
	};
}
