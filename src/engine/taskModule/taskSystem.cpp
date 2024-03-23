#include "taskSystem.h"

using namespace IKIGAI;
using namespace IKIGAI::TASK;

TaskSystemStatus ObjectStatus = TaskSystemStatus::TERMINANED;

TaskSystemStatus TaskSystem::getStatus() {
	return ObjectStatus;
}

bool TaskSystem::setup() {
	mThreadPoolExecutor = std::make_unique<ThreadPoolExecutor>();
	ObjectStatus = TaskSystemStatus::START;
	return mThreadPoolExecutor->setup();
}

bool TaskSystem::initialize() {
	if (ObjectStatus == TaskSystemStatus::START) {
		ObjectStatus = TaskSystemStatus::ACTIVE;
		return mThreadPoolExecutor->initialize();
	}
	return false;
}

bool TaskSystem::update() {
	if (ObjectStatus == TaskSystemStatus::ACTIVE) {
		return 	mThreadPoolExecutor->update();
	}
	else {
		ObjectStatus = TaskSystemStatus::SUSPEND;
		return false;
	}
}

bool TaskSystem::terminate() {
	ObjectStatus = TaskSystemStatus::TERMINANED;
	return mThreadPoolExecutor->terminate();
}

void TaskSystem::waitSync() {
	mThreadPoolExecutor->waitSync();
}

size_t TaskSystem::getThreadCounts() {
	return mThreadPoolExecutor->getThreadCounts();
}

std::shared_ptr<ITask> TaskSystem::addTask(std::unique_ptr<ITask>&& task, int threadID) {
	return mThreadPoolExecutor->addTask(std::move(task), threadID);
}

std::shared_ptr<ITask> TaskSystem::addTaskFront(std::unique_ptr<ITask>&& task, int threadID) {
	return mThreadPoolExecutor->addTaskFront(std::move(task), threadID);
}

std::shared_ptr<ITask> TaskSystem::addTaskInNewThread(std::unique_ptr<ITask>&& task) {
	return mNewThreadExecutor->addTask(std::move(task));
}
