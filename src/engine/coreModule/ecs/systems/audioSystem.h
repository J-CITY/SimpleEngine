#pragma once
#include "../systemManager.h"
#include "ecsModule/systemManager.h"

namespace IKIGAI::ECS { class Object; }

namespace IKIGAI::ECS {
	class AudioSystem : public ECS2::System {
	public:
		AudioSystem();

		void onLateUpdate(ECS2::World& world, ECS2::CommandBuffer& cb, std::chrono::duration<double> dt) override;
	};
}
