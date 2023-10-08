#pragma once

#include <atomic>
#include <optional>

namespace IKIGAI::UTILS {
	template <typename T>
	class LockFreeQueue {
		struct ListNode {
			T m_value;
			ListNode* m_next;
		};
	public:
		LockFreeQueue() :
			m_first(nullptr),
			m_list(nullptr),
			m_beforeNext(nullptr),
			m_last(nullptr) {}

		void push(T t) {
			auto node = [&] {
				if (auto node = m_list) {
					m_list = node->m_next;
					node->m_value = std::move(t);
					return node;
				}
				return new ListNode{ std::move(t), nullptr };
			}();

			auto last = m_last.load(std::memory_order_acquire);

			if (!last) {
				m_first = new ListNode{ {}, node };
				m_beforeNext.store(m_first, std::memory_order_relaxed);
				m_last.store(node, std::memory_order_release);
			}
			else {
				last->m_next = node;
				m_last.store(node, std::memory_order_release);
			}

			// cleanup
			auto before_next = m_beforeNext.load(std::memory_order_acquire);
			while (m_first != before_next) {
				auto node = m_first;
				m_first = node->m_next;
				node->m_next = m_list;
				m_list = node;
			}
		}

		auto pop() -> std::optional<T> {
			auto last = m_last.load(std::memory_order_acquire);
			if (!last) {
				return std::nullopt;
			}

			auto before_next = m_beforeNext.load(std::memory_order_relaxed);
			if (before_next == last) {
				return std::nullopt;
			}

			auto value = std::move(before_next->m_next->m_value);
			m_beforeNext.store(before_next->m_next, std::memory_order_release);
			return value;
		}

	private:
		ListNode* m_first;
		ListNode* m_list;
		std::atomic<ListNode*> m_beforeNext;
		std::atomic<ListNode*> m_last;
	};
}
