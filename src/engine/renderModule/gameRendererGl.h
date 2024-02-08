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
	VOLUMETRIC_LIGHT = 8,
	GOD_RAYS = 16,
	HDR = 32,
	COLOR_GRADING = 64,
	VIGNETTE = 128,
	DEPTH_OF_FIELD = 256,
	OUTLINE = 512,
	CHROMATIC_ABBERATION = 1024,
	POSTERIZE = 2048,
	PIXELIZE = 4096,
	SHARPEN = 8192,
	DILATION = 16384,
	FILM_GRAIN = 32768,
	FOG = 65536,
	GUI = 131072,
	SSAO = 262144,
	SSR = 524288,
	SSGI = 1048576,
	SSS = 2097152
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

		struct Fog {
			MATHGL::Vector3 color = MATHGL::Vector3(0.7f, 0.7f, 0.7f);
			float linearStart = 20.0f;
			float linearEnd = 75.0f;
			float density = 0.01f;
			int equation = 1;
			bool isEnabled = true;
		};

		struct VolumetricLight {
			float godRayMaxSteps = 150.0f;
			float godRaySampleStep = 0.15f;
			float godRayStepIncrement = 1.01f;
			float godRayAsymmetry = 0.5f;
			float maxDist= 10.0f;

			//debug
			bool tex = true;
			bool dir = true;
			bool map = false;
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
			Fog fog;
			VolumetricLight vl;

		};

		Pipeline mPipeline;

		std::shared_ptr<TextureGl> mRenderToScreenTexture;

		std::shared_ptr<FrameBufferGl> mDeferredFb;
		std::shared_ptr<ShaderGl> mDeferredShader;
		std::shared_ptr<TextureGl> mPrevDeferredTexture; //texture after deferred and forward render without post processing
		std::shared_ptr<TextureGl> mDeferredTexture; //texture after deferred and forward render without post processing

		std::shared_ptr<TextureGl> mEmptyTexture;
		std::shared_ptr<TextureGl> mEditorTexture;
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
		void renderEditorScene(IKIGAI::UTILS::Ref<IKIGAI::ECS::CameraComponent> editorCamera);
		void renderScene(IKIGAI::UTILS::Ref<IKIGAI::ECS::CameraComponent> mainCameraComponent);
		void renderToScreen();
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
		void applyFog();
		void applyChromaticAbberation();
		void applyPosterize();
		void applyPixelize();
		void applySharpen();
		void applyDilation();
		void applyFilmGrain();
		void applyMotionBlur();
		void applyTAA();
		void sendSSAOData();
		void applyVolumetricLight();
		void sendBounseDataToShader(std::shared_ptr<MaterialGl> material, ECS::Skeletal& animator, std::shared_ptr<ShaderGl> shader);
		void drawGUISubtree(UTILS::Ref<ECS::Object> obj);

		virtual void resize() override;

		void drawGUI();

		std::shared_ptr<FrameBufferGl> debug3dTextureFB;
		std::shared_ptr<TextureGl> debug3dTexture;
		int debug3dTextureLayers = 0;
		int debug3dTextureLayersCur = 0;
		bool debug3dTextureIsPersp = false;
		bool debug3dTextureIsRGB = false;
		void initDebug3dTextureFB(std::shared_ptr<TextureGl> _debug3dTexture);
		void updateDebug3dTextureFB();

		struct EngineDirShadowUBO {
			std::vector<MATHGL::Matrix4> lightSpaceMatrices;
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
			{ RenderStates::VOLUMETRIC_LIGHT, [this]() { applyVolumetricLight(); }},
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
			{ RenderStates::FOG, [this]() {applyFog(); } },
			{ RenderStates::GUI, [this]() {drawGUI(); } },
		};

		void preparePipeline();

		void prepareIBL();


		std::shared_ptr<FrameBufferGl> gbufferGlobalFb;
		std::shared_ptr<TextureGl> gPositionGlobalTex;
		std::shared_ptr<TextureGl> gEyePositionGlobalTex;
		std::shared_ptr<TextureGl> gVelocityGlobalTex;
	protected:
		
		EngineDirShadowUBO getLightSpaceMatrices(const MATHGL::Vector3& lightDir, const MATHGL::Vector3& lightPos);
		MATHGL::Matrix4 getLightSpaceMatrix(float nearPlane, float farPlane, const MATHGL::Vector3& lightDir, const MATHGL::Vector3& lightPos);
		bool prepareDirShadowMap(const std::string& id);
		bool prepareDirCascadeShadowMap(const std::string& id);
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
		void drawDrawableDebug(const Drawable& p_toDraw);
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
