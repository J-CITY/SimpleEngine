#include "skeletalStateGraphSystem.h"
#include "coreModule/ecs/components/skeletalStateGraphComponent.h"
#include "sceneModule/scene.h"
namespace IKIGAI::ECS {
void SkeletalStateGraphSystem::update(SCENE_SYSTEM::Scene& scene, float dt) {
	for (auto& o : scene.getObjects()) {
		if (o->getIsActive()) {
			if (auto c = o->getComponent<SkeletalStateGraphComponent>()) {
				c->update(scene, dt);
			}
		}
	}
}
}
