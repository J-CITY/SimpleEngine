#pragma once

#include <map>
#include "camera.h"
#include "drawable.h"
#include "material.h"
#include "primitiveRender.h"
import glmath;
#include "model.h"
#include "../../game/World.h"
#include "../gui/guiObject.h"
#include "buffers/depthBuffer.h"

namespace KUMA {
	namespace ECS {
		class CameraComponent;
		class PointLight;
		class DirectionalLight;
		class SpotLight;
	}
}

namespace KUMA {
	namespace SCENE_SYSTEM {
		class Scene;
	}
}

namespace KUMA {
	namespace CORE_SYSTEM {
		class Core;
	}
}

namespace KUMA::RENDER {
	class ShaderStorageBuffer;
	class UniformBuffer;
	struct LightOGL;


	struct DirLightData {
		uint32_t id;
		MATHGL::Matrix4 projMap;
		MATHGL::Vector3 pos;
		bool useShadow = true;
	};
	struct SpotLightData {
		uint32_t id;
		MATHGL::Matrix4 projMap;
		MATHGL::Vector3 pos;
		float nearPlane;
		float farPlane;
		bool useShadow = false;
	};

	struct PointInfo {
		MATHGL::Vector3 pos;
		unsigned id;
	};
	struct PointLightData {
		int size;
		float farPlane;
		std::array<PointInfo, 4> data;
		bool useShadow = true;
	};

	enum class ShadowMapResolution {
		LOW, MEDIUM, HIGH
	};

	struct ShadowLightData {
		ShadowMapResolution resolution = ShadowMapResolution::MEDIUM;
	};

	struct Hdr {
		bool isEnabled = true;
		float exposure = 1.0f;
		float gamma = 2.2f;
	};

	struct Bloom {
		std::shared_ptr<RESOURCES::Texture> brightTexture;
		std::array<FrameBuffer, 2> pingpongFBO;
		std::array<std::shared_ptr<RESOURCES::Texture>, 2> pingpongColorbuffers;
	};

	struct MotionBlur {
		bool isInit = false;
		std::vector<std::shared_ptr<RESOURCES::Texture>> motionBlurTextures;
	};

	struct GodRays {
		std::shared_ptr<RESOURCES::Texture> godRaysTexture;
	};

	struct ScreenSpaceAmbientOcclusion {
		FrameBuffer ssaoFBO;
		FrameBuffer ssaoBlurFBO;
		std::shared_ptr<RESOURCES::Texture> ssaoColorBuffer;
		std::shared_ptr<RESOURCES::Texture> ssaoColorBufferBlur;
		std::vector<MATHGL::Vector3> ssaoKernel;
		std::shared_ptr<RESOURCES::Texture> noiseTexture;
		bool useSSAO = true;
	};

	struct ImageBasedLightning {
		std::shared_ptr<RESOURCES::CubeMap> irradianceMap;
		std::shared_ptr<RESOURCES::CubeMap> prefilterMap;
		std::shared_ptr<RESOURCES::Texture> brdfLUTTexture;
		bool useIBL = true;
	};

	struct DeferredRender {
		FrameBuffer gBuffer;
		std::shared_ptr<RESOURCES::Texture> gPosition;
		std::shared_ptr<RESOURCES::Texture> gNormal;
		std::shared_ptr<RESOURCES::Texture> gAlbedoSpec;
		std::shared_ptr<RESOURCES::Texture> gRoughAO;
	};

	struct Fog {
		MATHGL::Vector3 color = MATHGL::Vector3(0.2f, 0.2f, 0.2f);
		float density = 0.01f;
		int equation = 2;
		bool isEnabled = true;
		float linearStart = 0.0f;
		float linearEnd = 0.0f;
	};

	struct Deffered {
		bool usePbr = false;
	};

	struct Vignette {
		float radius = 0.1f;
		float intensity = 100.0f;
	};

	struct ColorGrading {
		MATHGL::Vector3 r = MATHGL::Vector3(1.0f, 0.0f, 0.0f);
		MATHGL::Vector3 g = MATHGL::Vector3(0.0f, 1.0f, 0.0f);
		MATHGL::Vector3 b = MATHGL::Vector3(0.0f, 0.0f, 1.0f);
	};

	struct ChromaticAbberation {
		MATHGL::Vector3 params = MATHGL::Vector3(0.8f, 0.08f, 0.8f);
	};

	struct RenderPipeline {
		Deffered deferred;
		FrameBuffer depthMapFBO;
		std::vector<DirLightData> dirLightsData;
		SpotLightData spotLightData;
		PointLightData pointLightsData;
		ShadowLightData shadowLightData;
		Hdr hdr;
		Bloom bloom;
		MotionBlur motionBlur;
		GodRays godRays;
		Fog fog;
		DeferredRender deferredRender;
		ScreenSpaceAmbientOcclusion ssao;
		ImageBasedLightning ibl;
		FrameBuffer finalFBOBeforePostprocessing;
		Vignette vignette;
		ColorGrading colorGrading;
		ChromaticAbberation chromaticAbberation;
		std::shared_ptr<RESOURCES::Texture> finalTextureBeforePostprocessing;

