#pragma once

#include <utilsModule/idGenerator.h>

#include "mathModule/math.h"

namespace IKIGAI
{
	namespace ECS
	{
		class Transform;
		class Object;
	}
}

namespace IKIGAI {
	namespace RENDER {
		
		struct LightOGL {
			float pos[3];
			float cutoff;
			float forward[3];
			float outerCutoff;
			float color[3];
			float constant;
			int type;
			float linear;
			float quadratic;
			float intensity;
#ifdef OPENGL_BACKEND
			float radius;
			float padding[3];
#endif
		};
		
		struct Light {
			enum class Type { NONE, POINT, DIRECTIONAL, SPOT, AMBIENT_BOX, AMBIENT_SPHERE, INPUT };

			Light(Id<ECS::Object> objId, Type p_type = Type::NONE);
			[[nodiscard]] LightOGL generateOGLStruct() const;
			[[nodiscard]] float getEffectRange() const;
			[[nodiscard]] const ECS::Transform& getTransform() const;

			MATH::Vector3f color = {1.f, 1.f, 1.f};
			float intensity = 1.f;
			float constant = 0.0f;
			float linear = 0.0f;
			float quadratic = 1.0f;
			float cutoff = 12.f;
			float outerCutoff = 15.f;
			Type type = Type::NONE;

		protected:
			Id<ECS::Object> objId;
		};
		
	}
}
