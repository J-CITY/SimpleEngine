#pragma once
#include <any>
#include <unordered_map>

namespace KUMA {
	namespace RESOURCES	{
		class ServiceManager {
		public:
			template<typename T>
			static void Set(T& p_service) {
				SERVICES[typeid(T).hash_code()] = std::any(&p_service);
			}

			template<typename T>
			static T& Get() {
				return *std::any_cast<T*>(SERVICES[typeid(T).hash_code()]);
			}

		private:
			static std::unordered_map<size_t, std::any> SERVICES;
		};
	}
	
}
