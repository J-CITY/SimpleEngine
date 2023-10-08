#include "luaActorBinder.h"

#include <sol/sol.hpp>

#include <coreModule/ecs/object.h>
#include <coreModule/ecs/components/transform.h>
#include <coreModule/ecs/components/cameraComponent.h>
#include <coreModule/ecs/components/directionalLight.h>
#include <coreModule/ecs/components/pointLight.h>
#include <coreModule/ecs/components/spotLight.h>
#include <coreModule/ecs/components/ambientLight.h>
#include <coreModule/ecs/components/ambientSphereLight.h>
#include <coreModule/ecs/components/modelRenderer.h>
#include <coreModule/ecs/components/materialRenderer.h>

using namespace IKIGAI::SCRIPTING;

void IKIGAI::SCRIPTING::LuaActorBinder::BindActor(sol::state & p_luaState) {
	using namespace IKIGAI::ECS;

	p_luaState.new_usertype<Object>("Object",
		/* Methods */
		"GetName", &Object::getName,
		"SetName", &Object::setName,
		"GetTag", &Object::getTag,
		"GetChildren", &Object::getChildren,
		"SetTag", &Object::setTag,
		"GetID", &Object::getID,
		"GetParent", &Object::getParent,
		"SetParent", &Object::setParent,
		"DetachFromParent", &Object::detachFromParent,
		"Destroy", &Object::markAsDestroy,
		"IsSelfActive", &Object::getIsSelfActive,
		"IsActive", &Object::getIsActive,
		"SetActive", &Object::setActive,


		/* Components Getters */
		"GetTransform", [](Object& p_this) -> TransformComponent* {
			return p_this.getComponent<TransformComponent>().get();
		},
		"GetCamera", &Object::getComponent<CameraComponent>,
		"GetLight", &Object::getComponent<LightComponent>,
		"GetPointLight", &Object::getComponent<PointLight>,
		"GetSpotLight", &Object::getComponent<SpotLight>,
		"GetDirectionalLight", &Object::getComponent<DirectionalLight>,
		"GetAmbientBoxLight", &Object::getComponent<AmbientLight>,
		"GetAmbientSphereLight", &Object::getComponent<AmbientSphereLight>,
		"GetModelRenderer", &Object::getComponent<ModelRenderer>,
		"GetMaterialRenderer", &Object::getComponent<MaterialRenderer>,
		
		/* Behaviours relatives */
		//"GetBehaviour", [](Object& p_this, const std::string& p_name) -> sol::table {
		//	auto behaviour = p_this.getScript(p_name);
		//	if (behaviour)
		//		return behaviour->getTable();
		//	else
		//		return sol::nil;
		//},

		/* Components Creators */
		"AddTransform", &Object::addComponent<TransformComponent>,
		"AddModelRenderer", &Object::addComponent<ModelRenderer>,
		"AddCamera", &Object::addComponent<CameraComponent>,
		"AddPointLight", &Object::addComponent<PointLight>,
		"AddSpotLight", &Object::addComponent<SpotLight>,
		"AddDirectionalLight", &Object::addComponent<DirectionalLight>,
		"AddAmbientBoxLight", &Object::addComponent<AmbientLight>,
		"AddAmbientSphereLight", &Object::addComponent<AmbientSphereLight>,
		"AddMaterialRenderer", &Object::addComponent<MaterialRenderer>,
		
		/* Components Destructors */
		"RemoveModelRenderer", &Object::removeComponent<ModelRenderer>,
		"RemoveCamera", &Object::removeComponent<CameraComponent>,
		"RemovePointLight", &Object::removeComponent<PointLight>,
		"RemoveSpotLight", &Object::removeComponent<SpotLight>,
		"RemoveDirectionalLight", &Object::removeComponent<DirectionalLight>,
		"RemoveAmbientBoxLight", &Object::removeComponent<AmbientLight>,
		"RemoveAmbientSphereLight", &Object::removeComponent<AmbientSphereLight>,
		"RemoveMaterialRenderer", &Object::removeComponent<MaterialRenderer>//,
		
		/* Behaviour management */
		//"AddBehaviour", &Object::addScript,
		//"RemoveBehaviour", sol::overload
		//(
		//	//sol::resolve<bool(ScriptComponent&)>(&Object::removeScript),
		//	sol::resolve<bool(const std::string&)>(&Object::removeScript)
		//)
	);
}
