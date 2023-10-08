#pragma once

#include "component.h"
import glmath;
#include <renderModule/backends/interface/materialInterface.h>
#include <rttr/registration_friend.h>

#include "../../resourceManager/serializerInterface.h"

#define MAX_MATERIAL_COUNT 255


namespace IKIGAI::ECS { class Object; }

namespace IKIGAI::ECS {
	class MaterialRenderer : public Component {
		RTTR_REGISTRATION_FRIEND
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
		
	private:
		void setMaterialsByPath(std::vector<std::string> paths);
		std::vector<std::string> getMaterialsPaths();

		MaterialList materials;
		MaterialNames materialNames;
	};
}