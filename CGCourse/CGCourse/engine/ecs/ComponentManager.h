#pragma once
#include <map>
#include <memory>
#include <unordered_map>

//#include "object.h"
#include "components/audioComponent.h"
#include "components/cameraComponent.h"
#include "components/directionalLight.h"
#include "components/lightComponent.h"
#include "components/modelRenderer.h"
#include "components/materialRenderer.h"
#include "components/pointLight.h"
#include "components/script.h"
#include "components/skeletal.h"
#include "components/spotLight.h"

namespace KUMA {
	namespace SCENE_SYSTEM {
		class Scene;
	}
}

namespace KUMA::ECS {
	class ComponentManager {
		static ComponentManager* instance;
	public:
		static ComponentManager* getInstance() {
			if (!instance)
				instance = new ComponentManager();
			return instance;
		}
		
		template<class TYPE>
		void addComponent(ObjectId<Object> id, std::shared_ptr<TYPE> component) {}
		
		template <>
		void addComponent<TransformComponent>(ObjectId<Object> id, std::shared_ptr<TransformComponent> component) {
			transformComponents[id] = component;
		}
		template <>
		void addComponent<CameraComponent>(ObjectId<Object> id, std::shared_ptr<CameraComponent> component) {
			cameraComponents[id] = component;
		}
		template <>
		void addComponent<PointLight>(ObjectId<Object> id, std::shared_ptr<PointLight> component) {
			lightComponents[id] = component;
		}
		template <>
		void addComponent<DirectionalLight>(ObjectId<Object> id, std::shared_ptr<DirectionalLight> component) {
			lightComponents[id] = component;
		}
		template <>
		void addComponent<SpotLight>(ObjectId<Object> id, std::shared_ptr<SpotLight> component) {
			lightComponents[id] = component;
		}
		template <>
		void addComponent<LightComponent>(ObjectId<Object> id, std::shared_ptr<LightComponent> component) {
			lightComponents[id] = component;
		}
		template <>
		void addComponent<ModelRenderer>(ObjectId<Object> id, std::shared_ptr<ModelRenderer> component) {
			modelComponents[id] = component;
		}
		template <>
		void addComponent<MaterialRenderer>(ObjectId<Object> id, std::shared_ptr<MaterialRenderer> component) {
			materialComponents[id] = component;
		}
		template <>
		void addComponent<Script>(ObjectId<Object> id, std::shared_ptr<Script> component) {
			if (!scriptComponents.count(id)) {
				scriptComponents[id] = std::unordered_map<std::string, std::shared_ptr<Script>>();
			}
			scriptComponents[id][component->getName()] = component;
		}
		template <>
		void addComponent<AudioComponent>(ObjectId<Object> id, std::shared_ptr<AudioComponent> component) {
			audioComponents[id] = component;
		}
		template <>
		void addComponent<Skeletal>(ObjectId<Object> id, std::shared_ptr<Skeletal> component) {
			skeletalComponents[id] = component;
		}
		

		template<class TYPE>
		void removeComponents(ObjectId<Object> id) {}
		
		template <>
		void removeComponents<TransformComponent>(ObjectId<Object> id) {
			transformComponents.erase(id);
		}
		template <>
		void removeComponents<CameraComponent>(ObjectId<Object> id) {
			cameraComponents.erase(id);
		}
		template <>
		void removeComponents<LightComponent>(ObjectId<Object> id) {
			lightComponents.erase(id);
		}
		template <>
		void removeComponents<ModelRenderer>(ObjectId<Object> id) {
			modelComponents.erase(id);
		}
		template <>
		void removeComponents<MaterialRenderer>(ObjectId<Object> id) {
			materialComponents.erase(id);
		}
		template <>
		void removeComponents<Script>(ObjectId<Object> id) {
			scriptComponents.erase(id);
		}
		template <>
		void removeComponents<AudioComponent>(ObjectId<Object> id) {
			audioComponents.erase(id);
		}
		template <>
		void removeComponents<Skeletal>(ObjectId<Object> id) {
			skeletalComponents.erase(id);
		}

		void enable(ObjectId<Object> id) {
			if (transformComponentsOff.count(id)) {
				transformComponents[id] = transformComponentsOff[id];
				transformComponentsOff.erase(id);
			}
			if (cameraComponentsOff.count(id)) {
				cameraComponents[id] = cameraComponentsOff[id];
				cameraComponentsOff.erase(id);
			}
			if (lightComponentsOff.count(id)) {
				lightComponents[id] = lightComponentsOff[id];
				lightComponentsOff.erase(id);
			}
			if (modelComponentsOff.count(id)) {
				modelComponents[id] = modelComponentsOff[id];
				modelComponentsOff.erase(id);
			}
			if (materialComponentsOff.count(id)) {
				materialComponents[id] = materialComponentsOff[id];
				materialComponentsOff.erase(id);
			}
			if (scriptComponentsOff.count(id)) {
				scriptComponents[id] = scriptComponentsOff[id];
				scriptComponentsOff.erase(id);
			}
			if (audioComponentsOff.count(id)) {
				audioComponents[id] = audioComponentsOff[id];
				audioComponentsOff.erase(id);
			}
			if (skeletalComponentsOff.count(id)) {
				skeletalComponents[id] = skeletalComponentsOff[id];
				skeletalComponentsOff.erase(id);
			}
		}

