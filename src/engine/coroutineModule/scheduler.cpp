#include "scheduler.h"

#include <algorithm>
#include <cmath>
#include <list>
#include <map>
#include <unordered_map>

namespace IKIGAI::COROUTINE {
namespace {
bool IsTerminal(TaskStatus status) {
    return status == TaskStatus::Completed || status == TaskStatus::Cancelled || status == TaskStatus::Failed;
}
void ValidateConfig(const SchedulerConfig& config) {
    if (!std::isfinite(config.frameBudget.count()) || config.frameBudget.count() < 0.0 || !config.maxResumesPerFrame)
        throw std::invalid_argument("Invalid coroutine scheduler budget or resume limit");
}
struct ProcessingGuard {
    bool& processing;
    explicit ProcessingGuard(bool& value) : processing(value) { processing = true; }
    ~ProcessingGuard() { processing = false; }
};
}

struct Scheduler::Impl {
    using Queue = std::list<std::uint64_t>;
    using Timers = std::multimap<double, std::uint64_t>;
    enum class Location { None, Ready, NextFrame, GameTimer, RealTimer, Child };
    struct Node {
        std::coroutine_handle<> frame;
        detail::PromiseBase* promise = nullptr;
        std::shared_ptr<detail::TaskState> state;
        std::uint64_t parent = 0;
        std::uint64_t child = 0;
        Location location = Location::None;
        Queue::iterator queueEntry;
        Timers::iterator timerEntry;
    };

    SchedulerConfig config;
    Now now;
    ErrorHandler errorHandler;
    std::shared_ptr<detail::SchedulerLifetime> lifetime;
    std::unordered_map<std::uint64_t, std::unique_ptr<Node>> nodes;
    Queue ready;
    Queue nextFrame;
    Timers gameTimers;
    Timers realTimers;
    double gameTime = 0.0;
    double realTime = 0.0;
    std::uint64_t nextId = 1;
    std::uint64_t current = 0;
    bool processing = false;
    bool cancellationPending = false;
    bool accepting = true;
    SteadyClock::time_point tickStart;
    SchedulerStats stats;

