#if USE_IKIGAI_SCRIPT
#include "ikigaiScriptSmoke.h"
#include "coreModule/core/app.h"
#include "coreModule/core/core.h"
#include "coreModule/ecs/components/ikigaiScriptComponent.h"
#include "sceneModule/sceneManager.h"
#include <iostream>
#include <stdexcept>
namespace IKIGAI::SCRIPTING {
namespace {
	void require(bool ok, const char* message) {
		if (!ok)
			throw std::runtime_error(message);
	}
}
void RunIkigaiScriptSmoke(CORE::App& app) {
	auto& manager = *app.getCore()->sceneManager;
	auto load = manager.loadFromFileAsync("scenes/scripting-smoke.json");
	auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(90);
	while (!load.done()) {
		require(std::chrono::steady_clock::now() < deadline, "Script scene timed out");
		require(app.runMainLoop(), "Window closed");
	}
	require(load.status() == SCENE_SYSTEM::SceneLoadStatus::Completed, "Script scene did not load");
	auto& scene = manager.getCurrentScene();
	auto probe = scene.findObjectByName("ScriptProbe"), second = scene.findObjectByName("SecondProbe"), camera = scene.findObjectByName("Camera");
	require(probe && second && camera, "Missing script objects");
	for (auto& object : { probe, second, camera }) {
		auto script = object->getComponent<ECS::IkigaiScriptComponent>();
		require(bool(script), "Script component did not deserialize");
		require(script->isScriptOk(), "Script did not initialize");
	}
	require(app.runMainLoop(), "Window closed during script update");
	for (auto& object : { probe, second }) {
		require(object->getComponent<ECS::IkigaiScriptComponent>()->isScriptOk(), "Script update failed");
		auto position = object->getTransform()->getLocalPosition();
		require(position.x == 1.f && position.y == 2.f && position.z == 3.f, "Native transform binding failed");
	}
	probe->setActive(false);
	require(probe->getTag() == "disabled", "OnDisable did not run");
	probe->setActive(true);
	require(probe->getTag() == "enabled", "OnEnable did not run");
	auto component = probe->getComponent<ECS::IkigaiScriptComponent>();
	auto serialized = UTILS::ToJson(component->getDescriptor());
	require(serialized.isOk(), "Script descriptor did not serialize");
	auto data = serialized.unwrap();
	auto parsed = UTILS::FromJson<ECS::IkigaiScriptComponent::Descriptor>(data);
	require(parsed.isOk() && parsed.unwrap().Path == "ScriptingSmoke", "Script descriptor did not roundtrip");
	component->setScript("__missing_script__");
	require(!component->isScriptOk(), "Missing script did not fail");
	component->setScript("ScriptingSmoke");
	require(component->isScriptOk(), "Script did not recover after Path change");
	require(probe->removeComponent<ECS::IkigaiScriptComponent>(), "Script component did not remove");
	require(probe->getTag() == "destroyed", "OnDestroy did not run");
	require(app.runMainLoop(), "Window closed after component removal");
	require(second->getComponent<ECS::IkigaiScriptComponent>()->isScriptOk(), "ECS relocation invalidated another script");
	auto inactive = scene.createObject("InactiveScriptProbe");
	inactive->setActive(false);
	auto inactiveScript = inactive->addComponent<ECS::IkigaiScriptComponent>(std::string("ScriptingSmoke"));
	inactiveScript->onAwake();
	require(inactiveScript->isScriptOk(), "Inactive script did not initialize");
	inactive->dispose();
	require(inactive->getTag() == "destroyed", "Inactive script missed OnDestroy");
	manager.unloadCurrentScene();
	std::cout << "IKIGAI_SCRIPT_SMOKE_OK lifecycle serialization isolation native-bindings removal reload" << std::endl;
}
}
#endif
