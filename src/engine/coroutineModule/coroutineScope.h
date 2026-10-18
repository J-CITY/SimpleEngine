#pragma once

#include "scheduler.h"
#include <algorithm>

namespace IKIGAI::COROUTINE {
// Declare after the resources used by the tasks, so it is destroyed first.
// A scope may outlive the scheduler; starting after cancel() is an error.
class CoroutineScope final {
public:
    explicit CoroutineScope(Scheduler& scheduler);
    ~CoroutineScope();
    CoroutineScope(const CoroutineScope&) = delete;
    CoroutineScope& operator=(const CoroutineScope&) = delete;
    CoroutineScope(CoroutineScope&&) = delete;
    CoroutineScope& operator=(CoroutineScope&&) = delete;

    template<class T> TaskHandle start(Task<T>&& task, std::string name = {}) {
        auto lifetime = mLifetime.lock();
        if (mCancelled || !lifetime || !lifetime->owner)
            throw std::logic_error("Coroutine scope is closed");
        lifetime->owner->checkThread();
        std::erase_if(mTasks, [](const auto& handle) { return handle.done(); });
        // Reserve before transferring frame ownership, so allocation failure
        // cannot leave an untracked task running outside this scope.
        if (mTasks.size() == mTasks.capacity())
            mTasks.reserve(std::max<std::size_t>(4, mTasks.capacity() * 2));
        auto handle = lifetime->owner->start(std::move(task), std::move(name));
        mTasks.push_back(handle);
        return handle;
    }
    void cancel();

private:
    std::weak_ptr<detail::SchedulerLifetime> mLifetime;
    std::vector<TaskHandle> mTasks;
    bool mCancelled = false;
};
}
