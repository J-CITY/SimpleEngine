#include "taskSystem.h"


using namespace KUMA;
using namespace KUMA::TASK;

TaskSystemStatus ObjectStatus = TaskSystemStatus::TERMINANED;

TaskSystemStatus TaskSystem::getStatus() {
	return ObjectStatus;
}

bool TaskSystem::setup() {
	ObjectStatus = TaskSystemStatus::START;
	return TaskExecutor::setup();
}

bool TaskSystem::initialize() {
	if (ObjectStatus == TaskSystemStatus::START) {
		ObjectStatus = TaskSystemStatus::ACTIVE;
		return TaskExecutor::initialize();
	}
	return false;
}

bool TaskSystem::update() {
	if (ObjectStatus == TaskSystemStatus::ACTIVE) {
		return 	TaskExecutor::update();
	}
	else {
		ObjectStatus = TaskSystemStatus::SUSPEND;
		return false;
	}
}

bool TaskSystem::terminate() {
	ObjectStatus = TaskSystemStatus::TERMINANED;
	return TaskExecutor::terminate();
}

void TaskSystem::waitSync() {
	TaskExecutor::waitSync();
}

size_t TaskSystem::getThreadCounts() {
	return TaskExecutor::getThreadCounts();
}

std::shared_ptr<ITask> TaskSystem::addTask(std::unique_ptr<ITask>&& task, int threadID) {
	return TaskExecutor::addTask(std::move(task), threadID);
}
