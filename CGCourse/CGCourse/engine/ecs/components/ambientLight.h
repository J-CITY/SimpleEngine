#pragma once

#include "directionalLight.h"
import glmath;

namespace KUMA::ECS { class Object; }

namespace KUMA::ECS {
	class AmbientLight: public LightComponent {
	public:
		AmbientLight(const ECS::Object& p_owner);
		MATHGL::Vector3 getSize() const;
		void setSize(const MATHGL::Vector3& val);

		virtual void onDeserialize(nlohmann::json& j) override {
			LightComponent::onDeserialize(j);
			data.constant = j["data"]["constant"];
			data.linear = j["data"]["linear"];
			data.quadratic = j["data"]["quadratic"];
		}
		virtual void onSerialize(nlohmann::json& j) override {
			LightComponent::onSerialize(j);
			j["data"]["constant"] = data.constant;
			j["data"]["linear"] = data.linear;
			j["data"]["quadratic"] = data.quadratic;
		}
	};
}
