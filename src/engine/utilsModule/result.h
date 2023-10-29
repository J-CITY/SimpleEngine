#pragma once
#include <optional>
#include <variant>

namespace IKIGAI::UTILS {
	template <class T>
	struct Ok;
	template <class E>
	struct Err;

	template <class T, class E>
	class Result {
		std::variant<Ok<T>, Err<E>> value;
	public:
		Result() = default;
		Result(const Ok<T>& val) : value(val) {}
		Result(const Err<E>& val) : value(val) {}

		Result& operator=(Ok<T> val) {
			value = val;
			return *this;
		}

		Result& operator=(Err<E> val) {
			value = val;
			return *this;
		}

		bool operator==(const Ok<T>& val) const {
			return isOk() && unwrap() == val.value;
		}

		bool operator==(const Err<E>& val) const {
			return isErr() && unwrapErr() == val.value;
		}

		bool operator==(const Result<T, E>& rhs) const {
			return (
				(isOk() && rhs.isOk() && unwrap() == rhs.unwrap()) ||
				(isErr() && rhs.isErr() && unwrapErr() == rhs.unwrapErr())
				);
		}

		bool operator!=(const Result<T, E>& rhs) const {
			return !(*this == rhs);
		}

		[[nodiscard]] bool isOk() const {
			return std::holds_alternative<Ok<T>>(value);
		}

		[[nodiscard]] bool isErr() const {
			return std::holds_alternative<Err<E>>(value);
		}
		
		std::optional<T> ok() const {
			if (isOk()) {
				return std::get<0>(value).value;
			}
			return std::nullopt;
		}
		
		std::optional<E> err() const {
			if (isErr()) {
				return std::get<1>(value).value;
			}
			return std::nullopt;
		}
		
		template <typename Function>
		Result andThen(Function op) {
			if (isOk()) {
				return op(unwrap());
			}
			return Err(err().value());
		}
		
		template <typename Function>
		Result orElse(Function op) {
			if (isErr()) {
				return op(unwrapErr());
			}
			return Ok(ok().value());
		}
		
		T unwrapOr(T val) {
			if (isOk()) {
				return unwrap();
			}
			return val;
		}

		template <typename Function>
		auto map(Function fn) -> Result<decltype(fn(T())), E> {
			if (isOk()) {
				return Result<decltype(fn(T())), E>(Ok<decltype(fn(T()))>(fn(unwrap())));
			}
			return Result<decltype(fn(T())), E>(Err<E>(unwrapErr()));
		}

		template <typename Value, typename Function>
		auto mapOr(Value default_value, Function fn) -> decltype(fn(T())) {
			if (isOk()) {
				return fn(unwrap());
			}
			return default_value;
		}

		template <typename ErrorFunction, typename OkFunction>
		auto map_or_else(ErrorFunction err_fn, OkFunction ok_fn) -> decltype(ok_fn(T())) {
			if (isOk()) {
				return ok_fn(unwrap());
			}
			return err_fn(unwrapErr());
		}

		template <typename Function>
		auto mapErr(Function fn) -> Result<T, decltype(fn(E()))> {
			if (isErr()) {
				return Result<T, decltype(fn(E()))>(Err<decltype(fn(E()))>(fn(unwrapErr())));
			}
			return Result<T, decltype(fn(E()))>(Ok<T>(unwrap()));
		}
		
		auto unwrap() const {
			if (isOk()) {
				return ok().value();
			}
			throw err().value();
		}

		auto unwrapErr() const {
			if (isErr()) {
				return err().value();
			}
			throw ok().value();
		}
	};

	template <typename T>
	struct Ok {
		T value;
		Ok(T value) : value(value) {}

		template <typename Function>
		Result<T, T> andThen(Function op) {
			return Result<T, T>(*this).and_then(op);
		}
	};

	template <typename E>
	struct Err {
		E value;
		Err(E value) : value(value) {}

		template <typename Function>
		Result<E, E> andThen(Function op) {
			return Result<E, E>(*this).and_then(op);
		}
	};
}
