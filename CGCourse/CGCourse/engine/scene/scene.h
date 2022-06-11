#pragma once

#include <string>
#include <vector>

#include "../ecs/object.h"
#include "../render/drawable.h"

namespace KUMA
{
	namespace GUI
	{
		class GuiObject;
	}
}

namespace KUMA::RENDER {
	enum class CullingOptions;
	struct LightOGL;
	struct Drawable;
}

namespace KUMA::SCENE_SYSTEM {
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

		bool destroyObject(std::shared_ptr<ECS::Object> p_target);

		std::shared_ptr<ECS::Object> findObjectByName(const std::string& p_name);
		std::shared_ptr<ECS::Object> findObjectByTag(const std::string& p_tag);
		std::shared_ptr<ECS::Object> findObjectByID(ObjectId<ECS::Object> p_id);

		std::vector<std::shared_ptr<ECS::Object>> findObjectsByName(const std::string& p_name);
		std::vector<std::shared_ptr<ECS::Object>> findObjectsByTag(const std::string& p_tag);

		const std::vector<std::shared_ptr<ECS::Object>>& getObjects() const;

		virtual void onDeserialize(nlohmann::json& j) override {
			for (auto& oj : j["objects"]) {
				auto obj = createObject();
				obj->onDeserialize(oj);
			}
			for (auto& oj : j["objects"]) {
				auto obj = findObjectByID(ObjectId<ECS::Object>(oj["id"]));
				auto parent = findObjectByID(ObjectId<ECS::Object>(oj["parent"]));
				obj->setParent(parent);
			}
		}
		virtual void onSerialize(nlohmann::json& j) override {
			for (auto& obj : objects) {
				obj->onSerialize(j["objects"]);
			}
		}

		ECS::Object& getSkybox() {
			return *skyboxObject;
		}

		RESOURCES::CubeMap& getSkyboxTexture() {
			return *skyboxTexture;
		}

		std::vector<std::reference_wrapper<KUMA::RESOURCES::Mesh>> Scene::getMeshesInFrustum(
			const RENDER::Model& model,
			const RENDER::BoundingSphere& modelBoundingSphere,
			const ECS::Transform& modelTransform,
			const RENDER::Frustum& frustum,
			RENDER::CullingOptions cullingOptions
		);

		std::tuple<KUMA::RENDER::OpaqueDrawables,
			KUMA::RENDER::TransparentDrawables,
			KUMA::RENDER::OpaqueDrawables,
			KUMA::RENDER::TransparentDrawables>  findDrawables(const MATHGL::Vector3& p_cameraPosition,
			const RENDER::Camera& p_camera,
			const RENDER::Frustum* p_customFrustum,
			std::shared_ptr<RENDER::Material> p_defaultMaterial
		);

		std::tuple<KUMA::RENDER::OpaqueDrawables,
			KUMA::RENDER::TransparentDrawables,
			KUMA::RENDER::OpaqueDrawables,
			KUMA::RENDER::TransparentDrawables>  findAndSortFrustumCulledDrawables(
			const MATHGL::Vector3& cameraPosition,
			const RENDER::Frustum& frustum,
			std::shared_ptr<RENDER::Material> defaultMaterial
		);

		std::tuple<KUMA::RENDER::OpaqueDrawables,
			KUMA::RENDER::TransparentDrawables,
			KUMA::RENDER::OpaqueDrawables,
			KUMA::RENDER::TransparentDrawables>  findAndSortDrawables(
			const MATHGL::Vector3& cameraPosition,
			std::shared_ptr<RENDER::Material> defaultMaterial
		);

		std::shared_ptr<ECS::CameraComponent> findMainCamera();
		std::vector<RENDER::LightOGL> findLightData();
		std::vector<RENDER::LightOGL> findLightDataInFrustum(const RENDER::Frustum& frustum);


		std::vector<std::shared_ptr<KUMA::GUI::GuiObject>> guiObjs;
	private:
		IdGenerator idGenerator;
		
		bool isExecute = false;
		std::vector<std::shared_ptr<ECS::Object>> objects;

		std::unique_ptr<ECS::Object> skyboxObject;
		std::unique_ptr<RESOURCES::CubeMap> skyboxTexture;
	};
};
