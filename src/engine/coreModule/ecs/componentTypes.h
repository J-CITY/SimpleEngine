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
#include "components/batchComponent.h"
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
		ModelLODRenderer,\
		PointLight,\
		ScriptComponent,\
		Skeletal,\
		SpotLight,\
		PhysicsComponent,\
		AudioListenerComponent,\
		VrCameraComponent,\
		BatchComponent

//	RootGuiComponent, \
//		SpriteComponent, \
//		SpriteAnimateComponent, \
//		SpriteParticleComponent, \
//		SpineComponent, \

#define COMPONENTS_DESCRIPTOR_DEF TransformComponent::Descriptor,\
		AmbientLight::Descriptor,\
		AmbientSphereLight::Descriptor,\
		AudioComponent::Descriptor,\
		CameraComponent::Descriptor,\
		DirectionalLight::Descriptor,\
		InputComponent::Descriptor,\
		LogicComponent::Descriptor,\
		MaterialRenderer::Descriptor,\
		ModelRenderer::Descriptor,\
		ModelLODRenderer::Descriptor,\
		PointLight::Descriptor,\
		ScriptComponent::Descriptor,\
		Skeletal::Descriptor,\
		SpotLight::Descriptor,\
		PhysicsComponent::Descriptor,\
		AudioListenerComponent::Descriptor,\
		VrCameraComponent::Descriptor,\
		BatchComponent::Descriptor

	//ArCameraComponent,\

	template<typename...> class ComponentsTypeProvider {};
	using ComponentsTypeProviderType = ComponentsTypeProvider <
		COMPONENTS_DEF
	> ;

	using ComponentsType = std::variant<COMPONENTS_DEF>;
	using ComponentsDescriptorType = std::variant<COMPONENTS_DESCRIPTOR_DEF>;
}
