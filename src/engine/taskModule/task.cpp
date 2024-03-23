#include "task.h"


#include <iostream>
#include <random>
#include <sstream>

#include "safeQueue.h"
using namespace IKIGAI::TASK;

bool ThreadPoolExecutor::setup(std::optional<unsigned> threadCount) {
	if (threadCount) {
		mNumThreads = threadCount.value();
	}
	else {
		mNumThreads = std::max<size_t>(std::thread::hardware_concurrency(), 2u);
	}
	mThreads.resize(mNumThreads);

	try {
		for (std::uint32_t i = 0u; i < mNumThreads; ++i) {
			mThreads[i] = std::make_unique<Thread>(i);
		}
	}
	catch (...) {
		terminate();
		std::cout << "PROBLEM INIT THREAD" << std::endl;
		return false;
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
	for (size_t i = 0; i < mThreads.size(); i++) {
		mThreads[i].reset();
	}
	mThreads.clear();
	return true;
}

void ThreadPoolExecutor::waitSync() {
	std::atomic_bool isAllThreadsIdle = false;

	while (!isAllThreadsIdle) {
		for (size_t i = 0; i < mThreads.size(); i++) {
			isAllThreadsIdle = (mThreads[i]->getUnfinishedWorkCount() == 0);
		}
	}
}

std::shared_ptr<ITask> ThreadPoolExecutor::addTask(std::unique_ptr<ITask>&& task, int32_t threadID) {
	int threadIndex;
	if (threadID != -1) {
		threadIndex = threadID;
	}
	else {
		std::random_device RD;
		std::mt19937 Gen(RD());
		std::uniform_int_distribution<> Dis(0, (size_t)mNumThreads - 1);
		threadIndex = Dis(Gen);
	}

	return mThreads[threadIndex]->addTask(std::move(task));
}

std::shared_ptr<ITask> ThreadPoolExecutor::addTaskFront(std::unique_ptr<ITask>&& task, int32_t threadID) {
	int threadIndex;
	if (threadID != -1) {
		threadIndex = threadID;
	}
	else {
		std::random_device RD;
		std::mt19937 Gen(RD());
		std::uniform_int_distribution<> Dis(0, (size_t)mNumThreads - 1);
		threadIndex = Dis(Gen);
	}

	return mThreads[threadIndex]->addTaskFront(std::move(task));
}

size_t ThreadPoolExecutor::getThreadCounts() {
	return mNumThreads;
}

//--------------------- New thread executor


bool NewThreadExecutor::setup() {
	return true;
}

bool NewThreadExecutor::initialize() {
	return true;
}

// run in own thread
bool NewThreadExecutor::update() {
	std::erase_if(mThreads, [](const auto& e) {
		return e->getUnfinishedWorkCount() == 0;
	});
	return true;
}

bool NewThreadExecutor::terminate() {
	for (size_t i = 0; i < mThreads.size(); i++) {
		mThreads[i].reset();
	}
	return true;
}

void NewThreadExecutor::waitSync() {
	std::atomic_bool isAllThreadsIdle = false;

	while (!isAllThreadsIdle) {
		for (size_t i = 0; i < mThreads.size(); i++) {
			isAllThreadsIdle = (mThreads[i]->getUnfinishedWorkCount() == 0);
		}
	}
}

std::shared_ptr<ITask> NewThreadExecutor::addTask(std::unique_ptr<ITask>&& task) {
	static int id = 0;
	auto thread = std::make_unique<Thread>(id);
	++id;
	mThreads.push_back(std::move(thread));
	return mThreads.back()->addTask(std::move(task));
}

size_t NewThreadExecutor::getThreadCounts() {
	return mThreads.size();
}

//----------------------End

Thread::Thread(int ThreadIndex) {
	mThreadHandle = new std::thread(&Thread::worker, this, ThreadIndex);
}

Thread::~Thread() {
	mIsDone = true;
	mWorkQueue.invalidate();
	if (mThreadHandle->joinable()) {
		mThreadHandle->join();
		delete mThreadHandle;
	}
}

inline ThreadState Thread::getState() const {
	return mThreadState;
}

size_t Thread::getUnfinishedWorkCount() {
	return mWorkQueue.size();
}

std::shared_ptr<ITask> Thread::addTask(std::shared_ptr<ITask>&& task) {
	std::shared_ptr<ITask> l_result{task};
	mWorkQueue.push(std::move(task));
	return l_result;
}

std::shared_ptr<ITask> Thread::addTaskFront(std::shared_ptr<ITask>&& task) {
	std::shared_ptr<ITask> l_result{ task };
	mWorkQueue.pushFront(std::move(task));
	return l_result;
}

std::string Thread::getThreadID() {
	std::stringstream ss;
	ss << mId.second;
	return ss.str();
}

void Thread::executeTask(std::shared_ptr<ITask>&& task) {
	task->execute();
}

void Thread::worker(uint32_t ThreadIndex) {
	auto _id = std::this_thread::get_id();
	mId = std::make_pair(ThreadIndex, _id);
	mThreadState = ThreadState::FREE;
	
	while (!mIsDone) {
		std::shared_ptr<ITask> pTask{nullptr};
		if (auto res = mWorkQueue.waitPop()) {
			pTask = res.value();
			mThreadState = ThreadState::BUSY;
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

			mThreadState = ThreadState::FREE;
		}
	}

	mThreadState = ThreadState::FREE;
}
