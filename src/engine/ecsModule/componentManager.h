#pragma once
#include "signature.h"
#include "relationManager.h"
#include "events.h"
#include "access.h"
#include <memory>
#include <string>
#include <unordered_set>

#include "componentArray.h"
#include "entityManager.h"
#include "utilsModule/idGenerator.h"
#include "utilsModule/singleton.h"

#include <coreModule/ecs/components/component.h>

namespace IKIGAI::ECS2 {
	//TODO: use faster std container (EA for example)
	//TODO: use allocators for containers
	/*
	 *TODO:
	 *Signature class
	 *components relations
	 *events
	 *multythread
	 */

	class ComponentManager;

	using ComponentType = Id<ComponentManager>;
	using TagType = ComponentType;

	//constexpr unsigned MAX_COMPONENTS = 256;
	//using Signature = std::bitset<MAX_COMPONENTS>;
	//using Signature = IKIGAI::ECS2::Signature;

	/*
	class Signature1 {
	public:
		Signature1(size_t maskSize) {
			resize(maskSize);
		}

		void resize(size_t maskSize) {
			mMask.resize(maskSize);
		}

		void set(size_t pos, bool val = true) {
			if (pos > mMask.size()) {
				resize(pos);
			}
			mMask[pos] = val;
		}

		bool get(size_t pos) {
			if (pos > mMask.size()) {
				resize(pos);
			}
			return mMask[pos];
		}

		std::vector<bool> operator|(const std::vector<bool> A, const std::vector<bool>& B) {
			if (A.size() != B.size())
				throw std::invalid_argument("differently sized bitwise operands");

			auto res = A;

			std::vector<bool>::const_iterator itRES = res.begin();
			std::vector<bool>::const_iterator itA = A.begin();
			std::vector<bool>::const_iterator itB = B.begin();

			// c++ implementation-specific
			while (itA < A.end()) {
				*itRES = *itA |= *itB;
				itA++;
				itB++;
				itRES++;
			}
			return A;
		}

	private:
		std::vector<bool> mMask;
	};*/

	enum class StorageType {
		Archetype,
		Sparse
	};

	class /*alignas(32)*/ Component: public UTILS::ControlBlockHandler
	{
	public:
		virtual ~Component() override = default;
	};

	class Archetype {
	public:
		Signature mMask;
		std::unordered_map<ComponentType, std::shared_ptr<ComponentArrayInterface>> mComponentArrays;

		template<class T>
		std::shared_ptr<ComponentArray<T>> getComponents(ComponentType type) {
			if (!mComponentArrays.contains(type)) {
				return nullptr;
			}
			return std::static_pointer_cast<ComponentArray<T>>(mComponentArrays.at(type));
		}

		size_t size() const {
			return mComponentArrays.empty() ? 0 : mComponentArrays.begin()->second->getSize();
		}
	};

	struct Record {
		std::unordered_map<ComponentType, std::unordered_map<TagType, std::any>> mTagsMap;
		std::shared_ptr<Archetype> archetype;
	};

	class ComponentManager {
	public:
		ComponentManager() {};

	private:
		std::unordered_map<Entity, Signature> mSignatures;

		ComponentType mNextComponentType = IdGenerator<ComponentManager>::generateId();

		//Archetype structs
		std::unordered_map<Entity, Record> mEntityRecords;
		std::unordered_map<Signature, std::shared_ptr<Archetype>> mMaskToArchetype;
		std::unordered_map<ComponentType, std::shared_ptr<ComponentArrayInterface>> mDefaultArraysArchetype;

		std::unordered_map<ComponentType, std::any> mSingletonComponent;
		
		// Hybrid Storage
		std::unordered_map<ComponentType, StorageType> mComponentStorageType;
		std::unordered_map<ComponentType, std::shared_ptr<ComponentArrayInterface>> mSparseArrays;
		
		// Relations
		RelationManager mRelationManager;
		EventManager mEventManager;
	private:

	private:
		// Helper to lazily register component with default storage if not already registered
		template<typename T>
		void ensureComponentRegistered(ComponentType id) {
			if (mComponentStorageType.contains(id)) return;

			// Default to Archetype
			mComponentStorageType[id] = StorageType::Archetype;
			mDefaultArraysArchetype.insert({ id, std::make_shared<ComponentArray<T>>() });
		}

		// Called by getComponentType static initialization
		template<typename T>
		ComponentType registerNewComponentId() {
			ComponentType id = mNextComponentType;
			mNextComponentType = IdGenerator<ComponentManager>::generateId();
			return id;
		}

		template<typename T, typename... Args>
		void addComponentToArchetypeImpl(Entity entity, Archetype& to, T&& component) {
			static_assert(std::is_base_of_v<Component, T>, "Must inherit from class Component");
			const auto cname = getComponentType<T>();
			
			// We assume this is only called for Archetype components
			// But careful validation or design is needed.
			// Ideally addComponents logic separates them.
			
			if (to.mComponentArrays.contains(cname)) {
				std::static_pointer_cast<ComponentArray<T>>(to.mComponentArrays.at(cname))->insert(entity, std::move(component));
			}
		}

		template<typename T>
		void addComponentToArchetypeMaskImpl(Signature& mask) {
			static_assert(std::is_base_of_v<Component, T>, "Must inherit from class Component");
			ComponentType id = getComponentType<T>();
			ensureComponentRegistered<T>(id);
			if (mComponentStorageType[id] == StorageType::Archetype) {
				mask.set(static_cast<size_t>(id), true);
			}
		}

		template<typename... Component>
		void addComponentsToArchetypeMask(Signature& mask) {
			(addComponentToArchetypeMaskImpl<Component>(mask), ...);
		}

		template<typename T>
		void removeComponentFromArchetypeMaskImpl(Signature& mask) {
			static_assert(std::is_base_of_v<Component, T>, "Must inherit from class Component");
			ComponentType id = getComponentType<T>();
			if (mComponentStorageType.contains(id) && mComponentStorageType[id] == StorageType::Archetype) {
				mask.set(static_cast<size_t>(id), false); // Was true in original logic? logic in original was 'set(..., true)' confusingly named remove?
				// Original: mask.set(..., true). Wait.
				// removeComponentFromArchetypeMaskImpl logic in original was: mask.set(..., true); 
				// Uh oh. The original code for removeComponentsFromArchetypeMask calls removeComponentFromArchetypeMaskImpl which sets it to TRUE?
				// Ah, in removeComponents (public):
				// auto newMask = archetype->mMask;
				// removeComponentsFromArchetypeMask(newMask); 
				// -> sets bits to true??
				// If newMask starts as current mask (111).
				// If I want to remove C (idx 1).
				// I should set idx 1 to 0.
				// The original code was seemingly buggy or I misread it.
				// "mask.set(..., true)" -> sets bit to 1.
				// If it was already 1, no change.
				// Then getArchetype(newMask).
				// If the intention was to remove, it should be set to 0 (false).
				// I will fix this to set to false.
				mask.set(static_cast<size_t>(id), false);
			}
		}

		template<typename... Component>
		void removeComponentsFromArchetypeMask(Signature& mask) {
			(removeComponentFromArchetypeMaskImpl<Component>(mask), ...);
		}


		template<typename... Component>
		void addComponentsToArchetype(Entity entity, Archetype& to, Component&&... compontnts) {
			(addComponentToArchetypeImpl<Component>(entity, to, compontnts), ...);
		}

		template<class... Components>
		void moveEntity(Entity entity, std::shared_ptr<Archetype> from, std::shared_ptr<Archetype> to, Record& record, Components&&... compontnts) {
			for (auto& [ctype, cvec] : from->mComponentArrays) {
				if (to->mComponentArrays.contains(ctype)) {
					//moveComponent(entity, to->mComponentArrays.at(ctype), cvec, ctype);
				}
			}

			addComponentsToArchetype(entity, *to, compontnts);
			record.archetype = to;
		}

