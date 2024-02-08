#pragma once
#include <atomic>

namespace IKIGAI::ECS {
	class Component;
	class ScriptComponent;
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

	template <typename Ptr, bool = std::is_class<Ptr>::value>
		struct has_smart_pointer_ops {
			using false_test = char;
			template <typename T> struct true_test { false_test dummy[2]; };

			template <typename T> static false_test test_op_star(...);
			template <typename T>
			static true_test<decltype(*std::declval<T const&>())>
				test_op_star(T*);

			template <typename T> static false_test test_op_arrow(...);
			template <typename T>
			static true_test<decltype(std::declval<T const&>().operator->())>
				test_op_arrow(T*);

			template <typename T> static false_test test_get(...);
			template <typename T>
			static true_test<decltype(std::declval<T const&>().get())>
				test_get(T*);

			static constexpr bool value =
				!std::is_same<decltype(test_get<Ptr>(0)), false_test>::value &&
				!std::is_same<
				decltype(test_op_arrow<Ptr>(0)), false_test>::value &&
				!std::is_same<
				decltype(test_op_star<Ptr>(0)), false_test>::value;
		};

		/// Non-class types can't be smart pointers
		template <typename Ptr>
		struct has_smart_pointer_ops<Ptr, false> : std::false_type {};

		/// Ensure that the smart pointer operations give consistent return
		/// types
		template <typename Ptr>
		struct smart_pointer_ops_consistent
			: std::integral_constant<
			bool,
			std::is_pointer<decltype(
				std::declval<Ptr const&>().get())>::value&&
			std::is_reference<decltype(
				*std::declval<Ptr const&>())>::value&&
			std::is_pointer<decltype(
				std::declval<Ptr const&>().operator->())>::value&&
			std::is_same<
			decltype(std::declval<Ptr const&>().get()),
			decltype(std::declval<Ptr const&>().
				operator->())>::value&&
			std::is_same<
			decltype(*std::declval<Ptr const&>().get()),
			decltype(*std::declval<Ptr const&>())>::value> {
		};

		/// Assume Ptr is a smart pointer if it has the relevant ops and they
		/// are consistent
		template <typename Ptr, bool = has_smart_pointer_ops<Ptr>::value>
		struct is_smart_pointer
			: std::integral_constant<
			bool, smart_pointer_ops_consistent<Ptr>::value> {
		};

		/// If Ptr doesn't have the relevant ops then it can't be a smart
		/// pointer
		template <typename Ptr>
		struct is_smart_pointer<Ptr, false> : std::false_type {};

		/// Check if Ptr is a smart pointer that holds a pointer convertible to
		/// T*
		template <typename Ptr, typename T, bool = is_smart_pointer<Ptr>::value>
		struct is_convertible_smart_pointer
			: std::integral_constant<
			bool, std::is_convertible<
			decltype(std::declval<Ptr const&>().get()),
			T*>::value> {
		};

		/// If Ptr isn't a smart pointer then we don't want it
		template <typename Ptr, typename T>
		struct is_convertible_smart_pointer<Ptr, T, false> : std::false_type {};


	template <typename T>
	concept ComponentT = requires (T t) {
		std::is_base_of_v<ECS::Component, decltype(t)>;
	};

	template<ComponentT T>
	class WeakPtr {
		ControlBlock* m_cb = nullptr;
	public:
		using element_type = std::remove_extent_t<T>;
		WeakPtr() = default;

		constexpr WeakPtr(nullptr_t) noexcept {}

		explicit WeakPtr(ControlBlockHandler& component) {
			m_cb = component.getControlBlock();
			m_cb->m_rc += 1;
		}

		explicit WeakPtr(T* component) {
			m_cb = component->getControlBlock();
			if (m_cb && m_cb->m_ptr) {
				m_cb->m_rc += 1;
			}
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

		template<class T1>
		explicit WeakPtr(const WeakPtr& obj, T1* component) {
			m_cb = component->getControlBlock();
			m_cb->m_rc += 1;
		}

		template<class T1>
		explicit WeakPtr(WeakPtr&& obj, T1* component) {
			m_cb = component->getControlBlock();
			m_cb->m_rc += 1;
			obj = WeakPtr();
		}

		//template<class T1>
		//explicit WeakPtr(const WeakPtr<T1>& obj) {
		//	m_cb = obj->getControlBlock();
		//	m_cb->m_rc += 1;
		//}
		//
		//template<class T1>
		//explicit WeakPtr(WeakPtr<T1>&& obj) {
		//	m_cb = obj->getControlBlock();
		//	m_cb->m_rc += 1;
		//	obj = WeakPtr();
		//}
		template<typename U, typename = std::enable_if_t<std::is_convertible<U*, T*>::value>>
		constexpr WeakPtr(U* ptr_) noexcept {
			m_cb = ptr_->getControlBlock();
			if (m_cb && m_cb->m_ptr) {
				m_cb->m_rc += 1;
			}
		}

		template<typename Ptr, typename = std::enable_if_t<std::is_convertible<Ptr, T>::value>>
		constexpr WeakPtr(const WeakPtr<Ptr> & other) noexcept {
			m_cb = other->getControlBlock();
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

		bool operator==(const WeakPtr& c) {
			if (m_cb->m_ptr == c.m_cb->m_ptr) {
				return true;
			}
			return false;
		}

		bool operator!=(const WeakPtr& c) {
			if (m_cb->m_ptr != c.m_cb->m_ptr) {
				return true;
			}
			return false;
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

		T* operator->() const {
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