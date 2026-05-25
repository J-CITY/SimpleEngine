#include "scene.h"

#include <algorithm>


#include <coreModule/ecs/componentManager.h>
#include <resourceModule/parser/assimpParser.h>
#include <renderModule/backends/interface/renderEnums.h>

#include "coreModule/ecs/object.h"
#include "coreModule/ecs/components/renderTargetComponent.h"
#include "coreModule/ecs/components/cameraComponent.h"
#include "coreModule/ecs/components/skeletalComponent.h"
#include "coreModule/ecs/components/skeletalAnimationComponent.h"
#include "physicsModule/broadPhase.h"
#include "renderModule/backends/interface/meshInterface.h"
#include "utilsModule/log/loggerDefine.h"
#include <unordered_map>

namespace IKIGAI
{
	namespace ECS
	{
		class Skeletal;
	}
}

namespace IKIGAI
{
	namespace RENDER
	{
		enum class CullingOptions;
	}
}

using namespace IKIGAI;
using namespace IKIGAI::SCENE_SYSTEM;

void Scene::init() {
	//Create Default skybox
	
	//skyboxObject = std::make_unique<IKIGAI::ECS::Object>(IKIGAI::ObjectId<ECS::Object>(999), "Skybox", "");

	//skyboxTexture = IKIGAI::RESOURCES::TextureLoader::CreateSkybox("C:\\Projects\\SimpleEngine\\CGCourse\\CGCourse\\Assets\\sb\\");
	//auto mat = skyboxObject->addComponent<IKIGAI::ECS::MaterialRenderer>();
	//auto skyboxMat = std::make_shared<IKIGAI::RENDER::Material>();
	//auto s = IKIGAI::RESOURCES::ShaderLoader::CreateFromFile("Shaders\\skybox1.glsl");
	//skyboxMat->setShader(s);
	//skyboxMat->setBackfaceCulling(true);
	//skyboxMat->setBackfaceCulling(true);
	//skyboxMat->setDepthTest(true);
	//skyboxMat->setDepthWriting(true);
	//skyboxMat->setColorWriting(true);
	//skyboxMat->setGPUInstances(1);
	//skyboxMat->getShader()->setUniformInt("gCubemapTexture", 0);
	//mat->fillWithMaterial(skyboxMat);
	
	//auto model = skyboxObject->addComponent<IKIGAI::ECS::ModelRenderer>();
	//IKIGAI::RESOURCES::ModelParserFlags flags = IKIGAI::RESOURCES::ModelParserFlags::TRIANGULATE;
	//auto m = IKIGAI::RESOURCES::ModelLoader::Create("C:\\Projects\\SimpleEngine\\CGCourse\\CGCourse\\Assets\\Engine\\Models\\Cube.fbx", flags);
	//model->setModel(m);
	//model->setFrustumBehaviour(IKIGAI::ECS::ModelRenderer::EFrustumBehaviour::CULL_MODEL);
	//auto bs = IKIGAI::RENDER::BoundingSphere();
	//bs.position = {0.0f, 0.0f, 0.0f};
	//bs.radius = 1.0f;
	//model->setCustomBoundingSphere(bs);
	//skyboxObject->getTransform()->setLocalPosition({ 0.0f, 0.0f, 0.0f });
	//skyboxObject->getTransform()->setLocalRotation({0.0f, 0.0f, 0.0f, 1.0f});
	//skyboxObject->getTransform()->setLocalScale({20.0f, 20.0f, 20.0f});
	
}

Scene::Scene(const Descriptor& _descriptor) {
	for (const auto& object : _descriptor.Objects) {
		createObject(object);
	}
}

Scene::~Scene() {
	objects.clear();
}

void Scene::go() {
	isExecute = true;

	for (auto& o : objects) {
		if (o->getIsActive()) {
			o->onEnable();
		}
	}
	for (auto& o : objects) {
		if (o->getIsActive()) {
			o->onStart();
		}
	}
}

bool Scene::getIsExecute() const {
	return isExecute;
}

void Scene::update(std::chrono::duration<double> dt) {
	for (auto& o : objects) {
		o->onUpdate(dt);
	}
}

void Scene::fixedUpdate(std::chrono::duration<double> dt) {
	for (auto& o : objects) {
		o->onFixedUpdate(dt);
	}
}

void Scene::lateUpdate(std::chrono::duration<double> dt) {
	for (auto& o : objects) {
		o->onLateUpdate(dt);
	}
}

std::shared_ptr<IKIGAI::ECS::Object> Scene::createObject() {
	return createObject("New Actor");
}

//std::shared_ptr<IKIGAI::ECS::Object> Scene::_createObject(const std::string& name, const std::string& tag) {
//	auto instance = std::make_shared<ECS::Object>(idGenerator.generateId(), name, tag);
//	if (isExecute) {
//		instance->setActive(true);
//		if (instance->getIsActive()) {
//			instance->onEnable();
//			instance->onStart();
//		}
//	}
//	return instance;
//}

