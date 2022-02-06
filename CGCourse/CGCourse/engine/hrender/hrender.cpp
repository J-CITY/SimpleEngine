
#include "hrender.h"

#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>
using namespace KUMA;
using namespace KUMA::RENDER;


namespace OTHER
{
    bool ComputeShader::setup(const std::string computePath) {
        //Getting the compute shader code from the text file at file path
        std::string shaderFolderPath = "../assets/shaders/ComputeShaders/";
        std::string computeCode;
        std::ifstream cShaderFile(shaderFolderPath + computePath);
        std::stringstream cShaderStream;
        //Check if shader files exist
        if (!cShaderFile.good()) {
            printf("Couldn't find compute shader file: %s in shaders folder.\n ", computePath.c_str());
            return false;
        }
        else { //Compute Shader Exists
            cShaderStream << cShaderFile.rdbuf();

            //Close Files
            cShaderFile.close();

            //Passing code from string stream to string
            computeCode = cShaderStream.str();
            const char* cShaderCode = computeCode.c_str();

            //OpenGL initialization
            int computeShader = glCreateShader(GL_COMPUTE_SHADER);
            glShaderSource(computeShader, 1, &cShaderCode, NULL);
            glCompileShader(computeShader);
            int success;
            char infoLog[512];
            glGetShaderiv(computeShader, GL_COMPILE_STATUS, &success);
            if (!success) {
                glGetShaderInfoLog(computeShader, 512, NULL, infoLog);
                printf("Vertex shader compilation failed %s\n", infoLog);
                return false;
            }

            //Linking shaders
            id = glCreateProgram();
            glAttachShader(id, computeShader);
            glLinkProgram(id);

            glGetProgramiv(id, GL_LINK_STATUS, &success);
            if (!success) {
                glGetProgramInfoLog(id, 512, NULL, infoLog);
                printf("Shader Linking failed %s\n", infoLog);
                return false;
            }

            //Deleting shaders
            glDeleteShader(computeShader);
            return true;
        }
    }

    //Shorthand for dispatch compute with some default parameter values
    void ComputeShader::dispatch(unsigned int x, unsigned int y, unsigned int z) const {
        glDispatchCompute(x, y, z);
        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
    }
	///
    struct GPULight {
        glm::vec4 position;
        glm::vec4 color;
        unsigned int enabled;
        float intensity;
        float range;
        float padding;
    };
    struct VolumeTileAABB {
        glm::vec4 minPoint;
        glm::vec4 maxPoint;
    };
    struct ScreenToView {
        glm::mat4 inverseProjectionMat;
        unsigned int tileSizes[4];
        unsigned int screenWidth;
        unsigned int screenHeight;
        float sliceScalingFactor;
        float sliceBiasFactor;
    };

    
    void Primitive::draw(const unsigned int readTexture1, const unsigned int readTexture2, const unsigned int readTexture3) {
        glBindVertexArray(VAO);

        //This texture read could be compacted into a for loop and an array could be passed instead
        //But for now this is sufficient 
        if (readTexture1 != 0) {
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, readTexture1);
        }

        //A texture id of 0 is never assigned by opengl so we can
        //be sure that it means we haven't set any texture in the second paramenter and therefore
        //we only want one texture
        if (readTexture2 != 0) {
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, readTexture2);
        }

        if (readTexture3 != 0) {
            glActiveTexture(GL_TEXTURE2);
            glBindTexture(GL_TEXTURE_2D, readTexture3);
        }

        glDrawArrays(GL_TRIANGLES, 0, numVertices);

    }

    struct Cube : public Primitive {
        Cube() : Primitive(36) {};
        void setup();
    };
    void Cube::setup() {
        const float boxVertices[108] = {
            -1.0f, 1.0f, -1.0f,
            -1.0f, -1.0f, -1.0f,
            1.0f, -1.0f, -1.0f,
            1.0f, -1.0f, -1.0f,
            1.0f, 1.0f, -1.0f,
            -1.0f, 1.0f, -1.0f,

            -1.0f, -1.0f, 1.0f,
            -1.0f, -1.0f, -1.0f,
            -1.0f, 1.0f, -1.0f,
            -1.0f, 1.0f, -1.0f,
            -1.0f, 1.0f, 1.0f,
            -1.0f, -1.0f, 1.0f,

            1.0f, -1.0f, -1.0f,
            1.0f, -1.0f, 1.0f,
            1.0f, 1.0f, 1.0f,
            1.0f, 1.0f, 1.0f,
            1.0f, 1.0f, -1.0f,
            1.0f, -1.0f, -1.0f,

            -1.0f, -1.0f, 1.0f,
            -1.0f, 1.0f, 1.0f,
            1.0f, 1.0f, 1.0f,
            1.0f, 1.0f, 1.0f,
            1.0f, -1.0f, 1.0f,
            -1.0f, -1.0f, 1.0f,

            -1.0f, 1.0f, -1.0f,
            1.0f, 1.0f, -1.0f,
            1.0f, 1.0f, 1.0f,
            1.0f, 1.0f, 1.0f,
            -1.0f, 1.0f, 1.0f,
            -1.0f, 1.0f, -1.0f,

            -1.0f, -1.0f, -1.0f,
            -1.0f, -1.0f, 1.0f,
            1.0f, -1.0f, -1.0f,
            1.0f, -1.0f, -1.0f,
            -1.0f, -1.0f, 1.0f,
            1.0f, -1.0f, 1.0f};

        //Generate Buffers
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);

        //Bind Vertex Array Object and VBO in correct order
        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);

        //VBO initialization 
        glBufferData(GL_ARRAY_BUFFER, sizeof(boxVertices), &boxVertices, GL_STATIC_DRAW);

        //Vertex position pointer init
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

        //Unbinding VAO
        glBindVertexArray(0);
    }


    void Quad::setup() {
        const float quadVertices [] = {
            //positions //texCoordinates
            -1.0f, 1.0f, 0.0f, 1.0f,
            -1.0f, -1.0f, 0.0f, 0.0f,
            1.0f, -1.0f, 1.0f, 0.0f,

            -1.0f, 1.0f, 0.0f, 1.0f,
            1.0f, -1.0f, 1.0f, 0.0f,
            1.0f, 1.0f, 1.0f, 1.0f
        };

        //OpenGL postprocessing quad setup
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);

        //Bind Vertex Array Object and VBO in correct order
        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);

        //VBO initialization
        glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);

        //Quad position pointer initialization in attribute array
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);

        //Quad texcoords pointer initialization in attribute array
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

        glBindVertexArray(0);
    }

    //Quads never need to be depth tested when used for screen space rendering
    void Quad::draw(const unsigned int readTex1, const unsigned int readTex2, const unsigned int readTex3) {
        glDisable(GL_DEPTH_TEST);
        Primitive::draw(readTex1, readTex2, readTex3);
    }

    enum CubeMapType {
        SHADOW_MAP,
        HDR_MAP,
        PREFILTER_MAP
    };

    struct PointShadowBuffer : public FrameBuffer {
	    RESOURCES::CubeMap drawingTexture;
        bool setupFrameBuffer(unsigned int w, unsigned int h);

        int width, height;
    };
    bool PointShadowBuffer::setupFrameBuffer(unsigned int w, unsigned int h) {
        width = w;
        height = h;
        glGenFramebuffers(1, &id);
        glBindFramebuffer(GL_FRAMEBUFFER, id);

        //drawingTexture.generateCubeMap(width, height, SHADOW_MAP);
        //depthBuffer = drawingTexture.id;
        //glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depthBuffer, 0);
        AttachCubeMap(drawingTexture, Attachment::DEPTH_ATTACHMENT);

        glDrawBuffer(GL_NONE);
        glReadBuffer(GL_NONE);

        return true;// checkForCompleteness();
    }

}

