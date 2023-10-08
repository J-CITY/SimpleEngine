#pragma once
#include "../systemManager.h"

namespace IKIGAI::ECS { class Object; }

namespace IKIGAI::ECS {
	class InputSystem : public System {
	public:
		InputSystem();
		void onUpdate(std::chrono::duration<double> dt) override;
	};
}
