#include "taskSystem.h"


using namespace IKIGAI;
using namespace IKIGAI::TASK;

TaskSystemStatus ObjectStatus = TaskSystemStatus::TERMINANED;

TaskSystemStatus TaskSystem::getStatus() {
	return ObjectStatus;
}

bool TaskSystem::setup() {
	threadPoolExecutor = std::make_unique<ThreadPoolExecutor>();
	ObjectStatus = TaskSystemStatus::START;
	return threadPoolExecutor->setup();
}

bool TaskSystem::initialize() {
	if (ObjectStatus == TaskSystemStatus::START) {
		ObjectStatus = TaskSystemStatus::ACTIVE;
		return threadPoolExecutor->initialize();
	}
	return false;
}

bool TaskSystem::update() {
	if (ObjectStatus == TaskSystemStatus::ACTIVE) {
		return 	threadPoolExecutor->update();
	}
	else {
		ObjectStatus = TaskSystemStatus::SUSPEND;
		return false;
	}
}

bool TaskSystem::terminate() {
	ObjectStatus = TaskSystemStatus::TERMINANED;
	return threadPoolExecutor->terminate();
}

void TaskSystem::waitSync() {
	threadPoolExecutor->waitSync();
}

size_t TaskSystem::getThreadCounts() {
	return threadPoolExecutor->getThreadCounts();
}

std::shared_ptr<ITask> TaskSystem::addTask(std::unique_ptr<ITask>&& task, int threadID) {
	return threadPoolExecutor->addTask(std::move(task), threadID);
}

std::shared_ptr<ITask> TaskSystem::addTaskFront(std::unique_ptr<ITask>&& task, int threadID) {
	return threadPoolExecutor->addTaskFront(std::move(task), threadID);
}

//std::shared_ptr<ITask> TaskSystem::addTaskInNewThread(std::unique_ptr<ITask>&& task) {
//	return newThreadExecutor->addTask(std::move(task));
//}