		template<typename T>
		RawType<T>& forEachComponentsImpl(Archetype& archetype, size_t i) {
			using U = RawType<T>;
			static_assert(std::is_base_of_v<Component, U>, "Must inherit from class Component");
			return archetype.getComponents<U>()->at(i);
		}

		template<class... Components, class F>
		void forEachComponents(Archetype& archetype, size_t i, F&& func) {
			auto entity = archetype.mComponentArrays.begin()->second->getEntity(i);
			func(entity, (forEachComponentsImpl<Components>(archetype, i), ...));
		}

		template<typename T>
		void emitOnAdd(Entity entity) {
			auto c = getComponent<T>(entity);
			if (c) mEventManager.emit(OnAdd<T>{entity, c.get()});
		}

		template<typename T>
		void emitOnRemove(Entity entity) {
			auto c = getComponent<T>(entity);
			if (c) mEventManager.emit(OnRemove<T>{entity, c.get()});
		}

	public:
		template<typename T>
		ComponentType getComponentTypeImpl() {
			static ComponentType componentId = registerNewComponentId<T>();
			return componentId;
		}

		template<typename T>
		ComponentType getComponentType() {
			return getComponentTypeImpl<RawType<T>>();
		}

		template<typename T>
		void registerComponent(StorageType type) {
			static_assert(std::is_base_of_v<Component, T>, "Must inherit from class Component");
			ComponentType id = getComponentType<T>();
			
			if (mComponentStorageType.contains(id)) {
				// Already registered. Check if consistent? or throw?
				// For now allow re-registration if same type? Or ignore.
				// If we want to change type runtime, that's complex (need data migration).
				// Assume initialization time.
				return;
			}

			mComponentStorageType[id] = type;
			if (type == StorageType::Archetype) {
				mDefaultArraysArchetype.insert({ id, std::make_shared<ComponentArray<T>>() });
			}
			else {
				mSparseArrays.insert({ id, std::make_shared<ComponentArray<T>>() });
			}
		}

		RelationManager& getRelationManager() { return mRelationManager; }
		const RelationManager& getRelationManager() const { return mRelationManager; }
		EventManager& getEventManager() { return mEventManager; }
		const EventManager& getEventManager() const { return mEventManager; }

		template<typename T>
		UTILS::WeakPtr<T> getSingletonComponent() {
			static_assert(std::is_base_of_v<Component, T>, "Must inherit from class Component");
			const auto type = getComponentType<T>();
			if (!mSingletonComponent.contains(type)) {
				return nullptr;
			}
			return mSingletonComponent.at(type);
		}

		template<typename T>
		UTILS::WeakPtr<T> addSingletonComponent() {
			static_assert(std::is_base_of_v<Component, T>, "Must inherit from class Component");
			const auto type = getComponentType<T>();
			mSingletonComponent[type] = T();
			return getSingletonComponent<T>();
		}

		template<typename T>
		UTILS::WeakPtr<T> addComponent(Entity entity) {
			static_assert(std::is_base_of_v<Component, T>, "Must inherit from class Component");
			addComponents<T>(entity, T());
			return getComponent<T>(entity);
		}

		template<typename T>
		UTILS::WeakPtr<T> addComponent(Entity entity, T&& newComponent) {
			static_assert(std::is_base_of_v<Component, T>, "Must inherit from class Component");
			addComponents<T>(entity, std::move(newComponent));
			return getComponent<T>(entity);
		}

		template<class... Components>
		void addComponents(Entity entity) {
			addComponents(entity, (Components(), ...));
		}

		template<class... Components>
		void addComponents(Entity entity, Components&&... newComponent) {
			Record& record = mEntityRecords.at(entity);
			const auto& archetype = record.archetype;
			auto newMask = archetype->mMask;
			// set all components to mask (Only Archetype ones)
			addComponentsToArchetypeMask(newMask);
			const auto& nextArchetype = getArchetype(newMask);

			// Move entity (Handles Archetype components)
			moveEntity(entity, archetype, nextArchetype, record, std::forward<Components>(newComponent)...);
			
			// Handle Sparse components
			(addComponentToSparseImpl<Components>(entity, std::forward<Components>(newComponent)), ...);
			(emitOnAdd<Components>(entity), ...);

			//auto signature = getSignature(entity);
			//signature.set(static_cast<int>(getComponentType<T>()), true);
			//setSignature(entity, newMask);
			//systemManager->entitySignatureChanged(entity, newMask);
		}

