#include "systemManager.h"

#include "componentManager.h"
#include <taskModule/taskSystem.h>
#include <utilsModule/stringUtils.h>

using namespace IKIGAI;
using namespace IKIGAI::ECS;

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
#ifndef __EMSCRIPTEN__
			auto task = RESOURCES::ServiceManager::Get<TASK::TaskSystem>().submit(("___system_task___" + std::to_string(taskId)).c_str(), -1, nullptr, [dt, system]() {
				system->onUpdate(dt);
				system->onLateUpdate(dt);
			});
			taskId += 1;
			waitTasks.push_back(task);
#else
			system->onUpdate(dt);
			system->onLateUpdate(dt);
#endif
		}
		for (auto& t : waitTasks) {
			t.mTask->wait();
		}
		waitTasks.clear();
	}
	// Run delayed tasks
	for (auto& g : mGroups) {
		for (auto& systemId : g.mIds) {
			auto system = ECS::ComponentManager::GetInstance().getSystemManager().getSystem(systemId);
#ifndef __EMSCRIPTEN__
			auto task = RESOURCES::ServiceManager::Get<TASK::TaskSystem>().submit(("___system_task___" + std::to_string(taskId)).c_str(), -1, nullptr, [dt, system]() {
				system->onUpdateDelayed(dt);
			});
			taskId += 1;
			waitTasks.push_back(task);
#else
			system->onUpdateDelayed(dt);
#endif
		}
		for (auto& t : waitTasks) {
			t.mTask->wait();
		}
		waitTasks.clear();
	}
}

std::unordered_map<const char*, std::shared_ptr<System>>& SystemManager::getSystems() {
	return systems;
}
