#pragma once
#include <memory>
#include <string>
#include <vector>


namespace IKIGAI::UTILS {
	class IndexOutOfRange final: public std::exception {
	public:
		IndexOutOfRange() : std::exception("Index is out of range") {}
	};

	template<class T, size_t CHUNK_SIZE>
	class ChunkList {
	public:
		ChunkList() = default;
		
		void push_back(T& value) {
			if (m_List.empty() || m_List.back()->mData.size() == CHUNK_SIZE) {
				m_List.push_back(std::make_shared<Node>());
				if (m_List.size() > 1) {
					m_List[m_List.size() - 2]->mNext = m_List[m_List.size() - 1].get();
				}
			}
			m_List.back()->mData.push_back(std::move(value));
			m_Size++;
		}

		void pop_back() {
			m_List.back()->mData.pop_back();
			if (m_List.back()->mData.empty()) {
				m_List.pop_back();
				if (!m_List.empty()) {
					m_List.back()->mNext = nullptr;
				}
			}
			m_Size--;
		}

		[[nodiscard]] size_t size() const {
			return m_Size;
		}

		[[nodiscard]] bool isEmpty() const {
			return !m_Size;
		}

		T& operator[] (const int index) {
			auto chunkId = index / CHUNK_SIZE;
			auto elemId = index % CHUNK_SIZE;
			return m_List[chunkId]->mData[elemId];
		}

		const T& operator[] (const int index) const {
			auto chunkId = index / CHUNK_SIZE;
			auto elemId = index % CHUNK_SIZE;
			return m_List[chunkId]->mData[elemId];
		}

		T& at(const int index) {
			auto chunkId = index / CHUNK_SIZE;
			auto elemId = index % CHUNK_SIZE;
			if (chunkId >= m_List.size()) {
				throw IndexOutOfRange();
			}
			return m_List[chunkId]->mData[elemId];
		}

		const T& at(const int index) const {
			auto chunkId = index / CHUNK_SIZE;
			auto elemId = index % CHUNK_SIZE;
			if (chunkId >= m_List.size()) {
				throw IndexOutOfRange();
			}
			return m_List[chunkId]->mData[elemId];
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
		std::vector<std::shared_ptr<Node>> m_List;
		size_t m_Size = 0;

	public:
		class Iterator {
		public:
			Iterator() noexcept : m_Node(nullptr) { }
			Iterator(Node* node) noexcept : m_Node(node) { }

			Iterator& operator++() {
				if (m_Node) {
					m_PosInChunk++;
					if (m_PosInChunk == CHUNK_SIZE) {
						m_Node = m_Node->mNext;
						m_PosInChunk = 0;
					}
					if (m_PosInChunk >= m_Node->mData.size()) {
						m_Node = nullptr;
						m_PosInChunk = 0;
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
				return m_Node != iterator.m_Node
					|| m_PosInChunk != iterator.m_PosInChunk;
			}

			T& operator*() {
				return m_Node->mData[m_PosInChunk];
			}

			const T& operator*() const {
				return m_Node->mData[m_PosInChunk];
			}

			T* operator->() {
				return &m_Node->mData[m_PosInChunk];
			}

		private:
			int m_PosInChunk = 0;
			Node* m_Node = nullptr;
		};

		Iterator begin() {
			if (m_List.empty()) {
				return Iterator(nullptr);
			}
			return Iterator(m_List[0].get());
		}

		Iterator end() {
			return Iterator(nullptr);
		}

		Iterator begin() const {
			if (m_List.empty()) {
				return Iterator(nullptr);
			}
			return Iterator(m_List[0].get());
		}

		Iterator end() const {
			return Iterator(nullptr);
		}
	};
}
