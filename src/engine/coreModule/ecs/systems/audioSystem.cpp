#include "audioSystem.h" 
#include <sceneModule/sceneManager.h>
#include <audioModule/audioManager.h>

IKIGAI::ECS::AudioSystem::AudioSystem() {
	mComponentsRead.insert(typeid(AudioListenerComponent).name());
	mComponentsWrite.insert(typeid(AudioComponent).name());
}

void IKIGAI::ECS::AudioSystem::onLateUpdate(std::chrono::duration<double> dt) {
	for (auto& listener : ECS::ComponentManager::GetInstance().getComponentArrayRef<ECS::AudioListenerComponent>()) {
		AUDIO::AudioManagerAL::GetInstance().setListenerPos(
			listener.obj->getTransform()->getWorldPosition(),
			listener.obj->getTransform()->getWorldRotation() * -MATH::Vector3f::Forward
		);
		break;
	}

	for (auto& audio : ECS::ComponentManager::GetInstance().getComponentArrayRef<ECS::AudioComponent>()) {
		if (!audio.res) {
			continue;
		}
		if (audio.res->data.is3D) {
			AUDIO::AudioManagerAL::GetInstance().setSourcePos(*audio.res, audio.obj->getTransform()->getWorldPosition());
		}
	}
}

//#include <rttr/registration>
//RTTR_REGISTRATION
//{
//	rttr::registration::class_<IKIGAI::ECS::AudioSystem>("AudioSystem")
//	(
//		rttr::metadata(IKIGAI::ECS::System::SYSTEM_COMPONENTS_READ, "AudioListenerComponent"),
//		rttr::metadata(IKIGAI::ECS::System::SYSTEM_COMPONENTS_WRITE, "AudioComponent")
//	);
//}
