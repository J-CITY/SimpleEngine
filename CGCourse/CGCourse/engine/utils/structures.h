#pragma once

#include <atomic>
#include <memory>
#include <ranges>
#include <string>
#include <type_traits>
#include <vector>

#include <string>

namespace KUMA
{
	namespace ECS
	{
		class Component;
	}
}

namespace KUMA::UTILS
{
	inline std::vector<std::string> split(const std::string& str, char delimiter) {
		auto to_string = [](auto&& r) {
			const auto data = &*r.begin();
			const auto size = static_cast<std::size_t>(std::ranges::distance(r));
			return std::string{ data, size };
		};
		auto range = str |
			std::ranges::views::split(delimiter) |
			std::ranges::views::transform(to_string);

		return { std::ranges::begin(range), std::ranges::end(range) };
	}

	class ControlBlock {
	public:
		std::atomic_int rc = 0;
		ECS::Component* ptr = nullptr;
	};

	class ControlBlockHandler {
	public:
		virtual ~ControlBlockHandler() = default;
		virtual ControlBlock* getControlBlock() = 0;
	};

	template <typename T>
	concept ComponentT = requires (T t) {
		std::is_base_of_v<ECS::Component, decltype(t)>;
	};

	template<ComponentT T>
	class WeakPtr {
		ControlBlock* mCb = nullptr;
	public:
		WeakPtr() = default;

		WeakPtr(ControlBlockHandler& component) {
			mCb = component.getControlBlock();
			mCb->rc += 1;
		}

		~WeakPtr() {
			__cleanup__();
		}

		WeakPtr(const WeakPtr& obj) {
			this->mCb = obj.mCb;
			if (mCb && mCb->ptr) {
				mCb->rc += 1;
			}
		}

		WeakPtr& operator=(const WeakPtr& obj) {
			__cleanup__();

			this->mCb = obj.mCb;
			if (obj.mCb->ptr) {
				this->mCb->rc += 1;
			}
		}
		
		WeakPtr(WeakPtr&& obj) {
			this->mCb = obj.mCb;
			obj.mCb = nullptr;
		}

		WeakPtr& operator=(WeakPtr&& obj) {
			__cleanup__();
			this->mCb = obj.mCb;
			obj.mCb = nullptr;
		}

		explicit operator bool() const {
			return isAlive();
		}

		[[nodiscard]] bool isAlive() const {
			return mCb && (mCb->ptr != nullptr);
		}

		T* operator->() {
			return static_cast<T>(mCb->ptr);
		}

		T* get() {
			if (!isAlive) {
				throw;
			}
			return static_cast<T>(mCb->ptr);
		}

	private:
		void __cleanup__() {
			if (mCb && mCb->rc > 0) {
				mCb->rc -= 1;
				if (mCb->rc == 0) {
					delete mCb;
				}
			}
		}
	};

	// ChunkList
	class IndexOutOfRange {
	public:
		std::string Msg() {
			return "Index is out of range";
		}
	};

	template<class T, size_t CHUNK_SIZE>
	class ChunkList {
	public:
		ChunkList() = default;
		
		
		~ChunkList() = default;
		
		void push_back(T value) {
			if (mList.empty() || mList.back()->mData.size() == CHUNK_SIZE) {
				mList.push_back(std::make_shared<Node>());
				if (mList.size() > 1) {
					mList[mList.size() - 2]->mNext = mList[mList.size() - 1].get();
				}
			}
			mList.back()->mData.push_back(std::move(value));
			mSize++;
		}

		void pop_back() {
			mList.back()->mData.pop_back();
			if (mList.back()->mData.empty()) {
				mList.pop_back();
				if (!mList.empty()) {
					mList.back()->mNext = nullptr;
				}
			}
			mSize--;
		}

		[[nodiscard]] size_t size() const {
			return mSize;
		}

		[[nodiscard]] bool isEmpty() const {
			return !mSize;
		}

		T& operator[] (const int index) {
			auto chunkId = index / CHUNK_SIZE;
			auto elemId = index % CHUNK_SIZE;
			return mList[chunkId]->mData[elemId];
		}

