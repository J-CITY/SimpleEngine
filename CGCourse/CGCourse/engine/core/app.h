#pragma once
#include "core.h"

namespace KUMA {
	namespace ECS {
		class Component;
	}

	namespace CORE_SYSTEM {
		class App {
		public:
			App();
			~App();
			void run();
			bool isRunning() const;
			void preUpdate();
			void update(std::chrono::duration<double> dt);
			void postUpdate();
			Core& getCore() { return core; }
		protected:
			Core core;
		};
	}
}
