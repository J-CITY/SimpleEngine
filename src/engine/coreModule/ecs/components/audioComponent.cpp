#include "audioComponent.h"

#include "../../resourceManager/ServiceManager.h"
#include "../../resourceManager/audioSourceManager.h"
#include <sceneModule/sceneManager.h>



using namespace IKIGAI;
using namespace IKIGAI::ECS;

AudioComponent::AudioComponent(Ref<ECS::Object> obj, std::string& path): Component(obj) {
	__NAME__ = "AudioComponent";
	res = RESOURCES::ServiceManager::Get<RESOURCES::AudioSourceLoader>().CreateFromFile(path);
}

AudioComponent::AudioComponent(Ref<ECS::Object> obj) : Component(obj) {
	__NAME__ = "AudioComponent";
}

std::string AudioComponent::getSourcePath() {
	return res->path;
}
void AudioComponent::setSourcePath(std::string val) {
	res = RESOURCES::ServiceManager::Get<RESOURCES::AudioSourceLoader>().CreateFromFile(val);
}

std::string AudioComponent::getSoundPath() {
	return res->data.pathSoundSource;
}
void AudioComponent::setSoundPath(std::string) {}

bool AudioComponent::getIs3D() {
	return res->data.is3D;
}
void AudioComponent::setIs3D(bool val) {
	res->data.is3D = val;
	AUDIO::AudioManagerAL::GetInstance().stop(*res);
	AUDIO::AudioManagerAL::GetInstance().play(*res);
}

bool AudioComponent::getIsLooped() {
	return res->data.isLooped;
}
void AudioComponent::setIsLooped(bool val) {
	res->data.isLooped = val;
	AUDIO::AudioManagerAL::GetInstance().setLoop(*res, val);
}

float AudioComponent::getVolume() {
	return res->data.volume;
}
void AudioComponent::setVolume(float val) {
	res->data.volume = val;
	AUDIO::AudioManagerAL::GetInstance().setVolume(*res, val);
}

float AudioComponent::getPan() {
	return res->data.pan;
}
void AudioComponent::setPan(float val) {
	res->data.pan = val;
	AUDIO::AudioManagerAL::GetInstance().setPan(*res, val);
}

AUDIO::SoundStatus AudioComponent::getState() {
	return res->data.state;
}
void AudioComponent::setState(AUDIO::SoundStatus val) {
	auto prev = res->data.state;
	res->data.state = val;
	switch (res->data.state)
	{
	case AUDIO::SoundStatus::PLAY:
		{
			if (prev == AUDIO::SoundStatus::PAUSE) {
				AUDIO::AudioManagerAL::GetInstance().unpause(*res);
			}
			else {
				AUDIO::AudioManagerAL::GetInstance().play(*res);
			}
			break;
		}
	case AUDIO::SoundStatus::PAUSE: AUDIO::AudioManagerAL::GetInstance().pause(*res); break;
	case AUDIO::SoundStatus::STOP: AUDIO::AudioManagerAL::GetInstance().stop(*res); break;
	default: ;
	}
}

AudioListenerComponent::AudioListenerComponent(Ref<ECS::Object> obj): Component(obj) {
	__NAME__ = "AudioListenerComponent";
}


#include <rttr/registration>

RTTR_REGISTRATION
{
	rttr::registration::class_<IKIGAI::ECS::AudioListenerComponent>("AudioListenerComponent")
	(
		rttr::metadata(MetaInfo::FLAGS, MetaInfo::SERIALIZABLE)
	);

rttr::registration::class_<IKIGAI::ECS::AudioComponent>("AudioComponent")
(
	rttr::metadata(MetaInfo::FLAGS, MetaInfo::SERIALIZABLE)
	)
	.property("Source", &IKIGAI::ECS::AudioComponent::getSourcePath, &IKIGAI::ECS::AudioComponent::setSourcePath)
	(
		rttr::metadata(MetaInfo::FLAGS, MetaInfo::SERIALIZABLE || MetaInfo::USE_IN_EDITOR_COMPONENT_INSPECTOR),
		rttr::metadata(EditorMetaInfo::EDIT_WIDGET, EditorMetaInfo::WidgetType::STRING)
		)
	.property("Sound", &IKIGAI::ECS::AudioComponent::getSoundPath, &IKIGAI::ECS::AudioComponent::setSoundPath)
	(
		rttr::metadata(MetaInfo::FLAGS,MetaInfo::USE_IN_EDITOR_COMPONENT_INSPECTOR),
		rttr::metadata(EditorMetaInfo::EDIT_WIDGET, EditorMetaInfo::WidgetType::STRING)
		)
	.property("3D", &IKIGAI::ECS::AudioComponent::getIs3D, &IKIGAI::ECS::AudioComponent::setIs3D)
	(
		rttr::metadata(MetaInfo::FLAGS, MetaInfo::USE_IN_EDITOR_COMPONENT_INSPECTOR),
		rttr::metadata(EditorMetaInfo::EDIT_WIDGET, EditorMetaInfo::WidgetType::BOOL)
		)
	.property("Loop", &IKIGAI::ECS::AudioComponent::getIsLooped, &IKIGAI::ECS::AudioComponent::setIsLooped)
	(
		rttr::metadata(MetaInfo::FLAGS, MetaInfo::USE_IN_EDITOR_COMPONENT_INSPECTOR),
		rttr::metadata(EditorMetaInfo::EDIT_WIDGET, EditorMetaInfo::WidgetType::BOOL)
		)
	.property("Volume", &IKIGAI::ECS::AudioComponent::getVolume, &IKIGAI::ECS::AudioComponent::setVolume)
	(
		rttr::metadata(MetaInfo::FLAGS, MetaInfo::USE_IN_EDITOR_COMPONENT_INSPECTOR),
		rttr::metadata(EditorMetaInfo::EDIT_WIDGET, EditorMetaInfo::WidgetType::DRAG_FLOAT)
		)
	.property("Pan", &IKIGAI::ECS::AudioComponent::getPan, &IKIGAI::ECS::AudioComponent::setPan)
	(
		rttr::metadata(MetaInfo::FLAGS, MetaInfo::USE_IN_EDITOR_COMPONENT_INSPECTOR),
		rttr::metadata(EditorMetaInfo::EDIT_WIDGET, EditorMetaInfo::WidgetType::DRAG_FLOAT)
		)
	.property("State", &IKIGAI::ECS::AudioComponent::getState, &IKIGAI::ECS::AudioComponent::setState)
	(
		rttr::metadata(MetaInfo::FLAGS, MetaInfo::USE_IN_EDITOR_COMPONENT_INSPECTOR),
		rttr::metadata(EditorMetaInfo::EDIT_WIDGET, EditorMetaInfo::WidgetType::COMBO)
		);
}

