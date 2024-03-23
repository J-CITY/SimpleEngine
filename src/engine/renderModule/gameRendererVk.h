#pragma once
#ifdef VULKAN_BACKEND
#include "backends/vk/storageBufferVk.h"
#include "backends/vk/uniformBufferVk.h"
#include "drawable.h"
#include "frustum.h"
#include "gameRendererInterface.h"
#include "backends/vk/driverVk.h"


namespace IKIGAI::SCENE_SYSTEM {
	class Scene;
}

namespace IKIGAI::ECS {
	class CameraComponent;
}

namespace IKIGAI::CORE {
	class Core;
}

namespace IKIGAI::RENDER {
	struct LightOGL;

	class GameRendererVk:  public GameRendererInterface {
	public:
		GameRendererVk(IKIGAI::CORE::Core& context);
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
		void renderScene(IKIGAI::UTILS::Ref<IKIGAI::ECS::CameraComponent> mainCameraComponent);

		void drawDrawableDeferred(const Drawable& p_toDraw);

		void updateLights(SCENE_SYSTEM::Scene& scene);
		void updateLightsInFrustum(SCENE_SYSTEM::Scene& scene, const Frustum& frustum);

		IKIGAI::CORE::Core& mContext;

		OpaqueDrawables	mOpaqueMeshesForward;
		TransparentDrawables mTransparentMeshesForward;

		OpaqueDrawables	mOpaqueMeshesDeferred;
		TransparentDrawables mTransparentMeshesDeferred;
		DriverVk* mDriver;
#if defined(USE_EDITOR) || defined(USE_CHEATS)
		void initForVk();
#endif
	};
}
#endif