MATHGL::Matrix4 fromGlm(glm::mat4& m) {
    MATHGL::Matrix4 out(
		m[0][0], m[0][1], m[0][2], m[0][3],
        m[1][0], m[1][1], m[1][2], m[1][3],
        m[2][0], m[2][1], m[2][2], m[2][3],
        m[3][0], m[3][1], m[3][2], m[3][3]
    );
}

RESOURCES::CubeMap irradianceMap, specFilteredMap;
KUMA::RESOURCES::Texture brdfLUTTexture;
Skybox mainSkyBox;

RenderManager::RenderManager(KUMA::CORE_SYSTEM::Core& c): context(c) {}
RenderManager::~RenderManager() = default;

std::shared_ptr<KUMA::ECS::CameraComponent> findMainCamera(const KUMA::SCENE_SYSTEM::Scene & p_scene) {
    for (auto& camera : KUMA::ECS::ComponentManager::getInstance()->cameraComponents)
        if (camera.second->obj.getIsActive())
            return camera.second;

    return nullptr;
}

bool RenderManager::startUp() {
    currentScene = context.sceneManager.getCurrentScene();
    if (!currentScene) {
        return false;
    }
    mainCamera = findMainCamera(*currentScene);
    if (!mainCamera) {
        return false;
    }
    printf("Loading FBO's...\n");
    if (!initFBOs()) {
        printf("FBO's failed to be initialized correctly.\n");
        return false;
    }

    printf("Loading Shaders...\n");
    if (!loadShaders()) {
        printf("Shaders failed to be initialized correctly.\n");
        return false;
    }

    printf("Loading SSBO's...\n");
    if (!initSSBOs()) {
        printf("SSBO's failed to be initialized correctly.\n");
        return false;
    }

    printf("Preprocessing...\n");
    if (!preProcess()) {
        printf("SSBO's failed to be initialized correctly.\n");
        return false;
    }

    printf("Renderer Initialization complete.\n");
    return true;
}

bool RenderManager::preProcess() {
    //Initializing the surface that we use to draw screen-space effects
    canvas.setup();

    //Building the grid of AABB enclosing the view frustum clusters
    buildAABBGridCompShader.bind();
    buildAABBGridCompShader.setUniformFloat("zNear", mainCamera->getNear());
    buildAABBGridCompShader.setUniformFloat("zFar", mainCamera->getFar());
    buildAABBGridCompShader.dispatch(gridSizeX, gridSizeY, gridSizeZ);

    //Environment Mapping
    //Passing equirectangular map to cubemap
    captureFBO.bind();
    mainSkyBox.fillCubeMapWithTexture(*fillCubeMapShader);

    //Cubemap convolution TODO:: This could probably be moved to a function of the scene or environment maps
    //themselves as a class / static function
    int res = irradianceMap.width;
    captureFBO.resizeFrameBuffer(res);
    unsigned int environmentID = mainSkyBox.skyBoxCubeMap.id;
    irradianceMap.convolveCubeMap(environmentID, *convolveCubeMap);

    //Cubemap prefiltering TODO:: Same as above
    unsigned int captureRBO = captureFBO.depthBuffer;
    specFilteredMap.preFilterCubeMap(environmentID, captureRBO, *preFilterSpecShader);

    //BRDF lookup texture
    integrateBRDFShader->bind();
    res = brdfLUTTexture.height;
    captureFBO.resizeFrameBuffer(res);
    unsigned int id = brdfLUTTexture.id;
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, id, 0);
    glViewport(0, 0, res, res);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    canvas.draw();

    //Making sure that the viewport is the correct size after rendering
    glViewport(0, 0, 800, 600);

    glEnable(GL_DEPTH_TEST);
    glDepthMask(true);

    //Populating depth cube maps for the point light shadows
    auto& pointLightView = KUMA::ECS::ComponentManager::getInstance()->getAllPointLights();
    for (unsigned int i = 0; i < pointLightView.size(); ++i) {
        pointLightShadowFBOs[i].bind();
        pointLightShadowFBOs[i].clear(GL_DEPTH_BUFFER_BIT, glm::vec3(1.0f));
        drawPointLightShadow(*pointShadowShader, i, pointLightShadowFBOs[i].depthBuffer);
    }

    // Directional shadows
    dirShadowFBO.bind();
    dirShadowFBO.clear(GL_DEPTH_BUFFER_BIT, glm::vec3(1.0f));
    drawDirLightShadows(*dirShadowShader, dirShadowFBO.depthBuffer);

    //As we add more error checking this will change from a dummy variable to an actual thing
    return true;
}