std::shared_ptr<IKIGAI::ECS::Object> Scene::createObject(ECS2::Entity actorID, const std::string& name, const std::string& tag) {
	objects.push_back(std::make_shared<ECS::Object>(actorID, name, tag));
	auto& instance = objects.back();
	if (isExecute) {
		instance->setActive(true);
		if (instance->getIsActive()) {
			instance->onEnable();
			instance->onStart();
		}
	}

	if (isSceneReady) {
		instance->componentAddedEvent.add(std::bind(&Scene::addToBVH, this, std::placeholders::_1));
		instance->componentRemovedEvent.add(std::bind(&Scene::removeFromBVH, this, std::placeholders::_1));
		instance->componentChangedEvent.add(std::bind(&Scene::updateInBVH, this, std::placeholders::_1));
	}
	return instance;
}

std::shared_ptr<IKIGAI::ECS::Object> Scene::createObject(const ECS::Object::Descriptor& data) {
	objects.push_back(std::make_shared<ECS::Object>(data));
	auto& instance = objects.back();
	if (isExecute) {
		instance->setActive(data.IsActive);
		if (instance->getIsActive()) {
			instance->onEnable();
			instance->onStart();
		}
	}
	if (data.ParentId > 0) {
		auto p = findObjectByID(ECS2::Entity(ECS2::Entity::ID(data.ParentId)));
		if (!p) {
			LOG_ERROR << "Object::setParentId: can not find actor with id: " + std::to_string(data.ParentId);
		}
		else {
			instance->setParent(p);
		}
	}
	return instance;
}


std::shared_ptr<IKIGAI::ECS::Object> Scene::createObject(const std::string& name, const std::string& tag) {
	objects.push_back(std::make_shared<ECS::Object>(RESOURCES::ServiceManager::Get<ECS2::World>().createEntity(), name, tag));
	auto& instance = objects.back();
	if (isExecute) {
		instance->setActive(true);
		if (instance->getIsActive()) {
			instance->onEnable();
			instance->onStart();
		}
	}
	return instance;
}

struct BVHData {
	UTILS::Ref<ECS::Object> obj;
	std::shared_ptr<RENDER::MeshInterface> mesh;

	BVHData(UTILS::Ref<ECS::Object> obj,
		std::shared_ptr<RENDER::MeshInterface> mesh): obj(obj), mesh(mesh) {}

	const ECS::Transform& getTransform() {
		//if (!obj) {
		//	return MATHGL::Matrix4::Identity;
		//}
		return obj->mTransform->getTransform();
	}
};

PHYSICS::BVHTree<RENDER::BoundingSphere, std::shared_ptr<BVHData>> meshesTree;
std::unordered_map<ECS::Object::Id, std::list<std::shared_ptr<BVHData>>> objectToBVHElements;

//TODO: move to utils
RENDER::BoundingSphere getGlobalBoundingSphere(const RENDER::BoundingSphere& boundingSphere, const ECS::Transform& transform) {
	const auto& position = transform.getWorldPosition();
	const auto& rotation = transform.getWorldRotation();
	const auto& scale = transform.getWorldScale();

	float maxScale = std::max(std::max(std::max(scale.x, scale.y), scale.z), 0.0f);
	float scaledRadius = boundingSphere.radius * maxScale;
	auto sphereOffset = MATH::QuaternionF::RotatePoint(boundingSphere.position, rotation) * maxScale;

	MATH::Vector3f worldCenter = position + sphereOffset;

	return RENDER::BoundingSphere{ worldCenter, scaledRadius };
}

void Scene::addToBVH(UTILS::WeakPtr<ECS::ComponentBase> component) {
	if (component->getName() != "ModelRenderer") {
		return;
	}

	auto modelRenderer = component->obj->getComponent<ECS::ModelRenderer>();
	if (auto model = modelRenderer->getModel()) {
		RENDER::CullingOptions cullingOptions = RENDER::CullingOptions::NONE;

		if (modelRenderer->getFrustumBehaviour() != ECS::EFrustumBehaviour::DISABLED) {
			cullingOptions |= RENDER::CullingOptions::FRUSTUM_PER_MODEL;
		}

		if (modelRenderer->getFrustumBehaviour() == ECS::EFrustumBehaviour::CULL_MESHES) {
			cullingOptions |= RENDER::CullingOptions::FRUSTUM_PER_MESH;
		}

		const auto& modelBoundingSphere = modelRenderer->getFrustumBehaviour() == ECS::EFrustumBehaviour::CULL_CUSTOM ? modelRenderer->getCustomBoundingSphere() : model->getBoundingSphere();
		//TODO: add support RENDER::CullingOptions::FRUSTUM_PER_MESH
		//TODO: add event for change CullingOptions
		auto gbs = getGlobalBoundingSphere(modelBoundingSphere, modelRenderer->obj->mTransform->getTransform());
		for (auto mesh : model->getMeshes()) {
			auto node = std::make_shared<BVHData>(component->obj, mesh);
			meshesTree.Insert(node, gbs);
			objectToBVHElements[component->obj->getID()].push_back(node);
		}
	}
}

