#pragma once
#include "backends/gl/materialGl.h"
#include "utilsModule/enum.h"
#ifdef OPENGL_BACKEND
#include "drawable.h"
#include "frustum.h"
#include "gameRendererInterface.h"
#include <utilsModule/pointers/objPtr.h>

import glmath;


namespace IKIGAI::SCENE_SYSTEM {
	class Scene;
}

namespace IKIGAI::ECS {
	class CameraComponent;
}

namespace IKIGAI::CORE_SYSTEM {
		class Core;
}

IKIGAI_ENUM_NS(IKIGAI::RENDER, RenderStates,
	TAA = 1,
	MOTION_BLUR = 2,
	BLOOM = 4,
	GOD_RAYS = 8,
	HDR = 16,
	COLOR_GRADING = 32,
	VIGNETTE = 64,
	DEPTH_OF_FIELD = 128,
	OUTLINE = 256,
	CHROMATIC_ABBERATION = 512,
	POSTERIZE = 1024,
	PIXELIZE = 2048,
	SHARPEN = 4096,
	DILATION = 8192,
	FILM_GRAIN = 16384,
	GUI = 32768,
	SSAO = 65536,
	SSR = 131072,
	SSGI = 262144,
	SSS = 524288
)

namespace IKIGAI::RENDER
{
	DEFINE_ENUM_CLASS_BITWISE_OPERATORS(RenderStates)
}


namespace IKIGAI::RENDER {

	class FrameBufferGl;
	class TextureGl;
	class ShaderGl;
	class DriverGl;
	class ShaderStorageBufferGl;

	class GameRendererGl:  public GameRendererInterface {
	public:
		struct SSAO {
			std::vector<MATHGL::Vector3> mSSAOKernel;
			bool mUseSSAO = true;
		};
		struct SSR {
			bool mUse = true;
		};
		struct SSGI {
			bool mUse = false;
		};
		struct SSS {
			bool mUse = false;
		};
		struct DirShadowMap {
			unsigned int mDirShadowMapResolution = 4096;
			float mDirNearPlane = 0.1f;
			float mDirFarPlane = 500.0f;
			std::vector<float> mShadowCascadeLevels{ mDirFarPlane / 50.0f, mDirFarPlane / 25.0f, mDirFarPlane / 10.0f, mDirFarPlane / 2.0f };
			MATHGL::Matrix4 dirLightSpaceMatrix;

			unsigned int mSpotShadowMapResolution = 1024;
			float mSpotNearPlane = 1.0f;
			float mSpotFarPlane = 8.0;

			unsigned int mPointShadowMapResolution = 1024;
			float mPointNearPlane = 1.0f;
			float mPointFarPlane = 25.0f;

			bool mUseShadow = true;
			bool mUseShadowBaked = true;
			bool mIsShadowBakedInit = false;
		};

		struct ChromaticAbberation {
			MATHGL::Vector3 params = MATHGL::Vector3(0.8f, 0.08f, 0.8f);
		};

		struct HDR {
			bool isEnabled = true;
			float exposure = 1.0f;
			float gamma = 2.2f;
		};

		struct ColorGrading {
			MATHGL::Vector3 r = MATHGL::Vector3(1.0f, 0.0f, 0.0f);
			MATHGL::Vector3 g = MATHGL::Vector3(0.0f, 1.0f, 0.0f);
			MATHGL::Vector3 b = MATHGL::Vector3(0.0f, 0.0f, 1.0f);
		};

		struct Vignette {
			float radius = 0.1f;
			float intensity = 100.0f;
		};

		struct IBL {
			bool useIBL = true;
		};

		struct Pipeline {
			MATHGL::Vector4 mClearColor = MATHGL::Vector4(0.0f, 0.0f, 0.50f, 1.0f);

			bool mIsPBR = false;

			DirShadowMap mDirShadowMap;

			SSAO mSSAO;
			SSR mSSR;
			SSGI mSSGI;
			SSS mSSS;

			ChromaticAbberation chromaticAbberation;
			HDR hdr;
			ColorGrading colorGrading;
			Vignette vignette;
			IBL ibl;

		};
		Pipeline mPipeline;

		std::shared_ptr<TextureGl> mRenderToScreenTexture;

		std::shared_ptr<FrameBufferGl> mDeferredFb;
		std::shared_ptr<ShaderGl> mDeferredShader;
		std::shared_ptr<TextureGl> mPrevDeferredTexture; //texture after deferred and forward render without post processing
		std::shared_ptr<TextureGl> mDeferredTexture; //texture after deferred and forward render without post processing

		std::shared_ptr<TextureGl> mEmptyTexture;
		std::shared_ptr<TextureGl> mHDRSkyBoxTexture;

		std::unordered_map<std::string, std::shared_ptr<TextureGl>> mTextures;
		bool pingPong = false;
		std::array<std::shared_ptr<TextureGl>, 2> pingPongTex;

