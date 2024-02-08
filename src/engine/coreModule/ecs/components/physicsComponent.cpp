#include "physicsComponent.h"

#include "transform.h"

using namespace IKIGAI;
using namespace IKIGAI::ECS;


PhysicsComponent::PhysicsComponent(UTILS::Ref<ECS::Object> obj): Component(obj){
	__NAME__ = "PhysicsComponent";
}

#include <rttr/registration>

RTTR_REGISTRATION
{
	rttr::registration::enumeration<IKIGAI::ECS::CollisionType>("CollisionType")
	(
		rttr::value("NONE", IKIGAI::ECS::CollisionType::NONE),
		rttr::value("SPHERE", IKIGAI::ECS::CollisionType::SPHERE),
		rttr::value("AABB", IKIGAI::ECS::CollisionType::OBB),
		rttr::value("OBB", IKIGAI::ECS::CollisionType::OBB),
		rttr::value("COUNT", IKIGAI::ECS::CollisionType::COUNT)
	);

	rttr::registration::class_<IKIGAI::ECS::PhysicsComponent>("PhysicsComponent")
	(
		rttr::metadata(MetaInfo::FLAGS, MetaInfo::SERIALIZABLE)
	);
}