//TODO:: some of the buffer generation and binding should be abstracted into a function
bool RenderManager::initSSBOs() {
    //Setting up tile size on both X and Y 
    sizeX = (unsigned int)std::ceilf(800 / (float)gridSizeX);

    float zFar = mainCamera->getFar();
    float zNear = mainCamera->getNear();

    //Buffer containing all the clusters
    {
        glGenBuffers(1, &AABBvolumeGridSSBO);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, AABBvolumeGridSSBO);

        //We generate the buffer but don't populate it yet.
        glBufferData(GL_SHADER_STORAGE_BUFFER, numClusters * sizeof(OTHER::VolumeTileAABB), NULL, GL_STATIC_COPY);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, AABBvolumeGridSSBO);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    }

    //Setting up screen2View ssbo
    {
        glGenBuffers(1, &screenToViewSSBO);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, screenToViewSSBO);

        //Setting up contents of buffer
        OTHER::ScreenToView screen2View;
        auto m = mainCamera->getCamera().calculateProjectionMatrix(800, 600);
        screen2View.inverseProjectionMat = glm::inverse(glm::mat4(
            m.data[0], m.data[1], m.data[2], m.data[3],
            m.data[4], m.data[5], m.data[6], m.data[7],
            m.data[8], m.data[9], m.data[10], m.data[11],
            m.data[12], m.data[13], m.data[14], m.data[15]
        ));
        screen2View.tileSizes[0] = gridSizeX;
        screen2View.tileSizes[1] = gridSizeY;
        screen2View.tileSizes[2] = gridSizeZ;
        screen2View.tileSizes[3] = sizeX;
        screen2View.screenWidth = 800;
        screen2View.screenHeight = 600;
        //Basically reduced a log function into a simple multiplication an addition by pre-calculating these
        screen2View.sliceScalingFactor = (float)gridSizeZ / std::log2f(zFar / zNear);
        screen2View.sliceBiasFactor = -((float)gridSizeZ * std::log2f(zNear) / std::log2f(zFar / zNear));

        //Generating and copying data to memory in GPU
        glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(OTHER::ScreenToView), &screen2View, GL_STATIC_COPY);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, screenToViewSSBO);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    }

    //Setting up lights buffer that contains all the lights in the scene
    {
        glGenBuffers(1, &lightSSBO);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, lightSSBO);
        glBufferData(GL_SHADER_STORAGE_BUFFER, maxLights * sizeof(OTHER::GPULight), NULL, GL_DYNAMIC_DRAW);

        GLint bufMask = GL_READ_WRITE;

        OTHER::GPULight* lights = (OTHER::GPULight*)glMapBuffer(GL_SHADER_STORAGE_BUFFER, bufMask);
        std::shared_ptr<ECS::PointLight> light;
        auto& pointLightView = KUMA::ECS::ComponentManager::getInstance()->getAllPointLights();
        for (unsigned int i = 0; i < numLights; ++i) {
            //Fetching the light from the current scene
            light = pointLightView[i];
            auto v = light->obj.transform->getWorldPosition();
            lights[i].position = glm::vec4(v.x, v.y, v.z, 1.0f);
            lights[i].color = glm::vec4(light->getColor().x, light->getColor().y, light->getColor().z, 1.0f);
            lights[i].enabled = 1;
            lights[i].intensity = 1.0f;
            lights[i].range = 65.0f;
        }
        glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, lightSSBO);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    }

    //A list of indices to the lights that are active and intersect with a cluster
    {
        unsigned int totalNumLights = numClusters * maxLightsPerTile; //50 lights per tile max
        glGenBuffers(1, &lightIndexListSSBO);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, lightIndexListSSBO);

        //We generate the buffer but don't populate it yet.
        glBufferData(GL_SHADER_STORAGE_BUFFER, totalNumLights * sizeof(unsigned int), NULL, GL_STATIC_COPY);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, lightIndexListSSBO);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    }

    //Every tile takes two unsigned ints one to represent the number of lights in that grid
    //Another to represent the offset to the light index list from where to begin reading light indexes from
    //This implementation is straight up from Olsson paper
    {
        glGenBuffers(1, &lightGridSSBO);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, lightGridSSBO);

        glBufferData(GL_SHADER_STORAGE_BUFFER, numClusters * 2 * sizeof(unsigned int), NULL, GL_STATIC_COPY);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, lightGridSSBO);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    }

    //Setting up simplest ssbo in the world
    {
        glGenBuffers(1, &lightIndexGlobalCountSSBO);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, lightIndexGlobalCountSSBO);

        //Every tile takes two unsigned ints one to represent the number of lights in that grid
        //Another to represent the offset 
        glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(unsigned int), NULL, GL_STATIC_COPY);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 6, lightIndexGlobalCountSSBO);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    }

    return true;
}

