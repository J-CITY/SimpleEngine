#pragma once

#include "component.h"
import glmath;
#include "../../render/material.h"
#include "../../resourceManager/serializerInterface.h"

#define MAX_MATERIAL_COUNT 255


namespace KUMA::ECS { class Object; }

namespace KUMA::ECS {
	class MaterialRenderer : public Component {
	public:
		using MaterialList = std::array<std::shared_ptr<RENDER::Material>, MAX_MATERIAL_COUNT>;
		using MaterialNames = std::array<std::string, MAX_MATERIAL_COUNT>;
		
		MaterialRenderer(Ref<ECS::Object> obj);
		void fillWithMaterial(std::shared_ptr<RENDER::Material> material);
		void setMaterial(unsigned index, std::shared_ptr<RENDER::Material> material);
		std::shared_ptr<RENDER::Material> GetMaterialAtIndex(unsigned index);
		void removeMaterial(unsigned index);
		void removeMaterial(std::shared_ptr<RENDER::Material> instance);
		void removeMaterials();
		void updateMaterialList();
		const MaterialList& getMaterials() const;

		virtual void onDeserialize(nlohmann::json& j) override;
		virtual void onSerialize(nlohmann::json& j) override;
	private:
		MaterialList materials;
		MaterialNames materialNames;
	};
}