		template<class... Components>
		void removeComponents(Entity entity) {
			static_assert(sizeof...(Components) == 0, "Must contain non 0 components");
			(emitOnRemove<Components>(entity), ...);
			Record& record = mEntityRecords[entity];
			auto archetype = record.archetype;
			auto newMask = archetype->mMask;
			// set all components to mask (Only Archetype ones)
			removeComponentsFromArchetypeMask(newMask);
			auto nextArchetype = getArchetype(newMask);

			moveEntity(entity, archetype, nextArchetype, record);
			
			// Remove Sparse components
			(removeComponentFromSparseImpl<Components>(entity), ...);
		}

		template<typename T>
		UTILS::WeakPtr<T> getComponent(Entity entity) {
			static_assert(std::is_base_of_v<Component, T>, "Must inherit from class Component");
			ComponentType id = getComponentType<T>();
			
			// Check Archetype first
			const Record& record = mEntityRecords[entity];
			const auto& archetype = record.archetype;

			auto it = archetype->mComponentArrays.find(id);
			if (it != archetype->mComponentArrays.end()) {
				auto& componentArr = it->second;
				return std::static_pointer_cast<ComponentArray<T>>(componentArr)->getDataPtr(entity);
			}
			
			// Check Sparse
			if (mSparseArrays.contains(id)) {
				return std::static_pointer_cast<ComponentArray<T>>(mSparseArrays[id])->getDataPtr(entity);
			}

			return nullptr;
		}

		template<typename T>
		UTILS::WeakPtr<Component> getComponentBase(Entity entity) {
			return getComponentBase<T>(entity);
		}

		template<typename T>
		bool checkComponent(Entity entity) {
			static_assert(std::is_base_of_v<Component, T>, "Must inherit from class Component");
			ComponentType id = getComponentType<T>();

			// Archetype check
			const Record& record = mEntityRecords[entity];
			const auto& archetype = record.archetype;
			if (archetype->mComponentArrays.contains(id)) return true;

			// Sparse check
			if (mSparseArrays.contains(id)) {
				// We need to check if entity exists in this array.
				// ComponentArray->getDataPtr returns nullptr if not found?
				// need to check `entityDestroyed` logic or `hasData`.
				// ComponentArray usually has `getDataPtr`.
				return std::static_pointer_cast<ComponentArray<T>>(mSparseArrays[id])->getDataPtr(entity) != nullptr;
			}
			return false;
		}

		std::shared_ptr<Archetype> getArchetype(Signature mask) {
			if (!mMaskToArchetype.contains(mask)) {
				mMaskToArchetype[mask] = std::make_shared<Archetype>();
				auto& newArchetype = mMaskToArchetype[mask];
				newArchetype->mMask = mask;
				for (int id = 0; id < static_cast<int>(mNextComponentType); ++id) {
					if (mask[static_cast<size_t>(id)]) {
						auto type = ComponentType(id);
						newArchetype->mComponentArrays[type] = mDefaultArraysArchetype[type]->createEmptyFromThis();
					}
				}
			}
			return mMaskToArchetype[mask];
		}

		void registerEntity(Entity entity) {
			mEntityRecords[entity] = Record{{}, getArchetype(Signature())};
		}

		template<class... Components, class Func>
		void forEach(Func&& func) {
			Signature mask;
			addComponentsToArchetypeMask<Components>(mask);

			for (auto& [m, a] : mMaskToArchetype) {
				if ((mask & m) != mask) {
					continue;
				}
				for (size_t i = 0; i < a->size(); ++i) {
					forEachComponents(*a, i, func);
				}
			}
		}


		//void setSignature(Entity entity, Signature signature);

