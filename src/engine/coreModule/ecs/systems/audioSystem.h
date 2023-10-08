#pragma once
#include "../systemManager.h"

namespace IKIGAI::ECS { class Object; }

namespace IKIGAI::ECS {
	class AudioSystem : public System {
	public:
		AudioSystem();

		void onLateUpdate(std::chrono::duration<double> dt) override;
	};
}
