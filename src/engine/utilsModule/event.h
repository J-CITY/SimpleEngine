#pragma once
#include <any>
#include <map>
#include <functional>
#include <ranges>
#include <mutex>

#include "idGenerator.h"

namespace IKIGAI::EVENT {
	template<class... ArgTypes>
	class Event: public IdGenerator<Event<>> {
	public:
		Event() = default;
		using Callback = std::function<void(ArgTypes...)>;
		
		id add(Callback p_callback) {
			id listenerID = generateId();
			mCallbacks.emplace(listenerID, p_callback);
			return listenerID;
		}
		bool removeListener(id listenerID) {
			return mCallbacks.erase(listenerID) != 0;
		}
		void removeAllListeners() {
			mCallbacks.clear();
		}
		size_t getListenerCount() {
			return mCallbacks.size();
		}
		void run(ArgTypes... p_args) {
			for (auto const& [key, value] : mCallbacks) {
				value(p_args...);
			}
		}

	private:
		std::map<id, Callback> mCallbacks;
	};

	using EventListener = Id<Event<>>;

	class EventBroadcaster: public IdGenerator<EventBroadcaster>
	{
	public:
		using Payload = std::unordered_map<std::string, std::any>;
		using Handler = std::function<void(std::string_view msg, const Payload&)>;
	private:
		std::mutex m;

		std::vector<std::pair<std::string, Payload>> mDeferredEventsMsgs;
		std::unordered_map<std::string, std::unordered_map<id, Handler>> mEventMap;
	public:

		enum class ExecutionPolicy {
			IMMEDIATELY,
			DEFERRED
		};

		id add(std::string_view msg, Handler cb) {
			std::lock_guard lock(m);
			const id eventId = generateId();
			mEventMap[std::string(msg)][eventId] = cb;
			return eventId;
		}

		void remove(std::string_view msg) {
			std::lock_guard lock(m);
			mEventMap.erase(std::string(msg));
		}

		void remove(std::string_view msg, id cbId) {
			std::lock_guard lock(m);
			const auto _msg = std::string(msg);
			if (!mEventMap.contains(_msg)) {
				return;
			}
			mEventMap[_msg].erase(cbId);
		}

		void removeAll() {
			std::lock_guard lock(m);
			mEventMap.clear();
		}

		void run(const std::string_view msg, const Payload& payload, ExecutionPolicy policy = ExecutionPolicy::IMMEDIATELY) {
			std::lock_guard lock(m);
			const auto _msg = std::string(msg);
			if (!mEventMap.contains(_msg)) {
				return;
			}
			if (policy == ExecutionPolicy::IMMEDIATELY) {
				for (auto& cb : mEventMap[_msg] | std::views::values) {
					cb(_msg, payload);
				}
			}
			else {
				mDeferredEventsMsgs.push_back({ _msg, payload });
			}
		}

		void runDeferred() {
			std::lock_guard lock(m);
			for (auto& [msg, payload] : mDeferredEventsMsgs) {
				for (auto& cb : mEventMap[msg] | std::views::values) {
					cb(msg, payload);
				}
			}
			mDeferredEventsMsgs.clear();
		}
	};
}
