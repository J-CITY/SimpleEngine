#pragma once
#include "component.h"
#include <renderModule/backends/interface/modelInterface.h>
#include <rttr/registration_friend.h>

#include "../../resourceManager/serializerInterface.h"
#include <utilsModule/event.h>

namespace IKIGAI::ECS { class Object; }

namespace IKIGAI::ECS{
	enum class EFrustumBehaviour {
		DISABLED = 0,
		CULL_MODEL = 1,
		CULL_MESHES = 2,
		CULL_CUSTOM = 3
	};
	class  ModelRenderer : public Component {
		RTTR_REGISTRATION_FRIEND
	public:
		
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

		std::shared_ptr<RENDER::ModelInterface> m_model = nullptr;
		RENDER::BoundingSphere m_customBoundingSphere = { {}, 1.0f };
		EFrustumBehaviour m_frustumBehaviour = EFrustumBehaviour::CULL_MODEL;
	};


	class  ModelLODRenderer : public Component {
		RTTR_REGISTRATION_FRIEND
	public:
		struct ModelLodRefl {
			float m_distance = 0.0;
			std::string m_path;
		};
		struct ModelLod {
			float m_distance = 0.0;
			std::shared_ptr<RENDER::ModelInterface> m_model;
		};

		ModelLODRenderer(Ref<ECS::Object> obj);
		void setModel(const ModelLod& model);
		void setModels(const std::vector<ModelLod>& models);
		[[nodiscard]] const std::vector<ModelLod>& getModels() const;
		std::shared_ptr<RENDER::ModelInterface> getModelByDistance(float distance);
		void setFrustumBehaviour(EFrustumBehaviour boundingMode);
		EFrustumBehaviour getFrustumBehaviour() const;
		const RENDER::BoundingSphere& getCustomBoundingSphere() const;
		void setCustomBoundingSphere(const RENDER::BoundingSphere& boundingSphere);

	private:
		void setModelsByPath(std::vector<ModelLodRefl> path);
		std::vector<ModelLodRefl> getModelsPath();

		std::vector<ModelLod> m_models;
		RENDER::BoundingSphere m_customBoundingSphere = { {}, 1.0f };
		EFrustumBehaviour m_frustumBehaviour = EFrustumBehaviour::CULL_MODEL;
	};
}