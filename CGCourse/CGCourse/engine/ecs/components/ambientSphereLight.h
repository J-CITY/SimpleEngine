#pragma once

#include "lightComponent.h"

namespace KUMA::ECS { class Object; }

namespace KUMA::ECS {
	class AmbientSphereLight : public LightComponent {
	public:
		AmbientSphereLight(ECS::Object& p_owner);

		float getRadius() const;
		void setRadius(float p_radius);

		virtual void onDeserialize(nlohmann::json& j) override {
			LightComponent::onDeserialize(j);
			setRadius(j["data"]["radius"]);
		}
		virtual void onSerialize(nlohmann::json& j) override {
			LightComponent::onSerialize(j);
			j["data"]["radius"] = getRadius();
		}
	};

}