bool RenderManager::loadShaders() {
    //bool stillValid = true;
    //Pre-processing
    buildAABBGridCompShader.setup("clusterShader.comp");
    cullLightsCompShader.setup("clusterCullLightShader.comp");
    std::string shaderFolder = "Shaders/deferred/";
    fillCubeMapShader = KUMA::RESOURCES::ShaderLoader().createResource(shaderFolder + "cubeMapShader.glsl"); //fillCubeMapShader.setup("cubeMapShader.vert", "buildCubeMapShader.frag");
    convolveCubeMap = KUMA::RESOURCES::ShaderLoader().createResource(shaderFolder + "cubeMapShader.glsl");//convolveCubeMap.setup("cubeMapShader.vert", "convolveCubemapShader.frag");
    preFilterSpecShader = KUMA::RESOURCES::ShaderLoader().createResource(shaderFolder + "cubeMapShader.glsl");//preFilterSpecShader.setup("cubeMapShader.vert", "preFilteringShader.frag");
    integrateBRDFShader = KUMA::RESOURCES::ShaderLoader().createResource(shaderFolder + "screenShader.glsl");//integrateBRDFShader.setup("screenShader.vert", "brdfIntegralShader.frag");

    //if (!stillValid) {
    //    printf("Error loading pre-processing Shaders!\n");
    //    return false;
    //}
    //Rendering
    depthPrePassShader =  KUMA::RESOURCES::ShaderLoader().createResource(shaderFolder + "depthPassShader.glsl");//depthPrePassShader.setup("depthPassShader.vert", "depthPassShader.frag");
    PBRClusteredShader = KUMA::RESOURCES::ShaderLoader().createResource(shaderFolder + "PBRClusteredShader.glsl");//PBRClusteredShader.setup("PBRClusteredShader.vert", "PBRClusteredShader.frag");
    skyboxShader = KUMA::RESOURCES::ShaderLoader().createResource(shaderFolder + "skyboxShader.glsl");//skyboxShader.setup("skyboxShader.vert", "skyboxShader.frag");
    screenSpaceShader = KUMA::RESOURCES::ShaderLoader().createResource(shaderFolder + "screenShader.glsl");//screenSpaceShader.setup("screenShader.vert", "screenShader.frag");

    //if (!stillValid) {
    //    printf("Error loading rendering Shaders!\n");
    //    return false;
    //}

    //Shadow mapping
    dirShadowShader = KUMA::RESOURCES::ShaderLoader().createResource(shaderFolder + "shadowShader.glsl");//dirShadowShader.setup("shadowShader.vert", "shadowShader.frag");
    pointShadowShader = KUMA::RESOURCES::ShaderLoader().createResource(shaderFolder + "pointShadowShader.glsl");//pointShadowShader.setup("pointShadowShader.vert", "pointShadowShader.frag", "pointShadowShader.geom");

   
    //Post-processing
    highPassFilterShader = KUMA::RESOURCES::ShaderLoader().createResource(shaderFolder + "splitHighShader.glsl");//highPassFilterShader.setup("splitHighShader.vert", "splitHighShader.frag");
    gaussianBlurShader = KUMA::RESOURCES::ShaderLoader().createResource(shaderFolder + "blurShader.glsl");//gaussianBlurShader.setup("blurShader.vert", "blurShader.frag");

    
    return true;
}

void RenderManager::shutDown() {

}

bool RenderManager::initFBOs() {
    //Init variables
    unsigned int shadowMapResolution = 2048;// currentScene->getShadowRes();
    int skyboxRes = mainSkyBox.resolution;
    numLights = KUMA::ECS::ComponentManager::getInstance()->getAllPointLights().size();
    bool stillValid = true;

    //Shadow Framebuffers
    pointLightShadowFBOs = new OTHER::PointShadowBuffer[numLights];

    //Directional light
    stillValid &= dirShadowFBO.setupFrameBuffer(shadowMapResolution, shadowMapResolution);

    //Point light
    for (unsigned int i = 0; i < numLights; ++i) {
        stillValid &= pointLightShadowFBOs[i].setupFrameBuffer(shadowMapResolution, shadowMapResolution);
    }

    if (!stillValid) {
        printf("Error initializing shadow map FBO's!\n");
        return false;
    }

    //Rendering buffers
    stillValid &= multiSampledFBO.setupFrameBuffer();
    stillValid &= captureFBO.setupFrameBuffer(skyboxRes, skyboxRes);

    if (!stillValid) {
        printf("Error initializing rendering FBO's!\n");
        return false;
    }

    //Post processing buffers
    stillValid &= pingPongFBO.setupFrameBuffer();
    stillValid &= simpleFBO.setupFrameBuffer();

    if (!stillValid) {
        printf("Error initializing postPRocessing FBO's!\n");
        return false;
    }

    return stillValid;
}

