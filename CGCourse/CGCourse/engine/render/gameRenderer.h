#pragma once

#include <map>
#include "Camera.h"
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
	struct LightOGL;
	class GameRenderer {
	public:
		std::vector<std::shared_ptr<KUMA::GUI::GuiObject>> guiObjs;
		
		GameRenderer(CORE_SYSTEM::Core& context);
		void renderScene();
		void updateEngineUBO(ECS::CameraComponent& mainCamera);
		void updateLights(SCENE_SYSTEM::Scene& scene);
		void updateLightsInFrustum(SCENE_SYSTEM::Scene& scene, const Frustum& frustum);
		
		//move later to component
		Game::World* world;
		
	public:
		void renderSkybox();
		
		CORE_SYSTEM::Core& context;
		Material emptyMaterial;
	};

	struct DirLightData {
		uint32_t id;
		MATHGL::Matrix4 projMap;
	};
	struct SpotLightData {
		uint32_t id;
		MATHGL::Matrix4 projMap;
	};
	struct PointLightData {

	};

	struct RenderPipeline {
		FrameBuffer depthMapFBO;

		std::vector<DirLightData> dirLightsData;
		std::vector<SpotLightData> spotLightsData;
		std::vector<PointLightData> pointLightsData;

	};

	//TODO: move to new file
	class Renderer : public BaseRender {

		unsigned int clearMask = 0;
	public:
		struct Drawable {
			MATHGL::Matrix4 world;
			RESOURCES::Mesh* mesh;
			Material* material;
			MATHGL::Matrix4 userMatrix;
		};

		RenderPipeline pipeline;
		std::unordered_map<std::string, std::shared_ptr<RESOURCES::Shader>> shadersMap;



		using OpaqueDrawables = std::multimap<float, Drawable, std::less<float>>;
		using TransparentDrawables = std::multimap<float, Drawable, std::greater<float>>;
		OpaqueDrawables	opaqueMeshes;
		TransparentDrawables transparentMeshes;

		Renderer(GL_SYSTEM::GlManager& driver);
		~Renderer();

		std::shared_ptr<ECS::CameraComponent> findMainCamera(const SCENE_SYSTEM::Scene& scene);
		std::vector<LightOGL> findLightMatrices(const SCENE_SYSTEM::Scene& scene);
		std::vector<LightOGL> findLightMatricesInFrustum(const SCENE_SYSTEM::Scene& scene, const Frustum& frustum);
		void renderScene(std::shared_ptr<RESOURCES::Shader> shader);

		void FindDrawables(const MATHGL::Vector3& p_cameraPosition,
			const Camera& p_camera,
			const Frustum* p_customFrustum,
			Material* p_defaultMaterial);
		
		std::pair<OpaqueDrawables, TransparentDrawables> findAndSortFrustumCulledDrawables(
			const MATHGL::Vector3& cameraPosition,
			const Frustum& frustum,
			Material* defaultMaterial
		);

		
		std::pair<OpaqueDrawables, TransparentDrawables> findAndSortDrawables(
			const MATHGL::Vector3& cameraPosition,
			Material* defaultMaterial
		);
		void drawDrawable(const Drawable& toDraw);
		void drawDrawable(const Drawable& p_toDraw, std::shared_ptr<RESOURCES::Shader> shader);
		void drawModelWithSingleMaterial(Model& model, Material& material, MATHGL::Matrix4 const* modelMatrix, Material* defaultMaterial = nullptr);
		void drawModelWithMaterials(Model& model, std::vector<Material*> materials, MATHGL::Matrix4 const* modelMatrix, Material* defaultMaterial = nullptr);

		void drawGBuffer(RESOURCES::Mesh& p_mesh, Material& p_material, MATHGL::Matrix4 const* p_modelMatrix, std::shared_ptr<RESOURCES::Shader> shader);
		void drawDirShadowMap(RESOURCES::Mesh& p_mesh, Material& p_material, MATHGL::Matrix4 const* p_modelMatrix, std::shared_ptr<RESOURCES::Shader> shader);
		void drawMesh(RESOURCES::Mesh& mesh, Material& material, MATHGL::Matrix4 const* modelMatrix, bool useTexutres=true);
		void registerModelMatrixSender(std::function<void(MATHGL::Matrix4)> modelMatrixSender);
		void registerUserMatrixSender(std::function<void(MATHGL::Matrix4)> userMatrixSender);
		void Clear() const;
		void ClearDepth() const;
		void Flush() const {
			glFlush();
			
		}
		void useColorMask(bool r, bool g, bool b, bool a) {
			glColorMask(r, g, b, a);
		}


		void prepareDirLightShadowMap();
		void prepareSpotLightShadowMap();
		void preparePointLightShadowMap();
	public:

		enum class PipelineRenderShaderType {
			POINT_SHADOW,
			DIR_SHADOW
		};
		std::function<void(MATHGL::Matrix4)> modelMatrixSender;
		std::function<void(MATHGL::Matrix4)> userMatrixSender;
		std::shared_ptr<RESOURCES::Texture> emptyTexture;
	};
}
