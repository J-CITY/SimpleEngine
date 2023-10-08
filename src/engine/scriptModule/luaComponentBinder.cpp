#include "luaComponentBinder.h"

#include <sol/sol.hpp>

#include <coreModule/ecs/Object.h>
#include <coreModule/ecs/components/Transform.h>
#include <coreModule/ecs/components/cameraComponent.h>
#include <coreModule/ecs/components/DirectionalLight.h>
#include <coreModule/ecs/components/PointLight.h>
#include <coreModule/ecs/components/SpotLight.h>
#include <coreModule/ecs/components/AmbientLight.h>
#include <coreModule/ecs/components/AmbientSphereLight.h>
#include <coreModule/ecs/components/ModelRenderer.h>
#include <coreModule/ecs/components/MaterialRenderer.h>

using namespace IKIGAI::SCRIPTING;

void LuaComponentBinder::BindComponent(sol::state & p_luaState)
{
	using namespace IKIGAI::MATHGL;
	using namespace IKIGAI::ECS;
	
	p_luaState.new_usertype<Component>("Component",
		"GetOwner", [](Component& p_component) -> const Object& { return p_component.obj; }
	);

	p_luaState.new_usertype<TransformComponent>("Transform",
		sol::base_classes, sol::bases<Component>(),
		/* Methods */
		"SetPosition", &TransformComponent::setLocalPosition,
		"SetRotation", &TransformComponent::setLocalRotation,
		"SetScale", &TransformComponent::setLocalScale,
		"SetLocalPosition", &TransformComponent::setLocalPosition,
		"SetLocalRotation", &TransformComponent::setLocalRotation,
		"SetLocalScale", &TransformComponent::setLocalScale,
		"GetPosition", [](TransformComponent& p_this) -> Vector3 { return p_this.getWorldPosition(); },
		"GetRotation", [](TransformComponent& p_this) -> Quaternion { return p_this.getWorldRotation(); },
		"GetScale", [](TransformComponent& p_this) -> Vector3 { return p_this.getWorldScale(); },
		"GetLocalPosition", [](TransformComponent& p_this) -> Vector3 { return p_this.getLocalPosition(); },
		"GetLocalRotation", [](TransformComponent& p_this) -> Quaternion { return p_this.getLocalRotation(); },
		"GetLocalScale", [](TransformComponent& p_this) -> Vector3 { return p_this.getLocalScale(); },
		"GetWorldPosition", [](TransformComponent& p_this) -> Vector3 { return p_this.getWorldPosition(); },
		"GetWorldRotation", [](TransformComponent& p_this) -> Quaternion { return p_this.getWorldRotation(); },
		"GetWorldScale", [](TransformComponent& p_this) -> Vector3 { return p_this.getWorldScale(); },
		"GetForward", &TransformComponent::getWorldForward,
		"GetUp", &TransformComponent::getWorldUp,
		"GetRight", &TransformComponent::getWorldRight,
		"GetLocalForward", &TransformComponent::getLocalForward,
		"GetLocalUp", &TransformComponent::getLocalUp,
		"GetLocalRight", &TransformComponent::getLocalRight,
		"GetWorldForward", &TransformComponent::getWorldForward,
		"GetWorldUp", &TransformComponent::getWorldUp,
		"GetWorldRight", &TransformComponent::getWorldRight
		);
    
    p_luaState.new_enum<IKIGAI::ECS::ModelRenderer::EFrustumBehaviour>("FrustumBehaviour",
        {
            {"DISABLED",		IKIGAI::ECS::ModelRenderer::EFrustumBehaviour::DISABLED},
            {"CULL_MODEL",		IKIGAI::ECS::ModelRenderer::EFrustumBehaviour::CULL_MODEL},
            {"CULL_MESHES",		IKIGAI::ECS::ModelRenderer::EFrustumBehaviour::CULL_MESHES},
            {"CULL_CUSTOM",		IKIGAI::ECS::ModelRenderer::EFrustumBehaviour::CULL_CUSTOM}
        });

	p_luaState.new_usertype<ModelRenderer>("ModelRenderer",
		sol::base_classes, sol::bases<Component>(),
		"GetModel", &ModelRenderer::getModel,
		"SetModel", &ModelRenderer::setModel,
		"GetFrustumBehaviour", &ModelRenderer::getFrustumBehaviour,
		"SetFrustumBehaviour", &ModelRenderer::setFrustumBehaviour
	);

	p_luaState.new_usertype<MaterialRenderer>("MaterialRenderer",
		sol::base_classes, sol::bases<Component>(),
		"SetMaterial", &MaterialRenderer::setMaterial
	);

    p_luaState.new_enum<IKIGAI::RENDER::Camera::ProjectionMode>("ProjectionMode",
    {
        {"ORTHOGRAPHIC",	IKIGAI::RENDER::Camera::ProjectionMode::ORTHOGRAPHIC},
        {"PERSPECTIVE",		IKIGAI::RENDER::Camera::ProjectionMode::PERSPECTIVE}
    });

	p_luaState.new_usertype<CameraComponent>("Camera",
		sol::base_classes, sol::bases<Component>(),
		"GetFov", &CameraComponent::getFov,
		"GetSize", &CameraComponent::getSize,
		"GetNear", &CameraComponent::getNear,
		"GetFar", &CameraComponent::getFar,
		"SetFov", &CameraComponent::setFov,
		"SetSize", &CameraComponent::setSize,
		"SetNear", &CameraComponent::setNear,
		"SetFar", &CameraComponent::setFar,
        "HasFrustumGeometryCulling", &CameraComponent::isFrustumGeometryCulling,
        "HasFrustumLightCulling", &CameraComponent::isFrustumLightCulling,
        "GetProjectionMode", &CameraComponent::getProjectionMode,
        "SetFrustumGeometryCulling", &CameraComponent::setFrustumGeometryCulling,
        "SetFrustumLightCulling", &CameraComponent::setFrustumLightCulling,
        "SetProjectionMode", &CameraComponent::setProjectionMode
		);
	/*
	p_luaState.new_usertype<LightComponent>("Light",
		sol::base_classes, sol::bases<LightComponent>(),
		"GetColor", &PointLight::getColor,
		"GetIntensity", &PointLight::getIntensity,
		"SetColor", &PointLight::setColor,
		"SetIntensity", &PointLight::setIntensity
		);*/

	p_luaState.new_usertype<PointLight>("PointLight",
		sol::base_classes, sol::bases<LightComponent>(),
		"GetConstant", &PointLight::getConstant,
		"GetLinear", &PointLight::getLinear,
		"GetQuadratic", &PointLight::getQuadratic,
		"SetConstant", &PointLight::setConstant,
		"SetLinear", &PointLight::setLinear,
		"SetQuadratic", &PointLight::setQuadratic
		);

	p_luaState.new_usertype<SpotLight>("SpotLight",
		sol::base_classes, sol::bases<LightComponent>(),
		"GetConstant", &SpotLight::getConstant,
		"GetLinear", &SpotLight::getLinear,
		"GetQuadratic", &SpotLight::getQuadratic,
		"GetCutOff", &SpotLight::getCutoff,
		"GetOuterCutOff", &SpotLight::getOuterCutoff,
		"SetConstant", &SpotLight::setConstant,
		"SetLinear", &SpotLight::setLinear,
		"SetQuadratic", &SpotLight::setQuadratic,
		"SetCutOff", &SpotLight::setCutoff,
		"SetOuterCutOff", &SpotLight::setOuterCutoff
		);

	p_luaState.new_usertype<AmbientLight>("AmbientBoxLight",
		sol::base_classes, sol::bases<LightComponent>(),
		"GetSize", &AmbientLight::getSize,
		"SetSize", &AmbientLight::setSize
		);

	p_luaState.new_usertype<AmbientSphereLight>("AmbientSphereLight",
		sol::base_classes, sol::bases<LightComponent>(),
		"GetRadius", &AmbientSphereLight::getRadius,
		"SetRadius", &AmbientSphereLight::setRadius
		);

	p_luaState.new_usertype<DirectionalLight>("DirectionalLight",
		sol::base_classes, sol::bases<LightComponent>()
		);
}
