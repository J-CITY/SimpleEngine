#pragma once
#include <any>
#include <map>
#include <functional>
#include <ranges>

#include "idObject.h"
#include <mutex>

namespace IKIGAI::EVENT {
	template<class... ArgTypes>
	class Event: public ObjectIdGenerator<Event<>> {
	public:
		Event() = default;
		using Callback = std::function<void(ArgTypes...)>;
		
		id add(Callback p_callback) {
			id listenerID = generateId();
			callbacks.emplace(listenerID, p_callback);
			return listenerID;
		}
		bool removeListener(id listenerID) {
			return callbacks.erase(listenerID) != 0;
		}
		void removeAllListeners() {
			callbacks.clear();
		}
		size_t getListenerCount() {
			return callbacks.size();
		}
		void run(ArgTypes... p_args) {
			for (auto const& [key, value] : callbacks) {
				value(p_args...);
			}
		}

	private:
		std::map<id, Callback> callbacks;
	};

	using EventListener = ObjectId<Event<>>;

	class EventBroadcaster: public ObjectIdGenerator<EventBroadcaster>
	{
	public:
		using Payload = std::unordered_map<std::string, std::any>;
		using Handler = std::function<void(std::string_view msg, const Payload&)>;
	private:
		std::mutex m;

		std::vector<std::pair<std::string, Payload>> deferredEventsMsgs;
		std::unordered_map<std::string, std::unordered_map<id, Handler>> eventMap;
	public:

		enum class ExecutionPolicy {
			IMMEDIATELY,
			DEFERRED
		};

		id add(std::string_view msg, Handler cb) {
			std::lock_guard lock(m);
			const id eventId = generateId();
			eventMap[std::string(msg)][eventId] = cb;
			return eventId;
		}

		void remove(std::string_view msg) {
			std::lock_guard lock(m);
			eventMap.erase(std::string(msg));
		}

		void remove(std::string_view msg, id cbId) {
			std::lock_guard lock(m);
			const auto _msg = std::string(msg);
			if (!eventMap.contains(_msg)) {
				return;
			}
			eventMap[_msg].erase(cbId);
		}

		void removeAll() {
			std::lock_guard lock(m);
			eventMap.clear();
		}

		void run(const std::string_view msg, const Payload& payload, ExecutionPolicy policy = ExecutionPolicy::IMMEDIATELY) {
			std::lock_guard lock(m);
			const auto _msg = std::string(msg);
			if (!eventMap.contains(_msg)) {
				return;
			}
			if (policy == ExecutionPolicy::IMMEDIATELY) {
				for (auto& cb : eventMap[_msg] | std::views::values) {
					cb(_msg, payload);
				}
			}
			else {
				deferredEventsMsgs.push_back({ _msg, payload });
			}
		}

		void runDeferred() {
			std::lock_guard lock(m);
			for (auto& [msg, payload] : deferredEventsMsgs) {
				for (auto& cb : eventMap[msg] | std::views::values) {
					cb(msg, payload);
				}
			}
			deferredEventsMsgs.clear();
		}
	};
}
