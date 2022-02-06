#pragma once
#include "component.h"
#include "../../render/Model.h"
#include "../../resourceManager/serializerInterface.h"
#include "../../utils/event.h"

namespace KUMA::ECS { class Object; }

namespace KUMA::ECS{
	class  ModelRenderer : public Component {
	public:
		enum class EFrustumBehaviour {
			DISABLED = 0,
			CULL_MODEL = 1,
			CULL_MESHES = 2,
			CULL_CUSTOM = 3
		};

		NLOHMANN_JSON_SERIALIZE_ENUM(EFrustumBehaviour, {
			{EFrustumBehaviour::DISABLED, nullptr},
			{EFrustumBehaviour::CULL_MODEL, "CULL_MODEL"},
			{EFrustumBehaviour::CULL_MESHES, "CULL_MESHES"},
			{EFrustumBehaviour::CULL_CUSTOM, "CULL_CUSTOM"},
		})

		ModelRenderer(const ECS::Object& obj);
		void setModel(std::shared_ptr<RENDER::Model> model);
		std::shared_ptr<RENDER::Model> getModel() const;
		void setFrustumBehaviour(EFrustumBehaviour boundingMode);
		EFrustumBehaviour getFrustumBehaviour() const;
		const RENDER::BoundingSphere& getCustomBoundingSphere() const;
		void setCustomBoundingSphere(const RENDER::BoundingSphere& boundingSphere);

		virtual void onDeserialize(nlohmann::json& j) override;
		virtual void onSerialize(nlohmann::json& j) override;
	private:
		std::shared_ptr<RENDER::Model> model = nullptr;
		RENDER::BoundingSphere customBoundingSphere = { {}, 1.0f };
		EFrustumBehaviour frustumBehaviour = EFrustumBehaviour::CULL_MODEL;
	};
}