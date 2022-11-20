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

	};

	class SystemManager {
	public:
		template<typename T>
		std::shared_ptr<T> registerSystem() {
			const char* typeName = typeid(T).name();

			assert(systems.find(typeName) == systems.end() && "Registering system more than once.");
			
			auto system = std::make_shared<T>();
			systems.insert({ typeName, system });
			return system;
		}

		template<typename T>
		std::shared_ptr<T> getSystem() {
			const char* typeName = typeid(T).name();
			return std::dynamic_pointer_cast<T>(systems.at(typeName));
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

	//private:
		std::unordered_map<const char*, Signature> signatures;
		std::unordered_map<const char*, std::shared_ptr<System>> systems;
	};
}
