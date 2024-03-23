#pragma once

#include <mutex>

#include <resourceModule/serviceManager.h>

namespace IKIGAI::UTILS {
	template <typename T>
	class Singleton {
		explicit Singleton() = default;
	public:
		inline static T& GetInstance() {
			static T instance{};
			return instance;
		}

		Singleton(const Singleton&) = delete;
		Singleton& operator=(const Singleton&) = delete;
		Singleton(Singleton&&) = default;
		Singleton& operator=(Singleton&&) = default;
		virtual ~Singleton() = default;
	};

	template <typename T>
	class SingletonService {
	protected:
		explicit SingletonService() = default;
		inline static std::once_flag mFlag;
	public:
		inline static T& GetInstance() {
			if (!RESOURCES::ServiceManager::Check<T>()) {
				std::call_once(mFlag, []() {
					RESOURCES::ServiceManager::Set<T>(new T());
				});
			}
			return RESOURCES::ServiceManager::Get<T>();
		}
		SingletonService(const SingletonService&) = delete;
		SingletonService& operator=(const SingletonService&) = delete;
		SingletonService(SingletonService&&) = default;
		SingletonService& operator=(SingletonService&&) = default;
		virtual ~SingletonService() {
			RESOURCES::ServiceManager::Remove<T>();
		}
	};


}


#define FRIEND_SINGLETON_SERVICE(S) friend IKIGAI::UTILS::SingletonService<S>;
#define FRIEND_SINGLETON(S) friend IKIGAI::UTILS::Singleton<S>;