		void disable(ObjectId<Object> id) {
			if (transformComponents.count(id)) {
				transformComponentsOff[id] = transformComponents[id];
				transformComponents.erase(id);
			}
			if (cameraComponents.count(id)) {
				cameraComponentsOff[id] = cameraComponents[id];
				cameraComponents.erase(id);
			}
			if (lightComponents.count(id)) {
				lightComponentsOff[id] = lightComponents[id];
				lightComponents.erase(id);
			}
			if (modelComponents.count(id)) {
				modelComponentsOff[id] = modelComponents[id];
				modelComponents.erase(id);
			}
			if (materialComponents.count(id)) {
				materialComponentsOff[id] = materialComponents[id];
				materialComponents.erase(id);
			}
			if (scriptComponents.count(id)) {
				scriptComponentsOff[id] = scriptComponents[id];
				scriptComponents.erase(id);
			}
			if (audioComponents.count(id)) {
				audioComponentsOff[id] = audioComponents[id];
				audioComponents.erase(id);
			}
			if (skeletalComponents.count(id)) {
				skeletalComponentsOff[id] = skeletalComponents[id];
				skeletalComponents.erase(id);
			}
		}

		void removeAllObjectComponents(ObjectId<Object> id) {
			if (transformComponents.count(id)) {
				transformComponents.erase(id);
				transformComponentsOff.erase(id);
			}
			if (cameraComponents.count(id)) {
				cameraComponents.erase(id);
				cameraComponentsOff.erase(id);
			}
			if (lightComponents.count(id)) {
				lightComponents.erase(id);
				lightComponentsOff.erase(id);
			}
			if (modelComponents.count(id)) {
				modelComponents.erase(id);
				modelComponentsOff.erase(id);
			}
			if (materialComponents.count(id)) {
				materialComponents.erase(id);
				materialComponentsOff.erase(id);
			}
			if (scriptComponents.count(id)) {
				scriptComponents.erase(id);
				scriptComponentsOff.erase(id);
			}
			if (audioComponents.count(id)) {
				audioComponents.erase(id);
				audioComponentsOff.erase(id);
			}
			if (skeletalComponents.count(id)) {
				skeletalComponents.erase(id);
				skeletalComponentsOff.erase(id);
			}
		}

		std::vector<std::shared_ptr<DirectionalLight>> getAllDirectionalLights() {
			std::vector<std::shared_ptr<DirectionalLight>> res;
			for (auto& light : lightComponents) {
				if (light.second->getName() == "DirectionalLight") {
					res.push_back(std::dynamic_pointer_cast<DirectionalLight>(light.second));
				}
			}
			return res;
		}
		std::vector<std::shared_ptr<SpotLight>> getAllSpotLights() {
			std::vector<std::shared_ptr<SpotLight>> res;
			for (auto& light : lightComponents) {
				if (light.second->getName() == "SpotLight") {
					res.push_back(std::dynamic_pointer_cast<SpotLight>(light.second));
				}
			}
			return res;
		}
		std::vector<std::shared_ptr<PointLight>> getAllPointLights() {
			std::vector<std::shared_ptr<PointLight>> res;
			for (auto& light : lightComponents) {
				if (light.second->getName() == "PointLight") {
					res.push_back(std::dynamic_pointer_cast<PointLight>(light.second));
				}
			}
			return res;
		}

		std::map<ObjectId<Object>, std::shared_ptr<TransformComponent>> transformComponents;
		std::map<ObjectId<Object>, std::shared_ptr<AudioComponent>> audioComponents;
		std::map<ObjectId<Object>, std::shared_ptr<Skeletal>> skeletalComponents;
		std::map<ObjectId<Object>, std::shared_ptr<CameraComponent>> cameraComponents;
		std::map<ObjectId<Object>, std::shared_ptr<LightComponent>> lightComponents;
		std::map<ObjectId<Object>, std::shared_ptr<ModelRenderer>> modelComponents;
		std::map<ObjectId<Object>, std::shared_ptr<MaterialRenderer>> materialComponents;
		std::map<ObjectId<Object>, std::unordered_map<std::string, std::shared_ptr<Script>>> scriptComponents;

		std::map<ObjectId<Object>, std::shared_ptr<TransformComponent>> transformComponentsOff;
		std::map<ObjectId<Object>, std::shared_ptr<AudioComponent>> audioComponentsOff;
		std::map<ObjectId<Object>, std::shared_ptr<Skeletal>> skeletalComponentsOff;
		std::map<ObjectId<Object>, std::shared_ptr<CameraComponent>> cameraComponentsOff;
		std::map<ObjectId<Object>, std::shared_ptr<LightComponent>> lightComponentsOff;
		std::map<ObjectId<Object>, std::shared_ptr<ModelRenderer>> modelComponentsOff;
		std::map<ObjectId<Object>, std::shared_ptr<MaterialRenderer>> materialComponentsOff;
		std::map<ObjectId<Object>, std::unordered_map<std::string, std::shared_ptr<Script>>> scriptComponentsOff;
	};
}