		const T& operator[] (const int index) const {
			auto chunkId = index / CHUNK_SIZE;
			auto elemId = index % CHUNK_SIZE;
			return mList[chunkId]->mData[elemId];
		}

		T& at(const int index) {
			auto chunkId = index / CHUNK_SIZE;
			auto elemId = index % CHUNK_SIZE;
			if (chunkId >= mList.size()) {
				throw IndexOutOfRange();
			}
			return mList[chunkId]->mData[elemId];
		}

		const T& at(const int index) const {
			auto chunkId = index / CHUNK_SIZE;
			auto elemId = index % CHUNK_SIZE;
			if (chunkId >= mList.size()) {
				throw IndexOutOfRange();
			}
			return mList[chunkId]->mData[elemId];
		}

	private:
		struct Node {
			std::vector<T> mData;
			Node* mNext = nullptr;
			Node() {
				mData.reserve(CHUNK_SIZE);
			}
		};

		//TODO: need unique
		std::vector<std::shared_ptr<Node>> mList;
		size_t mSize = 0;

	public:
		class Iterator {
		public:
			Iterator() noexcept : mNode(nullptr) { }
			Iterator(Node* node) noexcept: mNode(node) { }

			Iterator& operator++() {
				if (mNode) {
					mPosInChunk++;
					if (mPosInChunk == CHUNK_SIZE) {
						mNode = mNode->mNext;
						mPosInChunk = 0;
					}
					if (mPosInChunk >= mNode->mData.size()) {
						mNode = nullptr;
						mPosInChunk = 0;
					}
				}
				return *this;
			}
			
			Iterator operator++(int) {
				Iterator iterator = *this;
				++* this;
				return iterator;
			}

			bool operator!=(const Iterator& iterator) {
				return mNode != iterator.mNode
					|| mPosInChunk != iterator.mPosInChunk;
			}

			T& operator*() {
				return mNode->mData[mPosInChunk];
			}

			const T& operator*() const {
				return mNode->mData[mPosInChunk];
			}

			T* operator->() {
				return &mNode->mData[mPosInChunk];
			}

		private:
			int mPosInChunk = 0;
			Node* mNode = nullptr;
		};

		Iterator begin() {
			if (mList.empty()) {
				return Iterator(nullptr);
			}
			return Iterator(mList[0].get());
		}
		
		Iterator end() {
			return Iterator(nullptr);
		}

		Iterator begin() const {
			if (mList.empty()) {
				return Iterator(nullptr);
			}
			return Iterator(mList[0].get());
		}

		Iterator end() const {
			return Iterator(nullptr);
		}
	};

	template <typename T>
	class LockFreeQueue {
	public:
		using value_type = T;

		struct list_node {
			value_type value;
			list_node* next;
		};

		LockFreeQueue() :
			_first(nullptr),
			_free_list(nullptr),
			_before_next(nullptr),
			_last(nullptr) {}

		void push(T t) {
			auto node = [&] {
				if (auto node = _free_list) {
					_free_list = node->next;
					node->value = std::move(t);
					return node;
				}
				else {
					return new list_node{ std::move(t), nullptr };
				}
			}();

			auto last = _last.load(std::memory_order_acquire);

			if (!last) {
				_first = new list_node{ {}, node };
				_before_next.store(_first, std::memory_order_relaxed);
				_last.store(node, std::memory_order_release);
			}
			else {
				last->next = node;
				_last.store(node, std::memory_order_release);
			}

			// cleanup
			auto before_next = _before_next.load(std::memory_order_acquire);
			while (_first != before_next) {
				auto node = _first;
				_first = node->next;
				node->next = _free_list;
				_free_list = node;
			}
		}

		auto pop() -> std::optional<T> {
			auto last = _last.load(std::memory_order_acquire);

			if (!last) {
				return std::nullopt;
			}

			auto before_next = _before_next.load(std::memory_order_relaxed);

			if (before_next == last) {
				return std::nullopt;
			}

			auto value = std::move(before_next->next->value);

			_before_next.store(before_next->next, std::memory_order_release);

			return value;
		}

	private:
		list_node* _first;
		list_node* _free_list;
		std::atomic<list_node*> _before_next;
		std::atomic<list_node*> _last;
	};
}
