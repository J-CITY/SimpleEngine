#pragma once
#include "component.h"
#include <renderModule/backends/interface/modelInterface.h>
#include <rttr/registration_friend.h>

#include "../../resourceManager/serializerInterface.h"
#include <utilsModule/event.h>

namespace IKIGAI::ECS { class Object; }

namespace IKIGAI::ECS{
	class  ModelRenderer : public Component {
		RTTR_REGISTRATION_FRIEND
	public:
		enum class EFrustumBehaviour {
			DISABLED = 0,
			CULL_MODEL = 1,
			CULL_MESHES = 2,
			CULL_CUSTOM = 3
		};

		ModelRenderer(Ref<ECS::Object> obj);
		void setModel(std::shared_ptr<RENDER::ModelInterface> model);
		std::shared_ptr<RENDER::ModelInterface> getModel() const;
		void setFrustumBehaviour(EFrustumBehaviour boundingMode);
		EFrustumBehaviour getFrustumBehaviour() const;
		const RENDER::BoundingSphere& getCustomBoundingSphere() const;
		void setCustomBoundingSphere(const RENDER::BoundingSphere& boundingSphere);
		
	private:
		void setModelByPath(std::string path);
		std::string getModelPath();

		std::shared_ptr<RENDER::ModelInterface> model = nullptr;
		RENDER::BoundingSphere customBoundingSphere = { {}, 1.0f };
		EFrustumBehaviour frustumBehaviour = EFrustumBehaviour::CULL_MODEL;
	};
}