void Scene::removeFromBVH(UTILS::WeakPtr<ECS::ComponentBase> component) {
	if (component->getName() != "ModelRenderer" || component->getName() != "TransformComponent") {
		return;
	}
	if (!component->obj->getComponent<ECS::ModelRenderer>()) {
		return;
	}

	if (objectToBVHElements.contains(component->obj->getID())) {
		for (auto e : objectToBVHElements[component->obj->getID()]) {
			meshesTree.Remove(e);
		}
		objectToBVHElements.erase(component->obj->getID());
	}
}

//TODO: add subscribe for it
void Scene::updateInBVH(UTILS::WeakPtr<ECS::ComponentBase> component) {
	if (component->getName() != "ModelRenderer") {
		return;
	}

	removeFromBVH(component);
	addToBVH(component);
}

void Scene::postLoad() {
	for (auto obj : objects) {
		obj->componentAddedEvent.add(std::bind(&Scene::addToBVH, this, std::placeholders::_1));
		obj->componentRemovedEvent.add(std::bind(&Scene::removeFromBVH, this, std::placeholders::_1));
		obj->componentChangedEvent.add(std::bind(&Scene::updateInBVH, this, std::placeholders::_1));
		if (auto component = obj->getComponent<ECS::ModelRenderer>()) {
			addToBVH(component.get());
		}
	}
	isSceneReady = true;
}

std::shared_ptr<ECS::Object> Scene::createObjectAfter(ECS::Object::Id parentId, const std::string& name, const std::string& tag) {
	const auto iter = std::ranges::find_if(objects, [parentId](auto& e) {
		return e->getID() == parentId;
	});
	auto resIt = objects.insert(iter+1, std::make_shared<ECS::Object>(RESOURCES::ServiceManager::Get<ECS2::World>().createEntity(), name, tag));
	return *resIt;
}

std::shared_ptr<ECS::Object> Scene::createObjectBefore(ECS::Object::Id parentId, const std::string& name, const std::string& tag) {
	const auto iter = std::ranges::find_if(objects, [parentId](auto& e) {
		return e->getID() == parentId;
	});
	auto resIt = objects.insert(iter, std::make_shared<ECS::Object>(RESOURCES::ServiceManager::Get<ECS2::World>().createEntity(), name, tag));
	return *resIt;
}

bool Scene::destroyObject(std::shared_ptr<ECS::Object> p_target) {
	auto found = std::ranges::find_if(objects.begin(), objects.end(), [p_target](std::shared_ptr<ECS::Object> element) {
		return element == p_target;
	});

	if (found != objects.end()) {
		objects.erase(found);
		return true;
	}
	else {
		return false;
	}
}

std::shared_ptr<IKIGAI::ECS::Object> Scene::findObjectByName(const std::string& p_name) {
	auto result = std::ranges::find_if(objects.begin(), objects.end(), [p_name](std::shared_ptr<ECS::Object>& element) {
		return element->getName() == p_name;
	});

	if (result != objects.end()) {
		return *result;
	} 
	else {
		return nullptr;
	}
}

std::shared_ptr<IKIGAI::ECS::Object> Scene::findObjectByTag(const std::string& p_tag) {
	auto result = std::ranges::find_if(objects.begin(), objects.end(), [p_tag](std::shared_ptr<ECS::Object>& element) {
		return (*element).getTag() == p_tag;
	});

	if (result != objects.end()) {
		return *result;
	}
	else {
		return nullptr;
	}
}

std::shared_ptr<IKIGAI::ECS::Object> Scene::findObjectByID(ECS2::Entity p_id) {
	auto result = std::ranges::find_if(objects.begin(), objects.end(), [p_id](std::shared_ptr<ECS::Object>& element) {
		return element->getID() == p_id;
	});

	if (result != objects.end()) {
		return *result;
	}
	else {
		return nullptr;
	}
}

std::vector<std::shared_ptr<IKIGAI::ECS::Object>> Scene::findObjectsByName(const std::string& p_name) {
	std::vector<std::shared_ptr<IKIGAI::ECS::Object>> actors;
	for (const auto& actor : objects) {
		if (actor->getName() == p_name) {
			actors.push_back(actor);
		}
	}
	return actors;
}

std::vector<std::shared_ptr<IKIGAI::ECS::Object>> Scene::findObjectsByTag(const std::string& p_tag) {
	std::vector<std::shared_ptr<IKIGAI::ECS::Object>> actors;
	for (const auto& actor : objects) {
		if (actor->getTag() == p_tag) {
			actors.push_back(actor);
		}
	}
	return actors;
}

std::span<std::shared_ptr<IKIGAI::ECS::Object>> Scene::getObjects() {
	return objects;
}

//TODO: void Scene::setMainCamera(Entity id) {}


IKIGAI::UTILS::WeakPtr<IKIGAI::ECS::CameraComponent> Scene::findMainCamera() {
	auto cm = IKIGAI::RESOURCES::ServiceManager::Get<IKIGAI::ECS2::World>().getComponentManager();
	for (auto& camera : cm->getComponentsArray<ECS::CameraComponent>()) {
		//for (auto& camera : ECS::ComponentManager::GetInstance().getComponentArrayRef<ECS::CameraComponent>()) {
		if (camera.obj->getIsActive()) {
			if (camera.obj->getName().rfind("__", 0) == 0) { //TODO: refactor VR component and remove it
				continue;
			}
			return camera.getWeak<ECS::CameraComponent>();;
		}
	}
	for (auto& camera : cm->getComponentsArray<ECS::VrCameraComponent>()) {
		//for (auto& camera : ECS::ComponentManager::GetInstance().getComponentArrayRef<ECS::VrCameraComponent>()) {
		if (camera.obj->getIsActive()) {
			return camera.getWeak<ECS::CameraComponent>();
		}
	}
	return nullptr;
}


