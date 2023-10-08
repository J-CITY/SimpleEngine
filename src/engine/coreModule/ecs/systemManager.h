#pragma once
#include <bitset>
#include <cassert>
#include <chrono>
#include <functional>
#include <memory>
#include <unordered_map>

#include "componentArrayInterface.h"
#include <rttr/type.h>
#include "system.h"

namespace IKIGAI::ECS {
	constexpr unsigned MAX_COMPONENTS = 256;
	using Signature = std::bitset<MAX_COMPONENTS>;

	class SystemManager {
	public:
		template<typename T>
		std::shared_ptr<T> registerSystem(bool addToNewGroup = false) {
			const char* typeName = typeid(T).name();

			assert(!systems.contains(typeName) && "Registering system more than once.");
			
			auto system = std::make_shared<T>();
			systems.insert({ typeName, system });
			insertToGroup(typeName, addToNewGroup);
			return system;
		}

		template<typename T>
		std::shared_ptr<T> getSystem() {
			const char* typeName = typeid(T).name();
			return std::dynamic_pointer_cast<T>(systems.at(typeName));
		}

		std::shared_ptr<System> getSystem(const char* id);

		template<typename T>
		void setSignature(Signature signature) {
			const char* typeName = typeid(T).name();

			assert(systems.contains(typeName) && "System used before registered.");
			
			signatures.insert({ typeName, signature });
		}

		void entityDestroyed(Entity entity) const;
		void entitySignatureChanged(Entity entity, Signature entitySignature);
		void runSystemGroups(std::chrono::duration<double> dt) const;

		std::unordered_map<const char*, std::shared_ptr<System>>& getSystems();

	private:
		// Add systems to separated group for run update parallel for systems in group
		void insertToGroup(const char* id, bool addToNewGroup = false);


		struct SystemGroup {
			std::list<const char*> mIds;

			std::unordered_set<std::string> mReadComponents;
			std::unordered_set<std::string> mWriteComponents;
		};

		std::vector<SystemGroup> mGroups;
		std::unordered_map<const char*, Signature> signatures;
		std::unordered_map<const char*, std::shared_ptr<System>> systems;
	};
}
