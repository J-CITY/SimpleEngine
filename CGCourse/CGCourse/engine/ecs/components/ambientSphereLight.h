#pragma once

#include "lightComponent.h"

namespace KUMA::ECS { class Object; }

namespace KUMA::ECS {
	class AmbientSphereLight : public LightComponent {
	public:
		AmbientSphereLight(Ref<ECS::Object> p_owner);

		float getRadius() const;
		void setRadius(float p_radius);

		virtual void onDeserialize(nlohmann::json& j) override {
			LightComponent::onDeserialize(j);
			setRadius(j["radius"]);
		}
		virtual void onSerialize(nlohmann::json& j) override {
			LightComponent::onSerialize(j);
			j["radius"] = getRadius();
		}
	};

}