		std::shared_ptr<RESOURCES::Texture> globalDepth;
	};

	class Renderer : public BaseRender {
	public:
		enum class PostProcessing {
			BLOOM = 0u,
			GOOD_RAYS,
			MOTION_BLUR,
			FXAA,
			HDR,
			VIGNETTE,
			COLOR_GRADING,
			CHROMATIC_ABBERATION,
			VOLUMETRIC_LIGHT
		};

		Renderer(GL_SYSTEM::GlManager& driver, CORE_SYSTEM::Core& context);
		~Renderer();
		[[nodiscard]] RENDER::UniformBuffer& getUBO() const;
		void renderScene();
		void initShaders();
		void setPostProcessing(PostProcessing type, bool isEnable);
		void addCustomPostRocessing(std::string name, std::shared_ptr<Material> material, bool isEnabled = true);
		void setCustomPostRocessing(std::string name, bool isEnabled);
		const RESOURCES::Texture& getResultTexture();
	private:
		RenderPipeline pipeline;
		std::unordered_map<std::string, std::shared_ptr<RESOURCES::Shader>> shaderStorage;
		bool currentSwapBuffer = 0;
		std::array<FrameBuffer, 2> swapBuffers;
		std::array<std::shared_ptr<RESOURCES::Texture>, 2> swapTextures;

		std::shared_ptr<RESOURCES::Texture> emptyTexture;
		std::shared_ptr<Material> emptyMaterial;
		std::shared_ptr<RENDER::Model> sphere;

		OpaqueDrawables	opaqueMeshesForward;
		TransparentDrawables transparentMeshesForward;

		OpaqueDrawables	opaqueMeshesDeferred;
		TransparentDrawables transparentMeshesDeferred;

		std::vector<bool> postProcessingState = std::vector(9, true);
		std::vector<std::function<void()>> postProcessingFuncs;
		struct CustomPostProcessing {
			std::string name;
			std::shared_ptr<Material> material;
			bool isEnabled = true;
		};
		std::unordered_map<std::string, CustomPostProcessing> customPostProcessing;

		std::unique_ptr<RENDER::UniformBuffer>       engineUBO;
		std::unique_ptr<RENDER::ShaderStorageBuffer> lightSSBO;

		std::function<void(MATHGL::Matrix4)> modelMatrixSender;

		void init();
		void renderScene(std::shared_ptr<RESOURCES::Shader> shader);
		void renderSkybox();
		void renderDirShadowMap();
		void updateEngineUBO(ECS::CameraComponent& mainCamera);
		void updateLights(SCENE_SYSTEM::Scene& scene);
		void updateLightsInFrustum(SCENE_SYSTEM::Scene& scene, const Frustum& frustum);

		void drawDrawable(const Drawable& toDraw);
		void drawDrawable(const Drawable& p_toDraw, RESOURCES::Shader& shader);
		void drawModelWithSingleMaterial(const Model& model, Material& material, const MATHGL::Matrix4& modelMatrix, Material& defaultMaterial);
		void drawModelWithMaterials(const Model& model, std::vector<std::shared_ptr<Material>> materials, const MATHGL::Matrix4& modelMatrix, Material& defaultMaterial);
		void drawMeshWithShader(const RESOURCES::Mesh& p_mesh, const Material& p_material, const MATHGL::Matrix4& p_modelMatrix, const RESOURCES::Shader& shader);
		void drawMesh(const RESOURCES::Mesh& mesh, Material& material, const MATHGL::Matrix4& modelMatrix, bool useTexutres=true, bool sendModel=true);
		void registerModelMatrixSender(std::function<void(MATHGL::Matrix4)> modelMatrixSender);

		void clearDepth() const;
		void flush() const;
		void useColorMask(bool r, bool g, bool b, bool a) const;

		void sendBounseDataToShader(Material& material, ECS::Skeletal& animator, RESOURCES::Shader& shader);
		void sendShadowDirData(RESOURCES::Shader& shader);
		void sendShadowPointData(RESOURCES::Shader& shader);
		void sendShadowSpotData(RESOURCES::Shader& shader);
		void sendFogData(RESOURCES::Shader& shader);
		void sendIBLData(RESOURCES::Shader& shader);
		void sendSSAOData(RESOURCES::Shader& shader);

		void prepareDirLightShadowMap();
		void prepareSpotLightShadowMap();
		void preparePointLightShadowMap();
		void prepareTexturesForPostprocessing();

		void applyBloom();
		void applyGoodRays();
		void applyMotionBlur();
		void applyFXAA();
		void applyHDR();
		void applySSAO();
		void applyDeferred();
		void applyDeferredPbr();
		void configurePostProcessing();
		void applyVolumetricLight();
		void applyColorGrading();
		void applyVignette();
		void applyChromaticAbberation();
		void renderResultToScreen();

		[[nodiscard]] MATHGL::Vector2f getShadowMapResolution() const;
	public:
		
		//TODO: remove it
		CORE_SYSTEM::Core& context;

		static MATHGL::Matrix4 guiProjection;

		//KUMA::GUI::Font f;
		//std::shared_ptr<KUMA::GUI::Sprite> s;
		//move later to component
		//Game::World* world;
	};
}