/* This time using volume tiled forward
Algorithm steps:
//Initialization or view frustrum change
0. Determine AABB's for each volume
//Update Every frame
1. Depth-pre pass :: DONE
2. Mark Active tiles :: POSTPONED AS OPTIMIZATION
3. Build Tile list ::  POSTPONED AS OPTIMIZATION
4. Assign lights to tiles :: DONE (BUT SHOULD BE OPTIMIZED)
5. Shading by reading from the active tiles list :: DONE
6. Post processing and screen space effects :: DONE
*/
void RenderManager::render(const unsigned int start) {
    //Initiating rendering gui
    //ImGui::Begin("Rendering Controls");
    //ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
    //
    //if (ImGui::CollapsingHeader("Controls")) {
    //    ImGui::Text("Strafe: w a s d");
    //    ImGui::Text("Rotate Camera: hold left click + mouse");
    //    ImGui::Text("Up&Down: q e");
    //    ImGui::Text("Reset Camera: r");
    //    ImGui::Text("Exit: ESC");
    //    ImGui::InputFloat3("Camera Pos", (float*)&sceneCamera->position); //Camera controls
    //    ImGui::SliderFloat("Movement speed", &sceneCamera->camSpeed, 0.005f, 1.0f);
    //}
    //Making sure depth testing is enabled 
    glEnable(GL_DEPTH_TEST);
    glDepthMask(true);

    // Directional shadows
    dirShadowFBO.bind();
    dirShadowFBO.clear(GL_DEPTH_BUFFER_BIT, glm::vec3(1.0f));
    drawDirLightShadows(*dirShadowShader, dirShadowFBO.depthBuffer);

    //1.1- Multisampled Depth pre-pass
    multiSampledFBO.bind();
    multiSampledFBO.clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT, glm::vec3(0.0f));
    drawDepthPass(*depthPrePassShader);

    //4-Light assignment
    cullLightsCompShader.bind();
    //TODO: calc view
    cullLightsCompShader.setUniformMat4("viewMatrix", mainCamera->getCamera().getViewMatrix());
    cullLightsCompShader.dispatch(1, 1, 6);

    //5 - Actual shading;
    //5.1 - Forward render the scene in the multisampled FBO using the z buffer to discard early
    glDepthFunc(GL_LEQUAL);
    glDepthMask(false);
    drawFullScene(*PBRClusteredShader, *skyboxShader);

    //5.2 - resolve the from multisampled to normal resolution for postProcessing
    multiSampledFBO.blitTo(simpleFBO, GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    //6 -postprocessing, includes bloom, exposure mapping
    postProcess(start);

    //Rendering gui scope ends here cannot be done later because the whole frame
    //is reset in the display buffer swap
   // ImGui::End();

    //Drawing to the screen by swapping the window's surface with the
    //final buffer containing all rendering information
    //screen->swapDisplayBuffer();
}


void RenderManager::postProcess(const unsigned int start) {
    //if (ImGui::CollapsingHeader("Post-processing")) {
    //    ImGui::SliderInt("Blur", &sceneCamera->blurAmount, 0, 10);
    //    ImGui::SliderFloat("Exposure", &sceneCamera->exposure, 0.1f, 5.0f);
    //}

    //TODO:: should be a compute shader 
    pingPongFBO.bind();
    pingPongFBO.clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT, glm::vec3(0.0f));
    if (mainCamera->blurAmount > 0) {
        //Filtering pixel rgb values > 1.0
        highPassFilterShader->bind();
        canvas.draw(simpleFBO.texColorBuffer);
    }

    //Applying Gaussian blur in ping pong fashion
    //TODO:: ALso make it a compute shader
    gaussianBlurShader->bind();
    for (int i = 0; i < mainCamera->blurAmount; ++i) {
        //Horizontal pass
        glBindFramebuffer(GL_FRAMEBUFFER, simpleFBO.frameBufferID);
        glDrawBuffer(GL_COLOR_ATTACHMENT1);
        gaussianBlurShader->setUniformInt("horizontal", true);
        canvas.draw(pingPongFBO.texColorBuffer);

        //Vertical pass
        glBindFramebuffer(GL_FRAMEBUFFER, pingPongFBO.frameBufferID);
        gaussianBlurShader->setUniformInt("horizontal", false);
        canvas.draw(simpleFBO.blurHighEnd);
    }
    //Setting back to default framebuffer (screen) and clearing
    //No need for depth testing cause we're drawing to a flat quad
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    //Shader setup for postprocessing
    screenSpaceShader->bind();

    screenSpaceShader->setUniformInt("offset", start);
    screenSpaceShader->setUniformFloat("exposure", mainCamera->exposure);
    screenSpaceShader->setUniformInt("screenTexture", 0);
    screenSpaceShader->setUniformInt("bloomBlur", 1);
    screenSpaceShader->setUniformInt("computeTexture", 2);

    //Merging the blurred high pass image with the low pass values
    //Also tonemapping and doing other post processing
    canvas.draw(simpleFBO.texColorBuffer, pingPongFBO.texColorBuffer);
}

//TODO:: refactor this function too with the shadow mapping rewrite, could possibly use virtual 
//shadow maps to switch VAO and have one draw call per mesh, but render to multiple parts of the 
//texture.
void RenderManager::drawPointLightShadow(RESOURCES::Shader& pointLightShader, unsigned int index, unsigned int cubeMapTarget) {
    //Current light
    auto& light = *KUMA::ECS::ComponentManager::getInstance()->getAllPointLights()[index];
    light.depthMapTextureId = cubeMapTarget;
    //Shader setup
    pointLightShader.bind();
    pointLightShader.setUniformVec3("lightPos", light.obj.transform->getWorldPosition());
    pointLightShader.setUniformFloat("far_plane", light.zFar);

    //Matrix setup
    MATHGL::Matrix4 lightMatrix, M;
    glm::mat4 shadowProj = light.shadowProjectionMatGLM;
    for (unsigned int face = 0; face < 6; ++face) {
        std::string number = std::to_string(face);
        lightMatrix = fromGlm(shadowProj) * fromGlm(light.lookAtPerFace[face]);
        pointLightShader.setUniformMat4(("shadowMatrices[" + number + "]").c_str(), lightMatrix);
    }

    OTHER::OpaqueDrawables	opaqueMeshes;
    OTHER::TransparentDrawables transparentMeshes;

    if (mainCamera->getCamera().isFrustumGeometryCulling()) {
        const auto& frustum = mainCamera->getCamera().getFrustum();
        std::tie(opaqueMeshes, transparentMeshes) = findAndSortFrustumCulledDrawables(*currentScene, mainCamera->obj.transform->getWorldPosition(), frustum, nullptr);
    }
    else {
        std::tie(opaqueMeshes, transparentMeshes) = findAndSortDrawables(*currentScene, mainCamera->obj.transform->getWorldPosition(), nullptr);
    }

    for (auto& currentModel : opaqueMeshes) {
        
        M = currentModel.second.userMatrix;// ->modelMatrix;
        //Shader setup stuff that changes every frame
        pointLightShader.setUniformMat4("M", M);

        //Draw object
        //currentModel->draw(pointLightShader, false);
        pointLightShader.setUniformInt("IBL", false);
        drawMesh(pointLightShader, currentModel.second, true);
    }
}

