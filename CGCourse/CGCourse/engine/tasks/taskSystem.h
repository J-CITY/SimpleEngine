#pragma once
#include <memory>

#include "task.h"

namespace KUMA {
	namespace TASK {
		enum class TaskSystemStatus {
			START,
			ACTIVE,
			SUSPEND,
			TERMINANED
		};

		template <typename T>
		struct TaskHandle {
			std::shared_ptr<ITask> task;
			std::shared_ptr<Future<T>>future;
		};
		
		class TaskSystem {
		public:
			bool setup();
			bool initialize();
			bool update();
			bool terminate();

			TaskSystemStatus getStatus();

			void waitSync();

			size_t getThreadCounts();

			template <typename Func, typename... Args>
			auto submit(const char* name, int32_t threadID, const std::shared_ptr<ITask>& upstreamTask, Func&& func, Args&&... args) {
				auto boundTask = std::bind(std::forward<Func>(func), std::forward<Args>(args)...);
				using ResultType = std::invoke_result_t<decltype(boundTask)>;
				using PackagedTask = std::packaged_task<ResultType()>;

				PackagedTask _task{std::move(boundTask)};
				Future<ResultType> _future{_task.get_future()};
				TaskHandle<ResultType> _handle;
				_handle.task = addTask(std::make_unique<Task<PackagedTask>>(std::move(_task), name, upstreamTask), threadID);
				_handle.future = std::make_shared<Future<ResultType>>(std::move(_future));
				return _handle;
			}
		protected:
			std::shared_ptr<ITask> addTask(std::unique_ptr<ITask>&& task, int threadID);
		};
	}
}

