#pragma once
#include "exeptions.h"
#include <memory>
#include <stdexcept>
#include <vector>

namespace IKIGAI::UTILS {
template <class T, template <class, class> class CONT = std::vector, size_t CHUNK_SIZE = 100>
class ChunkList {
public:
  ChunkList() = default;

  ChunkList(const ChunkList &other) {
    if (&other == this) {
      return;
    }
    mList.clear();
    for (auto &e : other.mList) {
      mList.push_back(std::make_unique<Node>());
      mList.back()->mData = e->mData;
      if (mList.size() > 1) {
        mList[mList.size() - 2]->mNext = mList.back().get();
      }
    }
    m_Size = other.m_Size;
  }

  ChunkList &operator=(const ChunkList &other) {
    if (&other == this) {
      return *this;
    }
    mList.clear();
    for (auto &e : other.mList) {
      mList.push_back(std::make_unique<Node>());
      mList.back()->mData = e->mData;
      if (mList.size() > 1) {
        mList[mList.size() - 2]->mNext = mList.back().get();
      }
    }
    m_Size = other.m_Size;
    return *this;
  }

  void push_back(T &value) {
    if (mList.empty() || mList.back()->mData.size() == CHUNK_SIZE) {
      mList.push_back(std::make_unique<Node>());
      if (mList.size() > 1) {
        mList[mList.size() - 2]->mNext = mList[mList.size() - 1].get();
      }
    }
    mList.back()->mData.push_back(value);
    m_Size++;
  }

  void push_back(T &&value) {
    if (mList.empty() || mList.back()->mData.size() == CHUNK_SIZE) {
      mList.push_back(std::make_unique<Node>());
      if (mList.size() > 1) {
        mList[mList.size() - 2]->mNext = mList[mList.size() - 1].get();
      }
    }
    mList.back()->mData.push_back(std::move(value));
    m_Size++;
  }

  void pop_back() {
    mList.back()->mData.pop_back();
    if (mList.back()->mData.empty()) {
      mList.pop_back();
      if (!mList.empty()) {
        mList.back()->mNext = nullptr;
      }
    }
    m_Size--;
  }

  [[nodiscard]] size_t size() const { return m_Size; }

  [[nodiscard]] bool isEmpty() const { return !m_Size; }

  T &operator[](const int index) {
    auto chunkId = index / CHUNK_SIZE;
    auto elemId = index % CHUNK_SIZE;
    return mList[chunkId]->mData[elemId];
  }

  const T &operator[](const int index) const {
    auto chunkId = index / CHUNK_SIZE;
    auto elemId = index % CHUNK_SIZE;
    return mList[chunkId]->mData[elemId];
  }

  void insert(const int index, T &&data) {
    auto chunkId = index / CHUNK_SIZE;
    auto elemId = index % CHUNK_SIZE;
    mList[chunkId]->mData.insert(mList[chunkId]->mData.begin() + elemId,
                                 std::move(data));
  }

  T &&moveFrom(const int index) {
    auto chunkId = index / CHUNK_SIZE;
    auto elemId = index % CHUNK_SIZE;
    return std::move(mList[chunkId]->mData[elemId]);
  }

  void moveTo(const int index, T &&data) {
    auto chunkId = index / CHUNK_SIZE;
    auto elemId = index % CHUNK_SIZE;
    //*(mList[chunkId]->mData.data() + elemId*sizeof(T)) = std::move(data);
    mList[chunkId]->mData[elemId] = std::move(data);
  }

  T &at(const int index) {
    auto chunkId = index / CHUNK_SIZE;
    auto elemId = index % CHUNK_SIZE;
    if (chunkId >= mList.size()) {
      throw EXEPTIONS::IndexOutOfRange();
    }
    return mList[chunkId]->mData[elemId];
  }

  const T &at(const int index) const {
    auto chunkId = index / CHUNK_SIZE;
    auto elemId = index % CHUNK_SIZE;
    if (chunkId >= mList.size()) {
      throw EXEPTIONS::IndexOutOfRange();
    }
    return mList[chunkId]->mData[elemId];
  }

private:
  struct Node {
    CONT<T, std::allocator<T>> mData;
    Node *mNext = nullptr;
    Node() { mData.reserve(CHUNK_SIZE); }
  };

  CONT<std::unique_ptr<Node>, std::allocator<std::unique_ptr<Node>>> mList;
  size_t m_Size = 0;

public:
  class Iterator {
  public:
    Iterator() noexcept : m_Node(nullptr) {}
    Iterator(Node *node) noexcept : m_Node(node) {}

    Iterator &operator++() {
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
      ++*this;
      return iterator;
    }

    bool operator!=(const Iterator &iterator) {
      return m_Node != iterator.m_Node || m_PosInChunk != iterator.m_PosInChunk;
    }

    T &operator*() { return m_Node->mData[m_PosInChunk]; }

    const T &operator*() const { return m_Node->mData[m_PosInChunk]; }

    T *operator->() { return &m_Node->mData[m_PosInChunk]; }

  private:
    int m_PosInChunk = 0;
    Node *m_Node = nullptr;
  };

  Iterator begin() {
    if (mList.empty()) {
      return Iterator(nullptr);
    }
    return Iterator(mList[0].get());
  }

  Iterator end() { return Iterator(nullptr); }

  Iterator begin() const {
    if (mList.empty()) {
      return Iterator(nullptr);
    }
    return Iterator(mList[0].get());
  }

  Iterator end() const { return Iterator(nullptr); }
};
} // namespace IKIGAI::UTILS