std::vector<IKIGAI::RENDER::LightOGL> Scene::findLightData() const {
	std::vector<RENDER::LightOGL> result;

	auto cm = IKIGAI::RESOURCES::ServiceManager::Get<IKIGAI::ECS2::World>().getComponentManager();
	for (auto& light : cm->getComponentsArray<ECS::SpotLight>()) {
		//for (auto& light : ECS::ComponentManager::GetInstance().getComponentArrayRef<ECS::SpotLight>()) {
		if (light.obj->getIsActive()) {
			auto ldata = light.getData().generateOGLStruct();
			result.push_back(ldata);
		}
	}
	for (auto& light : cm->getComponentsArray<ECS::DirectionalLight>()) {
		//for (auto& light : ECS::ComponentManager::GetInstance().getComponentArrayRef<ECS::DirectionalLight>()) {
		if (light.obj->getIsActive()) {
			auto ldata = light.getData().generateOGLStruct();
			result.push_back(ldata);
		}
	}
	for (auto& light : cm->getComponentsArray<ECS::PointLight>()) {
		//for (auto& light : ECS::ComponentManager::GetInstance().getComponentArrayRef<ECS::PointLight>()) {
		if (light.obj->getIsActive()) {
			auto ldata = light.getData().generateOGLStruct();
			result.push_back(ldata);
		}
	}
	for (auto& light : cm->getComponentsArray<ECS::AmbientLight>()) {
		//for (auto& light : ECS::ComponentManager::GetInstance().getComponentArrayRef<ECS::AmbientLight>()) {
		if (light.obj->getIsActive()) {
			auto ldata = light.getData().generateOGLStruct();
			result.push_back(ldata);
		}
	}
	for (auto& light : cm->getComponentsArray<ECS::AmbientSphereLight>()) {
		//for (auto& light : ECS::ComponentManager::GetInstance().getComponentArrayRef<ECS::AmbientSphereLight>()) {
		if (light.obj->getIsActive()) {
			auto ldata = light.getData().generateOGLStruct();
			result.push_back(ldata);
		}
	}
	//for (auto light : ECS::ComponentManager::GetInstance().getComponentArrayRef<ECS::SpotLight>()) {
	//	if (light.second->obj.getIsActive()) {
	//		auto ldata = light.second->getData().generateOGLStruct();
	//		result.push_back(ldata);
	//	}
	//}
	return result;
}

std::vector<IKIGAI::RENDER::LightOGL> Scene::findLightDataInFrustum(const RENDER::Frustum& p_frustum) {
	std::vector<RENDER::LightOGL> result;

	auto cm = IKIGAI::RESOURCES::ServiceManager::Get<IKIGAI::ECS2::World>().getComponentManager();
	for (auto& light : cm->getComponentsArray<ECS::SpotLight>()) {
		//for (auto& light : ECS::ComponentManager::GetInstance().getComponentArrayRef<ECS::SpotLight>()) {
		if (light.obj->getIsActive()) {
			const auto& lightData = light.getData();
			const auto& position = lightData.getTransform().getWorldPosition();
			auto effectRange = lightData.getEffectRange();

			if (std::isinf(effectRange) || p_frustum.sphereInFrustum(position.x, position.y, position.z, lightData.getEffectRange())) {
				auto ldata = lightData.generateOGLStruct();
				result.push_back(ldata);
			}
		}
	}
	for (auto& light : cm->getComponentsArray<ECS::DirectionalLight>()) {
		//for (auto& light : ECS::ComponentManager::GetInstance().getComponentArrayRef<ECS::DirectionalLight>()) {
		if (light.obj->getIsActive()) {
			const auto& lightData = light.getData();
			const auto& position = lightData.getTransform().getWorldPosition();
			auto effectRange = lightData.getEffectRange();

			if (std::isinf(effectRange) || p_frustum.sphereInFrustum(position.x, position.y, position.z, lightData.getEffectRange())) {
				auto ldata = lightData.generateOGLStruct();
				result.push_back(ldata);
			}
		}
	}
	for (auto& light : cm->getComponentsArray<ECS::PointLight>()) {
		//for (auto& light : ECS::ComponentManager::GetInstance().getComponentArrayRef<ECS::PointLight>()) {
		if (light.obj->getIsActive()) {
			const auto& lightData = light.getData();
			const auto& position = lightData.getTransform().getWorldPosition();
			auto effectRange = lightData.getEffectRange();

			if (std::isinf(effectRange) || p_frustum.sphereInFrustum(position.x, position.y, position.z, lightData.getEffectRange())) {
				auto ldata = lightData.generateOGLStruct();
				result.push_back(ldata);
			}
		}
	}
	for (auto& light : cm->getComponentsArray<ECS::AmbientLight>()) {
		//for (auto& light : ECS::ComponentManager::GetInstance().getComponentArrayRef<ECS::AmbientLight>()) {
		if (light.obj->getIsActive()) {
			const auto& lightData = light.getData();
			const auto& position = lightData.getTransform().getWorldPosition();
			auto effectRange = lightData.getEffectRange();

			if (std::isinf(effectRange) || p_frustum.sphereInFrustum(position.x, position.y, position.z, lightData.getEffectRange())) {
				auto ldata = lightData.generateOGLStruct();
				result.push_back(ldata);
			}
		}
	}
	for (auto& light : cm->getComponentsArray<ECS::AmbientSphereLight>()) {
		//for (auto& light : ECS::ComponentManager::GetInstance().getComponentArrayRef<ECS::AmbientSphereLight>()) {
		if (light.obj->getIsActive()) {
			const auto& lightData = light.getData();
			const auto& position = lightData.getTransform().getWorldPosition();
			auto effectRange = lightData.getEffectRange();

			if (std::isinf(effectRange) || p_frustum.sphereInFrustum(position.x, position.y, position.z, lightData.getEffectRange())) {
				auto ldata = lightData.generateOGLStruct();
				result.push_back(ldata);
			}
		}
	}
	//for (auto& light : ECS::ComponentManager::GetInstance().lightComponents) {
	//	if (light.second->obj.getIsActive()) {
	//		const auto& lightData = light.second->getData();
	//		const auto& position = lightData.getTransform().getWorldPosition();
	//		auto effectRange = lightData.getEffectRange();
	//		
	//		if (std::isinf(effectRange) || p_frustum.sphereInFrustum(position.x, position.y, position.z, lightData.getEffectRange())) {
	//			auto ldata = lightData.generateOGLStruct();
	//			result.push_back(ldata);
	//		}
	//	}
	//}
	return result;
}

