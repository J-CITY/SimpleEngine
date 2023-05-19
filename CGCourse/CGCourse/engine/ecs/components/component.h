#pragma once

#include <chrono>
#include <string>

#include "../../resourceManager/serializerInterface.h"
#include "../../utils/pointers/objPtr.h"
#include "../../utils/structures.h"

namespace KUMA::ECS { class Object; }

namespace KUMA::ECS {

	class Component: public RESOURCES::Serializable, public UTILS::ControlBlockHandler {
		UTILS::ControlBlock* mCb = nullptr;
	public:
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

		const ECS::Object& getObject();

		//make it private
		Ref<ECS::Object> obj;

		virtual void onDeserialize(nlohmann::json& j) override {
		}
		virtual void onSerialize(nlohmann::json& j) override {;
		}

		template<class T>
		UTILS::WeakPtr<T> getWeak() {
			return UTILS::WeakPtr<T>(*this);
		}

		UTILS::ControlBlock* getControlBlock();
	protected:
		std::string __NAME__;
	};
}