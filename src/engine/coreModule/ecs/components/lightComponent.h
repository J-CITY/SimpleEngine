#pragma once

//#include <glm/glm.hpp>

#include "component.h"
//#include "../../../game/TextureAtlas.h"
#include <renderModule/light.h>
import glmath;

namespace IKIGAI::ECS { class Object; }

namespace IKIGAI::ECS {
	class LightComponent : public Component
	{
	public:
		LightComponent(UTILS::Ref<ECS::Object> obj);
		const RENDER::Light& getData() const;
		const MATHGL::Vector3& getColor() const;
		float getIntensity() const;
		void setColor(const MATHGL::Vector3& color);
		void setIntensity(float intensity);

		MATHGL::Matrix4 Transform;
	protected:
		RENDER::Light data;
	};
}