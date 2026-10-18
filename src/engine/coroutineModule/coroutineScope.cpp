#include "coroutineScope.h"

namespace IKIGAI::COROUTINE {
CoroutineScope::CoroutineScope(Scheduler& scheduler) : mLifetime(scheduler.lifetime()) {
    scheduler.checkThread();
}

CoroutineScope::~CoroutineScope() { cancel(); }

void CoroutineScope::cancel() {
    if (auto lifetime = mLifetime.lock(); lifetime && lifetime->owner)
        lifetime->owner->checkThread();
    if (mCancelled) return;
    mCancelled = true;
    // Move first: destructors of coroutine locals may re-enter this scope.
    auto tasks = std::move(mTasks);
    for (const auto& task : tasks) task.cancel();
}
}
