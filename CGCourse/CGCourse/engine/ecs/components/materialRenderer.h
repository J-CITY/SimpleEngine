#pragma once

#include <array>

#include "component.h"
#include "../../utils/math/Matrix4.h"
#include "../../render/Material.h"
#include "../../resourceManager/serializerInterface.h"

#define MAX_MATERIAL_COUNT 255


namespace KUMA::ECS { class Object; }

namespace KUMA::ECS {
	class MaterialRenderer : public Component {
	public:
		using MaterialList = std::array<std::shared_ptr<RENDER::Material>, MAX_MATERIAL_COUNT>;
		using MaterialNames = std::array<std::string, MAX_MATERIAL_COUNT>;
		
		MaterialRenderer(const ECS::Object& obj);
		void fillWithMaterial(std::shared_ptr<RENDER::Material> material);
		void setMaterial(unsigned index, std::shared_ptr<RENDER::Material> material);
		std::shared_ptr<RENDER::Material> GetMaterialAtIndex(unsigned index);
		void removeMaterial(unsigned index);
		void removeMaterial(std::shared_ptr<RENDER::Material> instance);
		void removeMaterials();
		void updateMaterialList();
		void setUserMatrixElement(unsigned row, unsigned col, float value);
		float getUserMatrixElement(unsigned row, unsigned col) const;
		const MATHGL::Matrix4& getUserMatrix() const;
		const MaterialList& getMaterials() const;

		virtual void onDeserialize(nlohmann::json& j) override;
		virtual void onSerialize(nlohmann::json& j) override;
	private:
		MaterialList materials;
		MaterialNames materialNames;
		MATHGL::Matrix4 userMatrix;
	};
}