#include "physicsModule/features.h"
#if !USE_PHYSICS_3D
// Public integration headers must not pull in ECS or physics when disabled.
#include "coreModule/ecs/components/physics3dComponent.h"
#include "coreModule/ecs/components/physicsComponent.h"
#include "coreModule/ecs/systems/physics3dSystem.h"
#include "renderModule/physics/deformableRenderer.h"
#include "coreModule/ecs/components/capsuleColliderComponent.h"
#include "coreModule/ecs/components/characterMotorComponent.h"
#include "coreModule/ecs/components/thirdPersonCameraComponent.h"
#include "coreModule/ecs/components/traversalComponent.h"
#include "coreModule/ecs/systems/characterSystem.h"
#include "coreModule/ecs/systems/traversalSystem.h"
#endif
#include <iostream>

int main() {
	try {
		using IKIGAI::PHYSICS::validateFeatures;
		validateFeatures(nlohmann::json { { "Objects", nlohmann::json::array() } });
		validateFeatures(nlohmann::json { { "Physics2DComponentType", "test" } });
		for (const char* key : { "Physics3DComponentType", "SoftBody3DComponentType", "Cloth3DComponentType", "Fluid3DComponentType",
			 "ForceField3DComponentType", "PhysicsComponentType", "ThirdPersonControllerType", "CapsuleColliderComponentType",
			 "CharacterMotorComponentType", "ThirdPersonCameraComponentType", "TraversalComponentType" }) {
			const auto component = nlohmann::json { { key, "test" } };
			const auto nested
				= nlohmann::json { { "Objects", nlohmann::json::array({ { { "Components", nlohmann::json::array({ component }) } } }) } };
			for (const auto& document : { component, nested }) {
				bool rejected = false;
				try {
					validateFeatures(document);
				} catch (const std::runtime_error& e) {
					if (std::string(e.what()).find("USE_PHYSICS_3D=OFF") == std::string::npos) {
						throw;
					}
					rejected = true;
				}
				if (rejected != !USE_PHYSICS_3D) {
					throw std::runtime_error("3D feature validation did not match the build flag");
				}
			}
		}
		std::cout << "3D feature diagnostics passed (USE_PHYSICS_3D=" << USE_PHYSICS_3D << ")\n";
		return 0;
	} catch (const std::exception& e) {
		std::cerr << e.what() << '\n';
		return 1;
	}
}
