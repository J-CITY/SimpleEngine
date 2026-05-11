#pragma once

#include <cstdio>
#include <functional>
#include <memory>

namespace IKIGAI::UTILS {
	class IAllocator {
	public:
		virtual ~IAllocator() = default;
		virtual void* allocate(size_t size) = 0;
		virtual void deallocate(void* ptr) = 0;
	};

	class Memory {
	public:
		static void* Allocate(const std::size_t size);
		static void* Reallocate(void* const ptr, const std::size_t size);
		static void Deallocate(void* const ptr);
	};

	template <typename T, typename... Args>
	std::shared_ptr<T> AllocateResource(UTILS::IAllocator* allocator, std::function<void(T*)> customDeleter, Args&&... args) {
		void* raw = allocator ? allocator->allocate(sizeof(T)) : ::operator new(sizeof(T));
		auto* obj = new(raw) T(std::forward<Args>(args)...);
		std::function<void(T*)> finalDeleter = [allocator, customDeleter](T* p) {
			if (customDeleter) {
				customDeleter(p);
			}
			p->~T();
			if (allocator) {
				allocator->deallocate(p);
			} else {
				::operator delete(p);
			}
		};
		return std::shared_ptr<T>(obj, std::move(finalDeleter));
	}
}