//IKIGAI::ECS::Object& Scene::getSkybox() const {
//	return *skyboxObject;
//}
//
//IKIGAI::RESOURCES::CubeMap& Scene::getSkyboxTexture() const {
//	return *skyboxTexture;
//}

std::vector<std::shared_ptr<RENDER::MeshInterface>> Scene::getMeshesInFrustum(
	const RENDER::ModelInterface& model,
	const RENDER::BoundingSphere& modelBoundingSphere,
	const ECS::Transform& modelTransform,
	const RENDER::Frustum& frustum,
	RENDER::CullingOptions cullingOptions
) {
	const bool frustumPerModel = isFlagSet(RENDER::CullingOptions::FRUSTUM_PER_MODEL, cullingOptions);

	if (!frustumPerModel || frustum.boundingSphereInFrustum(modelBoundingSphere, modelTransform)) {
		std::vector<std::shared_ptr<RENDER::MeshInterface>> result;

		const bool frustumPerMesh = isFlagSet(RENDER::CullingOptions::FRUSTUM_PER_MESH, cullingOptions);

		auto& meshes = model.getMeshes();

		for (auto mesh : meshes) {
			if (meshes.size() == 1 || !frustumPerMesh || frustum.boundingSphereInFrustum(mesh->getBoundingSphere(), modelTransform)) {
				result.emplace_back(mesh);
			}
		}
		return result;
	}
	return {};
}

namespace {
	void MapChildrenToChunk(IKIGAI::ECS::Object* obj, size_t chunkIdx, std::unordered_map<IKIGAI::ECS2::Entity, size_t>& map, const std::unordered_map<IKIGAI::ECS2::Entity, size_t>& rtMap) {
		for (auto child : obj->getChildren()) {
			if (rtMap.contains(child->getID())) {
				continue; // child has its own RenderTargetComponent, skip it and its children
			}
			map[child->getID()] = chunkIdx;
			MapChildrenToChunk(child.get(), chunkIdx, map, rtMap);
		}
	}

	struct ChunkContext {
		std::vector<IKIGAI::RENDER::RenderChunk> chunks;
		std::unordered_map<IKIGAI::ECS2::Entity, size_t> objToChunk;

		void init() {
			chunks.push_back({nullptr, nullptr}); // Main chunk (index 0)

			auto cm = IKIGAI::RESOURCES::ServiceManager::Get<IKIGAI::ECS2::World>().getComponentManager();
			std::unordered_map<IKIGAI::ECS2::Entity, size_t> rtMap;

			for (auto& rt : cm->getComponentsArray<IKIGAI::ECS::RenderTargetComponent>()) {
				if (!rt.obj->getIsActive()) continue;

				chunks.push_back({rt.getFrameBuffer(), rt.getCamera()});
				size_t chunkIdx = chunks.size() - 1;

				rtMap[rt.obj->getID()] = chunkIdx;
				objToChunk[rt.obj->getID()] = chunkIdx;
			}

			for (auto& rt : cm->getComponentsArray<IKIGAI::ECS::RenderTargetComponent>()) {
				if (!rt.obj->getIsActive()) continue;
				if (rt.getType() == IKIGAI::ECS::RenderFlowType::NODE_AND_CHILD) {
					MapChildrenToChunk(&rt.obj.get(), rtMap[rt.obj->getID()], objToChunk, rtMap);
				}
			}
		}

