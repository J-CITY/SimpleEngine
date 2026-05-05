#include "renderTargetComponent.h"

#include "renderModule/gameRendererGl.h"
#include "renderModule/gameRendererInterface.h"

namespace IKIGAI::ECS {
	//TODO: add signal update frame buffer? when set render pipeline
	//TODO: add fb to drawable
	RenderTargetComponent::RenderTargetComponent(UTILS::Ref<ECS::Object> _obj) : Component(_obj) {
		__NAME__ = "RenderTargetComponent";

	}

	RenderTargetComponent::RenderTargetComponent(UTILS::Ref<ECS::Object> _obj, const Descriptor& descriptor) : RenderTargetComponent(_obj) {
		mFlowType = descriptor.flowType;
		mFrameBufferName = descriptor.frameBufferName;
	}

	void RenderTargetComponent::setName(const std::string& name) {
		mFrameBufferName = name;
	}

	const std::string& RenderTargetComponent::getName() const {
		return mFrameBufferName;
	}

	RenderFlowType RenderTargetComponent::getType() const {
		return mFlowType;
	}

	std::shared_ptr<RENDER::FrameBufferInterface> RenderTargetComponent::getFrameBuffer() {
		auto& render = RESOURCES::ServiceManager::Get<RENDER::GameRendererInterface>();
		auto& pipeline = render.getCurrentPipeline();
		if (pipeline.mFrameBuffers.contains(mFrameBufferName)) {
			mFrameBuffer = pipeline.mFrameBuffers.at(mFrameBufferName);
		}
		else {
			mFrameBuffer = nullptr;
		}
		return mFrameBuffer;
	}

	RenderTargetComponent::Descriptor RenderTargetComponent::getDescriptor() const {
		Descriptor descriptor;
		descriptor.flowType = mFlowType;
		descriptor.frameBufferName = mFrameBufferName;
		return descriptor;
	}
}
