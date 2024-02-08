#include "skeletal.h"
#include <string>
#include "../componentManager.h"
#include "../object.h"

using namespace IKIGAI;
using namespace IKIGAI::ECS;

Skeletal::Skeletal(UTILS::Ref<ECS::Object> obj) : Component(obj) {
	__NAME__ = "Skeletal";
}

Skeletal::Skeletal(UTILS::Ref<ECS::Object> obj, const std::string& path, const std::optional<std::string>& startAnimation) : Component(obj), animationPath(path), curAnimation(startAnimation){
	__NAME__ = "Skeletal";
	auto model = ECS::ComponentManager::GetInstance().getComponent<ModelRenderer>(obj->getID());
	animations = RESOURCES::Animation::LoadAnimations(animationPath, model->getModel().get());

	if (startAnimation) {
		animator = std::make_unique<IKIGAI::RESOURCES::Animator>(animations.at(startAnimation.value()).get());
	}
}

void Skeletal::onUpdate(std::chrono::duration<double> dt) {
	animator->UpdateAnimation(static_cast<float>(dt.count()));
}

void Skeletal::setAnimation(std::string id) {
	if (!animations.contains(id)) {
		LOG_ERROR("Skeletal::setAnimation: Can not set animation (key do not exist)");
		return;
	}
	curAnimation = id;
	//if (!id.empty()) {
		animator = std::make_unique<IKIGAI::RESOURCES::Animator>(animations.at(id).get());
	//}
}

void Skeletal::setAnimationPath(std::string id) {
	animationPath = id;
	auto model = ECS::ComponentManager::GetInstance().getComponent<ModelRenderer>(obj->getID());
	animations = RESOURCES::Animation::LoadAnimations(animationPath, model->getModel().get());
}

std::string Skeletal::getAnimationPath() {
	return animationPath;
}

std::string Skeletal::getCurrentAnimationName() {
	if (!curAnimation) {
		return "";
	}
	return curAnimation.value();
}


#include <rttr/registration>

RTTR_REGISTRATION
{
	rttr::registration::class_<IKIGAI::ECS::Skeletal>("Skeletal")
	(
		rttr::metadata(MetaInfo::FLAGS, MetaInfo::SERIALIZABLE)
	)
	.property("AnimationPath", &IKIGAI::ECS::Skeletal::getAnimationPath, &IKIGAI::ECS::Skeletal::setAnimationPath)
	(
		rttr::metadata(MetaInfo::FLAGS, MetaInfo::SERIALIZABLE | MetaInfo::USE_IN_EDITOR_COMPONENT_INSPECTOR),
		rttr::metadata(EditorMetaInfo::EDIT_WIDGET, EditorMetaInfo::WidgetType::STRING)
	)
	.property("Animation", &IKIGAI::ECS::Skeletal::getCurrentAnimationName, &IKIGAI::ECS::Skeletal::setAnimation)
	(
		rttr::metadata(MetaInfo::FLAGS, MetaInfo::SERIALIZABLE | MetaInfo::USE_IN_EDITOR_COMPONENT_INSPECTOR | MetaInfo::OPTIONAL_PARAM),
		rttr::metadata(EditorMetaInfo::EDIT_WIDGET, EditorMetaInfo::WidgetType::STRING)
	);
}