		size_t getChunkIdxForObj(IKIGAI::ECS2::Entity id) const {
			auto it = objToChunk.find(id);
			if (it != objToChunk.end()) {
				return it->second;
			}
			return 0;
		}

		IKIGAI::RENDER::RenderChunk& getChunk(size_t idx) {
			return chunks[idx];
		}
	};
}

std::vector<IKIGAI::RENDER::RenderChunk> Scene::findAndSortFrustumCulledBVHDrawables
	(
		const MATH::Vector3f& cameraPosition,
		const RENDER::Frustum& frustum,
		std::shared_ptr<RENDER::MaterialInterface> defaultMaterial
	) {
	ChunkContext ctx;
	ctx.init();

	std::vector<std::shared_ptr<BVHData>> toDraw;
	meshesTree.GetNodeToDraw(toDraw, frustum);

	for (const auto& meshData : toDraw) {
		auto owner = meshData->obj;

		if (owner->getIsActive()) {
			if (auto materialRenderer = owner->getComponent<ECS::MaterialRenderer>()) {
				auto& transform = owner->getTransform()->getTransform();
				auto animator = owner->getComponent<ECS::Skeletal>();
				auto skeletonComp = owner->getComponent<ECS::SkeletalComponent>();
				auto animComp = owner->getComponent<ECS::SkeletalAnimationComponent>();
				std::shared_ptr<SKELETON::Skeleton> skeleton = skeletonComp ? skeletonComp->getSkeleton() : nullptr;
				SKELETON::IAnimationPlayable* animationPlayable = animComp ? animComp->getPlayable() : nullptr;
				SKELETON::AnimOffset* animOffset = skeletonComp ? skeletonComp->mAnimOffset.get() : nullptr;
				SKELETON::AnimLocalTransform* animLocalTransform = skeletonComp ? skeletonComp->mAnimLocalTransform.get() : nullptr;
				SKELETON::AnimGlobalTransform* animGlobalTransform = skeletonComp ? skeletonComp->mAnimGlobalTransform.get() : nullptr;

				size_t chunkIdx = ctx.getChunkIdxForObj(owner->getID());
				auto& chunk = ctx.getChunk(chunkIdx);
				
				MATH::Vector3f localCameraPos = cameraPosition;
				if (chunk.camera) {
					localCameraPos = chunk.camera->obj->mTransform->getWorldPosition();
				}

				float distanceToActor = MATH::Vector3f::Distance(transform.getWorldPosition(), localCameraPos);
				const ECS::MaterialRenderer::MaterialList& materials = materialRenderer->getMaterials();
					
				std::shared_ptr<RENDER::MaterialInterface> material;
				if (meshData->mesh->getMaterialIndex() < MAX_MATERIAL_COUNT) {
					material = materials.at(meshData->mesh->getMaterialIndex());
					if (!material || (!material->getShader() && !material->isDeferred())) {
						material = defaultMaterial;
					}
				}

				if (material) {
					RENDER::Drawable element = { transform.getPrevWorldMatrix(), transform.getWorldMatrix(), meshData->mesh, material, animator, skeleton, animationPlayable, animOffset, animLocalTransform, animGlobalTransform };
					transform.setPrevWorldMatrix(transform.getWorldMatrix());
					if (material->isBlendable()) {
						if (!material->isDeferred()) {
							chunk.transparentDrawablesForward.emplace(distanceToActor, element);
						}
						else {
							chunk.transparentDrawablesDeferred.emplace(distanceToActor, element);
						}
					}
					else {
						if (!material->isDeferred()) {
							chunk.opaqueDrawablesForward.emplace(distanceToActor, element);
						}
						else {
							chunk.opaqueDrawablesDeferred.emplace(distanceToActor, element);
						}
					}
				}
			}
		}
	}

	return std::move(ctx.chunks);
}

