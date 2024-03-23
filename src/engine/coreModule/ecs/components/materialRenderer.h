#pragma once

#include "component.h"
#include <renderModule/backends/interface/materialInterface.h>

#include "utilsModule/event.h"

#define MAX_MATERIAL_COUNT 255


namespace IKIGAI::ECS { class Object; }

namespace IKIGAI::ECS {
	class MaterialRenderer : public Component {
	public:
		struct Descriptor : public Component::Descriptor {
			std::string Type;
			std::vector<std::string> MaterialNames;
			std::vector<std::string> Materials;

			template<class Context>
			constexpr static auto serde(Context& context, Descriptor& value) {
				using Self = Descriptor;
				using namespace serde::attribute;
				serde::serde_struct(context, value)
					.field(&Self::Type, "MaterialRendererType")
					.field(&Self::MaterialNames, "MaterialNames")
					.field(&Self::Materials, "Materials");
			}
		};

		using MaterialList = std::array<std::shared_ptr<RENDER::MaterialInterface>, MAX_MATERIAL_COUNT>;
		using MaterialNames = std::array<std::string, MAX_MATERIAL_COUNT>;
		
		MaterialRenderer(UTILS::Ref<ECS::Object> obj);
		MaterialRenderer(UTILS::Ref<ECS::Object> obj, const Descriptor& _descriptor);
		MaterialRenderer(UTILS::Ref<ECS::Object> obj, const Component::Descriptor& descriptor) :
			MaterialRenderer(obj, static_cast<const Descriptor&>(descriptor)) {
		};
		~MaterialRenderer() override;
		void fillWithMaterial(std::shared_ptr<RENDER::MaterialInterface> material);
		void setMaterial(unsigned index, std::shared_ptr<RENDER::MaterialInterface> material);
		std::shared_ptr<RENDER::MaterialInterface> GetMaterialAtIndex(unsigned index);
		void removeMaterial(unsigned index);
		void removeMaterial(std::shared_ptr<RENDER::MaterialInterface> instance);
		void removeMaterials();
		void updateMaterialList();
		[[nodiscard]] const MaterialList& getMaterials() const;
		const MaterialNames& getMaterialNames();
	private:
		void setMaterialsByPath(std::vector<std::string> paths);
		std::vector<std::string> getMaterialsPaths();
		void setMaterialsNames(std::vector<std::string> paths);
		std::vector<std::string> getMaterialsNames();
		MaterialRenderer* getMaterialRenderer();

		MaterialList materials;
		MaterialNames materialNames;

		std::shared_ptr<EVENT::EventListener> setMaterialEventId;

	public:
		static auto GetMembers() {
			return std::tuple{
			};
		}
	};

	template <>
	inline std::string ECS::GetType<MaterialRenderer>() {
		return "class IKIGAI::ECS::MaterialRenderer";
	}

	template <>
	inline std::string IKIGAI::ECS::GetComponentName<MaterialRenderer>() {
		return "MaterialRenderer";
	}
}
