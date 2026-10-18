#pragma once

#include "task.h"
#include "taskHandle.h"
#include <chrono>
#include <functional>
#include <vector>

namespace IKIGAI::COROUTINE {
class CoroutineScope;

struct SchedulerConfig {
    std::chrono::duration<double> frameBudget = std::chrono::milliseconds(2);
    std::size_t maxResumesPerFrame = 1024;
};

struct SchedulerStats {
    std::uint64_t frame = 0;
    std::size_t active = 0;
    std::size_t ready = 0;
    std::size_t waiting = 0;
    std::size_t resumes = 0;
    std::size_t completed = 0;
    std::size_t cancelled = 0;
    std::size_t failed = 0;
    std::size_t stepOverruns = 0;
    bool budgetExceeded = false;
    std::chrono::duration<double> elapsed{};
    std::chrono::duration<double> longestStep{};
};

struct TaskInfo {
    std::uint64_t id;
    std::string name;
    TaskStatus status;
};

class Scheduler final {
public:
    using SteadyClock = std::chrono::steady_clock;
    // Injectable monotonic clock for deterministic budget tests.
    using Now = std::function<SteadyClock::time_point()>;
    using ErrorHandler = std::function<void(const TaskHandle&)>;

    explicit Scheduler(SchedulerConfig config = {}, Now now = SteadyClock::now);
    ~Scheduler();
    Scheduler(const Scheduler&) = delete;
    Scheduler& operator=(const Scheduler&) = delete;
    Scheduler(Scheduler&&) = delete;
    Scheduler& operator=(Scheduler&&) = delete;

    template<class T> TaskHandle start(Task<T>&& task, std::string name = {}) {
        if (!task.mFrame) throw std::invalid_argument("Cannot start an empty coroutine task");
        auto handle = adopt(task.mFrame, task.mFrame.promise(), std::move(name), 0);
        task.mFrame = {};
        return handle;
    }

    // Call exactly once per application frame, even when gameDelta is zero.
    void tick(std::chrono::duration<double> gameDelta, std::chrono::duration<double> realDelta);
    void setConfig(SchedulerConfig config);
    [[nodiscard]] SchedulerConfig config() const;
    void setErrorHandler(ErrorHandler handler);
    [[nodiscard]] SchedulerStats stats() const;
    [[nodiscard]] std::vector<TaskInfo> tasks() const;
    [[nodiscard]] bool acceptingTasks() const;
    // Nonblocking. When called inside a coroutine, disposal follows its resume().
    void shutdown();

private:
    friend class TaskHandle;
    friend class CoroutineScope;
    friend void detail::ScheduleChild(detail::PromiseBase&, std::coroutine_handle<>, detail::PromiseBase&);
    friend void detail::SuspendNextFrame(detail::PromiseBase&);
    friend void detail::SuspendFor(detail::PromiseBase&, double, bool);
    friend bool detail::SuspendIfBudgetExceeded(detail::PromiseBase&);
    struct Impl;
    std::unique_ptr<Impl> mImpl;
    TaskHandle adopt(std::coroutine_handle<> frame, detail::PromiseBase& promise, std::string name, std::uint64_t parent);
    void cancel(std::uint64_t id);
    void checkThread() const;
    std::weak_ptr<detail::SchedulerLifetime> lifetime() const;
};
}
