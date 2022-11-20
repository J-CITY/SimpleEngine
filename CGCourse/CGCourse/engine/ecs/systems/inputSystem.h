#pragma once
#include "../systemManager.h"

namespace KUMA::ECS { class Object; }

namespace KUMA::ECS {
	class InputSystem : public System {
	public:
		InputSystem();
		void onUpdate(std::chrono::duration<double> dt) override;
	};
}