void RenderManager::drawMesh(RESOURCES::Shader& shader, OTHER::Drawable& d, bool textured) {
    /*
	//Diffuse
    glActiveTexture(GL_TEXTURE0);
    shader.getUniformInt("albedoMap", 0);
    glBindTexture(GL_TEXTURE_2D, textures[0]);
    if (textured) {
        //Emissive
        glActiveTexture(GL_TEXTURE1);
        shader.getUniformInt("emissiveMap", 1);
        glBindTexture(GL_TEXTURE_2D, textures[1]);

        //Normals
        if (textures[2] == 0) {
            shader.setBool("normalMapped", false);
        }
        else {
            shader.setBool("normalMapped", true);
        }
        glActiveTexture(GL_TEXTURE2);
        shader.getUniformInt("normalsMap", 2);
        glBindTexture(GL_TEXTURE_2D, textures[2]);

        //Ambient Oclussion
        if (textures[3] == 0) {
            shader.getUniformInt("aoMapped", false);
        }
        else {
            shader.getUniformInt("aoMapped", true);
        }
        glActiveTexture(GL_TEXTURE3);
        shader.getUniformInt("lightMap", 3);
        glBindTexture(GL_TEXTURE_2D, textures[3]);

        //Metal / Roughness
        glActiveTexture(GL_TEXTURE4);
        shader.getUniformInt("metalRoughMap", 4);
        glBindTexture(GL_TEXTURE_2D, textures[4]);

    }

    //Mesh Drawing
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, (GLsizei)indices.size(), GL_UNSIGNED_INT, 0);
    */
}

std::pair<OTHER::OpaqueDrawables, OTHER::TransparentDrawables> RenderManager::findAndSortFrustumCulledDrawables
(
    const KUMA::SCENE_SYSTEM::Scene& p_scene,
    const KUMA::MATHGL::Vector3& p_cameraPosition,
    const KUMA::RENDER::Frustum& p_frustum,
    KUMA::RENDER::Material* p_defaultMaterial
) {
    OTHER::OpaqueDrawables opaqueDrawables;
    OTHER::TransparentDrawables transparentDrawables;

    for (auto& modelRenderer : KUMA::ECS::ComponentManager::getInstance()->modelComponents) {
        auto& owner = modelRenderer.second->obj;

        if (owner.getIsActive()) {
            if (auto model = modelRenderer.second->getModel()) {
                if (auto materialRenderer = modelRenderer.second->obj.getComponent<KUMA::ECS::MaterialRenderer>()) {
                    auto& transform = owner.transform->getTransform();

                    KUMA::RENDER::CullingOptions cullingOptions = KUMA::RENDER::CullingOptions::NONE;

                    if (modelRenderer.second->getFrustumBehaviour() != KUMA::ECS::ModelRenderer::EFrustumBehaviour::DISABLED) {
                        cullingOptions |= KUMA::RENDER::CullingOptions::FRUSTUM_PER_MODEL;
                    }

                    if (modelRenderer.second->getFrustumBehaviour() == KUMA::ECS::ModelRenderer::EFrustumBehaviour::CULL_MESHES) {
                        cullingOptions |= KUMA::RENDER::CullingOptions::FRUSTUM_PER_MESH;
                    }

                    const auto& modelBoundingSphere = modelRenderer.second->getFrustumBehaviour() == KUMA::ECS::ModelRenderer::EFrustumBehaviour::CULL_CUSTOM ? modelRenderer.second->getCustomBoundingSphere() : model->getBoundingSphere();

                    std::vector<std::reference_wrapper<KUMA::RESOURCES::Mesh>> meshes;
                    {
                        meshes = context.renderer->getMeshesInFrustum(*model, modelBoundingSphere, transform, p_frustum, cullingOptions);
                    }

                    if (!meshes.empty()) {
                        float distanceToActor = KUMA::MATHGL::Vector3::Distance(transform.getWorldPosition(), p_cameraPosition);
                        const KUMA::ECS::MaterialRenderer::MaterialList& materials = materialRenderer->getMaterials();

                        for (const auto& mesh : meshes) {
                            KUMA::RENDER::Material* material = nullptr;

                            if (mesh.get().getMaterialIndex() < MAX_MATERIAL_COUNT) {
                                material = materials.at(mesh.get().getMaterialIndex()).get();
                                if (!material || !material->getShader())
                                    material = p_defaultMaterial;
                            }

                            if (material) {
                                OTHER::Drawable element = {owner, transform.getWorldMatrix(), &mesh.get(), material, materialRenderer->getUserMatrix()};

                                if (material->isBlendable())
                                    transparentDrawables.emplace(distanceToActor, element);
                                else
                                    opaqueDrawables.emplace(distanceToActor, element);
                            }
                        }
                    }
                }
            }
        }
    }

    return {opaqueDrawables, transparentDrawables};
}

