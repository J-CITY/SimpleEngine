#pragma once
#include "freeListAllocator.h"
#include "poolAllocator.h"
#include "profiledAllocator.h"
#include <functional>
#include <memory>
#include <mutex>
#include <type_traits>
#include <utility>

namespace IKIGAI::MEMORY {
template <class Allocator> class SynchronizedAllocator final : public IAllocator {
public:
	template <class... Args>
	explicit SynchronizedAllocator(Args&&... args)
		: mAllocator(std::forward<Args>(args)...) { }
	void* allocate(std::size_t bytes) override {
		std::lock_guard lock(mMutex);
		return mAllocator.allocate(bytes);
	}
	void* allocate(std::size_t bytes, std::size_t alignment) override {
		std::lock_guard lock(mMutex);
		return static_cast<IAllocator&>(mAllocator).allocate(bytes, alignment);
	}
	void deallocate(void* ptr) override {
		std::lock_guard lock(mMutex);
		mAllocator.deallocate(ptr);
	}
	AllocatorStats stats() const {
		std::lock_guard lock(mMutex);
		return mAllocator.stats();
	}
	bool tracksAllocations() const noexcept override {
		return mAllocator.tracksAllocations();
	}

private:
	mutable std::mutex mMutex;
	Allocator mAllocator;
};

template <class T, Category Tag> std::shared_ptr<IAllocator> ObjectAllocator() {
	using Policy = SynchronizedAllocator<OptionalProfiledAllocator<PoolAllocator, Tag>>;
	static auto allocator = std::make_shared<Policy>(sizeof(T), alignof(T), 128);
	return allocator;
}
template <Category Tag> std::shared_ptr<IAllocator> BufferAllocator() {
	using Policy = SynchronizedAllocator<OptionalProfiledAllocator<FreeListAllocator, Tag>>;
	static auto allocator = std::make_shared<Policy>();
	return allocator;
}

// Stateful STL allocator: copies retain the original allocation domain. Rebinds
// allocate their actual T, including shared_ptr's implementation control block.
template <class T, Category Tag = Category::General> class StlAllocator {
	template <class, Category> friend class StlAllocator;

public:
	using value_type = T;
	using is_always_equal = std::false_type;
	using propagate_on_container_move_assignment = std::true_type;
	using propagate_on_container_copy_assignment = std::false_type;
	using propagate_on_container_swap = std::true_type;
	template <class U> struct rebind {
		using other = StlAllocator<U, Tag>;
	};
	StlAllocator()
		: mBuffer(BufferAllocator<Tag>()) { }
	explicit StlAllocator(std::shared_ptr<IAllocator> domain)
		: mBuffer(std::move(domain)) {
		if (!mBuffer)
			throw std::invalid_argument("Null allocation domain");
	}
	StlAllocator select_on_container_copy_construction() const noexcept {
		return *this;
	}
	const std::shared_ptr<IAllocator>& domain() const noexcept {
		return mBuffer;
	}
	StlAllocator(const StlAllocator&) noexcept = default;
	StlAllocator& operator=(const StlAllocator&) noexcept = default;
	// A moved-from container may still allocate/deallocate its debug proxy.
	// Keep both allocator instances bound to the same valid allocation domain.
	StlAllocator(StlAllocator&& other) noexcept
		: mBuffer(other.mBuffer) { }
	StlAllocator& operator=(StlAllocator&& other) noexcept {
		mBuffer = other.mBuffer;
		return *this;
	}
	template <class U>
	StlAllocator(const StlAllocator<U, Tag>& other) noexcept
		: mBuffer(other.mBuffer) { }
	T* allocate(std::size_t n) {
		void* ptr = mBuffer->allocate(Detail::CheckedMultiply(n, sizeof(T)), alignof(T));
		if (!ptr)
			throw std::bad_alloc();
		return static_cast<T*>(ptr);
	}
	void deallocate(T* ptr, std::size_t) {
		mBuffer->deallocate(ptr);
	}
	template <class U> bool operator==(const StlAllocator<U, Tag>& other) const noexcept {
		return mBuffer == other.mBuffer;
	}
	template <class U> bool operator!=(const StlAllocator<U, Tag>& other) const noexcept {
		return !(*this == other);
	}

private:
	std::shared_ptr<IAllocator> mBuffer;
};

template <class T, Category Tag = Category::General, class Deleter, class... Args>
std::shared_ptr<T> MakeSharedWithDeleter(IAllocator* allocator, Deleter customDeleter, Args&&... args) {
	// Initialize the control-block allocator before owning raw object memory.
	StlAllocator<std::byte, Tag> controlAllocator;
	std::function<void(T*)> callback(std::move(customDeleter));
	auto owner = allocator ? std::shared_ptr<IAllocator> { } : ObjectAllocator<T, Tag>();
	IAllocator* actual = allocator ? allocator : owner.get();
	void* raw = actual->allocate(sizeof(T), alignof(T));
	if (!raw)
		throw std::bad_alloc();
	T* object;
	try {
		object = ::new (raw) T(std::forward<Args>(args)...);
	} catch (...) {
		actual->deallocate(raw);
		throw;
	}
	auto finalDeleter = [actual, owner = std::move(owner), callback = std::move(callback)](T* ptr) {
		// Custom callbacks keep the existing engine contract: notification only,
		// no destruction/free. Destruction and allocation release still happen
		// if a notification unexpectedly throws.
		struct Cleanup {
			IAllocator* allocator;
			T* ptr;
			~Cleanup() {
				ptr->~T();
				allocator->deallocate(ptr);
			}
		} cleanup { actual, ptr };
		try {
			if (callback)
				callback(ptr);
		} catch (...) { }
	};
	return std::shared_ptr<T>(object, std::move(finalDeleter), controlAllocator);
}

template <class T, Category Tag = Category::General, class... Args> std::shared_ptr<T> MakeShared(Args&&... args) {
	return MakeSharedWithDeleter<T, Tag>(nullptr, std::function<void(T*)> { }, std::forward<Args>(args)...);
}
}
