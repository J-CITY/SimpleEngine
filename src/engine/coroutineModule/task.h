#pragma once

#include <coroutine>
#include <exception>
#include <memory>
#include <optional>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <cstdint>

namespace IKIGAI::COROUTINE {
class Scheduler;
template<class T = void> class Task;

namespace detail {
struct ResultBase { std::exception_ptr error; };
template<class T> struct Result : ResultBase { std::optional<T> value; };
template<> struct Result<void> : ResultBase {};

struct PromiseBase {
    Scheduler* scheduler = nullptr;
    std::uint64_t id = 0;
    std::shared_ptr<ResultBase> result;

    explicit PromiseBase(std::shared_ptr<ResultBase> value) : result(std::move(value)) {}
    std::suspend_always initial_suspend() const noexcept { return {}; }
    std::suspend_always final_suspend() const noexcept { return {}; }
    void unhandled_exception() noexcept { result->error = std::current_exception(); }
};

void ScheduleChild(PromiseBase& parent, std::coroutine_handle<> child, PromiseBase& promise);
void SuspendNextFrame(PromiseBase& promise);
void SuspendFor(PromiseBase& promise, double seconds, bool realTime);
bool SuspendIfBudgetExceeded(PromiseBase& promise);

template<class T> struct TaskPromise final : PromiseBase {
    TaskPromise() : PromiseBase(std::make_shared<Result<T>>()) {}
    Task<T> get_return_object() noexcept;
    template<class U> void return_value(U&& value) {
        std::static_pointer_cast<Result<T>>(result)->value.emplace(std::forward<U>(value));
    }
};

template<> struct TaskPromise<void> final : PromiseBase {
    TaskPromise() : PromiseBase(std::make_shared<Result<void>>()) {}
    Task<void> get_return_object() noexcept;
    void return_void() const noexcept {}
};
}

// Lazy, move-only, single-consumer task. Await an rvalue to transfer ownership.
// Use only this module's awaitables: arbitrary awaiters may resume on other threads.
template<class T> class [[nodiscard]] Task {
    static_assert(std::is_void_v<T> || (std::is_object_v<T> && !std::is_array_v<T>));
public:
    using promise_type = detail::TaskPromise<T>;
    using Handle = std::coroutine_handle<promise_type>;

    Task() = default;
    explicit Task(Handle frame) noexcept : mFrame(frame) {}
    ~Task() { if (mFrame) mFrame.destroy(); }
    Task(const Task&) = delete;
    Task& operator=(const Task&) = delete;
    Task(Task&& other) noexcept : mFrame(std::exchange(other.mFrame, {})) {}
    Task& operator=(Task&& other) noexcept {
        if (this != &other) {
            if (mFrame) mFrame.destroy();
            mFrame = std::exchange(other.mFrame, {});
        }
        return *this;
    }

    class Awaiter {
    public:
        explicit Awaiter(Handle frame) : mFrame(frame),
            mResult(std::static_pointer_cast<detail::Result<T>>(frame.promise().result)) {}
        Awaiter(const Awaiter&) = delete;
        Awaiter& operator=(const Awaiter&) = delete;
        ~Awaiter() { if (mFrame) mFrame.destroy(); }
        bool await_ready() const noexcept { return false; }
        template<class P> void await_suspend(std::coroutine_handle<P> parent) {
            static_assert(std::is_base_of_v<detail::PromiseBase, P>);
            detail::ScheduleChild(parent.promise(), mFrame, mFrame.promise());
            mFrame = {};
        }
        T await_resume() {
            if (mResult->error) std::rethrow_exception(mResult->error);
            if constexpr (!std::is_void_v<T>) return std::move(mResult->value.value());
        }
    private:
        Handle mFrame;
        std::shared_ptr<detail::Result<T>> mResult;
    };

    Awaiter operator co_await() && {
        if (!mFrame) throw std::logic_error("Cannot await an empty coroutine task");
        return Awaiter(std::exchange(mFrame, {}));
    }

private:
    friend class Scheduler;
    Handle mFrame;
};

template<class T> Task<T> detail::TaskPromise<T>::get_return_object() noexcept {
    return Task<T>{std::coroutine_handle<TaskPromise<T>>::from_promise(*this)};
}
inline Task<void> detail::TaskPromise<void>::get_return_object() noexcept {
    return Task<void>{std::coroutine_handle<TaskPromise<void>>::from_promise(*this)};
}
}
