#pragma once

#include <chrono>
#include <result.h>
#include <string>

#include "../../resourceManager/serializerInterface.h"
#include <utilsModule/pointers/objPtr.h>
#include <utilsModule/weakPtr.h>

#include "utilsModule/jsonParser/jsonParser.h"

namespace IKIGAI::ECS { class Object; }

namespace IKIGAI::ECS {

	class Component: public RESOURCES::Serializable, public UTILS::ControlBlockHandler {
		template<class T>
		friend Result<T, UTILS::JsonError> FromJson(nlohmann::json& data);
	public:
		//Component();
		Component(Ref<ECS::Object> obj);
		virtual ~Component();

		virtual void onAwake() {}
		virtual void onStart() {}
		
		virtual void onEnable() {}
		virtual void onDisable() {}

		virtual void onDestroy() {}

		virtual void onUpdate(std::chrono::duration<double> dt) {}
		virtual void onFixedUpdate(std::chrono::duration<double> dt) {}
		virtual void onLateUpdate(std::chrono::duration<double> dt) {}

		virtual std::string getName();
		virtual std::string getTypeidName();

		const ECS::Object& getObject();

		//make it private
		//TODO: use weak_ptr
		Ref<ECS::Object> obj;

		virtual void onDeserialize(nlohmann::json& j) override {
		}
		virtual void onSerialize(nlohmann::json& j) override {;
		}

		template<class T>
		UTILS::WeakPtr<T> getWeak() {
			return UTILS::WeakPtr<T>(*this);
		}
	protected:
		std::string __NAME__;
	};
}