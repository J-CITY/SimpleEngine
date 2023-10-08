#pragma once
#include <atomic>

namespace IKIGAI::ECS {
	class Component;
}

namespace IKIGAI::UTILS {

	class ControlBlock {
	public:
		std::atomic_int m_rc = 0;
		ECS::Component* m_ptr = nullptr;
	};

	class ControlBlockHandler {
		UTILS::ControlBlock* mCb = nullptr;
	public:
		ControlBlockHandler(ECS::Component* ptr);
		ControlBlockHandler(const ControlBlockHandler& obj) {
			mCb = obj.mCb;
			mCb->m_rc++;
		};
		ControlBlockHandler& operator=(const ControlBlockHandler& obj) {
			mCb = obj.mCb;
			mCb->m_rc++;
			return *this;
		};
		ControlBlockHandler(ControlBlockHandler&& obj) = default;
		ControlBlockHandler& operator=(ControlBlockHandler&& obj) = default;

		virtual ~ControlBlockHandler();;
		virtual ControlBlock* getControlBlock() {
			return mCb;
		};
	};

	template <typename T>
	concept ComponentT = requires (T t) {
		std::is_base_of_v<ECS::Component, decltype(t)>;
	};

	template<ComponentT T>
	class WeakPtr {
		ControlBlock* m_cb = nullptr;
	public:
		WeakPtr() = default;

		constexpr WeakPtr(nullptr_t) noexcept {}

		explicit WeakPtr(ControlBlockHandler& component) {
			m_cb = component.getControlBlock();
			m_cb->m_rc += 1;
		}

		explicit WeakPtr(T* component) {
			m_cb = component->getControlBlock();
			m_cb->m_rc += 1;
		}

		~WeakPtr() {
			cleanup();
		}

		WeakPtr(const WeakPtr& obj) {
			m_cb = obj.m_cb;
			if (m_cb && m_cb->m_ptr) {
				m_cb->m_rc += 1;
			}
		}

		WeakPtr& operator=(const WeakPtr& obj) {
			if (this == &obj) {
				return *this;
			}
			cleanup();
			m_cb = obj.m_cb;
			if (obj.m_cb->m_ptr) {
				m_cb->m_rc += 1;
			}
			return *this;
		}

		WeakPtr& operator=(nullptr_t) noexcept {
			cleanup();
			m_cb = nullptr;
			return *this;
		}

		WeakPtr(WeakPtr&& obj) noexcept {
			m_cb = obj.m_cb;
			obj.m_cb = nullptr;
		}

		WeakPtr& operator=(WeakPtr&& obj) noexcept {
			cleanup();
			m_cb = obj.m_cb;
			obj.m_cb = nullptr;
			return *this;
		}

		explicit operator bool() const {
			return isAlive();
		}

		[[nodiscard]] bool isAlive() const {
			return m_cb && (m_cb->m_ptr != nullptr);
		}

		T& operator*() const {
			return *static_cast<T*>(m_cb->m_ptr);
		}

		T* operator->() {
			return static_cast<T*>(m_cb->m_ptr);
		}

		T* get() {
			if (!isAlive()) {
				throw;
			}
			return static_cast<T*>(m_cb->m_ptr);
		}


	private:
		void cleanup() const {
			if (m_cb && m_cb->m_rc > 0) {
				m_cb->m_rc -= 1;
				if (m_cb->m_rc == 0) {
					delete m_cb;
				}
			}
		}
	};
}


namespace std {
	template <typename T> struct hash<IKIGAI::UTILS::WeakPtr<T>> {
		constexpr size_t operator()(IKIGAI::UTILS::WeakPtr<T> const& p) const
			noexcept {
			return hash<T*>()(p.get());
		}
	};

	template <typename To, typename From>
	typename std::enable_if<
		sizeof(decltype(static_cast<To*>(std::declval<From*>()))) != 0,
		IKIGAI::UTILS::WeakPtr<To>>::type
		static_pointer_cast(IKIGAI::UTILS::WeakPtr<From> p) {
		return IKIGAI::UTILS::WeakPtr(static_cast<To*>(p.get()));
	}
	
	template <typename To, typename From>
	typename std::enable_if<
		sizeof(decltype(dynamic_cast<To*>(std::declval<From*>()))) != 0,
		IKIGAI::UTILS::WeakPtr<To>>::type
		dynamic_pointer_cast(IKIGAI::UTILS::WeakPtr<From> p) {
		return IKIGAI::UTILS::WeakPtr(dynamic_cast<To*>(p.get()));
	}

}