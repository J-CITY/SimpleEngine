#pragma once

#include "ecsModule/systemManager.h"

namespace IKIGAI::ECS {
class Object;
}

namespace IKIGAI::ECS {
class InputSystem : public ECS::System {
public:
	InputSystem();
	void onUpdate(ECS::World& world, ECS::CommandBuffer& cb, std::chrono::duration<double> dt) override;
};
}
