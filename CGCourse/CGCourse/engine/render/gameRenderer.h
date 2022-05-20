#pragma once

#include <map>
#include "Camera.h"
#include "drawable.h"
#include "Material.h"
#include "primitiveRender.h"
#include "../utils/math/Matrix4.h"
#include "Model.h"
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
		//FrameBuffer hdrFBO;

		//Texture, which draw on screen
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
		std::shared_ptr<RESOURCES::Texture> finalTextureBeforePostprocessing;
	};

	class Renderer : public BaseRender {

		unsigned int clearMask = 0;
	public:
		enum class PostProcessing {
			BLOOM = 0u,
			GOOD_RAYS,
			MOTION_BLUR,
			FXAA,
			HDR
		};
		//defered render
		//unsigned int gBuffer;
		//unsigned int gPosition, gNormal, gAlbedoSpec;

		RenderPipeline pipeline;
		std::unordered_map<std::string, std::shared_ptr<RESOURCES::Shader>> shadersMap;


		OpaqueDrawables	opaqueMeshesForward;
		TransparentDrawables transparentMeshesForward;

		OpaqueDrawables	opaqueMeshesDeferred;
		TransparentDrawables transparentMeshesDeferred;


		Renderer(GL_SYSTEM::GlManager& driver, CORE_SYSTEM::Core& context);
		~Renderer();
		RENDER::UniformBuffer& getUBO() const {
			return *engineUBO;
		}
		void renderScene();

		void renderSkybox();
		void renderDirShadowMap();
		void updateEngineUBO(ECS::CameraComponent& mainCamera);
		void updateLights(SCENE_SYSTEM::Scene& scene);
		void updateLightsInFrustum(SCENE_SYSTEM::Scene& scene, const Frustum& frustum);

		void renderScene(std::shared_ptr<RESOURCES::Shader> shader);

		void drawDrawable(const Drawable& toDraw);
		void drawDrawable(const Drawable& p_toDraw, std::shared_ptr<RESOURCES::Shader> shader);
		void drawModelWithSingleMaterial(Model& model, Material& material, MATHGL::Matrix4 const* modelMatrix, Material* defaultMaterial = nullptr);
		void drawModelWithMaterials(Model& model, std::vector<Material*> materials, MATHGL::Matrix4 const* modelMatrix, Material* defaultMaterial = nullptr);

		//void drawGBuffer(RESOURCES::Mesh& p_mesh, Material& p_material, MATHGL::Matrix4 const* p_modelMatrix, std::shared_ptr<RESOURCES::Shader> shader);
		void drawMeshWithShader(RESOURCES::Mesh& p_mesh, Material& p_material, MATHGL::Matrix4 const* p_modelMatrix, std::shared_ptr<RESOURCES::Shader> shader);
		void drawMesh(RESOURCES::Mesh& mesh, Material& material, MATHGL::Matrix4 const* modelMatrix, bool useTexutres = true);
		void registerModelMatrixSender(std::function<void(MATHGL::Matrix4)> modelMatrixSender);
		void clear() const;
		void clearDepth() const;
		void flush() const {
			glFlush();

		}
		void useColorMask(bool r, bool g, bool b, bool a) {
			glColorMask(r, g, b, a);
		}


		void setdBounseDataToShader(Material& material, std::shared_ptr<ECS::Skeletal> animator, RESOURCES::Shader& shader);
		void sendShadowDirData(RESOURCES::Shader& shader);
		void sendShadowPointData(RESOURCES::Shader& shader);
		void sendShadowSpotData(RESOURCES::Shader& shader);
		void sendFogData(RESOURCES::Shader& shader);
		void Renderer::sendIBLData(RESOURCES::Shader& shader);
		void Renderer::sendSSAOData(RESOURCES::Shader& shader);

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

		void configuratePostProcessing();

		void setPostProcessing(PostProcessing type, bool isEnable) {
			if (isEnable == postProcessingState[static_cast<unsigned>(type)]) {
				return;
			}
			postProcessingState[static_cast<unsigned>(type)] = isEnable;
			configuratePostProcessing();
		}

		void init();
		MATHGL::Vector2f getShadowMapResolution();

		struct CustomPostProcessing {
			std::string name;
			std::shared_ptr<Material> material;
			bool isEnabled = true;
		};

		void addCustomPostRocessing(std::string name, std::shared_ptr<Material> material, bool isEnabled = true);
	public:
		void initShaders();
		std::vector<std::shared_ptr<KUMA::GUI::GuiObject>> guiObjs;

		enum class PipelineRenderShaderType {
			POINT_SHADOW,
			DIR_SHADOW
		};
		std::function<void(MATHGL::Matrix4)> modelMatrixSender;
		std::shared_ptr<RESOURCES::Texture> emptyTexture;


		std::unique_ptr<RENDER::UniformBuffer>       engineUBO;
		std::unique_ptr<RENDER::ShaderStorageBuffer> lightSSBO;


		std::shared_ptr<Material> emptyMaterial;

		CORE_SYSTEM::Core& context;

		bool currentSwapBuffer = 0;
		std::array<FrameBuffer, 2> swapBuffers;
		std::array<std::shared_ptr<RESOURCES::Texture>, 2> swapTextures;

		//RESOURCES::Texture textureForGodRays;
		//move later to component
		//Game::World* world;

		std::shared_ptr<RENDER::Model> sphere;


		std::vector<bool> postProcessingState = std::vector<bool>(5, true);
		std::vector<std::function<void()>> postProcessingFuncs;

		std::vector<CustomPostProcessing> customPostProcessing;
		
	};
}