		void setSkyBoxTexture(const std::string& path);
		GameRendererGl(IKIGAI::CORE_SYSTEM::Core& context);
		void createShaders();
		void sendEngineUBO();
		void sendEngineShadowUBO(std::shared_ptr<ShaderGl> shader);
		void renderScene();
		void renderScene(IKIGAI::Ref<IKIGAI::ECS::CameraComponent> mainCameraComponent);
		void sendIBLData();
		void prepareBrightTexture();
		bool prepareBlurTexture(std::shared_ptr<TextureGl> tex);
		void prepareGodRaysTexture();
		void applyGoodRays();
		void applyBloom();
		void applyFXAA();
		void applyHDR();
		void applyColorGrading();
		void applyVignette();
		void applyDepthOfField();
		void applyOutline();
		void applyChromaticAbberation();
		void applyPosterize();
		void applyPixelize();
		void applySharpen();
		void applyDilation();
		void applyFilmGrain();
		void applyMotionBlur();
		void applyTAA();
		void sendSSAOData();
		void sendBounseDataToShader(std::shared_ptr<MaterialGl> material, ECS::Skeletal& animator, std::shared_ptr<ShaderGl> shader);
		void drawGUISubtree(Ref<ECS::Object> obj);

		virtual void resize() override;

		void drawGUI();

		struct EngineDirShadowUBO {
			MATHGL::Matrix4 lightSpaceMatrices[16];
		};

		std::map<std::string, bool> activeCustomPP;
		std::map<std::string, std::shared_ptr<MaterialGl>> customPostProcessing;

		std::shared_ptr<MaterialGl> skyBoxMaterial;

		void setSkyBoxMaterial(std::shared_ptr<MaterialInterface> m);

		void addCustomPostProcessing(const std::string& name, std::shared_ptr<MaterialGl> material, bool isActive=true);

		RenderStates renderStateMask = (RenderStates::BLOOM | RenderStates::GUI);

		std::vector<std::function<void()>> ppFuncs;
		std::map<RenderStates, std::function<void()>> typeToFuncPP {
			{ RenderStates::TAA, [this]() { applyTAA(); }},
			{ RenderStates::MOTION_BLUR, [this]() { applyMotionBlur(); }},
			{ RenderStates::BLOOM, [this]() { applyBloom(); }},
			{ RenderStates::GOD_RAYS, [this]() { applyGoodRays(); }},
			{ RenderStates::HDR, [this]() { applyHDR(); } },
			{ RenderStates::COLOR_GRADING, [this]() {applyColorGrading(); } },
			{ RenderStates::VIGNETTE, [this]() {applyVignette(); } },
			{ RenderStates::DEPTH_OF_FIELD, [this]() {applyDepthOfField(); } },
			{ RenderStates::OUTLINE, [this]() {applyOutline(); } },
			{ RenderStates::CHROMATIC_ABBERATION, [this]() {applyChromaticAbberation(); } },
			{ RenderStates::POSTERIZE, [this]() {applyPosterize(); }},
			{ RenderStates::PIXELIZE, [this]() {applyPixelize(); } },
			{ RenderStates::SHARPEN, [this]() {applySharpen(); } },
			{ RenderStates::DILATION, [this]() {applyDilation(); } },
			{ RenderStates::FILM_GRAIN, [this]() {applyFilmGrain(); } },
			{ RenderStates::GUI, [this]() {drawGUI(); } },
		};

		void preparePipeline();

		void prepareIBL();
	protected:
		
		EngineDirShadowUBO getLightSpaceMatrices(const MATHGL::Vector3& lightDir, const MATHGL::Vector3& lightPos);
		MATHGL::Matrix4 getLightSpaceMatrix(float nearPlane, float farPlane, const MATHGL::Vector3& lightDir, const MATHGL::Vector3& lightPos);
		bool prepareDirShadowMap(const std::string& id);
		void prepareSpotShadow();
		void preparePointShadow();

		void drawDeferredGBuffer();
		void drawForward();

		void drawDrawableBatching(const Drawable& p_toDraw);
		void createFrameBuffers();
		void applySSS();
		void applySSR();
		void applySSGI();
		void renderSkybox();
		void applySSAO();

		void updateLights(IKIGAI::SCENE_SYSTEM::Scene& scene);
		void updateLightsInFrustum(SCENE_SYSTEM::Scene& scene, const Frustum& frustum);
		void drawDrawable(const Drawable& p_toDraw);
		void drawMesh(const Drawable& p_toDraw);
		void drawDrawableDeferred(const Drawable& p_toDraw);
		void drawDrawableWithShader(std::shared_ptr<ShaderGl> shader, const Drawable& p_toDraw);


		void sendEngineUBO(ShaderGl& shader, const MATHGL::Matrix4& world);
		void sendEngineShadowData(ShaderGl& shader);

		IKIGAI::CORE_SYSTEM::Core& mContext;

		OpaqueDrawables	mOpaqueMeshesForward;
		TransparentDrawables mTransparentMeshesForward;

		OpaqueDrawables	mOpaqueMeshesDeferred;
		TransparentDrawables mTransparentMeshesDeferred;


		std::shared_ptr<ShaderStorageBufferGl> mLightSSBO;

		DriverGl* mDriver;

		int frameCount = 0;
	};
}
#endif
