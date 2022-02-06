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
			void update(float dt);
			void postUpdate();
			Core& getCore() { return core; }
		protected:
			Core core;
			float delta = 0.0f;
		public:
			RENDER::GameRenderer renderer;
			
			EVENT::Event<std::shared_ptr<ECS::Component>> componentAddedEvent;
			EVENT::Event<std::shared_ptr<ECS::Component>> componentRemovedEvent;
		};
	}
}