std::vector<IKIGAI::RENDER::RenderChunk> Scene::findAndSortFrustumCulledDrawables
(
	const MATH::Vector3f& cameraPosition,
	const RENDER::Frustum& frustum,
	std::shared_ptr<RENDER::MaterialInterface> defaultMaterial
) {
	ChunkContext ctx;
	ctx.init();

	auto cm = IKIGAI::RESOURCES::ServiceManager::Get<IKIGAI::ECS2::World>().getComponentManager();
	for (const auto& modelRenderer : cm->getComponentsArray<ECS::ModelRenderer>()) {
		auto owner = modelRenderer.obj;

		if (owner->getIsActive()) {
			if (auto model = modelRenderer.getModel()) {
				if (auto materialRenderer = modelRenderer.obj.get().getComponent<ECS::MaterialRenderer>()) {
					auto& transform = owner->getTransform()->getTransform();
					auto animator = modelRenderer.obj.get().getComponent<ECS::Skeletal>();
					auto skeletonComp = modelRenderer.obj.get().getComponent<ECS::SkeletalComponent>();
					auto animComp = modelRenderer.obj.get().getComponent<ECS::SkeletalAnimationComponent>();
					std::shared_ptr<SKELETON::Skeleton> skeleton = skeletonComp ? skeletonComp->getSkeleton() : nullptr;
					SKELETON::IAnimationPlayable* animationPlayable = animComp ? animComp->getPlayable() : nullptr;
					SKELETON::AnimOffset* animOffset = skeletonComp ? skeletonComp->mAnimOffset.get() : nullptr;
					SKELETON::AnimLocalTransform* animLocalTransform = skeletonComp ? skeletonComp->mAnimLocalTransform.get() : nullptr;
					SKELETON::AnimGlobalTransform* animGlobalTransform = skeletonComp ? skeletonComp->mAnimGlobalTransform.get() : nullptr;

					RENDER::CullingOptions cullingOptions = RENDER::CullingOptions::NONE;

					if (modelRenderer.getFrustumBehaviour() != ECS::EFrustumBehaviour::DISABLED) {
						cullingOptions |= RENDER::CullingOptions::FRUSTUM_PER_MODEL;
					}

					if (modelRenderer.getFrustumBehaviour() == ECS::EFrustumBehaviour::CULL_MESHES) {
						cullingOptions |= RENDER::CullingOptions::FRUSTUM_PER_MESH;
					}

					const auto& modelBoundingSphere = modelRenderer.getFrustumBehaviour() == ECS::EFrustumBehaviour::CULL_CUSTOM ? modelRenderer.getCustomBoundingSphere() : model->getBoundingSphere();
					
					auto meshes = getMeshesInFrustum(*model, modelBoundingSphere, transform, frustum, cullingOptions);

					if (!meshes.empty()) {
						size_t chunkIdx = ctx.getChunkIdxForObj(owner->getID());
						auto& chunk = ctx.getChunk(chunkIdx);
						
						MATH::Vector3f localCameraPos = cameraPosition;
						if (chunk.camera) {
							localCameraPos = chunk.camera->obj->mTransform->getWorldPosition();
						}

						float distanceToActor = MATH::Vector3f::Distance(transform.getWorldPosition(), localCameraPos);
						const ECS::MaterialRenderer::MaterialList& materials = materialRenderer->getMaterials();

						for (const auto mesh : meshes) {
							std::shared_ptr<RENDER::MaterialInterface> material;
							if (mesh->getMaterialIndex() < MAX_MATERIAL_COUNT) {
								material = materials.at(mesh->getMaterialIndex());
								if (!material || (!material->getShader() && !material->isDeferred())) {
									material = defaultMaterial;
								}
							}

							if (material) {
								RENDER::Drawable element = { transform.getPrevWorldMatrix(), transform.getWorldMatrix(), mesh, material, animator, skeleton, animationPlayable, animOffset, animLocalTransform, animGlobalTransform };
								transform.setPrevWorldMatrix(transform.getWorldMatrix());
								if (material->isBlendable()) {
									if (!material->isDeferred()) {
										chunk.transparentDrawablesForward.emplace(distanceToActor, element);
									}
									else {
										chunk.transparentDrawablesDeferred.emplace(distanceToActor, element);
									}
								}
								else {
									if (!material->isDeferred()) {
										chunk.opaqueDrawablesForward.emplace(distanceToActor, element);
									}
									else {
										chunk.opaqueDrawablesDeferred.emplace(distanceToActor, element);
									}
								}
							}
						}
					}
				}
			}
		}
	}

	return std::move(ctx.chunks);
}

