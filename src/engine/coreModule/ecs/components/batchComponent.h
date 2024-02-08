#pragma once
#include "component.h"
#include "materialRenderer.h"
#include "modelRenderer.h"

namespace IKIGAI::ECS { class Object; }

namespace IKIGAI::ECS {
	class BatchComponent : public Component {
	public:
		inline static std::vector<unsigned> ids;
		BatchComponent(UTILS::Ref<ECS::Object> obj);
		void init();
		void CreateAtlases(const MaterialRenderer& material);
		void createBuffers(ModelRenderer& model, MaterialRenderer& material);
	};
}
