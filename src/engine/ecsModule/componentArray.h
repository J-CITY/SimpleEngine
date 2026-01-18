#pragma once

#include "entityManager.h"
#include "componentArrayInterface.h"
#include "utilsModule/assertion.h"
#include "utilsModule/chunkList.h"
#include "utilsModule/exeptions.h"
#include "utilsModule/weakPtr.h"

namespace IKIGAI::ECS2 {
	template<class T, template<class, class> class CONT=std::vector, unsigned CHUNK_SIZE=100>
	class ComponentArray : public ComponentArrayInterface {
	public:
		ComponentArray() = default;

		void insert(Entity entity, T&& component) {
			ASSERT_IF(!entityToIndexInArray.contains(entity), "Component added to same entity more than once");

			auto newIndex = size;
			entityToIndexInArray[entity] = newIndex;
			entityToIndexInArray[entity] = newIndex;
			indexToEntity.push_back(entity);
			if (componentArray.size() > newIndex) {
				componentArray[newIndex] = std::forward<T>(component);
			} else {
				componentArray.push_back(std::forward<T>(component));
			}
			// Update cb ptr
			componentArray[newIndex].getControlBlock()->mPtr = &componentArray[newIndex];
			size++;
		}

		T removeData(Entity entity) {
			ASSERT_IF(entityToIndexInArray.contains(entity), "Removing non-existent component");

			// Copy element at end into deleted element's place to maintain density
			const size_t indexOfRemovedEntity = entityToIndexInArray[entity];
			const size_t indexOfLastElement = size - 1;
			auto component = std::move(componentArray[indexOfRemovedEntity]);
			auto moveElem = std::move(componentArray[indexOfLastElement]);
			componentArray[indexOfRemovedEntity] = std::move(moveElem);
			// Update cb ptr
			componentArray[indexOfRemovedEntity].getControlBlock()->mPtr = &componentArray[indexOfRemovedEntity];

			// Update map to point to moved spot
			// Update map to point to moved spot
			const Entity entityOfLastElement = indexToEntity.back();
			entityToIndexInArray[entityOfLastElement] = indexOfRemovedEntity;
			indexToEntity[indexOfRemovedEntity] = entityOfLastElement;
			entityToIndexInArray.erase(entity);
			indexToEntity.pop_back();
			size--;

			componentArray.pop_back();
			return component;
		}

		T& getData(Entity entity) {
			ASSERT_IF(entityToIndexInArray.contains(entity), "Retrieving non-existent component");
			return componentArray[entityToIndexInArray[entity]];
		}

		UTILS::WeakPtr<T> getDataPtr(Entity entity) {
			static_assert(std::is_base_of_v<ECS::Component, T>, "Must inherit from class Component");
			ASSERT_IF(entityToIndexInArray.contains(entity), "Retrieving non-existent component");
			return componentArray[entityToIndexInArray[entity]].template getWeak<T>();
		}

		UTILS::WeakPtr<ECS::Component> getDataBasePtr(Entity entity) {
			static_assert(std::is_base_of_v<ECS::Component, T>, "Must inherit from class Component");
			ASSERT_IF(entityToIndexInArray.contains(entity), "Retrieving non-existent component");
			return UTILS::WeakPtr<ECS::Component>(componentArray[entityToIndexInArray[entity]]);
		}

		//??
		void entityDestroyed(Entity entity) /*override*/ {
			if (entityToIndexInArray.contains(entity)) {
				removeData(entity);
			}
		}

		//std::shared_ptr<ComponentArrayInterface> createEmptyFromThis() /*override*/ {
		//	return std::make_shared<ComponentArray<T>>();
		//}

		[[nodiscard]] bool contains(Entity entity) const {
			return entityToIndexInArray.contains(entity);
		}

		[[nodiscard]] int getSize() const /*override*/ {
			return size;
		}

		T& at(size_t i) {
			if (i >= getSize()) {
				throw UTILS::EXEPTIONS::IndexOutOfRange();
			}
			return componentArray[i];
		}

		bool empty() const {
			return size == 0;
		}

		auto begin() { return componentArray.begin(); }
		auto begin() const { return componentArray.begin(); }
		auto end() { return componentArray.end(); }
		auto end() const { return componentArray.end(); }

		Entity getEntity(size_t index) override {
			return indexToEntity[index];
		}

	private:
		UTILS::ChunkList<T, CONT, CHUNK_SIZE> componentArray;
		std::unordered_map<Entity, size_t> entityToIndexInArray;
		std::vector<Entity> indexToEntity;
		size_t size = 0;
	};

//	template<class T>
//	void For(std::shared_ptr<ComponentArray<T>> data, std::function<void(T&)> func, int threadsCount = 4) {
//#ifndef __EMSCRIPTEN__
//		static std::atomic_llong taskId = 0;
//
//		const int sz = data->size;
//		const int chunkSz = sz / 4;
//		int start = 0;
//		std::list<TASK::TaskHandle<void>> waitTasks;
//		for (int threadId = 0; threadId < threadsCount; threadId++) {
//			int end = (threadId == threadsCount - 1 ? sz : start + chunkSz);
//			auto task = RESOURCES::ServiceManager::Get<TASK::TaskSystem>().submit("___task___" + std::to_string(taskId), -1, nullptr, [data, start, end, func]() {
//				for (int i = start; i < end; i++) {
//					func((*data)[i]);
//				}
//				});
//			waitTasks.push_back(task);
//			taskId += 1;
//			start += chunkSz;
//		}
//		for (auto& t : waitTasks) {
//			t.mTask->wait();
//		}
//#else
//		for (int i = 0; i < data->size(); i++) {
//			func((*data)[i]);
//		}
//#endif
}
