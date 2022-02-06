#pragma once

#include <string>
#include <vector>

#include "../ecs/object.h"

namespace KUMA::SCENE_SYSTEM {
	class  Scene: public RESOURCES::Serializable {
	public:
		class IdGenerator: public ObjectIdGenerator<ECS::Object> {
		};
		Scene() = default;
		~Scene();

		void init();
		void go();
		bool getIsExecute() const;
		
		void update(float dt);
		void fixedUpdate(float dt);
		void lateUpdate(float dt);

		ECS::Object& createObject();
		ECS::Object& createObject(const std::string& p_name, const std::string& p_tag = "");

		bool destroyActor(ECS::Object& p_target);

		ECS::Object* findActorByName(const std::string& p_name);
		ECS::Object* findActorByTag(const std::string& p_tag);
		std::shared_ptr<ECS::Object> findActorByID(ObjectId<ECS::Object> p_id);

		std::vector<std::reference_wrapper<ECS::Object>> findActorsByName(const std::string& p_name);
		std::vector<std::reference_wrapper<ECS::Object>> findActorsByTag(const std::string& p_tag);

		
		void onComponentAdded(std::shared_ptr<ECS::Component> p_compononent);
		void onComponentRemoved(std::shared_ptr<ECS::Component> p_compononent);

		std::vector<std::shared_ptr<ECS::Object>>& getObjects();

		virtual void onDeserialize(nlohmann::json& j) override {
			for (auto& oj : j["objects"]) {
				auto obj = createObject();
				obj.onDeserialize(oj);
			}
			for (auto& oj : j["objects"]) {
				auto obj = findActorByID(ObjectId<ECS::Object>(oj["id"]));
				auto parent = findActorByID(ObjectId<ECS::Object>(oj["parent"]));
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

	private:
		IdGenerator idGenerator;
		
		bool isExecute = false;
		std::vector<std::shared_ptr<ECS::Object>> objects;
		std::unique_ptr<ECS::Object> skyboxObject;
		std::unique_ptr<RESOURCES::CubeMap> skyboxTexture;
	};
};
