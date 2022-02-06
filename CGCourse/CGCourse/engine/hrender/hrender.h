#pragma once

#include "../resourceManager/resource/shader.h"
#include "../core/core.h"
namespace OTHER {
    struct Drawable {
        const KUMA::ECS::Object& obj;
        KUMA::MATHGL::Matrix4 world;
        KUMA::RESOURCES::Mesh* mesh;
        KUMA::RENDER::Material* material;
        KUMA::MATHGL::Matrix4 userMatrix;
    };
    using OpaqueDrawables = std::multimap<float, Drawable, std::less<float>>;
    using TransparentDrawables = std::multimap<float, Drawable, std::greater<float>>;

    struct ComputeShader : public KUMA::RESOURCES::Shader {
        bool setup(const std::string computePath);
        void dispatch(unsigned int x, unsigned int y = 1, unsigned int z = 1) const;
    };

    struct Primitive {
        Primitive(unsigned int numVertex) : numVertices(numVertex) {};
        void draw(const unsigned int readTex1 = 0, const unsigned int readTex2 = 0, const unsigned int readTex3 = 0);
        virtual void setup() = 0;
        unsigned int VAO, VBO;
        const unsigned int numVertices;
    };

    struct Quad : public Primitive {
        Quad() : Primitive(6) {};
        void draw(const unsigned int readTex1 = 0, const unsigned int readTex2 = 0, const unsigned int readTex3 = 0);
        void setup();
    };
};
namespace KUMA
{
	namespace RENDER
	{
        class RenderManager {
            KUMA::CORE_SYSTEM::Core& context;
            std::shared_ptr<KUMA::ECS::CameraComponent> mainCamera;
            SCENE_SYSTEM::Scene* currentScene;
        public:
            RenderManager(KUMA::CORE_SYSTEM::Core& c);
            ~RenderManager();

            bool startUp();
            void shutDown();

            void render(unsigned int start);
        private:
            bool initFBOs();
            bool initSSBOs();
            bool loadShaders();
            bool preProcess();

            void drawPointLightShadow(RESOURCES::Shader& pointLightShader, unsigned int index, unsigned int cubeMapTarget);
            void drawDirLightShadows(RESOURCES::Shader& dirLightShader, unsigned int targetTextureID);
            void drawDepthPass(RESOURCES::Shader& depthPassShader);
            void drawFullScene(RESOURCES::Shader& mainSceneShader, RESOURCES::Shader& skyboxShader);

            void postProcess(const unsigned int start);

            std::shared_ptr<RESOURCES::Shader> depthPrePassShader, PBRClusteredShader, skyboxShader,
                highPassFilterShader, gaussianBlurShader, screenSpaceShader,
                dirShadowShader, pointShadowShader, fillCubeMapShader,
                convolveCubeMap, preFilterSpecShader, integrateBRDFShader;

            OTHER::ComputeShader buildAABBGridCompShader, cullLightsCompShader;

            Quad canvas;
            bool slices = false;
            const unsigned int gridSizeX = 16;
            const unsigned int gridSizeY = 9;
            const unsigned int gridSizeZ = 24;
            const unsigned int numClusters = gridSizeX * gridSizeY * gridSizeZ;
            unsigned int sizeX, sizeY;

            unsigned int numLights;
            const unsigned int maxLights = 1000; // pretty overkill for sponza, but ok for testing
            const unsigned int maxLightsPerTile = 50;

            unsigned int AABBvolumeGridSSBO, screenToViewSSBO;
            unsigned int lightSSBO, lightIndexListSSBO, lightGridSSBO, lightIndexGlobalCountSSBO;

            ResolveBuffer simpleFBO;
            CaptureBuffer captureFBO;
            QuadHDRBuffer pingPongFBO;
            DirShadowBuffer  dirShadowFBO;
            FrameBufferMultiSampled multiSampledFBO;
            PointShadowBuffer* pointLightShadowFBOs;

            std::pair<OTHER::OpaqueDrawables, OTHER::TransparentDrawables> findAndSortFrustumCulledDrawables
            (
                const KUMA::SCENE_SYSTEM::Scene& p_scene,
                const KUMA::MATHGL::Vector3& p_cameraPosition,
                const KUMA::RENDER::Frustum& p_frustum,
                KUMA::RENDER::Material* p_defaultMaterial
            );
            std::pair<OTHER::OpaqueDrawables, OTHER::TransparentDrawables> findAndSortDrawables
            (
                const KUMA::SCENE_SYSTEM::Scene& p_scene,
                const KUMA::MATHGL::Vector3& p_cameraPosition,
                KUMA::RENDER::Material* p_defaultMaterial
            );
            void drawMesh(RESOURCES::Shader& shader, OTHER::Drawable& d, bool textured);
        };
	}
}

