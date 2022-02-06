#pragma once

#include <string>

#include "../../resourceManager/serializerInterface.h"

namespace KUMA::ECS { class Object; }

namespace KUMA::ECS {
	class Component: public RESOURCES::Serializable {
	public:
		Component(const ECS::Object& obj);
		virtual ~Component();

		virtual void onAwake() {}
		virtual void onStart() {}
		
		virtual void onEnable() {}
		virtual void onDisable() {}

		virtual void onDestroy() {}

		virtual void onUpdate(float dt) {}
		virtual void onFixedUpdate(float dt) {}
		virtual void onLateUpdate(float dt) {}

		virtual std::string getName();

		const ECS::Object& getObject();

		const ECS::Object& obj;

		virtual void onDeserialize(nlohmann::json& j) override {
		}
		virtual void onSerialize(nlohmann::json& j) override {;
		}
	protected:
		std::string __NAME__ = "";
	};
}