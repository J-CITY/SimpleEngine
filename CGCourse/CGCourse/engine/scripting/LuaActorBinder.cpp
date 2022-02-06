#include "LuaActorBinder.h"

#include "../ecs/object.h"

#include "../ecs/components/transform.h"
#include "../ecs/components/cameraComponent.h"
#include "../ecs/components/directionalLight.h"
#include "../ecs/components/pointLight.h"
#include "../ecs/components/spotLight.h"
#include "../ecs/components/ambientLight.h"
#include "../ecs/components/ambientSphereLight.h"
#include "../ecs/components/modelRenderer.h"
#include "../ecs/components/materialRenderer.h"

using namespace KUMA::SCRIPTING;

void KUMA::SCRIPTING::LuaActorBinder::BindActor(sol::state & p_luaState) {
	using namespace KUMA::ECS;

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
		"GetTransform", &Object::getComponent<TransformComponent>,
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
		"GetBehaviour", [](Object& p_this, const std::string& p_name) -> sol::table {
			auto behaviour = p_this.getScript(p_name);
			if (behaviour)
				return behaviour->getTable();
			else
				return sol::nil;
		},

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
		"RemoveMaterialRenderer", &Object::removeComponent<MaterialRenderer>,
		
		/* Behaviour management */
		"AddBehaviour", &Object::addScript,
		"RemoveBehaviour", sol::overload
		(
			//sol::resolve<bool(Script&)>(&Object::removeScript),
			sol::resolve<bool(const std::string&)>(&Object::removeScript)
		)
	);
}
