#pragma once
#include "component.h"
#include "../../render/buffers/depthBuffer.h"
#include "../../audioManager/audioManager.h"


namespace KUMA::ECS { class Object; }

namespace KUMA::ECS {
	class AudioComponent : public Component {
	public:
		AudioComponent(const ECS::Object& obj, AUDIO::Sound3d sound);

		virtual void onLateUpdate(std::chrono::duration<double> dt) override;
		AUDIO::Sound3d sound;
	};
}