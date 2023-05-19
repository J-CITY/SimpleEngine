#pragma once
#include "../systemManager.h"

namespace KUMA::ECS { class Object; }

namespace KUMA::ECS {
	class AudioSystem : public System {
	public:
		AudioSystem();

		void onLateUpdate(std::chrono::duration<double> dt) override;
	};
}
