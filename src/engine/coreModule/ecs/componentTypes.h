#pragma once

#include "components/ambientLight.h"
#include "components/ambientSphereLight.h"
#include "components/audioComponent.h"
#include "components/cameraComponent.h"
#include "components/directionalLight.h"
#include "components/ikigaiScriptComponent.h"
#include "components/inputComponent.h"
#include "components/logicComponent.h"
#include "components/materialRenderer.h"
#include "components/modelRenderer.h"
#include "components/particleSystemComponent.h"
#include "components/pointLight.h"
#include "components/renderTargetComponent.h"
#include "components/scriptComponent.h"
#include "components/skeletal.h"
#include "components/skeletalAnimationComponent.h"
#include "components/skeletalComponent.h"
#include "components/transform.h"


#if USE_LUA
#define IKIGAI_LUA_COMPONENT ScriptComponent,
#define IKIGAI_LUA_DESCRIPTOR ScriptComponent::Descriptor,
#else
#define IKIGAI_LUA_COMPONENT
#define IKIGAI_LUA_DESCRIPTOR
#endif
#if USE_IKIGAI_SCRIPT
#define IKIGAI_SCRIPT_COMPONENT IkigaiScriptComponent,
#define IKIGAI_SCRIPT_DESCRIPTOR IkigaiScriptComponent::Descriptor,
#else
#define IKIGAI_SCRIPT_COMPONENT
#define IKIGAI_SCRIPT_DESCRIPTOR
#endif
#include "components/spotLight.h"
#if USE_PHYSICS_3D
#include "components/physics3dComponent.h"
#include "components/physicsComponent.h"

#define IKIGAI_PHYSICS3D_COMPONENTS Physics3DComponent, SoftBody3DComponent, Cloth3DComponent, Fluid3DComponent, ForceField3DComponent,
#define IKIGAI_PHYSICS3D_DESCRIPTORS                                                                                                                 \
	Physics3DComponent::Descriptor, SoftBody3DComponent::Descriptor, Cloth3DComponent::Descriptor, Fluid3DComponent::Descriptor,                     \
		ForceField3DComponent::Descriptor,
#define IKIGAI_PHYSICS3D_LEGACY_COMPONENT PhysicsComponent,
#define IKIGAI_PHYSICS3D_LEGACY_DESCRIPTOR PhysicsComponent::Descriptor,
#else
#define IKIGAI_PHYSICS3D_COMPONENTS
#define IKIGAI_PHYSICS3D_DESCRIPTORS
#define IKIGAI_PHYSICS3D_LEGACY_COMPONENT
#define IKIGAI_PHYSICS3D_LEGACY_DESCRIPTOR
#endif
#if USE_PHYSICS_2D
#include "components/cloth2dComponent.h"
#include "components/forceField2dComponent.h"
#include "components/physics2dComponent.h"

#define IKIGAI_PHYSICS2D_COMPONENTS                                                                                                                  \
	Physics2DComponent, SoftBody2DComponent, Fluid2DComponent, Joint2DComponent, ForceField2DComponent, Cloth2DComponent,
#define IKIGAI_PHYSICS2D_DESCRIPTORS                                                                                                                 \
	Physics2DComponent::Descriptor, SoftBody2DComponent::Descriptor, Fluid2DComponent::Descriptor, Joint2DComponent::Descriptor,                     \
		ForceField2DComponent::Descriptor, Cloth2DComponent::Descriptor,
#else
#define IKIGAI_PHYSICS2D_COMPONENTS
#define IKIGAI_PHYSICS2D_DESCRIPTORS
#endif
#include "../gui/components/spriteComponent.h"
#include "../gui/components/vectorComponents.h"
#include "components/batchComponent.h"


namespace IKIGAI::ECS {

#define COMPONENTS_DEF                                                                                                                               \
	TransformComponent, IKIGAI_PHYSICS3D_COMPONENTS IKIGAI_PHYSICS2D_COMPONENTS AmbientLight, AmbientSphereLight, AudioComponent, CameraComponent,   \
		DirectionalLight, InputComponent, LogicComponent, MaterialRenderer, ModelRenderer, ModelLODRenderer, RenderTargetComponent, PointLight,      \
		IKIGAI_LUA_COMPONENT IKIGAI_SCRIPT_COMPONENT Skeletal, SkeletalComponent, SkeletalAnimationComponent, SpotLight,                             \
		IKIGAI_PHYSICS3D_LEGACY_COMPONENT AudioListenerComponent, VrCameraComponent, BatchComponent, RootGuiComponent, SpriteComponent,              \
		SpriteAnimateComponent, SpriteParticleComponent, ParticleSystem2DComponent, ParticleSystem3DComponent, SpineComponent, LabelComponent,       \
		InteractionComponent, ClipComponent, ScrollComponent, LayoutComponent, ChunkModelRenderer, SdfTextComponent, VectorShapeComponent,           \
		VectorPathComponent, VectorCanvasComponent

#define COMPONENTS_DESCRIPTOR_DEF                                                                                                                    \
	TransformComponent::Descriptor, IKIGAI_PHYSICS3D_DESCRIPTORS IKIGAI_PHYSICS2D_DESCRIPTORS AmbientLight::Descriptor,                              \
		AmbientSphereLight::Descriptor, AudioComponent::Descriptor, CameraComponent::Descriptor, DirectionalLight::Descriptor,                       \
		InputComponent::Descriptor, LogicComponent::Descriptor, MaterialRenderer::Descriptor, ModelRenderer::Descriptor,                             \
		ModelLODRenderer::Descriptor, RenderTargetComponent::Descriptor, PointLight::Descriptor,                                                     \
		IKIGAI_LUA_DESCRIPTOR IKIGAI_SCRIPT_DESCRIPTOR Skeletal::Descriptor, SkeletalComponent::Descriptor, SkeletalAnimationComponent::Descriptor,  \
		SpotLight::Descriptor, IKIGAI_PHYSICS3D_LEGACY_DESCRIPTOR AudioListenerComponent::Descriptor, VrCameraComponent::Descriptor,                 \
		BatchComponent::Descriptor, RootGuiComponent::Descriptor, SpriteComponent::Descriptor, SpriteAnimateComponent::Descriptor,                   \
		SpriteParticleComponent::Descriptor, ParticleSystem2DComponent::Descriptor, ParticleSystem3DComponent::Descriptor,                           \
		SpineComponent::Descriptor, LabelComponent::Descriptor, InteractionComponent::Descriptor, ClipComponent::Descriptor,                         \
		ScrollComponent::Descriptor, LayoutComponent::Descriptor, ChunkModelRenderer::Descriptor, SdfTextComponent::Descriptor,                      \
		VectorShapeComponent::Descriptor, VectorPathComponent::Descriptor, VectorCanvasComponent::Descriptor

//ArCameraComponent,\

template <typename...> class ComponentsTypeProvider { };
using ComponentsTypeProviderType = ComponentsTypeProvider<COMPONENTS_DEF>;

using ComponentsType = std::variant<COMPONENTS_DEF>;
using ComponentsDescriptorType = std::variant<COMPONENTS_DESCRIPTOR_DEF>;
}
