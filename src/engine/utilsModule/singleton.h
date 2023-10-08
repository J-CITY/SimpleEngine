#pragma once

#include <mutex>

#include <coreModule/resourceManager/serviceManager.h>

namespace IKIGAI::UTILS {
	template <typename T>
	class Singleton {
		explicit Singleton() {};
	public:
		inline static T& GetInstance() {
			static T instance{};
			return instance;
		}

		Singleton(const Singleton&) = delete;
		Singleton& operator=(const Singleton&) = delete;
		virtual ~Singleton() {}
	};

	template <typename T>
	class SingletonService {
	protected:
		explicit SingletonService() {};
		inline static std::once_flag m_flag;
	public:
		inline static T& GetInstance() {
			if (!RESOURCES::ServiceManager::Check<T>()) {
				std::call_once(m_flag, []() {
					RESOURCES::ServiceManager::Set<T>(new T());
				});
			}
			return RESOURCES::ServiceManager::Get<T>();
		}
		SingletonService(const SingletonService&) = delete;
		SingletonService& operator=(const SingletonService&) = delete;
	
		virtual ~SingletonService() {
			RESOURCES::ServiceManager::Remove<T>();
		}
	};


}


#define FRIEND_SINGLETON_SERVICE(S) friend IKIGAI::UTILS::SingletonService<S>;
#define FRIEND_SINGLETON(S) friend IKIGAI::UTILS::Singleton<S>;
