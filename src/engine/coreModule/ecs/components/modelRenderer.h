#pragma once
#include "component.h"
#include <renderModule/backends/interface/modelInterface.h>
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
	public:
		struct Descriptor : public Component::Descriptor {
			std::string Type;
			std::string Path;

			template<class Context>
			constexpr static auto serde(Context& context, Descriptor& value) {
				using Self = Descriptor;
				using namespace serde::attribute;
				serde::serde_struct(context, value)
					.field(&Self::Type, "ModelRendererType")
					.field(&Self::Path, "Path");
			}
		};
		ModelRenderer(UTILS::Ref<ECS::Object> obj);
		ModelRenderer(UTILS::Ref<ECS::Object> obj, const Descriptor& _descriptor);
		ModelRenderer(UTILS::Ref<ECS::Object> obj, const Component::Descriptor& descriptor) :
			ModelRenderer(obj, static_cast<const Descriptor&>(descriptor)) {
		};
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
	public:
		static auto GetMembers() {
			return std::tuple{
			};
		}
	};

	class ModelLODRenderer : public Component {
	public:
		struct ModelLodRefl {
			float Distance = 0.0f;
			std::string Path;

			template<class Context>
			constexpr static auto serde(Context& context, ModelLodRefl& value) {
				using Self = ModelLodRefl;
				using namespace serde::attribute;
				serde::serde_struct(context, value)
					.field(&Self::Distance, "Distance")
					.field(&Self::Path, "Path");
			}
		};
		struct Descriptor : public Component::Descriptor {
			std::string Type;
			std::vector<ModelLodRefl> Paths;

			template<class Context>
			constexpr static auto serde(Context& context, Descriptor& value) {
				using Self = Descriptor;
				using namespace serde::attribute;
				serde::serde_struct(context, value)
					.field(&Self::Type, "PhysicsComponentType")
					.field(&Self::Paths, "Paths");
			}
		};
		
		struct ModelLod {
			float m_distance = 0.0;
			std::shared_ptr<RENDER::ModelInterface> m_model;
		};

		ModelLODRenderer(UTILS::Ref<ECS::Object> obj);
		ModelLODRenderer(UTILS::Ref<ECS::Object> obj, const Descriptor _descriptor);
		ModelLODRenderer(UTILS::Ref<ECS::Object> obj, const Component::Descriptor& descriptor) :
			ModelLODRenderer(obj, static_cast<const Descriptor&>(descriptor)) {
		};
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

	public:
		static auto GetMembers() {
			return std::tuple{
			};
		}
	};

	template <>
	inline std::string ECS::GetType<ModelLODRenderer>() {
		return "class IKIGAI::ECS::ModelLODRenderer";
	}

	template <>
	inline std::string ECS::GetType<ModelRenderer>() {
		return "class IKIGAI::ECS::ModelRenderer";
	}

	template <>
	inline std::string IKIGAI::ECS::GetComponentName<ModelLODRenderer>() {
		return "ModelLODRenderer";
	}

	template <>
	inline std::string IKIGAI::ECS::GetComponentName<ModelRenderer>() {
		return "ModelRenderer";
	}

}
