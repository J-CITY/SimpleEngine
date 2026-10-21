#pragma once

#include "ecsModule/systemManager.h"

namespace IKIGAI::ECS {
class Object;
}

namespace IKIGAI::ECS {
class AudioSystem : public ECS::System {
public:
	AudioSystem();

	void onLateUpdate(ECS::World& world, ECS::CommandBuffer& cb, std::chrono::duration<double> dt) override;
};
}
