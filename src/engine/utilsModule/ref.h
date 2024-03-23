#pragma once
#include <functional>

namespace IKIGAI::UTILS {
	template<typename T>
	class Ref : public std::reference_wrapper<T> {
	public:
		using base = std::reference_wrapper<T>;
		using base::base;

		T* operator->() {
			return getPtr();
		}

		T* getPtr() {
			return &(this->get());
		}
	};
}
