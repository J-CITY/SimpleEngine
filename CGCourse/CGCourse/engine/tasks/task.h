#pragma once
#include <memory>
#include <future>

namespace KUMA {
	namespace TASK {
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

		class TaskExecutor {
		public:
			static bool setup();
			static bool initialize();
			static bool update();
			static bool terminate();

			static void waitSync();

			static std::shared_ptr<ITask> addTask(std::unique_ptr<ITask>&& task, int threadID);
			static size_t getThreadCounts();
		};
	}
}