std::pair<OTHER::OpaqueDrawables, OTHER::TransparentDrawables> RenderManager::findAndSortDrawables
(
    const KUMA::SCENE_SYSTEM::Scene& p_scene,
    const KUMA::MATHGL::Vector3& p_cameraPosition,
    KUMA::RENDER::Material* p_defaultMaterial
) {
    OTHER::OpaqueDrawables opaqueDrawables;
    OTHER::TransparentDrawables transparentDrawables;

    for (auto& modelRenderer : KUMA::ECS::ComponentManager::getInstance()->modelComponents) {
        if (modelRenderer.second->obj.getIsActive()) {
            if (auto model = modelRenderer.second->getModel()) {
                float distanceToActor = KUMA::MATHGL::Vector3::Distance(modelRenderer.second->obj.transform->getWorldPosition(), p_cameraPosition);

                if (auto materialRenderer = modelRenderer.second->obj.getComponent<KUMA::ECS::MaterialRenderer>()) {
                    const auto& transform = modelRenderer.second->obj.transform->getTransform();

                    const KUMA::ECS::MaterialRenderer::MaterialList& materials = materialRenderer->getMaterials();

                    for (auto mesh : model->getMeshes()) {
                        KUMA::RENDER::Material* material = nullptr;

                        if (mesh->getMaterialIndex() < MAX_MATERIAL_COUNT) {
                            material = materials.at(mesh->getMaterialIndex()).get();
                            if (!material)
                                material = p_defaultMaterial;
                        }

                        if (material) {
                            OTHER::Drawable element = {modelRenderer.second->obj, transform.getWorldMatrix(), mesh, material, materialRenderer->getUserMatrix()};

                            if (material->isBlendable())
                                transparentDrawables.emplace(distanceToActor, element);
                            else
                                opaqueDrawables.emplace(distanceToActor, element);
                        }
                    }
                }
            }
        }
    }

    return {opaqueDrawables, transparentDrawables};
}

//Currently assumes there's only one directional light, also uses the simplest shadow map algorithm
//that leaves a lot to be desired in terms of resolution, thinking about moving to cascaded shadow maps
//or maybe variance idk yet.
void RenderManager::drawDirLightShadows(RESOURCES::Shader& dirLightShader, unsigned int targetTextureID) {
    auto& dirLight = *KUMA::ECS::ComponentManager::getInstance()->getAllDirectionalLights()[0];
    MATHGL::Matrix4 ModelLS;
    dirLight.depthMapTextureID = targetTextureID;

    float left = dirLight.orthoBoxSize;
    float right = -left;
    float top = left;
    float bottom = -top;
    dirLight.shadowProjectionMat = fromGlm(glm::ortho(left, right, bottom, top, dirLight.zNear, dirLight.zFar));
    dirLight.lightView = fromGlm(glm::lookAt(100.0f * -glm::vec3(dirLight.Direction.x, dirLight.Direction.y, dirLight.Direction.z),
        glm::vec3(0.0f, 0.0f, 0.0f),
        glm::vec3(0.0f, 1.0f, 0.0f)));

    dirLight.lightSpaceMatrix = dirLight.shadowProjectionMat * dirLight.lightView;

    //Drawing every object into the shadow buffer
    OTHER::OpaqueDrawables	opaqueMeshes;
    OTHER::TransparentDrawables transparentMeshes;

    if (mainCamera->getCamera().isFrustumGeometryCulling()) {
        const auto& frustum = mainCamera->getCamera().getFrustum();
        std::tie(opaqueMeshes, transparentMeshes) = findAndSortFrustumCulledDrawables(*currentScene, mainCamera->obj.transform->getWorldPosition(), frustum, nullptr);
    }
    else {
        std::tie(opaqueMeshes, transparentMeshes) = findAndSortDrawables(*currentScene, mainCamera->obj.transform->getWorldPosition(), nullptr);
    }

    for (auto& currentModel : opaqueMeshes) {
        
        //Matrix setup
        ModelLS = dirLight.lightSpaceMatrix * currentModel.second.userMatrix;// currentModel->modelMatrix;

        //Shader setup stuff that changes every frame
        dirLightShader.bind();
        dirLightShader.setUniformMat4("lightSpaceMatrix", ModelLS);

        //Draw object
        drawMesh(dirLightShader, currentModel.second, true);
    	//currentModel->draw(dirLightShader, false);
    }
}

void RenderManager::drawDepthPass(RESOURCES::Shader& depthPassShader) {
    //Matrix Setup
    MATHGL::Matrix4 MVP = MATHGL::Matrix4::Identity;
    MATHGL::Matrix4 VP = mainCamera->getCamera().calculateProjectionMatrix(800, 600) * mainCamera->getCamera().calculateViewMatrix(
        mainCamera->obj.transform->getWorldPosition(), mainCamera->obj.transform->getWorldRotation());

    //Drawing every object into the depth buffer
    OTHER::OpaqueDrawables	opaqueMeshes;
    OTHER::TransparentDrawables transparentMeshes;

    if (mainCamera->getCamera().isFrustumGeometryCulling()) {
        const auto& frustum = mainCamera->getCamera().getFrustum();
        std::tie(opaqueMeshes, transparentMeshes) = findAndSortFrustumCulledDrawables(*currentScene, mainCamera->obj.transform->getWorldPosition(), frustum, nullptr);
    }
    else {
        std::tie(opaqueMeshes, transparentMeshes) = findAndSortDrawables(*currentScene, mainCamera->obj.transform->getWorldPosition(), nullptr);
    }

    for (auto& currentModel : opaqueMeshes) {
        
        //Matrix setup
        MVP = VP * currentModel.second.userMatrix;//currentModel->modelMatrix;

        //Shader setup stuff that changes every frame
        depthPassShader.bind();
        depthPassShader.setUniformMat4("MVP", MVP);

        //Draw object
        drawMesh(depthPassShader, currentModel.second, true);
    	//currentModel->draw(depthPassShader, false);
    }
}

