#include "task.h"

import logger;

#include <random>
#include <sstream>

#include "safeQueue.h"
using namespace IKIGAI::TASK;

bool ThreadPoolExecutor::setup(std::optional<unsigned> threadCount) {
	if (threadCount) {
		NumThreads = threadCount.value();
	}
	else {
		NumThreads = std::max<size_t>(std::thread::hardware_concurrency(), 2u);
	}
	Threads.resize(NumThreads);

	try {
		for (std::uint32_t i = 0u; i < NumThreads; ++i) {
			Threads[i] = std::make_unique<Thread>(i);
		}
	}
	catch (...) {
		terminate();
		throw;
	}

	return true;
}

bool ThreadPoolExecutor::initialize() {
	return true;
}

bool ThreadPoolExecutor::update() {
	return true;
}

bool ThreadPoolExecutor::terminate() {
	for (size_t i = 0; i < Threads.size(); i++) {
		Threads[i].reset();
	}
	return true;
}

void ThreadPoolExecutor::waitSync() {
	std::atomic_bool isAllThreadsIdle = false;

	while (!isAllThreadsIdle) {
		for (size_t i = 0; i < Threads.size(); i++) {
			isAllThreadsIdle = (Threads[i]->getUnfinishedWorkCount() == 0);
		}
	}

	//LOG_INFO("TaskExecutor: Reached synchronization point\n");
}

std::shared_ptr<ITask> ThreadPoolExecutor::addTask(std::unique_ptr<ITask>&& task, int32_t threadID) {
	int threadIndex;
	if (threadID != -1) {
		threadIndex = threadID;
	}
	else {
		std::random_device RD;
		std::mt19937 Gen(RD());
		std::uniform_int_distribution<> Dis(0, (size_t)NumThreads - 1);
		threadIndex = Dis(Gen);
	}

	return Threads[threadIndex]->addTask(std::move(task));
}

std::shared_ptr<ITask> ThreadPoolExecutor::addTaskFront(std::unique_ptr<ITask>&& task, int32_t threadID) {
	int threadIndex;
	if (threadID != -1) {
		threadIndex = threadID;
	}
	else {
		std::random_device RD;
		std::mt19937 Gen(RD());
		std::uniform_int_distribution<> Dis(0, (size_t)NumThreads - 1);
		threadIndex = Dis(Gen);
	}

	return Threads[threadIndex]->addTaskFront(std::move(task));
}

size_t ThreadPoolExecutor::getThreadCounts() {
	return NumThreads;
}

//--------------------- New thread executor


//bool NewThreadExecutor::setup() {
//	return true;
//}
//
//bool NewThreadExecutor::initialize() {
//	return true;
//}
//
//// run in own thread
//bool NewThreadExecutor::update() {
//	while (true) {
//		auto fOpt = workQueue.tryPop();
//		if (fOpt) {
//			fOpt.value().get();
//		}
//		else {
//			std::this_thread::yield();
//		}
//	}
//	return true;
//}
//
//bool NewThreadExecutor::terminate() {
//	for (size_t i = 0; i < workQueue.size(); i++) {
//		auto t = workQueue.tryPop();
//		if (t) {
//			t.value().get();
//		}
//	}
//	return true;
//}
//
//void NewThreadExecutor::waitSync() {
//	std::atomic_bool isAllTasksDone = false;
//
//	while (!isAllTasksDone) {
//		isAllTasksDone = (workQueue.size() == 0);
//	}
//	//LOG_INFO("TaskExecutor: Reached synchronization point\n");
//}
//
//std::shared_ptr<ITask> NewThreadExecutor::addTask(std::unique_ptr<ITask>&& task) {
//	std::shared_ptr<ITask> l_result{ std::move(task) };
//
//	auto f = std::async(std::launch::async, [l_result]() {
//		l_result->execute();
//	});
//
//	workQueue.push(f);
//	return l_result;
//}

//----------------------End

Thread::Thread(int ThreadIndex) {
	threadHandle = new std::thread(&Thread::worker, this, ThreadIndex);
}

Thread::~Thread() {
	isDone = true;
	workQueue.invalidate();
	if (threadHandle->joinable()) {
		threadHandle->join();
		delete threadHandle;
	}
}

inline ThreadState Thread::getState() const {
	return threadState;
}

size_t Thread::getUnfinishedWorkCount() {
	return workQueue.size();
}

std::shared_ptr<ITask> Thread::addTask(std::shared_ptr<ITask>&& task) {
	std::shared_ptr<ITask> l_result{task};
	workQueue.push(std::move(task));
	return l_result;
}

std::shared_ptr<ITask> Thread::addTaskFront(std::shared_ptr<ITask>&& task) {
	std::shared_ptr<ITask> l_result{ task };
	workQueue.pushFront(std::move(task));
	return l_result;
}

std::string Thread::getThreadID() {
	std::stringstream ss;
	ss << id.second;
	return ss.str();
}

void Thread::executeTask(std::shared_ptr<ITask>&& task) {
	task->execute();
}

void Thread::worker(uint32_t ThreadIndex) {
	auto _id = std::this_thread::get_id();
	id = std::make_pair(ThreadIndex, _id);
	threadState = ThreadState::FREE;
	
	while (!isDone) {
		std::shared_ptr<ITask> pTask{nullptr};
		if (auto res = workQueue.waitPop()) {
			pTask = res.value();
			threadState = ThreadState::BUSY;
			auto upstreamTask = pTask->getUpstreamTask();

			if (upstreamTask != nullptr) {
				if (upstreamTask->isFinished()) {
					executeTask(std::move(pTask));
				}
				else {
					addTask(std::move(pTask));
				}
			}
			else {
				executeTask(std::move(pTask));
			}

			threadState = ThreadState::FREE;
		}
	}

	threadState = ThreadState::FREE;
}
