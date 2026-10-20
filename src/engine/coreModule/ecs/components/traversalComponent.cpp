#include "traversalComponent.h"
#if USE_PHYSICS_3D
namespace IKIGAI::ECS {
TraversalComponent::TraversalComponent(UTILS::Ref<Object> o)
	: ComponentBase(o) {
	__NAME__ = "TraversalComponent";
	settings.Type = "class IKIGAI::ECS::TraversalComponent";
}
TraversalComponent::TraversalComponent(UTILS::Ref<Object> o, const ComponentBase::Descriptor& d)
	: TraversalComponent(o) {
	settings = static_cast<const Descriptor&>(d);
}
}
#endif
