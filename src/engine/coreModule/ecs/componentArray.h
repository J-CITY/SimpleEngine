#pragma once

#include <cassert>
#include <functional>
#include <unordered_map>
#include <utility>

#include "componentArrayInterface.h"
#include <utilsModule/chunkList.h>
#include <taskModule/taskSystem.h>
#include "../resourceManager/ServiceManager.h"
#include <utilsModule/weakPtr.h>

namespace IKIGAI::ECS {
	template<typename T>
	class ComponentArray : public ComponentArrayInterface {
		inline static constexpr int CHUNK_SIZE = 100;
	public:
		void insertData(Entity entity, T& component) {
			assert(!entityToIndexInArray.contains(entity) && "Component added to same entity more than once.");

			auto newIndex = size;
			entityToIndexInArray[entity] = newIndex;
			indexInArrayToEntity.insert(std::make_pair(newIndex, entity));
			const char* typeName = typeid(T).name();
			if (componentArray.size() > newIndex) {
				componentArray[newIndex] = std::move(component);
			}
			else {
				componentArray.push_back(component);
			}
			// Update cb ptr
			componentArray[newIndex].getControlBlock()->m_ptr = &componentArray[newIndex];
			size++;
		}

		T removeData(Entity entity) {
			assert(entityToIndexInArray.contains(entity) && "Removing non-existent component.");

			// Copy element at end into deleted element's place to maintain density
			size_t indexOfRemovedEntity = entityToIndexInArray[entity];
			size_t indexOfLastElement = size - 1;
			auto component = std::move(componentArray[indexOfRemovedEntity]);
			componentArray[indexOfRemovedEntity] = std::move(componentArray[indexOfLastElement]);
			// Update cb ptr
			componentArray[indexOfRemovedEntity].getControlBlock()->m_ptr = &componentArray[indexOfRemovedEntity];

			// Update map to point to moved spot
			const Entity entityOfLastElement = indexInArrayToEntity.at(indexOfLastElement);
			entityToIndexInArray[entityOfLastElement] = indexOfRemovedEntity;
			indexInArrayToEntity.at(indexOfRemovedEntity) = entityOfLastElement;
			entityToIndexInArray.erase(entity);
			indexInArrayToEntity.erase(indexOfLastElement);
			size--;

			//TODO: add support change chunk size without remove last element
			componentArray.pop_back();

			return component;
		}

		T& getData(Entity entity) {
			assert(entityToIndexInArray.contains(entity) && "Retrieving non-existent component.");

			return componentArray[entityToIndexInArray[entity]];
		}

		UTILS::WeakPtr<T> getDataPtr(Entity entity) {
			static_assert(std::is_base_of_v<Component, T>, "Must inherit from class Component");
			assert(entityToIndexInArray.contains(entity) && "Retrieving non-existent component.");

			return componentArray[entityToIndexInArray[entity]].template getWeak<T>();
		}

		UTILS::WeakPtr<ECS::Component> getDataBasePtr(Entity entity) {
			static_assert(std::is_base_of_v<Component, T>, "Must inherit from class Component");
			assert(entityToIndexInArray.contains(entity) && "Retrieving non-existent component.");

			return UTILS::WeakPtr<ECS::Component>(componentArray[entityToIndexInArray[entity]]);
		}

		void entityDestroyed(Entity entity) override {
			if (entityToIndexInArray.contains(entity)) {
				removeData(entity);
			}
		}

		[[nodiscard]] bool count(Entity entity) const {
			return entityToIndexInArray.contains(entity);
		}

		[[nodiscard]] int getSize() const {
			return size;
		}

		auto begin() { return componentArray.begin(); }
		auto begin() const { return componentArray.begin(); }
		auto end() { return componentArray.end(); }
		auto end() const { return componentArray.end(); }

	private:
		UTILS::ChunkList<T, CHUNK_SIZE> componentArray;
		std::unordered_map<Entity, size_t> entityToIndexInArray;
		std::unordered_map<size_t, Entity> indexInArrayToEntity;
		int size = 0;
	};

	template<class T>
	void For(std::shared_ptr<ComponentArray<T>> data, std::function<void(T&)> func, int threadsCount = 4) {
		static std::atomic_llong taskId = 0;

		const int sz = data->size;
		const int chunkSz = sz / 4;
		int start = 0;
		std::list<TASK::TaskHandle<void>> waitTasks;
		for (int threadId = 0; threadId < threadsCount; threadId++) {
			int end = (threadId == threadsCount - 1 ? sz : start + chunkSz);
			auto task = RESOURCES::ServiceManager::Get<TASK::TaskSystem>().submit("___task___" + std::to_string(taskId), -1, nullptr, [data, start, end, func]() {
				for (int i = start; i < end; i++) {
					func((*data)[i]);
				}
				});
			waitTasks.push_back(task);
			taskId += 1;
			start += chunkSz;
		}
		for (auto& t : waitTasks) {
			t.task->wait();
		}
	}
}
