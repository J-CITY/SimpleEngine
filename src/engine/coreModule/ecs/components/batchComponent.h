#pragma once
#include "component.h"
#include "materialRenderer.h"
#include "modelRenderer.h"

namespace IKIGAI::ECS { class Object; }

namespace IKIGAI::ECS {
	class BatchComponent : public Component {
	public:
		struct Descriptor : public Component::Descriptor {
			std::string Type;

			template<class Context>
			constexpr static auto serde(Context& context, Descriptor& value) {
				using Self = Descriptor;
				using namespace serde::attribute;
				serde::serde_struct(context, value)
					.field(&Self::Type, "BatchComponentType");
			}
		};
		inline static std::vector<unsigned> ids;
		BatchComponent(UTILS::Ref<ECS::Object> obj);
		BatchComponent(UTILS::Ref<ECS::Object> obj, const Component::Descriptor& descriptor) :
			BatchComponent(obj) {
		};
#ifdef OPENGL_BACKEND
		void init();
		void CreateAtlases(const MaterialRenderer& material);
		void createBuffers(ModelRenderer& model, MaterialRenderer& material);
#endif
	public:
		static auto GetMembers() {
			return std::tuple{
			};
		}
	};


	template <>
	inline std::string ECS::GetType<BatchComponent>() {
		return "class IKIGAI::ECS::BatchComponent";
	}
	template <>
	inline std::string IKIGAI::ECS::GetComponentName<BatchComponent>() {
		return "BatchComponent";
	}
}
