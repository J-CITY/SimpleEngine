#pragma once
#include <bitset>
#include <cassert>
#include <chrono>
#include <functional>
#include <memory>
#include <set>
#include <unordered_map>

#include "componentArrayInterface.h"
#include "../utils/structures.h"
#include <rttr/type.h>
namespace KUMA::ECS {
	constexpr unsigned MAX_COMPONENTS = 256;
	using Signature = std::bitset<MAX_COMPONENTS>;

	class System {
	public:
		constexpr static const char* SYSTEM_COMPONENTS_READ = "SYSTEM_COMPONENTS_READ";
		constexpr static const char* SYSTEM_COMPONENTS_WRITE = "SYSTEM_COMPONENTS_WRITE";

		virtual ~System() = default;
		std::set<Entity> mEntities;

		virtual void onAwake() {}
		virtual void onStart() {}

		virtual void onEnable() {}
		virtual void onDisable() {}

		virtual void onDestroy() {}

		virtual void onUpdate(std::chrono::duration<double> dt) {}
		virtual void onFixedUpdate(std::chrono::duration<double> dt) {}
		virtual void onLateUpdate(std::chrono::duration<double> dt) {}

		void delayed(std::function<void()> task) {
			delayedTask.push(task);
		}

		void onUpdateDelayed(std::chrono::duration<double> dt) {
			while (auto task = delayedTask.pop()) {
				(*task)();
			}
		}

		UTILS::LockFreeQueue<std::function<void()>> delayedTask;
	};

	class SystemManager {
	public:
		template<typename T>
		std::shared_ptr<T> registerSystem(bool addToNewGroup = false) {
			const char* typeName = typeid(T).name();

			assert(systems.find(typeName) == systems.end() && "Registering system more than once.");
			
			auto system = std::make_shared<T>();
			systems.insert({ typeName, system });
			insertToGroup(typeName, addToNewGroup);
			return system;
		}

		// Add systems to separated group for run update parallel for systems in group
		void insertToGroup(const char* id, bool addToNewGroup = false) {
			if (addToNewGroup) {
				mGroups.push_back(SystemGroup{ {id} });
				return;
			}

			std::string _id(id);
			auto systemRefl = rttr::type::get_by_name(_id.substr(_id.find_last_of("::") + 1));
			if (!systemRefl.is_valid()) {
				mGroups.push_back(SystemGroup{ {id} });
				return;
			}
			auto meta = systemRefl.get_metadata(KUMA::ECS::System::SYSTEM_COMPONENTS_READ);
			auto readComps = meta.get_value<std::string>();
			auto writeComps = systemRefl.get_metadata(KUMA::ECS::System::SYSTEM_COMPONENTS_WRITE).get_value<std::string>();

			auto _readComps = UTILS::split(readComps, '|');
			auto _writeComps = UTILS::split(writeComps, '|');

			for (auto& group : mGroups) {
				bool skipGroup = false;
				for (auto& e : _writeComps) {
					if (group.mReadComponents.contains(e) || group.mWriteComponents.contains(e)) {
						skipGroup = true;
						break;
					}
				}

				for (auto& e : _readComps) {
					if (group.mWriteComponents.contains(e)) {
						skipGroup = true;
						break;
					}
				}

				if (skipGroup) {
					continue;
				}
				else {
					for (auto& e : _readComps) {
						group.mReadComponents.insert(e);
					}
					for (auto& e : _writeComps) {
						group.mWriteComponents.insert(e);
					}
					group.mIds.push_back(id);
					return;
				}
			}

			mGroups.push_back(SystemGroup{{id}});
		}

		template<typename T>
		std::shared_ptr<T> getSystem() {
			const char* typeName = typeid(T).name();
			return std::dynamic_pointer_cast<T>(systems.at(typeName));
		}

		std::shared_ptr<System> getSystem(const char* id) {
			return systems.at(id);
		}

		template<typename T>
		void setSignature(Signature signature) {
			const char* typeName = typeid(T).name();

			assert(systems.find(typeName) != systems.end() && "System used before registered.");
			
			signatures.insert({ typeName, signature });
		}

		void entityDestroyed(Entity entity) {
			for (auto const& pair : systems) {
				auto const& system = pair.second;

				system->mEntities.erase(entity);
			}
		}

		void entitySignatureChanged(Entity entity, Signature entitySignature) {
			for (auto const& pair : systems) {
				auto const& type = pair.first;
				auto const& system = pair.second;
				auto const& systemSignature = signatures[type];
				
				if ((entitySignature & systemSignature) == systemSignature) {
					system->mEntities.insert(entity);
				}
				else {
					system->mEntities.erase(entity);
				}
			}
		}

		void runSystemGroups(std::chrono::duration<double> dt) const;

		struct SystemGroup {
			std::list<const char*> mIds;

			std::unordered_set<std::string> mReadComponents;
			std::unordered_set<std::string> mWriteComponents;
		};

		std::vector<SystemGroup> mGroups;
	//private:
		std::unordered_map<const char*, Signature> signatures;
		std::unordered_map<const char*, std::shared_ptr<System>> systems;
	};
}
