#pragma once

#include <utilsModule/idGenerator.h>

#include "ecsModule/entityManager.h"
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
		
		// WebGL2 UBOs cannot hold unsized arrays; keep in sync with
		// kEsSsboRuntimeArraySize in shaderInterface.cpp.
		inline constexpr size_t MAX_LIGHTS = 64;

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
			float radius;
			int shadowEnabled;
			float shadowNear;
			float shadowFar;

			// generateOGLStruct stores Type as (int)type - 1, so NONE is -1.
			static LightOGL Inactive() {
				LightOGL light{};
				light.type = -1;
				return light;
			}
		};
		static_assert(sizeof(LightOGL) == 80, "Light ABI must match std430/std140 shader layout");
		
		struct Light {
			enum class Type { NONE, POINT, DIRECTIONAL, SPOT, AMBIENT_BOX, AMBIENT_SPHERE, INPUT };

			Light(ECS2::Entity objId, Type p_type = Type::NONE);
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
			bool castShadows = true;
			float shadowNear = -1.0f;
			float shadowFar = -1.0f;

		protected:
			ECS2::Entity objId;
		};
		
	}
}
