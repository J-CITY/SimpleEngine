#pragma once
#include "poolAllocator.h"
#include "profiledAllocator.h"
#include <algorithm>
#include <cstdint>
#include <utility>
#include <vector>

namespace IKIGAI::MEMORY {
class IObjectPool {
public:
	virtual ~IObjectPool() = default;
};

// Typed lifetime management over the shared fixed-slot allocation policy.
// Like the old ObjectPool this pool has a fixed capacity; Spawn returns nullptr
// on exhaustion. Clear and destruction invoke every remaining object's destructor.
template <class T, Category Tag = Category::General> class ObjectPool : public IObjectPool {
public:
	explicit ObjectPool(std::size_t capacity)
		: mPool(sizeof(T), alignof(T), capacity, false) {
		mObjects.reserve(capacity);
	}
	~ObjectPool() override {
		clear();
	}
	ObjectPool(const ObjectPool&) = delete;
	ObjectPool& operator=(const ObjectPool&) = delete;

	template <class... Args> T* Spawn(Args&&... args) {
		void* raw;
		try {
			raw = mPool.allocate(sizeof(T), alignof(T));
		} catch (const std::bad_alloc&) {
			return nullptr;
		}
		T* object;
		try {
			object = ::new (raw) T(std::forward<Args>(args)...);
		} catch (...) {
			mPool.deallocate(raw);
			throw;
		}
		mObjects.push_back(object); // Capacity was reserved in the constructor.
		return object;
	}
	void Destroy(T* ptr) {
		if (!ptr)
			return;
		auto it = std::find(mObjects.begin(), mObjects.end(), ptr);
		if (it == mObjects.end())
			throw std::invalid_argument("Object does not belong to this pool or is already destroyed");
		ptr->~T();
		mPool.deallocate(ptr);
		*it = mObjects.back();
		mObjects.pop_back();
	}
	void clear() noexcept {
		for (T* ptr : mObjects) {
			ptr->~T();
			mPool.deallocate(ptr);
		}
		mObjects.clear();
	}
	AllocatorStats stats() const noexcept {
		return mPool.stats();
	}
	static ObjectPool* Create(std::uint32_t capacity) {
		return new ObjectPool(capacity);
	}
	static void Clear(ObjectPool* pool) {
		if (pool)
			pool->clear();
	}
	static bool Destruct(ObjectPool* pool) {
		delete pool;
		return true;
	}

private:
	OptionalProfiledAllocator<PoolAllocator, Tag> mPool;
	std::vector<T*> mObjects;
};
}
