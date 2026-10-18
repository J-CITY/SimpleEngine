#pragma once
#include "coroutineModule/awaitables.h"
#include "coroutineModule/scheduler.h"
#include <functional>
#include <unordered_map>
#include <vector>

namespace IKIGAI::RESOURCES {
enum class ResourceLoadStatus { Pending, Loading, Ready, Failed, Cancelled };
class ResourceLoadCancelled : public std::runtime_error {
public:
    ResourceLoadCancelled() : std::runtime_error("Resource request cancelled") {}
};
struct ResourceLoadControl {
    bool cancelled = false;
    std::function<bool()> valid;
    void check() const { if (cancelled || (valid && !valid())) throw ResourceLoadCancelled(); }
};
template<class T> struct ResourceRequestState {
    ResourceLoadStatus status = ResourceLoadStatus::Pending;
    std::shared_ptr<T> value;
    std::exception_ptr error;
    std::shared_ptr<ResourceLoadControl> control = std::make_shared<ResourceLoadControl>();
    std::size_t consumers = 0;
};
template<class T> class ResourceRequests;

// Main-thread only. Copies share a subscription; request() creates a new one.
template<class T> class ResourceRequest {
    struct Subscription {
        std::shared_ptr<ResourceRequestState<T>> state;
        bool active = true;
        explicit Subscription(std::shared_ptr<ResourceRequestState<T>> value) : state(std::move(value)) { ++state->consumers; }
        void cancel() {
            if (!active) return;
            active = false;
            if (--state->consumers == 0 && state->status != ResourceLoadStatus::Ready)
                state->control->cancelled = true;
        }
        ~Subscription() { cancel(); }
    };
    std::shared_ptr<Subscription> mSubscription;
    friend class ResourceRequests<T>;
    explicit ResourceRequest(std::shared_ptr<ResourceRequestState<T>> state)
        : mSubscription(std::make_shared<Subscription>(std::move(state))) {}
    static COROUTINE::Task<std::shared_ptr<T>> Wait(std::shared_ptr<Subscription> subscription) {
        if (!subscription) throw ResourceLoadCancelled();
        while (subscription->active) {
            auto& state = *subscription->state;
            if (state.status == ResourceLoadStatus::Ready) co_return state.value;
            if (state.status == ResourceLoadStatus::Failed) std::rethrow_exception(state.error);
            if (state.status == ResourceLoadStatus::Cancelled) break;
            co_await COROUTINE::NextFrame();
        }
        throw ResourceLoadCancelled();
    }
public:
    ResourceRequest() = default;
    [[nodiscard]] ResourceLoadStatus status() const {
        return !mSubscription || !mSubscription->active ? ResourceLoadStatus::Cancelled : mSubscription->state->status;
    }
    void cancel() const { if (mSubscription) mSubscription->cancel(); }
    [[nodiscard]] COROUTINE::Task<std::shared_ptr<T>> wait() const { return Wait(mSubscription); }
};

template<class T> class ResourceRequests {
public:
    using State = ResourceRequestState<T>;
    using Factory = std::function<COROUTINE::Task<std::shared_ptr<T>>(std::shared_ptr<ResourceLoadControl>)>;
    ~ResourceRequests() { shutdown(); }
    ResourceRequest<T> request(COROUTINE::Scheduler& scheduler, std::string key, Factory factory) {
        if (!scheduler.acceptingTasks()) throw std::logic_error("Resource scheduler is shut down");
        std::erase_if(mProducers, [](const auto& task) { return task.done(); });
        std::erase_if(mStates, [](const auto& entry) { return entry.second.expired(); });
        if (auto state = mStates[key].lock(); state && !state->control->cancelled &&
            state->status != ResourceLoadStatus::Failed && state->status != ResourceLoadStatus::Cancelled)
            return ResourceRequest<T>(std::move(state));
        auto state = std::make_shared<State>();
        ResourceRequest<T> result(state);
        mProducers.emplace_back();
        try { mProducers.back() = scheduler.start(Produce(state, std::move(factory)), "Resource: " + key); }
        catch (...) { mProducers.pop_back(); throw; }
        mStates[std::move(key)] = state;
        return result;
    }
    void shutdown() {
        for (auto& [key, weak] : mStates) if (auto state = weak.lock()) {
            state->control->cancelled = true;
            if (state->status == ResourceLoadStatus::Pending || state->status == ResourceLoadStatus::Loading)
                state->status = ResourceLoadStatus::Cancelled;
        }
        for (const auto& producer : mProducers) producer.cancel();
        mProducers.clear();
        mStates.clear();
    }
private:
    static COROUTINE::Task<> Produce(std::shared_ptr<State> state, Factory factory) {
        try {
            state->control->check();
            state->status = ResourceLoadStatus::Loading;
            auto value = co_await factory(state->control);
            state->control->check();
            if (!value) throw std::runtime_error("Resource loader returned an empty resource");
            state->value = std::move(value);
            state->status = ResourceLoadStatus::Ready;
        } catch (const ResourceLoadCancelled&) {
            state->status = ResourceLoadStatus::Cancelled;
        } catch (...) {
            state->error = std::current_exception();
            state->status = ResourceLoadStatus::Failed;
        }
    }
    std::unordered_map<std::string, std::weak_ptr<State>> mStates;
    std::vector<COROUTINE::TaskHandle> mProducers;
};
}
