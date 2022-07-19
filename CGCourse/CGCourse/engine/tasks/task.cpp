#include "task.h"

import logger;

#include <random>
#include <sstream>

#include "safeQueue.h"
using namespace KUMA::TASK;

enum class ThreadState { FREE, BUSY };

class Thread {
public:
	explicit Thread(int ThreadIndex) {
		threadHandle = new std::thread(&Thread::worker, this, ThreadIndex);
	};

	~Thread() {
		isDone = true;
		workQueue.invalidate();
		if (threadHandle->joinable()) {
			threadHandle->join();
			delete threadHandle;
		}
	};

	Thread(const Thread&) = delete;
	Thread& operator=(const Thread& ) = delete;

	ThreadState getState() const;
	size_t getUnfinishedWorkCount();
	
	std::shared_ptr<ITask> addTask(std::shared_ptr<ITask>&& task);

private:
	std::string getThreadID();

	void worker(uint32_t ThreadIndex);

	void executeTask(std::shared_ptr<ITask>&& task);

	std::thread* threadHandle;
	std::pair<uint32_t, std::thread::id> id;
	std::atomic<ThreadState> threadState;
	std::atomic_bool isDone = false;
	ThreadSafeQueue<std::shared_ptr<ITask>> workQueue;
};

std::atomic_size_t NumThreads = 0;
std::vector<std::unique_ptr<Thread>> Threads;

bool TaskExecutor::setup() {
	NumThreads = std::max<size_t>(std::thread::hardware_concurrency(), 2u);
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

bool TaskExecutor::initialize() {
	return true;
}

bool TaskExecutor::update() {
	return true;
}

bool TaskExecutor::terminate() {
	for (size_t i = 0; i < Threads.size(); i++) {
		Threads[i].reset();
	}
	return true;
}

void TaskExecutor::waitSync() {
	std::atomic_bool isAllThreadsIdle = false;

	while (!isAllThreadsIdle) {
		for (size_t i = 0; i < Threads.size(); i++) {
			isAllThreadsIdle = (Threads[i]->getUnfinishedWorkCount() == 0);
		}
	}

	//LOG_INFO("TaskExecutor: Reached synchronization point\n");
}

std::shared_ptr<ITask> TaskExecutor::addTask(std::unique_ptr<ITask>&& task, int32_t threadID) {
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

size_t TaskExecutor::getThreadCounts() {
	return NumThreads;
}

inline ThreadState Thread::getState() const {
	return threadState;
}

size_t Thread::getUnfinishedWorkCount() {
	return workQueue.size();
}

inline std::shared_ptr<ITask> Thread::addTask(std::shared_ptr<ITask>&& task) {
	std::shared_ptr<ITask> l_result{task};
	workQueue.push(std::move(task));
	return l_result;
}

inline std::string Thread::getThreadID() {
	std::stringstream ss;
	ss << id.second;
	return ss.str();
}

inline void Thread::executeTask(std::shared_ptr<ITask>&& task) {
	task->execute();
}

inline void Thread::worker(uint32_t ThreadIndex) {
	auto _id = std::this_thread::get_id();
	id = std::make_pair(ThreadIndex, _id);
	threadState = ThreadState::FREE;
	
	while (!isDone) {
		std::shared_ptr<ITask> pTask{nullptr};
		if (workQueue.waitPop(pTask)) {
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
