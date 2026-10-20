#include "variableContextSystem.h"
#include "coreModule/ecs/components/variableContextComponent.h"
#include "sceneModule/scene.h"
namespace IKIGAI::ECS {
void VariableContextSystem::update(SCENE_SYSTEM::Scene& scene) {
	for (auto& object : scene.getObjects()) {
		if (object->getIsActive()) {
			if (auto c = object->getComponent<VariableContextComponent>()) {
				c->update(scene);
			}
		}
	}
}
}
