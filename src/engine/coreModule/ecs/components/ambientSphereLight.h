#pragma once

#include "lightComponent.h"

namespace IKIGAI::ECS { class Object; }

namespace IKIGAI::ECS {
	class AmbientSphereLight : public LightComponent {
	public:
		AmbientSphereLight(UTILS::Ref<ECS::Object> p_owner);

		float getRadius() const;
		void setRadius(float p_radius);
		
	};

}