		Signature getSignature(Entity entity) {
#ifdef __DEBUG__
			if (!mSignatures.contains(entity)) {
				throw;
			}
#endif
			return mSignatures[entity];
		}


		// System methods
		//template<typename T>
		//std::shared_ptr<T> registerSystem() {
		//	static_assert(std::is_base_of_v<System, T>, "Must inherit from class Component");
		//	return systemManager->registerSystem<T>();
		//}
		//
		//template<typename T>
		//void setSystemSignature(Signature signature) {
		//	static_assert(std::is_base_of_v<System, T>, "Must inherit from class Component");
		//	systemManager->setSignature<T>(signature);
		//}
		//SystemManager& getSystemManager();

		

		//template<typename T0>
		//void forEachComponents(std::function<void(T0&)> func) {
		//	for (auto& [m, a] : maskToArchetype) {
		//		auto& arr0 = a->getComponents<T0>();
		//		if (arr0) {
		//			for (size_t i = 0; i < arr0->getSize(); ++i) {
		//				func(arr0->at(i));
		//			}
		//		}
		//	}
		//}
		//
		//template<typename T0, typename T1>
		//void forEachComponents(std::function<void(T0&, T1&)> func) {
		//	for (auto& [m, a] : maskToArchetype) {
		//		auto arr0 = a->getComponents<T0>();
		//		auto arr1 = a->getComponents<T1>();
		//		if (arr0 && arr1) {
		//			for (size_t i = 0; i < arr0->getSize(); ++i) {
		//				func(arr0->at(i), arr1->at(i));
		//			}
		//		}
		//	}
		//}
		//
		//template<typename T0, typename T1, typename T2>
		//void forEachComponents(std::function<void(T0&, T1&, T2&)> func) {
		//	for (auto& [m, a] : maskToArchetype) {
		//		auto arr0 = a->getComponents<T0>();
		//		auto arr1 = a->getComponents<T1>();
		//		auto arr2 = a->getComponents<T2>();
		//		if (arr0 && arr1 && arr2) {
		//			for (size_t i = 0; i < arr0->getSize(); ++i) {
		//				func(arr0->at(i), arr1->at(i), arr2->at(i));
		//			}
		//		}
		//	}
		//}
		//
		//template<typename T>
		//void moveEntity(Entity entity, std::shared_ptr<Archetype> from, std::shared_ptr<Archetype> to, Record& record) {
		//	static_assert(std::is_base_of_v<Component, T>, "Must inherit from class Component");
		//	for (auto& [ctype, cvec] : from->mComponentArrays) {
		//		if (to->mComponentArrays.contains(ctype)) {
		//			moveComponent(entity, to->mComponentArrays.at(ctype), cvec, ctype);
		//		}
		//	}
		//
		//	const auto cname = getComponentType<T>();
		//	if (to->mComponentArrays.contains(cname)) {
		//		T newComponent;
		//		std::static_pointer_cast<ComponentArray<T>>(to->mComponentArrays.at(cname))->insert(entity, newComponent);
		//	}
		//
		//	record.archetype = to;
		//}




	private:
		//template <typename T>
		//void tryGetComponent(std::vector<UTILS::WeakPtr<Component>>& res, Entity entity) {
		//	auto arr = getComponentArray<T>();
		//	if (arr->count(entity)) {
		//		res.push_back(arr->getDataBasePtr(entity));
		//	}
		//}

		//template<template<typename...> class Container, typename...ComponentType>
		//std::vector<UTILS::WeakPtr<Component>> tryGetComponents(Entity entity, Container<ComponentType...> opt) {
		//	std::vector<UTILS::WeakPtr<Component>> res;
		//	(tryGetComponent<ComponentType>(res, entity), ...);
		//	return res;
		//}
	public:
		//std::vector<UTILS::WeakPtr<Component>> getComponents(Entity entity) {
		//	return tryGetComponents(entity, ComponentsTypeProviderType{});
		//}

		//void entityDestroyed(Entity entity);


	public:
		//void enable(Entity id);
		//void disable(Entity id);
	};
}
