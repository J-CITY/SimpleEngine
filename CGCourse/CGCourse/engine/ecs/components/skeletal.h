#pragma once
#include <optional>

#include "component.h"
#include "../../render/buffers/depthBuffer.h"

#include "../../resourceManager/resource/bone.h"

namespace KUMA::ECS { class Object; }

namespace KUMA::ECS {
	class Skeletal : public Component {
	public:
		Skeletal(const ECS::Object& obj, const std::string& path, const std::optional<std::string>& startAnimation=std::nullopt);

		virtual void onDeserialize(nlohmann::json& j) override {

		}
		virtual void onSerialize(nlohmann::json& j) override {

		}

		void onUpdate(std::chrono::duration<double> dt) override;
		void setAnimation(const std::string& id);
		//void setAnimation(int id);

		std::map<std::string, std::shared_ptr<KUMA::RESOURCES::Animation>> animations;
		//std::vector<std::shared_ptr<KUMA::RESOURCES::Animation>> animationsVec;
		std::unique_ptr<KUMA::RESOURCES::Animator> animator;
		std::string animationPath;
	};
}