#pragma once
#include "components/component.h"
#include "utilsModule/expressions/expression.h"
#include <functional>
#include <memory>
#include <serdepp/serde.hpp>
#include <string>

namespace IKIGAI::SCENE_SYSTEM {
class Scene;
}
namespace IKIGAI::ECS {
class Object;
class ComponentBase;
struct ComponentReference {
	int ObjectId = 0;
	std::string Component;
	template <class C> static auto serde(C& c, ComponentReference& v) {
		serde::serde_struct(c, v).field(&ComponentReference::ObjectId, "ObjectId").field(&ComponentReference::Component, "Component");
	}
	bool empty() const {
		return ObjectId == 0;
	}
};
struct PropertyReference {
	ComponentReference Target;
	std::string Property;
	template <class C> static auto serde(C& c, PropertyReference& v) {
		serde::serde_struct(c, v).field(&PropertyReference::Target, "Target").field(&PropertyReference::Property, "Property");
	}
};
// Weak component handles follow ECS relocations and expire on removal.
UTILS::WeakPtr<ComponentBase> ResolveComponent(SCENE_SYSTEM::Scene&, const ComponentReference&);
template <class T> UTILS::WeakPtr<T> Resolve(SCENE_SYSTEM::Scene& scene, const ComponentReference& reference);

class PropertyBinding {
public:
	ComponentReference target;
	DATA::ValueType type;
	std::function<DATA::Value(ComponentBase&)> getter;
	mutable UTILS::WeakPtr<ComponentBase> handle;
	DATA::Value read(SCENE_SYSTEM::Scene&) const;
};
class PropertyRegistry {
public:
	struct Accessor {
		DATA::ValueType type;
		std::function<DATA::Value(ComponentBase&)> get;
	};
	static void add(const std::string& component, const std::string& property, Accessor);
	static PropertyBinding bind(SCENE_SYSTEM::Scene&, const PropertyReference&, DATA::ValueType);
};
void RegisterSceneComponentReflection();
}
