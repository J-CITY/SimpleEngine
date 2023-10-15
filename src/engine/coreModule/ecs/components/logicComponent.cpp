#include "logicComponent.h"

#include "transform.h"

using namespace IKIGAI;
using namespace IKIGAI::ECS;

#include <rttr/registration>

RTTR_REGISTRATION
{
	rttr::registration::class_<IKIGAI::ECS::LogicComponent>("LogicComponent")
	(
		rttr::metadata(MetaInfo::FLAGS, MetaInfo::SERIALIZABLE)
	);
}

