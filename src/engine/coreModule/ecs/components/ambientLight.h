#pragma once

#include "directionalLight.h"
import glmath;

namespace IKIGAI::ECS { class Object; }

namespace IKIGAI::ECS {
	class AmbientLight: public LightComponent {
	public:
		AmbientLight(Ref<ECS::Object> p_owner);
		MATHGL::Vector3 getSize() const;
		void setSize(const MATHGL::Vector3& val);
		
	};
}
