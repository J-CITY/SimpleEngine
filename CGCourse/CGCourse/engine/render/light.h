#pragma once

import glmath;
#include "../ecs/components/transform.h"
#include "../utils/idObject.h"
namespace KUMA {
	namespace RENDER {

		struct LightOGL {
			float pos[3];
			float forward[3];
			float color[3];
			int type;
			float cutoff;
			float outerCutoff;
			float constant;
			float linear;
			float quadratic;
			float intensity;
		};
		
		struct Light {
			enum class Type { NONE, POINT, DIRECTIONAL, SPOT, AMBIENT_BOX, AMBIENT_SPHERE, INPUT };

			Light(ObjectId<ECS::Object> objId, Type p_type = Type::NONE);
			[[nodiscard]] LightOGL generateOGLStruct() const;
			[[nodiscard]] float getEffectRange() const;
			[[nodiscard]] const ECS::Transform& getTransform() const;

			MATHGL::Vector3	color = {1.f, 1.f, 1.f};
			float intensity = 1.f;
			float constant = 0.0f;
			float linear = 0.0f;
			float quadratic = 1.0f;
			float cutoff = 12.f;
			float outerCutoff = 15.f;
			Type type = Type::NONE;

		protected:
			ObjectId<ECS::Object> objId;
		};
	}
}
