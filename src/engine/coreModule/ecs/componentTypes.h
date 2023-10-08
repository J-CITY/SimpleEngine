#pragma once

#include "components/transform.h"
#include "components/ambientLight.h"
#include "components/ambientSphereLight.h"
#include "components/audioComponent.h"
#include "components/cameraComponent.h"
#include "components/directionalLight.h"
#include "components/inputComponent.h"
#include "components/logicComponent.h"
#include "components/materialRenderer.h"
#include "components/modelRenderer.h"
#include "components/pointLight.h"
#include "components/skeletal.h"
#include "components/scriptComponent.h"
#include "components/spotLight.h"
#include "components/physicsComponent.h"
#include "../gui/components/spriteComponent.h"

namespace IKIGAI::ECS {

#define COMPONENTS_DEF TransformComponent,\
		AmbientLight,\
		AmbientSphereLight,\
		AudioComponent,\
		CameraComponent,\
		DirectionalLight,\
		InputComponent,\
		LogicComponent,\
		MaterialRenderer,\
		ModelRenderer,\
		PointLight,\
		ScriptComponent,\
		Skeletal,\
		SpotLight,\
		PhysicsComponent,\
		AudioListenerComponent,\
		VrCameraComponent,\
		RootGuiComponent,\
		SpriteComponent

	//ArCameraComponent,\

	template<typename...> class ComponentsTypeProvider {};
	using ComponentsTypeProviderType = ComponentsTypeProvider <
		COMPONENTS_DEF
	> ;

	using ComponentsType = std::variant<COMPONENTS_DEF>;
}
