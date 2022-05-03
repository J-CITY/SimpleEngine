#include "skeletal.h"
#include <string>
#include "../ComponentManager.h"
#include "../object.h"
using namespace KUMA;
using namespace KUMA::ECS;

Skeletal::Skeletal(const ECS::Object& obj, std::string& path, int startAnimation) : Component(obj), animationPath(path){
	__NAME__ = "Skeletal";
	auto model = ECS::ComponentManager::getInstance()->modelComponents[obj.getID()];
	if (!model) {
		//panic
	}
	animations.push_back(std::make_shared<KUMA::RESOURCES::Animation>(animationPath, model->getModel().get()));
	animator = std::make_shared<KUMA::RESOURCES::Animator>(animations[startAnimation].get());
}

void Skeletal::onUpdate(float dt) {
	animator->UpdateAnimation(dt);
}

void Skeletal::setAnimation(int id) {
	
}