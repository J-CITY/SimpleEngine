#include "audioSystem.h"
#include "../../resourceManager/ServiceManager.h"
#include "../../scene/sceneManager.h"

KUMA::ECS::AudioSystem::AudioSystem() {

}

void KUMA::ECS::AudioSystem::onLateUpdate(std::chrono::duration<double> dt) {
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


