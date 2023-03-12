#pragma once

#include "component.h"
import glmath;
#include "../../render/backends/interface/materialInterface.h"
#include "../../resourceManager/serializerInterface.h"

#define MAX_MATERIAL_COUNT 255


namespace KUMA::ECS { class Object; }

namespace KUMA::ECS {
	class MaterialRenderer : public Component {
	public:
		using MaterialList = std::array<std::shared_ptr<RENDER::MaterialInterface>, MAX_MATERIAL_COUNT>;
		using MaterialNames = std::array<std::string, MAX_MATERIAL_COUNT>;
		
		MaterialRenderer(Ref<ECS::Object> obj);
		void fillWithMaterial(std::shared_ptr<RENDER::MaterialInterface> material);
		void setMaterial(unsigned index, std::shared_ptr<RENDER::MaterialInterface> material);
		std::shared_ptr<RENDER::MaterialInterface> GetMaterialAtIndex(unsigned index);
		void removeMaterial(unsigned index);
		void removeMaterial(std::shared_ptr<RENDER::MaterialInterface> instance);
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