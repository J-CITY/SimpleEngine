#include "audioSystem.h" 
#include <sceneModule/sceneManager.h>
#include <audioModule/audioManager.h>

IKIGAI::ECS::AudioSystem::AudioSystem() {
	mName = "AudioSystem";
	auto cm = RESOURCES::ServiceManager::Get<ECS2::World>().getComponentManager();
	mReads.insert(cm->getComponentType<AudioListenerComponent>());
	mWrites.insert(cm->getComponentType<AudioComponent>());
}

void IKIGAI::ECS::AudioSystem::onLateUpdate(ECS2::World& world, ECS2::CommandBuffer& cb, std::chrono::duration<double> dt) {
	//world.getComponentManager()->forEach<ECS::AudioListenerComponent>(
	//	[&](IKIGAI::ECS2::Entity e, ECS::AudioListenerComponent& listener) {
	//		if (!listener.getObject().getIsActive()) {
	//			return;
	//		}
	//		AUDIO::AudioManagerAL::GetInstance().setListenerPos(
	//			listener.obj->getTransform()->getWorldPosition(),
	//			listener.obj->getTransform()->getWorldRotation() * -MATH::Vector3f::Forward
	//		);
	//	}
	//);
	//world.getComponentManager()->forEach<ECS::AudioComponent>(
	//	[&](IKIGAI::ECS2::Entity e, ECS::AudioComponent& audio) {
	//		if (!audio.res) {
	//			return;
	//		}
	//		if (audio.res->data.is3D) {
	//			AUDIO::AudioManagerAL::GetInstance().setSourcePos(*audio.res, audio.obj->getTransform()->getWorldPosition());
	//		}
	//	}
	//);
	for (auto& listener : world.getComponentManager()->getComponentsArray<ECS::AudioListenerComponent>()) {
		AUDIO::AudioManagerAL::GetInstance().setListenerPos(
			listener.obj->getTransform()->getWorldPosition(),
			listener.obj->getTransform()->getWorldRotation() * -MATH::Vector3f::Forward
		);
		break;
	}
	
	for (auto& audio : world.getComponentManager()->getComponentsArray<ECS::AudioComponent>()) {
		if (!audio.res) {
			continue;
		}
		if (audio.res->data.is3D) {
			AUDIO::AudioManagerAL::GetInstance().setSourcePos(*audio.res, audio.obj->getTransform()->getWorldPosition());
		}
	}
	//for (auto& listener : ECS::ComponentManager::GetInstance().getComponentArrayRef<ECS::AudioListenerComponent>()) {
	//	AUDIO::AudioManagerAL::GetInstance().setListenerPos(
	//		listener.obj->getTransform()->getWorldPosition(),
	//		listener.obj->getTransform()->getWorldRotation() * -MATH::Vector3f::Forward
	//	);
	//	break;
	//}
	//
	//for (auto& audio : ECS::ComponentManager::GetInstance().getComponentArrayRef<ECS::AudioComponent>()) {
	//	if (!audio.res) {
	//		continue;
	//	}
	//	if (audio.res->data.is3D) {
	//		AUDIO::AudioManagerAL::GetInstance().setSourcePos(*audio.res, audio.obj->getTransform()->getWorldPosition());
	//	}
	//}
}

