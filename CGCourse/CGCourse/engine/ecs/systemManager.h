#pragma once
#include <bitset>
#include <cassert>
#include <chrono>
#include <memory>
#include <set>
#include <unordered_map>

#include "componentArrayInterface.h"

namespace KUMA::ECS {
	constexpr unsigned MAX_COMPONENTS = 256;
	using Signature = std::bitset<MAX_COMPONENTS>;

	class System {
	public:
		std::set<Entity> mEntities;

		virtual void onAwake() {}
		virtual void onStart() {}

		virtual void onEnable() {}
		virtual void onDisable() {}

		virtual void onDestroy() {}

		virtual void onUpdate(std::chrono::duration<double> dt) {}
		virtual void onFixedUpdate(std::chrono::duration<double> dt) {}
		virtual void onLateUpdate(std::chrono::duration<double> dt) {}

	};

	class SystemManager {
	public:
		template<typename T>
		std::shared_ptr<T> registerSystem() {
			const char* typeName = typeid(T).name();

			assert(systems.find(typeName) == systems.end() && "Registering system more than once.");

			// Create a pointer to the system and return it so it can be used externally
			auto system = std::make_shared<T>();
			systems.insert({ typeName, system });
			return system;
		}

		template<typename T>
		void setSignature(Signature signature) {
			const char* typeName = typeid(T).name();

			assert(systems.find(typeName) != systems.end() && "System used before registered.");

			// Set the signature for this system
			signatures.insert({ typeName, signature });
		}

		void entityDestroyed(Entity entity) {
			// Erase a destroyed entity from all system lists
			// mEntities is a set so no check needed
			for (auto const& pair : systems) {
				auto const& system = pair.second;

				system->mEntities.erase(entity);
			}
		}

		void entitySignatureChanged(Entity entity, Signature entitySignature) {
			// Notify each system that an entity's signature changed
			for (auto const& pair : systems) {
				auto const& type = pair.first;
				auto const& system = pair.second;
				auto const& systemSignature = signatures[type];

				// Entity signature matches system signature - insert into set
				if ((entitySignature & systemSignature) == systemSignature) {
					system->mEntities.insert(entity);
				}
				// Entity signature does not match system signature - erase from set
				else {
					system->mEntities.erase(entity);
				}
			}
		}

	//private:
		// Map from system type string pointer to a signature
		std::unordered_map<const char*, Signature> signatures;

		// Map from system type string pointer to a system pointer
		std::unordered_map<const char*, std::shared_ptr<System>> systems;
	};
}
