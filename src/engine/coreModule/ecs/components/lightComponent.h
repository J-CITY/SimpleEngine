#pragma once

//#include <glm/glm.hpp>

#include "component.h"
//#include "../../../game/TextureAtlas.h"
#include <renderModule/light.h>

namespace IKIGAI::ECS { class Object; }

namespace IKIGAI::ECS {
	class LightComponent : public Component
	{
	public:
		LightComponent(UTILS::Ref<ECS::Object> obj);
		const RENDER::Light& getData() const;
		const MATH::Vector3f& getColor() const;
		float getIntensity() const;
		void setColor(const MATH::Vector3f& color);
		void setIntensity(float intensity);

		MATH::Matrix4f Transform;
	protected:
		RENDER::Light data;
	};
}