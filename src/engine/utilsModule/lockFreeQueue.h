#pragma once

#include <atomic>
#include <optional>

namespace IKIGAI::UTILS {
	template <typename T>
	class LockFreeQueue {
		struct ListNode {
			T mValue;
			ListNode* mNext;
		};
	public:
		LockFreeQueue() :
			mFirst(nullptr),
			mList(nullptr),
			mBeforeNext(nullptr),
			mLast(nullptr) {}

		void push(T t) {
			auto node = [&] {
				if (auto node = mList) {
					mList = node->mNext;
					node->mValue = std::move(t);
					return node;
				}
				return new ListNode{ std::move(t), nullptr };
			}();

			auto last = mLast.load(std::memory_order_acquire);

			if (!last) {
				mFirst = new ListNode{ {}, node };
				mBeforeNext.store(mFirst, std::memory_order_relaxed);
				mLast.store(node, std::memory_order_release);
			}
			else {
				last->mNext = node;
				mLast.store(node, std::memory_order_release);
			}

			// cleanup
			auto before_next = mBeforeNext.load(std::memory_order_acquire);
			while (mFirst != before_next) {
				auto node = mFirst;
				mFirst = node->mNext;
				node->mNext = mList;
				mList = node;
			}
		}

		auto pop() -> std::optional<T> {
			auto last = mLast.load(std::memory_order_acquire);
			if (!last) {
				return std::nullopt;
			}

			auto before_next = mBeforeNext.load(std::memory_order_relaxed);
			if (before_next == last) {
				return std::nullopt;
			}

			auto value = std::move(before_next->mNext->mValue);
			mBeforeNext.store(before_next->mNext, std::memory_order_release);
			return value;
		}

	private:
		ListNode* mFirst;
		ListNode* mList;
		std::atomic<ListNode*> mBeforeNext;
		std::atomic<ListNode*> mLast;
	};
}
