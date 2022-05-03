#pragma once
#include "component.h"
#include "../../render/buffers/depthBuffer.h"

#include "../../resourceManager/resource/bone.h"

namespace KUMA::ECS { class Object; }

namespace KUMA::ECS {
	class Skeletal : public Component {
	public:
		Skeletal(const ECS::Object& obj, std::string& path, int startAnimation=0);

		virtual void onDeserialize(nlohmann::json& j) override {

		}
		virtual void onSerialize(nlohmann::json& j) override {

		}

		void onUpdate(float dt) override;
		void setAnimation(int id);

		std::vector<std::shared_ptr<KUMA::RESOURCES::Animation>> animations;
		std::shared_ptr<KUMA::RESOURCES::Animator> animator;
		std::string animationPath;
	};
}