std::vector<IKIGAI::RENDER::RenderChunk> Scene::findAndSortDrawables
(
	const MATH::Vector3f& cameraPosition,
	std::shared_ptr<RENDER::MaterialInterface> defaultMaterial
) {
	ChunkContext ctx;
	ctx.init();

	auto cm = IKIGAI::RESOURCES::ServiceManager::Get<IKIGAI::ECS2::World>().getComponentManager();
	for (auto& modelRenderer : cm->getComponentsArray<ECS::ModelRenderer>()) {
		if (modelRenderer.obj->getIsActive() && modelRenderer.obj->getName() != "Skybox") {
			if (auto model = modelRenderer.getModel()) {
				if (auto materialRenderer = modelRenderer.obj->getComponent<ECS::MaterialRenderer>()) {
					auto& transform = modelRenderer.obj->getTransform()->getTransform();
					auto animator = modelRenderer.obj->getComponent<ECS::Skeletal>();
					auto skeletonComp = modelRenderer.obj->getComponent<ECS::SkeletalComponent>();
					auto animComp = modelRenderer.obj->getComponent<ECS::SkeletalAnimationComponent>();
					std::shared_ptr<SKELETON::Skeleton> skeleton = skeletonComp ? skeletonComp->getSkeleton() : nullptr;
					SKELETON::IAnimationPlayable* animationPlayable = animComp ? animComp->getPlayable() : nullptr;
					SKELETON::AnimOffset* animOffset = skeletonComp ? skeletonComp->mAnimOffset.get() : nullptr;
					SKELETON::AnimLocalTransform* animLocalTransform = skeletonComp ? skeletonComp->mAnimLocalTransform.get() : nullptr;
					SKELETON::AnimGlobalTransform* animGlobalTransform = skeletonComp ? skeletonComp->mAnimGlobalTransform.get() : nullptr;

					size_t chunkIdx = ctx.getChunkIdxForObj(modelRenderer.obj->getID());
					auto& chunk = ctx.getChunk(chunkIdx);
					
					MATH::Vector3f localCameraPos = cameraPosition;
					if (chunk.camera) {
						localCameraPos = chunk.camera->obj->mTransform->getWorldPosition();
					}

					float distanceToActor = MATH::Vector3f::Distance(transform.getWorldPosition(), localCameraPos);

					const ECS::MaterialRenderer::MaterialList& materials = materialRenderer->getMaterials();

					for (const auto mesh : model->getMeshes()) {
						std::shared_ptr<RENDER::MaterialInterface> material;
						if (mesh->getMaterialIndex() < MAX_MATERIAL_COUNT) {
							material = materials.at(mesh->getMaterialIndex());
							if (!material || (!material->getShader() && !material->isDeferred())) {
								material = materials.at(0);
							}
						}

						if (material) {
							RENDER::Drawable element = { transform.getPrevWorldMatrix(),transform.getWorldMatrix(), mesh, material, animator, skeleton, animationPlayable, animOffset, animLocalTransform, animGlobalTransform };
							transform.setPrevWorldMatrix(transform.getWorldMatrix());
							if (material->isBlendable()) {
								if (!material->isDeferred()) {
									chunk.transparentDrawablesForward.emplace(distanceToActor, element);
								}
								else {
									chunk.transparentDrawablesDeferred.emplace(distanceToActor, element);
								}
							}
							else {
								if (!material->isDeferred()) {
									chunk.opaqueDrawablesForward.emplace(distanceToActor, element);
								}
								else {
									chunk.opaqueDrawablesDeferred.emplace(distanceToActor, element);
								}
							}
						}
					}
				}
			}
		}
	}

	//TODO: refactor it
	for (auto& modelRenderer : cm->getComponentsArray<ECS::ModelLODRenderer>()) {
		if (modelRenderer.obj->getIsActive()) {
			size_t chunkIdx = ctx.getChunkIdxForObj(modelRenderer.obj->getID());
			auto& chunk = ctx.getChunk(chunkIdx);
			
			MATH::Vector3f localCameraPos = cameraPosition;
			if (chunk.camera) {
				localCameraPos = chunk.camera->obj->mTransform->getWorldPosition();
			}

			float distanceToActor = MATH::Vector3f::Distance(modelRenderer.obj->getTransform()->getWorldPosition(), localCameraPos);
			if (auto model = modelRenderer.getModelByDistance(distanceToActor)) {
				if (auto materialRenderer = modelRenderer.obj->getComponent<ECS::MaterialRenderer>()) {
					auto& transform = modelRenderer.obj->getTransform()->getTransform();
					auto animator = modelRenderer.obj->getComponent<ECS::Skeletal>();

					const ECS::MaterialRenderer::MaterialList& materials = materialRenderer->getMaterials();

					for (const auto mesh : model->getMeshes()) {
						std::shared_ptr<RENDER::MaterialInterface> material;
						if (mesh->getMaterialIndex() < MAX_MATERIAL_COUNT) {
							material = materials.at(mesh->getMaterialIndex());
							if (!material || (!material->getShader() && !material->isDeferred())) {
								material = defaultMaterial;
							}
						}

						if (material) {
							RENDER::Drawable element = { transform.getPrevWorldMatrix(),transform.getWorldMatrix(), mesh, material, animator };
							transform.setPrevWorldMatrix(transform.getWorldMatrix());
							if (material->isBlendable()) {
								if (!material->isDeferred()) {
									chunk.transparentDrawablesForward.emplace(distanceToActor, element);
								}
								else {
									chunk.transparentDrawablesDeferred.emplace(distanceToActor, element);
								}
							}
							else {
								if (!material->isDeferred()) {
									chunk.opaqueDrawablesForward.emplace(distanceToActor, element);
								}
								else {
									chunk.opaqueDrawablesDeferred.emplace(distanceToActor, element);
								}
							}
						}
					}
				}
			}
		}
	}

	return std::move(ctx.chunks);
}


std::vector<IKIGAI::RENDER::RenderChunk> Scene::findDrawables(
	const MATH::Vector3f& cameraPosition,
	const RENDER::Camera& camera,
	const RENDER::Frustum* customFrustum,
	std::shared_ptr<RENDER::MaterialInterface> defaultMaterial
) {
	if (camera.isFrustumGeometryBVHCulling()) {
		const auto& frustum = customFrustum ? *customFrustum : camera.getFrustum();
		return findAndSortFrustumCulledBVHDrawables(cameraPosition, frustum, defaultMaterial);
	}
	else if (camera.isFrustumGeometryCulling()) {
		const auto& frustum = customFrustum ? *customFrustum : camera.getFrustum();
		return findAndSortFrustumCulledDrawables(cameraPosition, frustum, defaultMaterial);
	}
	else {
		return findAndSortDrawables(cameraPosition, defaultMaterial);
	}
}
