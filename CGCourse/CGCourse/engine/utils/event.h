#pragma once
#include <map>
#include <functional>

#include "idObject.h"

namespace KUMA::EVENT {
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
}
