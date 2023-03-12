#pragma once
#ifdef VULKAN_BACKEND
#include "backends/vk/storageBufferVk.h"
#include "backends/vk/uniformBufferVk.h"
#include "drawable.h"
#include "frustum.h"
#include "gameRendererInterface.h"
#include "../../../utils/pointers/objPtr.h"
#include "backends/vk/driverVk.h"

import glmath;


namespace KUMA::SCENE_SYSTEM {
	class Scene;
}

namespace KUMA::ECS {
	class CameraComponent;
}

namespace KUMA::CORE_SYSTEM {
	class Core;
}

namespace KUMA::RENDER {
	struct LightOGL;

	class GameRendererVk:  public GameRendererInterface {
	public:
		GameRendererVk(KUMA::CORE_SYSTEM::Core& context);
		void createVkResources();
		void renderScene() override;

		DriverVk* getDriver() { return mDriver; }
	//protected:
		std::shared_ptr<TextureVk> mEmptyTexture;

		std::shared_ptr<ShaderStorageBufferVk<LightOGL>> mLightSSBO;
		//std::shared_ptr<UniformBufferVk<LightOGL>> mLightUBO;
		std::shared_ptr<UniformTexturesVk> mDeferredTexturesUniform;
		std::shared_ptr<UniformTexturesVk> mRenderToScreenTexturesUniform;
		std::shared_ptr<UniformBufferVk<EngineUBO>> mEngineUbo;

		std::unordered_map<std::string, std::shared_ptr<TextureVk>> mTextures;
		std::unordered_map<std::string, std::shared_ptr<FrameBufferVk>> mFramebuffers;
		std::unordered_map<std::string, std::shared_ptr<ShaderVk>> mShaders;

		void drawDeferredGBuffer();

		void sendEngineUBO();
		void renderScene(KUMA::Ref<KUMA::ECS::CameraComponent> mainCameraComponent);

		void drawDrawableDeferred(const Drawable& p_toDraw);

		void updateLights(SCENE_SYSTEM::Scene& scene);
		void updateLightsInFrustum(SCENE_SYSTEM::Scene& scene, const Frustum& frustum);

		KUMA::CORE_SYSTEM::Core& mContext;

		OpaqueDrawables	mOpaqueMeshesForward;
		TransparentDrawables mTransparentMeshesForward;

		OpaqueDrawables	mOpaqueMeshesDeferred;
		TransparentDrawables mTransparentMeshesDeferred;
		DriverVk* mDriver;
	};
}
#endif

