#pragma once
#include <bitset>
#include <cassert>
#include <chrono>
#include <functional>
#include <memory>
#include <list>
#include <unordered_map>
#include <unordered_set>

#include "componentArrayInterface.h"
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
			insertToGroup(system, typeName, addToNewGroup);
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
		template<typename T>
		void insertToGroup(std::shared_ptr<T> system, const char* id, bool addToNewGroup) {
			if (addToNewGroup) {
				mGroups.push_back(SystemGroup{{id}});
				return;
			}

			const auto& _readComps = system->getComponentsRead();
			const auto& _writeComps = system->getComponentsWrite();
			if (_readComps.empty() && _writeComps.empty()) {
				mGroups.push_back(SystemGroup{{id}});
				return;
			}

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
				} else {
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
