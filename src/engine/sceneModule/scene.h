#pragma once

#include <string>
#include <vector>

#include <coreModule/ecs/object.h>
#include <renderModule/drawable.h>

namespace IKIGAI::GUI {
	class GuiObject;
}

namespace IKIGAI::RENDER {
	class Camera;
	class Frustum;
	class Model;
	enum class CullingOptions;
	struct LightOGL;
	struct Drawable;
}

namespace IKIGAI::SCENE_SYSTEM {
	class  Scene: public RESOURCES::Serializable {
	public:
		class IdGenerator: public ObjectIdGenerator<ECS::Object> {};

		Scene() = default;
		virtual ~Scene();

		void init();
		void go();
		[[nodiscard]] bool getIsExecute() const;
		
		void update(std::chrono::duration<double> dt);
		void fixedUpdate(std::chrono::duration<double> dt);
		void lateUpdate(std::chrono::duration<double> dt);

		std::shared_ptr<ECS::Object> createObject();
		std::shared_ptr<ECS::Object> createObject(const std::string& p_name, const std::string& p_tag = "");
		std::shared_ptr<ECS::Object> createObjectAfter(ECS::Object::Id parentId, const std::string& p_name, const std::string& p_tag = "");
		std::shared_ptr<ECS::Object> createObjectBefore(ECS::Object::Id parentId, const std::string& p_name, const std::string& p_tag = "");
		//std::shared_ptr<ECS::Object> _createObject(const std::string& p_name, const std::string& p_tag = "");
		std::shared_ptr<ECS::Object> createObject(ObjectId<ECS::Object> actorID, const std::string& name, const std::string& tag = "");
		std::shared_ptr<ECS::Object> createObject(const ECS::ObjectData& data);
		bool destroyObject(std::shared_ptr<ECS::Object> p_target);

		std::shared_ptr<ECS::Object> findObjectByName(const std::string& p_name);
		std::shared_ptr<ECS::Object> findObjectByTag(const std::string& p_tag);
		std::shared_ptr<ECS::Object> findObjectByID(ObjectId<ECS::Object> p_id);

		std::vector<std::shared_ptr<ECS::Object>> findObjectsByName(const std::string& p_name);
		std::vector<std::shared_ptr<ECS::Object>> findObjectsByTag(const std::string& p_tag);

		[[nodiscard]] std::span<std::shared_ptr<ECS::Object>> getObjects();

		virtual void onDeserialize(nlohmann::json& j) override {
			//for (auto& oj : j["objects"]) {
			//	auto obj = createObject();
			//	obj->onDeserialize(oj);
			//}
			//for (auto& oj : j["objects"]) {
			//	auto obj = findObjectByID(ObjectId<ECS::Object>(oj["id"]));
			//	auto parent = findObjectByID(ObjectId<ECS::Object>(oj["parent"]));
			//	obj->setParent(parent);
			//}
		}
		virtual void onSerialize(nlohmann::json& j) override {
		//	for (auto& obj : objects) {
		//		obj->onSerialize(j["objects"]);
		//	}
		}

		//[[nodiscard]] ECS::Object& getSkybox() const;
		//[[nodiscard]] RESOURCES::CubeMap& getSkyboxTexture() const;

		std::vector<std::shared_ptr<RENDER::MeshInterface>> getMeshesInFrustum(
			const RENDER::ModelInterface& model,
			const RENDER::BoundingSphere& modelBoundingSphere,
			const ECS::Transform& modelTransform,
			const RENDER::Frustum& frustum,
			RENDER::CullingOptions cullingOptions
		);

		std::tuple<IKIGAI::RENDER::OpaqueDrawables,
			IKIGAI::RENDER::TransparentDrawables,
			IKIGAI::RENDER::OpaqueDrawables,
			IKIGAI::RENDER::TransparentDrawables>  findDrawables(const MATHGL::Vector3& p_cameraPosition,
			const RENDER::Camera& p_camera,
			const RENDER::Frustum* p_customFrustum,
			std::shared_ptr<RENDER::MaterialInterface> p_defaultMaterial
		);

		std::tuple<IKIGAI::RENDER::OpaqueDrawables,
			IKIGAI::RENDER::TransparentDrawables,
			IKIGAI::RENDER::OpaqueDrawables,
			IKIGAI::RENDER::TransparentDrawables>  findAndSortFrustumCulledBVHDrawables(
				const MATHGL::Vector3& cameraPosition,
				const RENDER::Frustum& frustum,
				std::shared_ptr<RENDER::MaterialInterface> defaultMaterial
			);

		std::tuple<IKIGAI::RENDER::OpaqueDrawables,
			IKIGAI::RENDER::TransparentDrawables,
			IKIGAI::RENDER::OpaqueDrawables,
			IKIGAI::RENDER::TransparentDrawables>  findAndSortFrustumCulledDrawables(
			const MATHGL::Vector3& cameraPosition,
			const RENDER::Frustum& frustum,
			std::shared_ptr<RENDER::MaterialInterface> defaultMaterial
		);

		std::tuple<IKIGAI::RENDER::OpaqueDrawables,
			IKIGAI::RENDER::TransparentDrawables,
			IKIGAI::RENDER::OpaqueDrawables,
			IKIGAI::RENDER::TransparentDrawables>  findAndSortDrawables(
			const MATHGL::Vector3& cameraPosition,
			std::shared_ptr<RENDER::MaterialInterface> defaultMaterial
		);

		std::optional<UTILS::Ref<IKIGAI::ECS::CameraComponent>> findMainCamera();
		std::vector<RENDER::LightOGL> findLightData();
		std::vector<RENDER::LightOGL> findLightDataInFrustum(const RENDER::Frustum& frustum);


		std::vector<std::shared_ptr<IKIGAI::GUI::GuiObject>> guiObjs;


		void addToBVH(UTILS::WeakPtr<ECS::Component> component);
		void removeFromBVH(UTILS::WeakPtr<ECS::Component> component);
		void updateInBVH(UTILS::WeakPtr<ECS::Component> component);

		bool isSceneReady = false;
		void postLoad();
	private:
		IdGenerator idGenerator;
		
		bool isExecute = false;
		std::vector<std::shared_ptr<ECS::Object>> objects;

		//std::unique_ptr<ECS::Object> skyboxObject;
		//std::unique_ptr<RESOURCES::CubeMap> skyboxTexture;
	};
};
