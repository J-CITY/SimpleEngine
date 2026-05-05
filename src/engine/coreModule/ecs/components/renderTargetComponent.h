#pragma once
#include "component.h"
#include "mathModule/math.h"
#include "renderModule/backends/interface/frameBufferInterface.h"
#include "utilsModule/reflection/reflection_macros.h"
#include "utilsModule/enum.h"

namespace IKIGAI::ECS { class Object; }

IKIGAI_ENUM_NS(IKIGAI::ECS, RenderFlowType,
	NODE,
	NODE_AND_CHILD
)

namespace IKIGAI::ECS {
	IKI_CLASS()
	class RenderTargetComponent : public Component {
		IKI_GENERATED_BODY(RenderTargetComponent)
	public:
		IKI_CLASS(Name= RenderTargetComponent::Descriptor)
		struct Descriptor : public Component::Descriptor {
			IKI_GENERATED_BODY(RenderTargetComponent::Descriptor)
			IKI_PROPERTY(SEREALIZE(name="FlowType"))
			RenderFlowType flowType = RenderFlowType::NODE;
			IKI_PROPERTY(SEREALIZE(name = "FrameBuffer"))
			std::string frameBufferName;
			
		};
		RenderTargetComponent(UTILS::Ref<ECS::Object> obj);
		RenderTargetComponent(UTILS::Ref<ECS::Object> obj, const Descriptor& descriptor);
		RenderTargetComponent(UTILS::Ref<ECS::Object> obj, const Component::Descriptor& descriptor) :
			RenderTargetComponent(obj, static_cast<const Descriptor&>(descriptor)) {
		}

		void setName(const std::string& name);
		const std::string& getName() const;
		RenderFlowType getType() const;
		std::shared_ptr<RENDER::FrameBufferInterface> getFrameBuffer();

		[[nodiscard]] Descriptor getDescriptor() const;
	public:
		static auto GetMembers() {
			return std::tuple{
			};
		}

	private:
		std::string mFrameBufferName;
		std::shared_ptr<RENDER::FrameBufferInterface> mFrameBuffer;
		RenderFlowType mFlowType = RenderFlowType::NODE;
	};

	template <>
	inline std::string ECS::GetType<RenderTargetComponent>() {
		return "class IKIGAI::ECS::RenderTargetComponent";
	}

	template <>
	inline std::string IKIGAI::ECS::GetComponentName<RenderTargetComponent>() {
		return "RenderTargetComponent";
	}
}

#include "generated/renderTargetComponent.generated.h"
