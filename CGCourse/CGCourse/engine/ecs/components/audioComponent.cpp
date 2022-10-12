#include "audioComponent.h"

#include "../../resourceManager/ServiceManager.h"
#include "../../scene/sceneManager.h"

using namespace KUMA;
using namespace KUMA::ECS;

AudioComponent::AudioComponent(Ref<ECS::Object> obj, AUDIO::Sound3d sound): Component(obj) {
	__NAME__ = "AudioComponent";
	this->sound = sound;

	auto& am = RESOURCES::ServiceManager::Get<AUDIO::AudioManager>();
	am.play3d(sound, true);
}

void AudioComponent::onLateUpdate(std::chrono::duration<double> dt) {
	Component::onLateUpdate(dt);

	//TODO: update pos
	auto& sceneManager = RESOURCES::ServiceManager::Get<SCENE_SYSTEM::SceneManager>();
	if (sceneManager.hasCurrentScene()) {
		auto& currentScene = sceneManager.getCurrentScene();
		if (auto mainCameraComponent = currentScene.findMainCamera()) {
			RESOURCES::ServiceManager::Get<AUDIO::AudioManager>().setListenerPos(
				mainCameraComponent.value().get().obj->getTransform()->getWorldPosition(),
				mainCameraComponent.value().get().obj->getTransform()->getWorldRotation() * -MATHGL::Vector3::Forward
			);
		}
	}
}