    Node* find(std::uint64_t id) const {
        const auto it = nodes.find(id);
        return it == nodes.end() ? nullptr : it->second.get();
    }
    Node& running(detail::PromiseBase& promise) const {
        auto* node = find(promise.id);
        if (!processing || current != promise.id || !node || node->promise != &promise)
            throw std::logic_error("Coroutine awaitable used outside its scheduler resume");
        return *node;
    }
    void unlink(Node& node) {
        switch (node.location) {
        case Location::Ready: ready.erase(node.queueEntry); break;
        case Location::NextFrame: nextFrame.erase(node.queueEntry); break;
        case Location::GameTimer: gameTimers.erase(node.timerEntry); break;
        case Location::RealTimer: realTimers.erase(node.timerEntry); break;
        default: break;
        }
        node.location = Location::None;
    }
    void enqueue(Node& node) {
        ready.push_back(node.state->id);
        node.queueEntry = std::prev(ready.end());
        node.location = Location::Ready;
    }
    void defer(Node& node) {
        nextFrame.push_back(node.state->id);
        node.queueEntry = std::prev(nextFrame.end());
        node.location = Location::NextFrame;
    }
    void markCancelled(Node& node) {
        cancellationPending = true;
        auto* next = &node;
        while (next) {
            next->state->cancelRequested = true;
            next = find(next->child);
        }
    }
    // Remove ownership before running user destructors or callbacks. Both may
    // enqueue/cancel other tasks, and unordered_map may rehash during callbacks.
    void finish(std::uint64_t id, TaskStatus status) {
        auto* node = find(id);
        const auto parentId = node->parent;
        auto state = node->state;
        auto frame = node->frame;
        unlink(*node);
        state->status = status;
        state->error = status == TaskStatus::Failed ? node->promise->result->error : nullptr;
        if (auto* parent = find(parentId)) parent->child = 0;
        nodes.erase(id);
        switch (status) {
        case TaskStatus::Completed: ++stats.completed; break;
        case TaskStatus::Cancelled: ++stats.cancelled; break;
        case TaskStatus::Failed: ++stats.failed; break;
        default: break;
        }
        frame.destroy();
        if (auto* parent = find(parentId); parent && !parent->state->cancelRequested) {
            enqueue(*parent);
        }
        if (!parentId && status == TaskStatus::Failed && errorHandler) {
            // A handler may replace itself. Its exceptions must not strand tasks.
            auto handler = errorHandler;
            try { handler(TaskHandle(state)); } catch (...) {}
        }
    }
    void drainCancelled() {
        if (!cancellationPending) return;
        for (;;) {
            auto it = std::find_if(nodes.begin(), nodes.end(), [](const auto& item) {
                return item.second->state->cancelRequested;
            });
            if (it == nodes.end()) { cancellationPending = false; return; }
            auto* node = it->second.get();
            // Destroy the deepest child first, without recursive destroy/resume.
            while (auto* child = find(node->child)) node = child;
            do {
                const auto parent = node->parent;
                finish(node->state->id, TaskStatus::Cancelled);
                node = find(parent);
            } while (node && node->state->cancelRequested);
        }
    }
    void wake(Timers& timers, double time) {
        while (!timers.empty() && timers.begin()->first <= time) {
            auto* node = find(timers.begin()->second);
            // Splice by ID into the ready queue; timer entries are removed on cancel.
            enqueue(*node);
            timers.erase(timers.begin());
        }
    }
    SchedulerStats snapshot() const {
        auto result = stats;
        result.active = nodes.size();
        result.ready = ready.size();
        result.waiting = result.active - result.ready;
        return result;
    }
};

Scheduler::Scheduler(SchedulerConfig config, Now now) : mImpl(std::make_unique<Impl>()) {
    ValidateConfig(config);
    if (!now) throw std::invalid_argument("Coroutine scheduler needs a clock");
    mImpl->config = config;
    mImpl->now = std::move(now);
    mImpl->lifetime = std::make_shared<detail::SchedulerLifetime>();
    mImpl->lifetime->owner = this;
    mImpl->lifetime->thread = std::this_thread::get_id();
}

Scheduler::~Scheduler() {
    shutdown();
    mImpl->lifetime->owner = nullptr;
}

void Scheduler::checkThread() const {
    if (std::this_thread::get_id() != mImpl->lifetime->thread)
        throw std::logic_error("Coroutine scheduler must be used on its owning thread");
}

std::weak_ptr<detail::SchedulerLifetime> Scheduler::lifetime() const { return mImpl->lifetime; }

TaskHandle Scheduler::adopt(std::coroutine_handle<> frame, detail::PromiseBase& promise,
                          std::string name, std::uint64_t parentId) {
    checkThread();
    auto& impl = *mImpl;
    if (!impl.accepting) throw std::logic_error("Coroutine scheduler is shut down");
    if (promise.scheduler) throw std::logic_error("Coroutine task already belongs to a scheduler");
    auto* parent = impl.find(parentId);
    if (parentId && (!parent || parent->child)) throw std::logic_error("Invalid coroutine parent");
    auto state = std::make_shared<detail::TaskState>();
    state->id = impl.nextId++;
    state->name = std::move(name);
    state->lifetime = impl.lifetime;
    auto node = std::make_unique<Impl::Node>();
    node->state = state;
    node->parent = parentId;
    node->promise = &promise;
    auto* adopted = node.get();
    impl.nodes.emplace(state->id, std::move(node));
    try { impl.enqueue(*adopted); }
    catch (...) { impl.nodes.erase(state->id); throw; }
    // Ownership transfers only after all allocations have succeeded.
    adopted->frame = frame;
    promise.scheduler = this;
    promise.id = state->id;
    if (parent) {
        parent->child = state->id;
        parent->location = Impl::Location::Child;
        if (parent->state->cancelRequested) adopted->state->cancelRequested = true;
    }
    return TaskHandle(std::move(state));
}

void Scheduler::tick(std::chrono::duration<double> gameDelta, std::chrono::duration<double> realDelta) {
    checkThread();
    auto& impl = *mImpl;
    if (impl.processing) throw std::logic_error("Cannot re-enter coroutine scheduler tick");
    if (!std::isfinite(gameDelta.count()) || gameDelta.count() < 0.0 ||
        !std::isfinite(realDelta.count()) || realDelta.count() < 0.0)
        throw std::invalid_argument("Coroutine deltas must be finite and nonnegative");
    ProcessingGuard guard(impl.processing);
    const auto frame = impl.stats.frame + 1;
    impl.stats = {};
    impl.stats.frame = frame;
    impl.tickStart = impl.now();
    impl.gameTime += gameDelta.count();
    impl.realTime += realDelta.count();
    impl.drainCancelled();
    for (auto id : impl.nextFrame) impl.find(id)->location = Impl::Location::Ready;
    impl.ready.splice(impl.ready.end(), impl.nextFrame);
    impl.wake(impl.gameTimers, impl.gameTime);
    impl.wake(impl.realTimers, impl.realTime);

    while (!impl.ready.empty() && impl.stats.resumes < impl.config.maxResumesPerFrame && impl.accepting) {
        const auto before = impl.now();
        if (before - impl.tickStart >= impl.config.frameBudget) break;
        const auto id = impl.ready.front();
        auto* node = impl.find(id);
        impl.unlink(*node);
        node->state->status = TaskStatus::Running;
        impl.current = id;
        node->frame.resume();
        impl.current = 0;
        const std::chrono::duration<double> elapsed = impl.now() - before;
        ++impl.stats.resumes;
        impl.stats.longestStep = std::max(impl.stats.longestStep, elapsed);
        if (elapsed > impl.config.frameBudget) ++impl.stats.stepOverruns;

        if (!node->state->cancelRequested) {
            if (node->frame.done()) {
                impl.finish(id, node->promise->result->error ? TaskStatus::Failed : TaskStatus::Completed);
            } else if (node->location == Impl::Location::None) {
                node->promise->result->error = std::make_exception_ptr(
                    std::logic_error("Unsupported awaitable: use coroutineModule awaitables"));
                impl.finish(id, TaskStatus::Failed);
            } else {
                node->state->status = TaskStatus::Suspended;
            }
        }
        impl.drainCancelled();
    }
    impl.stats.elapsed = impl.now() - impl.tickStart;
    impl.stats.budgetExceeded = impl.stats.elapsed >= impl.config.frameBudget;
}

void Scheduler::cancel(std::uint64_t id) {
    checkThread();
    if (auto* node = mImpl->find(id)) mImpl->markCancelled(*node);
    if (!mImpl->processing) {
        ProcessingGuard guard(mImpl->processing);
        mImpl->drainCancelled();
    }
}

void Scheduler::shutdown() {
    checkThread();
    auto& impl = *mImpl;
    impl.accepting = false;
    impl.cancellationPending = true;
    for (auto& [id, node] : impl.nodes) node->state->cancelRequested = true;
    if (!impl.processing) {
        ProcessingGuard guard(impl.processing);
        impl.drainCancelled();
    }
}

void Scheduler::setConfig(SchedulerConfig config) {
    checkThread();
    ValidateConfig(config);
    mImpl->config = config;
}
SchedulerConfig Scheduler::config() const { checkThread(); return mImpl->config; }
void Scheduler::setErrorHandler(ErrorHandler handler) { checkThread(); mImpl->errorHandler = std::move(handler); }
SchedulerStats Scheduler::stats() const { checkThread(); return mImpl->snapshot(); }
bool Scheduler::acceptingTasks() const { checkThread(); return mImpl->accepting; }
std::vector<TaskInfo> Scheduler::tasks() const {
    checkThread();
    std::vector<TaskInfo> result;
    result.reserve(mImpl->nodes.size());
    for (const auto& [id, node] : mImpl->nodes)
        result.push_back({id, node->state->name, node->state->status});
    std::sort(result.begin(), result.end(), [](const auto& a, const auto& b) { return a.id < b.id; });
    return result;
}

void TaskHandle::checkThread() const {
    if (mState) {
        if (auto lifetime = mState->lifetime.lock(); lifetime && std::this_thread::get_id() != lifetime->thread)
            throw std::logic_error("Coroutine handle must be used on its owning thread");
    }
}
TaskStatus TaskHandle::status() const {
    checkThread();
    if (!mState) throw std::logic_error("Empty coroutine handle");
    return mState->status;
}
bool TaskHandle::done() const { checkThread(); return !mState || IsTerminal(mState->status); }
const std::string& TaskHandle::name() const {
    checkThread();
    static const std::string empty;
    return mState ? mState->name : empty;
}
std::exception_ptr TaskHandle::error() const { checkThread(); return mState ? mState->error : nullptr; }
void TaskHandle::rethrowIfFailed() const { if (auto exception = error()) std::rethrow_exception(exception); }
void TaskHandle::cancel() const {
    checkThread();
    if (mState) {
        if (auto lifetime = mState->lifetime.lock(); lifetime && lifetime->owner)
            lifetime->owner->cancel(mState->id);
    }
}

namespace detail {
void ScheduleChild(PromiseBase& parent, std::coroutine_handle<> child, PromiseBase& promise) {
    if (!parent.scheduler) throw std::logic_error("Coroutine has no scheduler");
    parent.scheduler->checkThread();
    auto& impl = *parent.scheduler->mImpl;
    auto& node = impl.running(parent);
    // Inherit the operation name without growing it with nesting depth.
    parent.scheduler->adopt(child, promise, node.state->name, parent.id);
}
void SuspendNextFrame(PromiseBase& promise) {
    if (!promise.scheduler) throw std::logic_error("Coroutine has no scheduler");
    promise.scheduler->checkThread();
    auto& impl = *promise.scheduler->mImpl;
    impl.defer(impl.running(promise));
}
void SuspendFor(PromiseBase& promise, double seconds, bool realTime) {
    if (!std::isfinite(seconds) || seconds < 0.0)
        throw std::invalid_argument("Coroutine delay must be finite and nonnegative");
    if (seconds == 0.0) { SuspendNextFrame(promise); return; }
    if (!promise.scheduler) throw std::logic_error("Coroutine has no scheduler");
    promise.scheduler->checkThread();
    auto& impl = *promise.scheduler->mImpl;
    auto& node = impl.running(promise);
    auto& timers = realTime ? impl.realTimers : impl.gameTimers;
    node.timerEntry = timers.emplace((realTime ? impl.realTime : impl.gameTime) + seconds, promise.id);
    node.location = realTime ? Scheduler::Impl::Location::RealTimer : Scheduler::Impl::Location::GameTimer;
}
bool SuspendIfBudgetExceeded(PromiseBase& promise) {
    if (!promise.scheduler) throw std::logic_error("Coroutine has no scheduler");
    promise.scheduler->checkThread();
    auto& impl = *promise.scheduler->mImpl;
    auto& node = impl.running(promise);
    if (node.state->cancelRequested || !impl.accepting || impl.now() - impl.tickStart >= impl.config.frameBudget) {
        impl.defer(node);
        return true;
    }
    return false;
}
}
}
