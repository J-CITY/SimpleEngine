#include "skeletal.h"
#include <string>
#include "../ComponentManager.h"
#include "../object.h"
#include "../../resourceManager/resource/bone.h"
using namespace KUMA;
using namespace KUMA::ECS;

Skeletal::Skeletal(const ECS::Object& obj, const std::string& path, const std::optional<std::string>& startAnimation) : Component(obj), animationPath(path){
	__NAME__ = "Skeletal";
	auto model = ECS::ComponentManager::getInstance()->modelComponents[obj.getID()];
	if (!model) {
		//panic
	}
	animations = RESOURCES::Animation::LoadAnimations(animationPath, model->getModel().get());
	//for (auto a : animations) {
	//	animationsVec.push_back(a.second);
	//}
	if (startAnimation) {
		animator = std::make_unique<KUMA::RESOURCES::Animator>(animations.at(startAnimation.value()).get());
	}
}

void Skeletal::onUpdate(std::chrono::duration<double> dt) {
	animator->UpdateAnimation(static_cast<float>(dt.count()));
}

void Skeletal::setAnimation(const std::string& id) {
	if (!animations.count(id)) {
		LOG_ERROR("Skeletal::setAnimation: Can not set animation (key do not exist)");
		return;
	}
	//if (!id.empty()) {
		animator = std::make_unique<KUMA::RESOURCES::Animator>(animations.at(id).get());
	//}
}

//void Skeletal::setAnimation(int id) {
//	//if (!id.empty()) {
//	if (id >= animationsVec.size()) {
//		LOG_ERROR("Skeletal::setAnimation: Can not set animation (out of vector range)");
//		return;
//	}
//	animator = std::make_unique<KUMA::RESOURCES::Animator>(animationsVec[id].get());
//	//}
//}

