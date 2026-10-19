#pragma once

#include "task.h"
#include <chrono>
#include <cmath>

namespace IKIGAI::COROUTINE {
enum class Clock { Game, Real };

struct NextFrameAwaitable {
	bool await_ready() const noexcept {
		return false;
	}
	template <class P> void await_suspend(std::coroutine_handle<P> frame) const {
		detail::SuspendNextFrame(frame.promise());
	}
	void await_resume() const noexcept { }
};
[[nodiscard]] inline NextFrameAwaitable NextFrame() noexcept {
	return { };
}

struct WaitForSecondsAwaitable {
	double seconds;
	Clock clock;
	bool await_ready() const noexcept {
		return false;
	}
	template <class P> void await_suspend(std::coroutine_handle<P> frame) const {
		detail::SuspendFor(frame.promise(), seconds, clock == Clock::Real);
	}
	void await_resume() const noexcept { }
};
[[nodiscard]] inline WaitForSecondsAwaitable WaitForSeconds(double seconds, Clock clock = Clock::Game) {
	if (!std::isfinite(seconds) || seconds < 0.0)
		throw std::invalid_argument("Coroutine delay must be finite and nonnegative");
	return { seconds, clock };
}
template <class Rep, class Period>
[[nodiscard]] WaitForSecondsAwaitable WaitForSeconds(std::chrono::duration<Rep, Period> duration, Clock clock = Clock::Game) {
	return WaitForSeconds(std::chrono::duration<double>(duration).count(), clock);
}

struct BudgetAwaitable {
	bool await_ready() const noexcept {
		return false;
	}
	template <class P> bool await_suspend(std::coroutine_handle<P> frame) const {
		return detail::SuspendIfBudgetExceeded(frame.promise());
	}
	void await_resume() const noexcept { }
};
[[nodiscard]] inline BudgetAwaitable YieldIfBudgetExceeded() noexcept {
	return { };
}
}
