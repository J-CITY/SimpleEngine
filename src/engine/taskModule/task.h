#pragma once
#include <memory>
#include <future>
#include <optional>

#include "safeQueue.h"


namespace IKIGAI {
	namespace TASK {
		class ITask;

		enum class ThreadState { FREE, BUSY };

		class Thread {
		public:
			explicit Thread(int ThreadIndex);;
			~Thread();;

			Thread(const Thread&) = delete;
			Thread& operator=(const Thread&) = delete;

			ThreadState getState() const;
			size_t getUnfinishedWorkCount();

			std::shared_ptr<ITask> addTask(std::shared_ptr<ITask>&& task);
			std::shared_ptr<ITask> addTaskFront(std::shared_ptr<ITask>&& task);

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

		class ITask {
		public:
			ITask() = default;
			virtual ~ITask() = default;
			ITask(const ITask& rhs) = delete;
			ITask& operator=(const ITask& rhs) = delete;
			ITask(ITask&& other) = default;
			ITask& operator=(ITask&& other) = default;

			virtual void execute() = 0;
			virtual const std::string getName() = 0;
			virtual const std::shared_ptr<ITask>& getUpstreamTask() = 0;
			virtual bool isFinished() = 0;
			virtual void wait() = 0;
		};

		template <typename Functor>
		class Task : public ITask {
		public:
			Task(Functor&& functor, const char* name, const std::shared_ptr<ITask>& upstreamTask)
				:functor{std::move(functor)}, name{name}, upstreamTask{upstreamTask} {}

			~Task() override = default;
			Task(const Task&) = delete;
			Task& operator=(const Task&) = delete;
			Task(Task&& other) = default;
			Task& operator=(Task&& other) = default;

			void execute() override {
				functor();
				_isFinished = true;
			}

			const std::string getName() override {
				return name;
			}

			const std::shared_ptr<ITask>& getUpstreamTask() override {
				return upstreamTask;
			}

			bool isFinished() override {
				return _isFinished;
			}

			void wait() override {
				while (!_isFinished);
			}

		private:
			Functor functor;
			std::string name;
			std::shared_ptr<ITask> upstreamTask;
			std::atomic_bool _isFinished = false;
		};

		template <typename T>
		class Future {
		public:
			Future(std::future<T>&& future)
				:future{std::move(future)} {
			}

			~Future() {
				if (future.valid()) {
					future.get();
				}
			}

			Future(const Future&) = delete;
			Future& operator=(const Future&) = delete;
			Future(Future&& other) = default;
			Future& operator=(Future&& other) = default;

			auto get() {
				return future.get();
			}

		private:
			std::future<T> future;
		};

		class ThreadPoolExecutor {
			std::atomic_size_t NumThreads = 0;
			std::vector<std::unique_ptr<Thread>> Threads;
		public:
			ThreadPoolExecutor() { setup(std::nullopt); };
			ThreadPoolExecutor(int threadCount) { setup(threadCount); };
			bool setup(std::optional<unsigned> threadCount=std::nullopt);
			bool initialize();
			bool update();
			bool terminate();

			void waitSync();

			std::shared_ptr<ITask> addTask(std::unique_ptr<ITask>&& task, int threadID);
			std::shared_ptr<ITask> addTaskFront(std::unique_ptr<ITask>&& task, int threadID);
			size_t getThreadCounts();
		};

		//class NewThreadExecutor {
		//	ThreadSafeQueue<std::future<void>> workQueue;
		//public:
		//	bool setup();
		//	bool initialize();
		//	bool update();
		//	bool terminate();
		//
		//	void waitSync();
		//
		//	std::shared_ptr<ITask> addTask(std::unique_ptr<ITask>&& task);
		//	size_t getThreadCounts();
		//};

		class MainThreadExecutor {
		public:
			bool setup(std::optional<unsigned> threadCount);
			bool initialize();
			bool update();
			bool terminate();

			void waitSync();

			std::shared_ptr<ITask> addTask(std::unique_ptr<ITask>&& task, int threadID);
			size_t getThreadCounts();
		};
	}
}
