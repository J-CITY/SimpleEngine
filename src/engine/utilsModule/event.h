#pragma once
#include <any>
#include <map>
#include <functional>
#include <ranges>

#include "idObject.h"
#include <mutex>

#include "assertion.h"
#include "singleton.h"

#define EVENT_BIND(Function, Object) std::bind(Function, Object, std::placeholders::_1)

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
	/*
	class Subscription : public ObjectIdGenerator<Subscription> {};

	class MessageBroadcaster
	{
	public:
		MessageBroadcaster(): mThreadId(std::this_thread::get_id()) {}

		virtual ~MessageBroadcaster() = default;

		using Handler = std::function<void(const std::string&)>;
	protected:
		struct SubscriptionWrapper {
			Subscription mSubscription;
			Handler mCd;
		};
	public:
		[[nodiscard]] virtual Subscription subscribe(const std::string& messageId, Handler cb) {
			if (isThisThreadIsOwner()) {
				//TODO: return result
				ASSERT("Try call broadcaster from different thread");
				return Subscription();
			}
			return _subscribe(messageId, cb);
		}

		virtual void broadcast(const std::string& messageId) {
			if (isThisThreadIsOwner()) {
				//TODO: return result
				ASSERT("Try call broadcaster from different thread");
				return;
			}
			_broadcast(messageId);
		}

		virtual void remove(Subscription id) {
			if (isThisThreadIsOwner()) {
				//TODO: return result
				ASSERT("Try call broadcaster from different thread");
				return;
			}
			_remove(id);
		}

		virtual void clear() {
			if (isThisThreadIsOwner()) {
				//TODO: return result
				ASSERT("Try call broadcaster from different thread");
				return;
			}
			_clear();
		}

		void setThisThreadAsOwner() {
			mThreadId = std::this_thread::get_id();
		}

		[[nodiscard]] bool isThisThreadIsOwner() const {
			return mThreadId == std::this_thread::get_id();
		}

	protected:
		[[nodiscard]] Subscription _subscribe(const std::string& messageId, Handler cb) {
			const Subscription subscription;
			mCallbacks[messageId].push_back({ subscription, cb });
			mSubscriptionToMessage[subscription] = messageId;
			return subscription;
		}

		void _broadcast(const std::string& messageId) {
			if (!mCallbacks.contains(messageId)) {
				ASSERT(std::format("Message id does not exist {}", messageId).c_str());
				return;
			}
			for (auto& cb : mCallbacks.at(messageId)) {
				cb.mCd(messageId);
			}
		}

		void _remove(Subscription id) {
			if (!mSubscriptionToMessage.contains(id)) {
				return;
			}
			for (auto& val : mCallbacks | std::views::values) {
				if (std::erase_if(val, [id](const SubscriptionWrapper& x) { return id.getUniqueId() == x.mSubscription.getUniqueId(); })) {
					break;
				}
			}
			mSubscriptionToMessage.erase(id);
		}

		void _clear() {
			mSubscriptionToMessage.clear();
			mCallbacks.clear();
		}

		std::thread::id mThreadId;

		std::unordered_map<std::string, std::list<SubscriptionWrapper>> mCallbacks;
		std::unordered_map<Subscription, std::string> mSubscriptionToMessage;
	};

	class MessageBroadcasterThreadSafe: public MessageBroadcaster {
	protected:
		std::mutex mMutex;
	public:
		MessageBroadcasterThreadSafe() : MessageBroadcaster() {}

		virtual ~MessageBroadcasterThreadSafe() override = default;

		[[nodiscard]] virtual Subscription subscribe(const std::string& messageId, Handler cb) override {
			const std::lock_guard lock(mMutex);
			return MessageBroadcaster::_subscribe(messageId, cb);
		}

		virtual void broadcast(const std::string& messageId) override {
			const std::lock_guard lock(mMutex);
			MessageBroadcaster::_broadcast(messageId);
		}

		virtual void remove(Subscription id) override {
			const std::lock_guard lock(mMutex);
			MessageBroadcaster::_remove(id);
		}

		virtual void clear() override {
			const std::lock_guard lock(mMutex);
			MessageBroadcaster::_clear();
		}

	protected:
		std::thread::id mThreadId;

		std::unordered_map<std::string, std::list<SubscriptionWrapper>> mCallbacks;
		std::unordered_map<Subscription, std::string> mSubscriptionToMessage;
	};

	class GlobalMessageBroadcaster :
		public MessageBroadcaster,
		public UTILS::SingletonService<GlobalMessageBroadcaster> {};

	class GlobalMessageBroadcasterThreadSafe :
		public MessageBroadcasterThreadSafe,
		public UTILS::SingletonService<GlobalMessageBroadcasterThreadSafe> {};*/
}
