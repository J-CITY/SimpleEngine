#include "systemManager.h"

#include "ComponentManager.h"
#include "../resourceManager/ServiceManager.h"
#include "../tasks/taskSystem.h"

using namespace KUMA;
using namespace KUMA::ECS;


void SystemManager::runSystemGroups(std::chrono::duration<double> dt) const {
	static std::atomic_llong taskId = 0;

	std::list<TASK::TaskHandle<void>> waitTasks;
	for (auto& g : mGroups) {
		for (auto& systemId : g.mIds) {
			auto system = ECS::ComponentManager::getInstance()->systemManager->getSystem(systemId);
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
			auto system = ECS::ComponentManager::getInstance()->systemManager->getSystem(systemId);
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
