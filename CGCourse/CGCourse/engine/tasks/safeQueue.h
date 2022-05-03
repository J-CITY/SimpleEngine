#pragma 

#include <queue>
#include <shared_mutex>

namespace KUMA {
	namespace TASK {
		template <typename T>
		class ThreadSafeQueue {
		public:
			~ThreadSafeQueue() {
				invalidate();
			}

			bool tryPop(T& out) {
				std::lock_guard<std::shared_mutex> lock{m_mutex};
				if (m_queue.empty() || !m_valid) {
					return false;
				}
				out = std::move(m_queue.front());
				m_queue.pop();
				return true;
			}

			bool waitPop(T& out) {
				std::unique_lock<std::shared_mutex> lock{m_mutex};
				m_condition.wait(lock, [this]() {
					return !m_queue.empty() || !m_valid;
				});

				if (!m_valid) {
					return false;
				}
				out = std::move(m_queue.front());
				m_queue.pop();
				return true;
			}

			void push(const T& value) {
				std::lock_guard<std::shared_mutex> lock{m_mutex};
				m_queue.push(value);
				m_condition.notify_one();
			}

			void push(T&& value) {
				std::lock_guard<std::shared_mutex> lock{m_mutex};
				m_queue.push(std::move(value));
				m_condition.notify_one();
			}

			bool empty() const {
				std::shared_lock<std::shared_mutex> lock{m_mutex};
				return m_queue.empty();
			}

			void clear() {
				std::lock_guard<std::shared_mutex> lock{m_mutex};
				while (!m_queue.empty()) {
					m_queue.pop();
				}
				m_condition.notify_all();
			}

			bool isValid() const {
				std::shared_lock<std::shared_mutex> lock{m_mutex};
				return m_valid;
			}

			void invalidate() {
				std::lock_guard<std::shared_mutex> lock{m_mutex};
				m_valid = false;
				m_condition.notify_all();
			}

			size_t size() {
				std::shared_lock<std::shared_mutex> lock{m_mutex};
				return m_queue.size();
			}

			std::queue<T>& getRawData() {
				std::shared_lock<std::shared_mutex> lock{m_mutex};
				return m_queue;
			}

			void setRawData(std::queue<T>&& values) {
				std::lock_guard<std::shared_mutex> lock{m_mutex};
				m_queue = values;
			}

		private:
			std::atomic_bool m_valid{true};
			mutable std::shared_mutex m_mutex;
			std::queue<T> m_queue;
			std::condition_variable_any m_condition;
		};
	}
}