//Sets up the common uniforms for each model and loaded all texture units. A lot of driver calls here
//Re-watch the beyond porting talk to try to reduce api calls. Specifically texture related calls.
void RenderManager::drawFullScene(RESOURCES::Shader& mainSceneShader, RESOURCES::Shader& skyboxShader) {
    //Matrix Setup
    MATHGL::Matrix4 MVP = MATHGL::Matrix4::Identity;// glm::mat4(1.0);
    MATHGL::Matrix4 M = MATHGL::Matrix4::Identity;// = glm::mat4(1.0);
    //??
    MATHGL::Matrix4 VP = mainCamera->getCamera().calculateProjectionMatrix(800, 600) * mainCamera->getCamera().calculateViewMatrix(
        mainCamera->obj.transform->getWorldPosition(), mainCamera->obj.transform->getWorldRotation());
    MATHGL::Matrix4 VPCubeMap = mainCamera->getCamera().calculateProjectionMatrix(800, 600) * mainCamera->getCamera().calculateViewMatrix(
        mainCamera->obj.transform->getWorldPosition(), mainCamera->obj.transform->getWorldRotation());

    //Just to avoid magic constants
    const unsigned int numTextures = 5;

    //Setting colors in the gui
    //if (ImGui::CollapsingHeader("Directional Light Settings")) {
    //    ImGui::TextColored(ImVec4(1, 1, 1, 1), "Directional light Settings");
    //    ImGui::ColorEdit3("Color", (float*)&dirLight.color);
    //    ImGui::SliderFloat("Strength", &dirLight.strength, 0.1f, 200.0f);
    //    ImGui::SliderFloat("BoxSize", &dirLight.orthoBoxSize, 0.1f, 500.0f);
    //    ImGui::SliderFloat3("Direction", (float*)&dirLight.direction, -5.0f, 5.0f);
    //}
    auto& dirLight = *KUMA::ECS::ComponentManager::getInstance()->getAllDirectionalLights()[0];

    mainSceneShader.bind();
    //if (ImGui::CollapsingHeader("Cluster Debugging Light Settings")) {
    //    ImGui::Checkbox("Display depth Slices", &slices);
    //}
    mainSceneShader.setUniformVec3("dirLight.direction", dirLight.Direction);
    mainSceneShader.setUniformInt("slices", slices);
    mainSceneShader.setUniformVec3("dirLight.color", dirLight.getData().color * dirLight.getData().intensity);
    mainSceneShader.setUniformMat4("lightSpaceMatrix", dirLight.lightSpaceMatrix);
    mainSceneShader.setUniformVec3("cameraPos_wS", mainCamera->obj.transform->getWorldPosition());
    mainSceneShader.setUniformFloat("zFar", mainCamera->getFar());
    mainSceneShader.setUniformFloat("zNear", mainCamera->getNear());

    auto& pointLights = KUMA::ECS::ComponentManager::getInstance()->getAllPointLights();
    for (unsigned int i = 0; i < pointLights.size(); ++i) {
        auto light = pointLights[i];
        std::string number = std::to_string(i);

        glActiveTexture(GL_TEXTURE0 + numTextures + i);
        mainSceneShader.setUniformInt(("depthMaps[" + number + "]").c_str(), numTextures + i);
        glBindTexture(GL_TEXTURE_CUBE_MAP, light->depthMapTextureId);
        mainSceneShader.setUniformFloat("far_plane", light->zFar);
    }

    //Setting directional shadow depth map textures
    glActiveTexture(GL_TEXTURE0 + numTextures + pointLights.size());
    mainSceneShader.setUniformInt("shadowMap", numTextures + pointLights.size());
    glBindTexture(GL_TEXTURE_2D, dirLight.depthMapTextureID);

    //TODO:: Formalize htis a bit more
    //Setting environment map texture
    glActiveTexture(GL_TEXTURE0 + numTextures + pointLights.size() + 1);
    mainSceneShader.setUniformInt("irradianceMap", numTextures + pointLights.size() + 1);
    glBindTexture(GL_TEXTURE_CUBE_MAP, irradianceMap.id);

    //Setting environment map texture for specular
    glActiveTexture(GL_TEXTURE0 + numTextures + pointLights.size() + 2);
    mainSceneShader.setUniformInt("prefilterMap", numTextures + pointLights.size() + 2);
    glBindTexture(GL_TEXTURE_CUBE_MAP, specFilteredMap.id);

    //Setting lookup table
    glActiveTexture(GL_TEXTURE0 + numTextures + pointLights.size() + 3);
    mainSceneShader.setUniformInt("brdfLUT", numTextures + pointLights.size() + 3);
    glBindTexture(GL_TEXTURE_2D, brdfLUTTexture.id);

    OTHER::OpaqueDrawables	opaqueMeshes;
    OTHER::TransparentDrawables transparentMeshes;

    if (mainCamera->getCamera().isFrustumGeometryCulling()) {
        const auto& frustum = mainCamera->getCamera().getFrustum();
        std::tie(opaqueMeshes, transparentMeshes) = findAndSortFrustumCulledDrawables(*currentScene, mainCamera->obj.transform->getWorldPosition(), frustum, nullptr);
    }
    else {
        std::tie(opaqueMeshes, transparentMeshes) = findAndSortDrawables(*currentScene, mainCamera->obj.transform->getWorldPosition(), nullptr);
    }

    for (auto& currentModel : opaqueMeshes) {
        
        //Matrix setup
        M = currentModel.second.userMatrix;// ->modelMatrix;
        MVP = VP * M;

        //Shader setup stuff that changes every frame
        mainSceneShader.setUniformMat4("MVP", MVP);
        mainSceneShader.setUniformMat4("M", M);

        //Draw object
        drawMesh(mainSceneShader, currentModel.second, true);
        //currentModel->draw(mainSceneShader, true);
    }

    //Drawing skybox
    skyboxShader.bind();
    skyboxShader.setUniformMat4("VP", VPCubeMap);
    mainSkyBox.draw();
}
