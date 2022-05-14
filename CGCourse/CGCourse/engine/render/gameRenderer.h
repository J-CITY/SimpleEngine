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
	};
	struct SpotLightData {
		uint32_t id;
		MATHGL::Matrix4 projMap;
	};
	struct PointLightData {
		uint32_t id;
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
	};

	struct ImageBasedLightning {
		//std::shared_ptr<RESOURCES::Texture> irradianceMap;
		//std::shared_ptr<RESOURCES::Texture> prefilterMap;
		std::shared_ptr<RESOURCES::Texture> brdfLUTTexture;
	};

	struct RenderPipeline {
		FrameBuffer depthMapFBO;

		std::vector<DirLightData> dirLightsData;
		std::vector<SpotLightData> spotLightsData;
		std::vector<PointLightData> pointLightsData;

		ShadowLightData shadowLightData;

		Hdr hdr;
		Bloom bloom;
		MotionBlur motionBlur;
		GodRays godRays;

		ScreenSpaceAmbientOcclusion ssao;
		ImageBasedLightning ibl;

		FrameBuffer finalFBOBeforePostprocessing;
		std::shared_ptr<RESOURCES::Texture> finalTextureBeforePostprocessing;
	};

	class Renderer : public BaseRender {

		unsigned int clearMask = 0;
	public:

		//defered render
		unsigned int gBuffer;
		unsigned int gPosition, gNormal, gAlbedoSpec;

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

		void drawGBuffer(RESOURCES::Mesh& p_mesh, Material& p_material, MATHGL::Matrix4 const* p_modelMatrix, std::shared_ptr<RESOURCES::Shader> shader);
		void drawDirShadowMap(RESOURCES::Mesh& p_mesh, Material& p_material, MATHGL::Matrix4 const* p_modelMatrix, std::shared_ptr<RESOURCES::Shader> shader);
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


		void prepareDirLightShadowMap();
		void prepareSpotLightShadowMap();
		void preparePointLightShadowMap();

		void init();

		MATHGL::Vector2f getShadowMapResolution();
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

		RESOURCES::Texture textureForGodRays;
		//move later to component
		//Game::World* world;

		std::shared_ptr<RENDER::Model> sphere;
	};
}
