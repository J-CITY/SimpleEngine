#include "systemManager.h"

#include "componentManager.h"
#include "../resourceManager/ServiceManager.h"
#include <taskModule/taskSystem.h>
#include <utilsModule/stringUtils.h>

using namespace IKIGAI;
using namespace IKIGAI::ECS;


void SystemManager::insertToGroup(const char* id, bool addToNewGroup) {
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
	auto meta = systemRefl.get_metadata(IKIGAI::ECS::System::SYSTEM_COMPONENTS_READ);
	auto readComps = meta.get_value<std::string>();
	auto writeComps = systemRefl.get_metadata(IKIGAI::ECS::System::SYSTEM_COMPONENTS_WRITE).get_value<std::string>();

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

std::shared_ptr<System> SystemManager::getSystem(const char* id) {
	return systems.at(id);
}

void SystemManager::entityDestroyed(Entity entity) const {
	for (auto const& pair : systems) {
		auto const& system = pair.second;
		system->getEntities().erase(entity);
	}
}

void SystemManager::entitySignatureChanged(Entity entity, Signature entitySignature) {
	for (auto const& pair : systems) {
		auto const& type = pair.first;
		auto const& system = pair.second;
		auto const& systemSignature = signatures[type];
				
		if ((entitySignature & systemSignature) == systemSignature) {
			system->getEntities().insert(entity);
		}
		else {
			system->getEntities().erase(entity);
		}
	}
}

void SystemManager::runSystemGroups(std::chrono::duration<double> dt) const {
	static std::atomic_llong taskId = 0;

	std::list<TASK::TaskHandle<void>> waitTasks;
	for (auto& g : mGroups) {
		for (auto& systemId : g.mIds) {
			auto system = ECS::ComponentManager::GetInstance().getSystemManager().getSystem(systemId);
			auto task = RESOURCES::ServiceManager::Get<TASK::TaskSystem>().submit(("___system_task___" + std::to_string(taskId)).c_str(), -1, nullptr, [dt, system]() {
				system->onUpdate(dt);
				system->onLateUpdate(dt);
			});
			taskId += 1;
			waitTasks.push_back(task);
		}
		for (auto& t : waitTasks) {
			t.task->wait();
		}
		waitTasks.clear();
	}
	// Run delayed tasks
	for (auto& g : mGroups) {
		for (auto& systemId : g.mIds) {
			auto system = ECS::ComponentManager::GetInstance().getSystemManager().getSystem(systemId);
			auto task = RESOURCES::ServiceManager::Get<TASK::TaskSystem>().submit(("___system_task___" + std::to_string(taskId)).c_str(), -1, nullptr, [dt, system]() {
				system->onUpdateDelayed(dt);
			});
			taskId += 1;
			waitTasks.push_back(task);
		}
		for (auto& t : waitTasks) {
			t.task->wait();
		}
		waitTasks.clear();
	}
}

std::unordered_map<const char*, std::shared_ptr<System>>& SystemManager::getSystems() {
	return systems;
}
