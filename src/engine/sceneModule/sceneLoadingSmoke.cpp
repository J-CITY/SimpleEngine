#include "sceneLoadingSmoke.h"
#include "captureModule/screenshot.h"
#include "coreModule/core/app.h"
#include "resourceModule/fileSystem/fileSystem.h"
#include "resourceModule/serviceManager.h"
#include "sceneManager.h"

#ifdef USE_EDITOR
#include "editorModule/prefabEditor.h"
#endif
#include <fstream>
#include <iostream>

namespace IKIGAI::SCENE_SYSTEM {
namespace {
	void Require(bool value, const char* message) {
		if (!value)
			throw std::runtime_error(message);
	}
	void Pump(CORE::App& app, const SceneLoadHandle& operation) {
		const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(90);
		unsigned frames = 0;
		while (!operation.done()) {
			if (std::chrono::steady_clock::now() > deadline)
				throw std::runtime_error("Scene test timed out: " + operation.phase());
			Require(app.runMainLoop(), "Window closed during scene test");
			++frames;
		}
		std::cout << "LOADING_TEST_OPERATION frames=" << frames << " phase=" << operation.phase() << " error=" << operation.error() << '\n';
	}
}

void RunSceneLoadingSmoke(CORE::App& app) {
	auto modelJson
		= nlohmann::json { { "PathModel", "/models/pica_pica/scene.gltf" }, { "Flags", { "TRIANGULATE", "PRE_TRANSFORM_VERTICES", "FLIP_UVS" } } };
	auto modelConfig = UTILS::FromJson<RENDER::ModelResource>(modelJson);
	Require(!modelConfig.isErr(), "High-bit model import flags failed to deserialize");
	Require(
		serde::serialize<nlohmann::json>(modelConfig.unwrap())["Flags"] == modelJson["Flags"], "High-bit model import flags failed to round trip");
	auto& manager = *app.getCore()->sceneManager;
	auto& fs = RESOURCES::ServiceManager::Get<RESOURCES::FileSystem>();
	auto seedFile = fs.getFile("/scenes/render-smoke.json");
	auto seed = nlohmann::json::parse(seedFile->readStr());
	auto camera = seed["Objects"][0];
	auto object = nlohmann::json { { "Name", "Child" }, { "Id", 2 }, { "ParentId", 3 },
		{ "Components", nlohmann::json::array({ camera["Components"][0] }) } };
	auto parent = object;
	parent["Name"] = "Parent";
	parent["Id"] = 3;
	parent["ParentId"] = -1;
	auto smallScene = nlohmann::json { { "Objects", nlohmann::json::array({ camera, object, parent }) } };
	smallScene["RenderPipeline"] = "/renderPipeline/migrated-lite.json";
	auto large = smallScene;
	for (int i = 4; i < 260; ++i) {
		auto item = parent;
		item["Id"] = i;
		large["Objects"].push_back(std::move(item));
	}
	ECS::VectorShapeComponent::Descriptor shape;
	shape.Type = ECS::GetType<ECS::VectorShapeComponent>();
	shape.Size = { 240, 80 };
	shape.Color = { 0.1f, .8f, .3f, 1.f };
	auto banner = parent;
	banner["Name"] = "LoadingBanner";
	banner["Components"][0]["LocalPosition"] = { { "X", 80 }, { "Y", 80 }, { "Z", 0 } };
	banner["Components"][0]["LocalRotation"] = { { "X", 0 }, { "Y", 0 }, { "Z", 0 } };
	banner["Components"].push_back(serde::serialize<nlohmann::json>(shape));
	auto loading = nlohmann::json { { "Objects", nlohmann::json::array({ camera, banner }) } };
	auto duplicate = smallScene;
	duplicate["Objects"][2]["Id"] = 2;
	auto cycle = smallScene;
	cycle["Objects"][2]["ParentId"] = 2;
	const auto directory = std::filesystem::path(fs.getAbsolutePath("/scenes/render-smoke.json").value()).parent_path();
	std::vector<std::filesystem::path> paths;
	struct Cleanup {
		std::vector<std::filesystem::path>& paths;
		~Cleanup() {
			for (const auto& path : paths) {
				std::error_code error;
				std::filesystem::remove(path, error);
			}
		}
	} cleanup { paths };
	auto write = [&](const char* name, const nlohmann::json& json) {
		auto path = directory / name;
		paths.push_back(path);
		std::ofstream output(path);
		output << json.dump();
		output.close();
		return std::string("/__loadingtests/") + name;
	};
	const auto smallPath = write("__coroutine_test_small.json", smallScene);
	const auto largePath = write("__coroutine_test_large.json", large);
	const auto cachePath = write("__coroutine_test_loading.json", loading);
	const auto duplicatePath = write("__coroutine_test_duplicate.json", duplicate);
	const auto cyclePath = write("__coroutine_test_cycle.json", cycle);
	fs.addNativeFileSystem(directory.generic_string(), "/__loadingtests/");
	const auto originalConfig = app.getCore()->coroutineScheduler->config();
	app.getCore()->coroutineScheduler->setConfig({ std::chrono::microseconds(500), 8 });
	auto first = manager.loadFromFileAsync(smallPath);
	Require(manager.getPresentedScene() == nullptr, "Missing loading scene must present black");
	Pump(app, first);
	Require(first.status() == SceneLoadStatus::Completed, "Initial scene did not load");
	Require(manager.getCurrentScene().renderPipelineAsset == smallScene["RenderPipeline"]
			&& manager.serializeScene()["RenderPipeline"] == smallScene["RenderPipeline"],
		"Scene pipeline metadata was lost during asynchronous load or serialization");
	auto child = manager.getCurrentScene().findObjectByName("Child");
	Require(child && child->getParentId() == 3 && child->getIsActive(), "Forward parent reference or activation failed");
	auto cached = manager.prepareLoadingScene(cachePath);
	Pump(app, cached);
	Require(cached.status() == SceneLoadStatus::Completed, "Loading scene cache failed");
	auto second = manager.loadFromFileAsync(largePath);
	auto* loadingScene = manager.getPresentedScene();
	Require(loadingScene && &loadingScene->getWorld() != app.getCore()->world.get(), "Loading scene must have its own world");
	const auto captureStamp = std::to_string(std::chrono::steady_clock::now().time_since_epoch().count());
	auto capture = [&](const char* label) {
		auto& screenshots = CAPTURE::ScreenshotService::Get();
		const bool canCapture = RENDER::DriverInterface::settings.backend == RENDER::RenderSettings::Backend::OPENGL;
		if (canCapture)
			Require(
				screenshots.request(std::filesystem::current_path() / (std::string("utils/.pack/loading-") + label + "-" + captureStamp + ".png")),
				"Cannot request loading capture");
		Require(app.runMainLoop(), "Window closed");
		if (canCapture) {
			screenshots.wait();
			Require(screenshots.status().state == CAPTURE::ScreenshotService::State::SAVED, "Loading capture failed");
		}
	};
	capture("scene");
	Pump(app, second);
	Require(second.status() == SceneLoadStatus::Completed && manager.getCurrentScene().getObjects().size() == 259, "Large scene did not load");
	Require(!child->isAlive(), "External shared_ptr retained the old entity after scene unload");
	auto* gameScene = &manager.getCurrentScene();
	for (const auto& path : { duplicatePath, cyclePath, std::string("/scenes/__missing_coroutine_test.json") }) {
		auto invalid = manager.loadFromFileAsync(path);
		Require(manager.getPresentedScene() == loadingScene, "Loading scene was not reused");
		Pump(app, invalid);
		Require(invalid.status() == SceneLoadStatus::Failed && &manager.getCurrentScene() == gameScene, "Preflight failure lost the old scene");
	}
	auto cancelled = manager.loadFromFileAsync(largePath);
	bool reachedBuild = false;
	for (int frame = 0; frame < 2000 && !cancelled.done(); ++frame) {
		Require(app.runMainLoop(), "Window closed");
		if (manager.getTransitionState() == SceneTransitionState::Loading) {
			reachedBuild = true;
			break;
		}
	}
	Require(reachedBuild, "Test did not reach partial scene creation");
	auto replacement = manager.loadFromFileAsync(smallPath);
	Pump(app, replacement);
	Require(cancelled.status() == SceneLoadStatus::Cancelled && replacement.status() == SceneLoadStatus::Completed,
		"Queued replacement did not cancel and clean up the old operation");
	auto pending = manager.loadFromFileAsync(largePath);
	pending.cancel();
	Pump(app, pending);
	Require(pending.status() == SceneLoadStatus::Cancelled, "Cancellation before start failed");
	auto disabled = manager.prepareLoadingScene({ });
	Require(disabled.done(), "Disabling the cache must be immediate");
	auto final = manager.loadFromFileAsync(smallPath);
	Require(manager.getPresentedScene() == nullptr, "Disabled loading scene must present black");
	capture("black");
	Pump(app, final);
	Require(final.status() == SceneLoadStatus::Completed, "Loading after cache removal failed");
	// Prefab integration: export a real subtree containing an object reference,
	// instantiate twice, persist links, reload through both loading paths.
	auto& scene = manager.getCurrentScene();
	auto prefabRoot = scene.findObjectByName("Parent");
	auto prefabChild = scene.findObjectByName("Child");
	auto cameraJson = camera["Components"][1];
	auto cameraDescriptor = UTILS::FromJson<ECS::CameraComponent::Descriptor>(cameraJson).unwrap();
	prefabChild->addComponent<ECS::CameraComponent>(cameraDescriptor);
	ECS::RenderTargetComponent::Descriptor renderTarget;
	renderTarget.cameraObjectId = prefabChild->getIDInt();
	prefabRoot->addComponent<ECS::RenderTargetComponent>(renderTarget);
	const auto prefabPath = std::string("/__loadingtests/__subtree.prefab.json");
	const auto prefabScenePath = std::string("/__loadingtests/__prefab_scene.json");
	paths.push_back(directory / "__subtree.prefab.json");
	paths.push_back(directory / "__prefab_scene.json");
	const auto position = prefabRoot->getTransform()->getLocalPosition();
	manager.createPrefab(prefabRoot->getIDInt(), prefabPath);
	Require(scene.prefabInstances.size() == 1 && manager.serializeScene()["Objects"].size() == 2, "Prefab export did not compact the subtree");
	auto instanceOperation = manager.instantiatePrefabAsync(prefabPath);
	Pump(app, instanceOperation);
	Require(instanceOperation.status() == SceneLoadStatus::Completed, "Prefab instantiation failed");
	const int secondRootId = instanceOperation.resultRootId();
	auto secondRoot = scene.findObjectByID(ECS2::Entity(ECS2::Entity::ID(secondRootId)));
	Require(secondRoot && secondRootId != prefabRoot->getIDInt() && scene.getObjects().size() == 5, "Prefab instances overlap");
	Require(secondRoot->getComponent<ECS::RenderTargetComponent>()->getCamera()->obj->getIDInt() == secondRoot->getChildren()[0]->getIDInt(),
		"Prefab camera reference was not remapped");
	Require(secondRoot->getChildren()[0]->getIDInt() != prefabChild->getIDInt(), "Prefab child IDs overlap");
	Require(prefabRoot->getTransform()->getLocalPosition() == position, "Prefab conversion moved the original root");
#ifdef USE_EDITOR
	EDITOR::EditorRender::GlobalState.mCommandHistory.clear();
	EDITOR::PREFABS::RecordCreation(secondRootId);
	EDITOR::EditorRender::GlobalState.mCommandHistory.undo();
	Require(scene.getObjects().size() == 3, "Undo instantiation left objects");
	EDITOR::EditorRender::GlobalState.mCommandHistory.redo();
	Require(scene.getObjects().size() == 5 && scene.isPrefabRoot(secondRootId), "Redo instantiation lost the prefab link");
#endif
#ifdef USE_EDITOR
	EDITOR::PREFABS::Move(secondRootId, 1);
	Require(scene.findObjectByID(ECS2::Entity(ECS2::Entity::ID(secondRootId)))->getParentId() == 1, "Prefab reparent failed");
	EDITOR::EditorRender::GlobalState.mCommandHistory.undo();
	Require(scene.findObjectByID(ECS2::Entity(ECS2::Entity::ID(secondRootId)))->getParentId() <= 0, "Undo prefab reparent failed");
#endif
	manager.saveToFile(prefabScenePath);
	auto expected = manager.serializeScene();
	auto reload = manager.loadFromFileAsync(prefabScenePath);
	Pump(app, reload);
	Require(
		reload.status() == SceneLoadStatus::Completed && manager.serializeScene() == expected, "Prefab async round trip changed values or ID maps");
	manager.loadFromFile(prefabScenePath);
	Require(manager.serializeScene() == expected, "Prefab synchronous loading diverged");
	auto& reloaded = manager.getCurrentScene();
	auto edited = reloaded.findObjectByID(ECS2::Entity(ECS2::Entity::ID(secondRootId)))->getChildren()[0];
	const auto originalName = edited->getName();
	edited->setName("Unsupported prefab edit");
	bool rejected = false;
	try {
		manager.serializeScene();
	} catch (const std::exception&) {
		rejected = true;
	}
	Require(rejected, "Unsupported prefab edit would be silently lost");
	edited->setName(originalName);
	auto cancelledInstance = manager.instantiatePrefabAsync(prefabPath);
	cancelledInstance.cancel();
	Pump(app, cancelledInstance);
	Require(cancelledInstance.status() == SceneLoadStatus::Cancelled && reloaded.getObjects().size() == 5, "Cancelled prefab left partial entities");
#ifdef USE_EDITOR
	EDITOR::EditorRender::GlobalState.mCommandHistory.clear();
	EDITOR::PREFABS::Unpack(secondRootId);
	EDITOR::EditorRender::GlobalState.mCommandHistory.undo();
	Require(reloaded.prefabInstances.size() == 2, "Undo unpack lost metadata");
	EDITOR::EditorRender::GlobalState.mCommandHistory.redo();
#else
	reloaded.unpackPrefab(secondRootId);
#endif
	Require(reloaded.prefabInstances.size() == 1 && manager.serializeScene()["Objects"].size() == 4, "Unpack lost objects or retained the link");
#ifdef USE_EDITOR
	EDITOR::PREFABS::Delete(secondRootId);
	EDITOR::EditorRender::GlobalState.mCommandHistory.undo();
	Require(reloaded.getObjects().size() == 5, "Undo subtree deletion lost children");
	EDITOR::EditorRender::GlobalState.mCommandHistory.redo();
#else
	reloaded.destroySubtree(secondRootId);
#endif
	Require(reloaded.getObjects().size() == 3 && !edited->isAlive(), "Prefab deletion left live entities");
	const auto savedPrefab = fs.getFile(prefabPath)->readStr();
	bool writeRejected = false;
	try {
		manager.createPrefab(1, prefabPath + "/invalid.prefab.json");
	} catch (const std::exception&) {
		writeRejected = true;
	}
	Require(writeRejected && !reloaded.prefabFor(1) && fs.getFile(prefabPath)->readStr() == savedPrefab,
		"Failed prefab write changed the original asset or linked the scene");
	auto missingPrefab = expected;
	missingPrefab["Objects"][1]["Prefab"] = "/__loadingtests/missing.prefab.json";
	const auto missingPrefabPath = write("__missing_prefab_scene.json", missingPrefab);
	fs.writeTextAtomic(missingPrefabPath, missingPrefab.dump());
	auto* preserved = &manager.getCurrentScene();
	auto failedPrefab = manager.loadFromFileAsync(missingPrefabPath);
	Pump(app, failedPrefab);
	Require(failedPrefab.status() == SceneLoadStatus::Failed && &manager.getCurrentScene() == preserved, "Missing prefab lost the current scene");
	auto largePrefabFile = fs.getFile(prefabPath);
	auto largePrefab = nlohmann::json::parse(largePrefabFile->readStr());
	for (int local = 3; local < 180; ++local) {
		auto node = largePrefab["Objects"][1];
		node["Id"] = local;
		node["Name"] = "Budgeted node";
		largePrefab["Objects"].push_back(std::move(node));
	}
	const auto largePrefabPath = std::string("/__loadingtests/__large.prefab.json");
	paths.push_back(directory / "__large.prefab.json");
	fs.writeTextAtomic(largePrefabPath, largePrefab.dump());
	const auto initialObjects = manager.getCurrentScene().getObjects().size();
	auto partialInstance = manager.instantiatePrefabAsync(largePrefabPath);
	bool partialCreated = false;
	for (int frame = 0; frame < 2000 && !partialInstance.done(); ++frame) {
		Require(app.runMainLoop(), "Window closed during prefab cancellation test");
		if (manager.getCurrentScene().getObjects().size() > initialObjects) {
			partialCreated = true;
			break;
		}
	}
	Require(partialCreated, "Prefab test did not reach budgeted object creation");
	partialInstance.cancel();
	Pump(app, partialInstance);
	Require(partialInstance.status() == SceneLoadStatus::Cancelled && manager.getCurrentScene().getObjects().size() == initialObjects,
		"Cancelled partial prefab left entities in the scene");
	manager.serializeScene();
	std::cout << "PREFAB_SMOKE_OK: export, references, instances, async/sync roundtrip, unpack, cancellation, deletion, preflight\n";
	app.getCore()->coroutineScheduler->setConfig(originalConfig);
	std::cout << "SCENE_LOADING_SMOKE_OK: cache, isolation, black fallback, hierarchy, validation, cancellation, replacement\n";
}
}
