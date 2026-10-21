#pragma once
#include "componentReference.h"
#include "object.h"
#include "sceneModule/scene.h"
namespace IKIGAI::ECS {
template <class T> UTILS::WeakPtr<T> Resolve(SCENE_SYSTEM::Scene& scene, const ComponentReference& r) {
	if (r.Component != GetComponentName<T>()) {
		throw std::runtime_error("Reference expects " + GetComponentName<T>() + ", got " + r.Component);
	}
	auto object = scene.findObjectByID(ECS::Entity(ECS::Entity::ID(r.ObjectId)));
	return object ? object->getComponent<T>() : nullptr;
}
}
