#pragma once
#include <chrono>

#include "core.h"
#include "../utils/pointers/objPtr.h"

namespace KUMA {
	namespace CORE_SYSTEM {
		class App {
		public:
			App();
			~App();
			void run();
			[[nodiscard]] bool isRunning() const;
			void preUpdate(std::chrono::duration<double> dt);
			void update(std::chrono::duration<double> dt);
			void postUpdate(std::chrono::duration<double> dt);
			Ref<Core> getCore();
		protected:
			Core core;
		};
	}
}
