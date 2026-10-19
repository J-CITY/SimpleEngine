#pragma once

#include <cstdint>
#include <exception>
#include <memory>
#include <string>
#include <thread>

namespace IKIGAI::COROUTINE {
class Scheduler;

enum class TaskStatus { Pending, Running, Suspended, Completed, Cancelled, Failed };

namespace detail {
	struct SchedulerLifetime {
		Scheduler* owner = nullptr;
		std::thread::id thread;
	};

	// Handles retain only this metadata; coroutine frames belong to the scheduler.
	struct TaskState {
		TaskStatus status = TaskStatus::Pending;
		std::string name;
		std::exception_ptr error;
		bool cancelRequested = false;
		std::uint64_t id = 0;
		std::weak_ptr<SchedulerLifetime> lifetime;
	};
}

// All access, including cancellation, is confined to the scheduler's thread.
class TaskHandle {
public:
	TaskHandle() = default;
	explicit operator bool() const noexcept {
		return static_cast<bool>(mState);
	}
	[[nodiscard]] TaskStatus status() const;
	[[nodiscard]] bool done() const;
	[[nodiscard]] const std::string& name() const;
	[[nodiscard]] std::exception_ptr error() const;
	void rethrowIfFailed() const;
	void cancel() const;

private:
	friend class Scheduler;
	explicit TaskHandle(std::shared_ptr<detail::TaskState> state)
		: mState(std::move(state)) { }
	void checkThread() const;
	std::shared_ptr<detail::TaskState> mState;
};
}
