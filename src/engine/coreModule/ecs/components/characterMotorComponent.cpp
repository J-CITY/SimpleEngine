#include "characterMotorComponent.h"
#if USE_PHYSICS_3D
namespace IKIGAI::ECS {
CharacterMotorComponent::CharacterMotorComponent(UTILS::Ref<Object> o)
	: ComponentBase(o) {
	__NAME__ = "CharacterMotorComponent";
	settings.Type = "class IKIGAI::ECS::CharacterMotorComponent";
}
CharacterMotorComponent::CharacterMotorComponent(UTILS::Ref<Object> o, const ComponentBase::Descriptor& d)
	: CharacterMotorComponent(o) {
	settings = static_cast<const Descriptor&>(d);
}
}
#endif
