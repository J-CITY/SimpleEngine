#pragma once
#include <optional>

#include "../../resourceManager/resource/bone.h"
#include "component.h"


namespace IKIGAI::RESOURCES {
	class Animator;
	class Animation;
}

namespace IKIGAI::ECS { class Object; }

namespace IKIGAI::ECS {
	class Skeletal : public Component {
	public:
		Skeletal(UTILS::Ref<ECS::Object> obj);
		Skeletal(UTILS::Ref<ECS::Object> obj, const std::string& path, const std::optional<std::string>& startAnimation=std::nullopt);

		void onUpdate(std::chrono::duration<double> dt) override;
		void setAnimation(std::string id);

		void setAnimationPath(std::string id);
		std::string getAnimationPath();
		//void setAnimation(int id);

		std::string getCurrentAnimationName();

		std::map<std::string, std::shared_ptr<IKIGAI::RESOURCES::Animation>> animations;
		//std::unique_ptr<IKIGAI::RESOURCES::Animator> animator;
		std::shared_ptr<IKIGAI::RESOURCES::Animator> animator;
		std::string animationPath;
		std::optional<std::string> curAnimation;
	};
}