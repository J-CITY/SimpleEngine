#include "gameRendererGl.h"



#ifdef OPENGL_BACKEND
#include "backends/gl/materialGl.h"
#include "backends/gl/shaderGl.h"
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>

#include "backends/gl/textureGl.h"
#include "../../../core/core.h"
#include "../../../scene/sceneManager.h"
#include "../../../window/window.h"
#include <random>
#include "backends/gl/storageBufferGl.h"
#include "backends/gl/uniformBufferGl.h"
#include "../../../utils/loader.h"
#include "backends/gl/driverGl.h"
#include "backends/gl/frameBufferGl.h"
#include "backends/gl/meshGl.h"
#include "../utils/time/time.h"

using namespace KUMA;
using namespace KUMA::RENDER;

std::optional<KUMA::Ref<KUMA::ECS::CameraComponent>> mainCameraComponent = std::nullopt;

std::shared_ptr<UniformBufferGl<EngineUBO>> mEngineUbo;

//std::shared_ptr<UniformBufferGl<GameRendererGl::EngineDirShadowUBO>> mEngineDirShadowUBO;

std::shared_ptr<TextureGl> mEmptyTexture;
std::shared_ptr<TextureGl> mHDRSkyBoxTexture;

struct EnginePointShadowUBO {
	MATHGL::Vector3 LightPos;
	float FarPlane = 0.0f;
	MATHGL::Matrix4  Matricies[6];
};
std::shared_ptr<UniformBufferGl<EnginePointShadowUBO>> mEnginePointShadowUBO;


struct EngineSpotShadowUBO{
	MATHGL::Matrix4 LightSpaceMatrix;
};
std::shared_ptr<UniformBufferGl<EngineSpotShadowUBO>> mEngineSpotShadowUBO;


struct EngineShadowDataUBO {
	MATHGL::Matrix4 dirMatrices[16];
	//dir light
	float dirCascadePlaneDistances[16];
	int dirCascadeCount;
	MATHGL::Vector3 dirLightDir;
	MATHGL::Vector3 dirLightPos;
	float dirFarPlane;
	//PCSS light
	int   dirBLOCKER_SEARCH_NUM_SAMPLES = 64;
	int   dirPCF_NUM_SAMPLES=64;
	float dirNEAR_PLANE=0.05f;
	float dirLIGHT_WORLD_SIZE=0.5f;
	float dirLIGHT_FRUSTUM_WIDTH=3.75f;
	float dirLIGHT_SIZE = (dirLIGHT_WORLD_SIZE / dirLIGHT_FRUSTUM_WIDTH);

	bool useDirLightShadow = true;
	bool useDirBakedLightShadow = false;
	bool useSpotLightShadow = false;
	bool usePointLightShadow = false;
};
std::shared_ptr<UniformBufferGl<EngineShadowDataUBO>> mEngineShadowDataUBO;



std::shared_ptr<TextureGl> gPrevPositionTex;
std::shared_ptr<TextureGl> gPositionTex;
std::shared_ptr<TextureGl> gNormalTex;
std::shared_ptr<TextureGl> gAlbedoSpecTex;
std::shared_ptr<TextureGl> gRoughAOTex;
std::shared_ptr<FrameBufferGl> gbufferFb;

std::shared_ptr<FrameBufferGl> gbufferGlobalFb;
std::shared_ptr<TextureGl> gPositionGlobalTex;
std::shared_ptr<TextureGl> gVelocityGlobalTex;

std::shared_ptr<TextureGl> deferredResTex;
std::shared_ptr<FrameBufferGl> deferredFb;

std::shared_ptr<TextureGl> deferredResPBRTex;
std::shared_ptr<FrameBufferGl> deferredPBRFb;


std::shared_ptr<FrameBufferGl> resultFb;

std::unordered_map<std::string, std::shared_ptr<TextureGl>> mTextures;
std::unordered_map<std::string, std::shared_ptr<FrameBufferGl>> mFramebuffers;
std::unordered_map<std::string, std::shared_ptr<ShaderGl>> mShaders;


void renderQuad();
void renderQuadGUI();
void renderCube();

#include "../resourceManager/modelManager.h"
#include "backends/gl/materialGl.h"


RESOURCES::ResourcePtr<RENDER::ModelInterface> sphere;
std::shared_ptr<RENDER::MaterialGl> emptyMaterial;
GameRendererGl::GameRendererGl(KUMA::CORE_SYSTEM::Core& context): mContext(context) {
	mLightSSBO = std::make_shared<ShaderStorageBufferGl>(AccessSpecifier::STREAM_DRAW);

	mEngineUbo = std::make_shared<UniformBufferGl<EngineUBO>>("Engine_UBO", 0);

	mEmptyTexture = TextureGl::create(KUMA::UTILS::getRealPath("Textures/snow.png"));

	mHDRSkyBoxTexture = TextureGl::CreateHDR(KUMA::UTILS::getRealPath("Textures/sky.hdr"));

	//mEngineDirShadowUBO = std::make_shared<UniformBufferGl<EngineDirShadowUBO>>("EngineDirShadowUBO", 1);
	mEnginePointShadowUBO = std::make_shared<UniformBufferGl<EnginePointShadowUBO>>("EnginePointShadowUBO", 1);
	mEngineSpotShadowUBO = std::make_shared<UniformBufferGl<EngineSpotShadowUBO>>("EngineSpotShadowUBO", 1);


	mEngineShadowDataUBO = std::make_shared<UniformBufferGl<EngineShadowDataUBO>>("EngineShadowDataUBO", 2);

	mDriver = dynamic_cast<DriverGl*>(context.driver.get());

	createShaders();
	createFrameBuffers();

	prepareIBL();


	sphere = RESOURCES::ModelLoader().CreateFromFile("Models/Sphere.fbx");

	emptyMaterial = std::make_shared<RENDER::MaterialGl>();
	emptyMaterial->setShader(std::make_shared<ShaderGl>("Shaders/gl/Unlit.vs.glsl", "Shaders/gl/Unlit.fs.glsl"));
	emptyMaterial->set("u_Diffuse", MATHGL::Vector3(1.f, 0.f, 1.f));
	emptyMaterial->set("u_DiffuseMap", nullptr);

	//mEmptyTexture = TextureGl::create(KUMA::UTILS::getRealPath("textures/brick_albedo.jpg"));
	//u_SpecularMap = TextureGl::create(KUMA::UTILS::getRealPath("Textures/brick_roughness.jpg"));
	//u_NormalMap = TextureGl::create(KUMA::UTILS::getRealPath("Textures/brick_normal.jpg"));
	//u_HeightMap = TextureGl::create(KUMA::UTILS::getRealPath("Textures/snow.png"));
}

MATHGL::Vector2f haltonSequence[128];
float CreateHaltonSequence(unsigned int index, int base) {
	float f = 1;
	float r = 0;
	int current = index;
	do
	{
		f = f / base;
		r = r + f * (current % base);
		current = glm::floor(current / base);
	} while (current > 0);
	return r;
}

void GameRendererGl::createShaders() {

	mShaders["deferredGBuffer"] = std::make_shared<ShaderGl>("./Shaders/gl/deferredGBuffer.vs.glsl", "./Shaders/gl/deferredGBuffer.fs.glsl");
	mShaders["deferredGBuffer"]->bind();
	mShaders["deferredGBuffer"]->setBool("engine_Settings.useTAA", true);
	mShaders["deferredGBuffer"]->setFloat("engine_JitterSettings.haltonScale", 1.0f);
	mShaders["deferredGBuffer"]->setInt("engine_JitterSettings.haltonScale", 1);
	mShaders["deferredGBuffer"]->setInt("engine_JitterSettings.numSamples", 16);
	mShaders["deferredGBuffer"]->setFloat("engine_JitterSettings.ditheringScale", 0.0f);
	for (int iter = 0; iter < 128; iter++) {
		haltonSequence[iter] = MATHGL::Vector2f(CreateHaltonSequence(iter + 1, 2), CreateHaltonSequence(iter + 1, 3));
		mShaders["deferredGBuffer"]->setVec2("engine_JitterSettings.haltonSequence[" + std::to_string(iter) + "]", haltonSequence[iter]);
	}
	mShaders["deferredGBuffer"]->unbind();


	mShaders["deferredLightning"] = std::make_shared<ShaderGl>("./Shaders/gl/deferredLightning.vs.glsl", "./Shaders/gl/deferredLightning.fs.glsl");
	mShaders["deferredLightningPbr"] = std::make_shared<ShaderGl>("./Shaders/gl/deferredLightningPbr.vs.glsl", "./Shaders/gl/deferredLightningPbr.fs.glsl");
	
	//TODO: update it when mPipeline.mIsPbr change
	mDeferredShader = mShaders["deferredLightning"];
	mDeferredShader->bind();
	mDeferredShader->setBool("engine_IsPBR", mPipeline.mIsPBR);
	mDeferredShader->setInt("u_PositionMap", 0);
	mDeferredShader->setInt("u_NormalMap", 1);
	mDeferredShader->setInt("u_AlbedoSpecMap", 2);
	mDeferredShader->setInt("u_RoughAO", 3);
	//mDeferredShader->setInt("u_Velocity", 4);
	mDeferredShader->unbind();

	mShaders["renderToScreen"] = std::make_shared<ShaderGl>("./Shaders/gl/renderToScreen.vs.glsl", "./Shaders/gl/renderToScreen.fs.glsl");
	mShaders["renderToScreen"]->bind();
	mShaders["renderToScreen"]->setInt("inputTexture", 0);
	mShaders["renderToScreen"]->unbind();
	
	mShaders["dirShadowMap"] = std::make_shared<ShaderGl>(
		"./Shaders/gl/dirShadow.vs.glsl", "./Shaders/gl/dirShadow.fs.glsl", "./Shaders/gl/dirShadow.gs.glsl");

	mShaders["spotShadowMap"] = std::make_shared<ShaderGl>(
		"./Shaders/gl/spotShadow.vs.glsl", "./Shaders/gl/spotShadow.fs.glsl");

	mShaders["pointShadowMap"] = std::make_shared<ShaderGl>(
		"./Shaders/gl/pointShadow.vs.glsl", "./Shaders/gl/pointShadow.fs.glsl", "./Shaders/gl/pointShadow.gs.glsl");

	mShaders["equirectangularToCubemap"] = std::make_shared<ShaderGl>(
		"./Shaders/gl/equirectangularToCubemap.vs.glsl", "./Shaders/gl/equirectangularToCubemap.fs.glsl");

	mShaders["irradianceConvolution"] = std::make_shared<ShaderGl>(
		"./Shaders/gl/irradianceConvolution.vs.glsl", "./Shaders/gl/irradianceConvolution.fs.glsl");

	mShaders["prefilterShader"] = std::make_shared<ShaderGl>(
		"./Shaders/gl/prefilterShader.vs.glsl", "./Shaders/gl/prefilterShader.fs.glsl");

	mShaders["brdf"] = std::make_shared<ShaderGl>(
		"./Shaders/gl/brdf.vs.glsl", "./Shaders/gl/brdf.fs.glsl");

	mShaders["hdrSkyboxShader"] = std::make_shared<ShaderGl>(
		"./Shaders/gl/hdrSkyboxShader.vs.glsl", "./Shaders/gl/hdrSkyboxShader.fs.glsl");



	mShaders["ssao"] = std::make_shared<ShaderGl>("./Shaders/gl/ssao.vs.glsl", "./Shaders/gl/ssao.fs.glsl");
	mShaders["ssao"]->bind();
	mShaders["ssao"]->setInt("gPosition", 0);
	mShaders["ssao"]->setInt("gNormal", 1);
	mShaders["ssao"]->setInt("gTexNoise", 2);
	mShaders["ssao"]->unbind();

	mShaders["ssaoBlur"] = std::make_shared<ShaderGl>("./Shaders/gl/ssaoBlur.vs.glsl", "./Shaders/gl/ssaoBlur.fs.glsl");
	mShaders["ssaoBlur"]->bind();
	mShaders["ssaoBlur"]->setInt("gInput", 0);
	mShaders["ssaoBlur"]->unbind();

	mShaders["ssr"] = std::make_shared<ShaderGl>("./Shaders/gl/ssr.vs.glsl", "./Shaders/gl/ssr.fs.glsl");
	mShaders["ssr"]->bind();
	//mShaders["ssr"]->setInt("albedoTex", 0);
	mShaders["ssr"]->setInt("normalTex", 0);
	mShaders["ssr"]->setInt("depthTex", 1);
	mShaders["ssr"]->setInt("HDRTex", 2);
	mShaders["ssr"]->unbind();

	mShaders["ssgi"] = std::make_shared<ShaderGl>("./Shaders/gl/ssgi.vs.glsl", "./Shaders/gl/ssgi.fs.glsl");
	mShaders["ssgi"]->bind();
	mShaders["ssgi"]->setInt("depthTex", 0);
	mShaders["ssgi"]->setInt("inputTex", 1);
	//mShaders["ssgi"]->setInt("albedoTex", 1);
	//mShaders["ssgi"]->setInt("normalTex", 2);
	//mShaders["ssgi"]->setInt("depthTex", 3);
	mShaders["ssgi"]->unbind();

	mShaders["sss"] = std::make_shared<ShaderGl>("./Shaders/gl/sss.vs.glsl", "./Shaders/gl/sss.fs.glsl");
	mShaders["sss"]->bind();
	mShaders["sss"]->setInt("depthTex", 0);
	//mShaders["sss"]->setInt("inputTex", 1);
	//mShaders["ssgi"]->setInt("albedoTex", 1);
	//mShaders["ssgi"]->setInt("normalTex", 2);
	//mShaders["ssgi"]->setInt("depthTex", 3);
	mShaders["ssgi"]->unbind();

	mShaders["ssrApply"] = std::make_shared<ShaderGl>("./Shaders/gl/ssrApply.vs.glsl", "./Shaders/gl/ssrApply.fs.glsl");
	mShaders["ssrApply"]->bind();
	mShaders["ssrApply"]->setInt("albedoTex", 0);
	mShaders["ssrApply"]->setInt("SSRTex", 1);
	mShaders["ssrApply"]->setInt("HDRTex", 2);
	mShaders["ssrApply"]->setInt("roughAO", 3);
	mShaders["ssrApply"]->setInt("inputTex", 4);
	mShaders["ssrApply"]->unbind();

	mShaders["ssgiApply"] = std::make_shared<ShaderGl>("./Shaders/gl/ssgiApply.vs.glsl", "./Shaders/gl/ssgiApply.fs.glsl");
	mShaders["ssgiApply"]->bind();
	mShaders["ssgiApply"]->setInt("inputTex", 0);
	mShaders["ssgiApply"]->setInt("SSGITex", 1);
	mShaders["ssgiApply"]->setInt("albedoTex", 2);
	mShaders["ssgiApply"]->unbind();

	mShaders["sssApply"] = std::make_shared<ShaderGl>("./Shaders/gl/sssApply.vs.glsl", "./Shaders/gl/sssApply.fs.glsl");
	mShaders["sssApply"]->bind();
	mShaders["sssApply"]->setInt("inputTex", 1);
	mShaders["sssApply"]->setInt("SSSTex", 0);
	//mShaders["sssApply"]->setInt("albedoTex", 2);
	mShaders["sssApply"]->unbind();


	mShaders["taa"] = std::make_shared<ShaderGl>(
		"./Shaders/gl/taa.vs.glsl", "./Shaders/gl/taa.fs.glsl");
	mShaders["taa"]->bind();
	mShaders["taa"]->setFloat("engine_TaaSettings.feedbackFactor", 0.9f);
	mShaders["taa"]->setFloat("engine_TaaSettings.maxDepthFalloff", 1.0f);
	mShaders["taa"]->setFloat("engine_TaaSettings.velocityScale", 1.0f);
	mShaders["taa"]->setInt("currentColorTex", 0);
	mShaders["taa"]->setInt("currentDepthTex", 1);
	mShaders["taa"]->setInt("previousColorTex", 2);
	mShaders["taa"]->setInt("previousDepthTex", 3);
	mShaders["taa"]->setInt("velocityTex", 4);
	mShaders["taa"]->unbind();

	mShaders["motionBlur"] = std::make_shared<ShaderGl>(
		"./Shaders/gl/motionBlur.vs.glsl", "./Shaders/gl/motionBlur.fs.glsl");
	mShaders["motionBlur"]->bind();
	mShaders["motionBlur"]->setInt("velTex", 0);
	mShaders["motionBlur"]->setInt("colorTexture", 1);
	mShaders["motionBlur"]->unbind();

	mShaders["brightTexture"] = std::make_shared<ShaderGl>(
		"./Shaders/gl/base.vs.glsl", "./Shaders/gl/bright.fs.glsl");
	mShaders["brightTexture"]->bind();
	mShaders["brightTexture"]->setInt("u_Scene", 0);
	mShaders["brightTexture"]->unbind();

	mShaders["blurTexture"] = std::make_shared<ShaderGl>(
		"./Shaders/gl/base.vs.glsl", "./Shaders/gl/blur.fs.glsl");
	mShaders["blurTexture"]->bind();
	mShaders["blurTexture"]->setInt("image", 0);
	mShaders["blurTexture"]->unbind();

	mShaders["bloom"] = std::make_shared<ShaderGl>(
		"./Shaders/gl/base.vs.glsl", "./Shaders/gl/bloom.fs.glsl");
	mShaders["bloom"]->bind();
	mShaders["bloom"]->setInt("u_Scene", 0);
	mShaders["bloom"]->setInt("u_BloomBlur", 1);
	mShaders["bloom"]->unbind();

	mShaders["godRaysTexture"] = std::make_shared<ShaderGl>(
		"./Shaders/gl/godRaysTexture.vs.glsl", "./Shaders/gl/godRaysTexture.fs.glsl");
	mShaders["godRaysTexture"]->bind();
	mShaders["godRaysTexture"]->setInt("image", 0);
	mShaders["godRaysTexture"]->unbind();

	mShaders["godRays"] = std::make_shared<ShaderGl>(
		"./Shaders/gl/godRays.vs.glsl", "./Shaders/gl/godRays.fs.glsl");
	mShaders["godRays"]->bind();
	mShaders["godRays"]->setInt("u_Scene", 0);
	mShaders["godRays"]->setInt("u_BinaryScene", 1);
	mShaders["godRays"]->unbind();

	mShaders["fxaa"] = std::make_shared<ShaderGl>(
		"./Shaders/gl/base.vs.glsl", "./Shaders/gl/fxaa.fs.glsl");
	mShaders["fxaa"]->bind();
	mShaders["fxaa"]->setInt("u_Scene", 0);
	mShaders["fxaa"]->unbind();

	mShaders["hdr"] = std::make_shared<ShaderGl>(
		"./Shaders/gl/base.vs.glsl", "./Shaders/gl/hdr.fs.glsl");
	mShaders["hdr"]->bind();
	mShaders["hdr"]->setInt("u_Scene", 0);
	mShaders["hdr"]->unbind();

	mShaders["colorGrading"] = std::make_shared<ShaderGl>(
		"./Shaders/gl/base.vs.glsl", "./Shaders/gl/colorGrading.fs.glsl");
	mShaders["colorGrading"]->bind();
	mShaders["colorGrading"]->setInt("u_Scene", 0);
	mShaders["colorGrading"]->unbind();

	mShaders["vignette"] = std::make_shared<ShaderGl>(
		"./Shaders/gl/base.vs.glsl", "./Shaders/gl/vignette.fs.glsl");
	mShaders["vignette"]->bind();
	mShaders["vignette"]->setInt("u_Scene", 0);
	mShaders["vignette"]->unbind();

	mShaders["depthOfField"] = std::make_shared<ShaderGl>(
		"./Shaders/gl/base.vs.glsl", "./Shaders/gl/depthOfField.fs.glsl");
	mShaders["depthOfField"]->bind();
	mShaders["depthOfField"]->setInt("u_Scene", 0);
	mShaders["depthOfField"]->setInt("positionTexture", 1);
	mShaders["depthOfField"]->setInt("noiseTexture", 2);
	mShaders["depthOfField"]->setInt("outOfFocusTexture", 3);
	mShaders["depthOfField"]->unbind();

	mShaders["outline"] = std::make_shared<ShaderGl>(
		"./Shaders/gl/base.vs.glsl", "./Shaders/gl/outline.fs.glsl");
	mShaders["outline"]->bind();
	mShaders["outline"]->setInt("u_Scene", 0);
	mShaders["outline"]->setInt("positionTexture", 1);
	mShaders["outline"]->setInt("noiseTexture", 2);
	mShaders["outline"]->unbind();

	mShaders["chromaticAbberation"] = std::make_shared<ShaderGl>(
		"./Shaders/gl/base.vs.glsl", "./Shaders/gl/chromaticAbberation.fs.glsl");
	mShaders["chromaticAbberation"]->bind();
	mShaders["chromaticAbberation"]->setInt("u_Scene", 0);
	mShaders["chromaticAbberation"]->unbind();

	mShaders["posterize"] = std::make_shared<ShaderGl>(
		"./Shaders/gl/base.vs.glsl", "./Shaders/gl/posterize.fs.glsl");
	mShaders["posterize"]->bind();
	mShaders["posterize"]->setInt("u_Scene", 0);
	mShaders["posterize"]->setInt("positionTexture", 1);
	mShaders["posterize"]->unbind();

	mShaders["pixelize"] = std::make_shared<ShaderGl>(
		"./Shaders/gl/base.vs.glsl", "./Shaders/gl/pixelize.fs.glsl");
	mShaders["pixelize"]->bind();
	mShaders["pixelize"]->setInt("u_Scene", 0);
	mShaders["pixelize"]->setInt("positionTexture", 1);
	mShaders["pixelize"]->unbind();

	mShaders["sharpen"] = std::make_shared<ShaderGl>(
		"./Shaders/gl/base.vs.glsl", "./Shaders/gl/sharpen.fs.glsl");
	mShaders["sharpen"]->bind();
	mShaders["sharpen"]->setInt("u_Scene", 0);
	mShaders["sharpen"]->unbind();

	mShaders["dilation"] = std::make_shared<ShaderGl>(
		"./Shaders/gl/base.vs.glsl", "./Shaders/gl/dilation.fs.glsl");
	mShaders["dilation"]->bind();
	mShaders["dilation"]->setInt("u_Scene", 0);
	mShaders["dilation"]->unbind();

	mShaders["filmGrain"] = std::make_shared<ShaderGl>(
		"./Shaders/gl/base.vs.glsl", "./Shaders/gl/filmGrain.fs.glsl");
	mShaders["filmGrain"]->bind();
	mShaders["filmGrain"]->setInt("u_Scene", 0);
	mShaders["filmGrain"]->unbind();

	mShaders["GBuffer"] = std::make_shared<ShaderGl>(
		"./Shaders/gl/GBuffer.vs.glsl", "./Shaders/gl/GBuffer.fs.glsl");
	mShaders["GBuffer"]->bind();
	mShaders["GBuffer"]->setBool("engine_Settings.useTAA", true);
	mShaders["GBuffer"]->setFloat("engine_JitterSettings.haltonScale", 1.0f);
	mShaders["GBuffer"]->setInt("engine_JitterSettings.haltonScale", 1);
	mShaders["GBuffer"]->setInt("engine_JitterSettings.numSamples", 16);
	mShaders["GBuffer"]->setFloat("engine_JitterSettings.ditheringScale", 0.0f);
	for (int iter = 0; iter < 128; iter++) {
		haltonSequence[iter] = MATHGL::Vector2f(CreateHaltonSequence(iter + 1, 2), CreateHaltonSequence(iter + 1, 3));
		mShaders["GBuffer"]->setVec2("engine_JitterSettings.haltonSequence[" + std::to_string(iter) + "]", haltonSequence[iter]);
	}
	mShaders["GBuffer"]->unbind();

	//gui
	mShaders["sprite"] = std::make_shared<ShaderGl>(
		"./Shaders/gui/sprite.vs.glsl", "./Shaders/gui/sprite.fs.glsl");
	mShaders["sprite"]->bind();
	mShaders["sprite"]->setInt("image", 0);
	mShaders["sprite"]->bind();

	mShaders["label"] = std::make_shared<ShaderGl>(
		"./Shaders/gui/text.vs.glsl", "./Shaders/gui/text.fs.glsl");
	mShaders["label"]->bind();
	mShaders["label"]->setInt("u_engine_text", 0);
	mShaders["label"]->bind();
}

bool pingPong = false;
std::array<std::shared_ptr<FrameBufferGl>, 2> pingPongFb;
std::array<std::shared_ptr<TextureGl>, 2> pingPongTex;

std::array<std::shared_ptr<FrameBufferGl>, 2> pingPongBlurFb;
std::array<std::shared_ptr<TextureGl>, 2> pingPongBlurTex;

void GameRendererGl::createFrameBuffers() {
	auto [winWidth, winHeight] = mContext.window->getSize();

	pingPongFb[0] = std::make_shared<FrameBufferGl>();
	pingPongTex[0] = TextureGl::createForAttach(winWidth, winHeight, GL_FLOAT);
	pingPongFb[0]->create({ pingPongTex[0] });
	pingPongFb[1] = std::make_shared<FrameBufferGl>();
	pingPongTex[1] = TextureGl::createForAttach(winWidth, winHeight, GL_FLOAT);
	pingPongFb[1]->create({ pingPongTex[1] });

	pingPongBlurFb[0] = std::make_shared<FrameBufferGl>();
	pingPongBlurTex[0] = TextureGl::createForAttach(winWidth, winHeight, GL_FLOAT);
	pingPongBlurFb[0]->create({ pingPongBlurTex[0] });
	pingPongBlurFb[1] = std::make_shared<FrameBufferGl>();
	pingPongBlurTex[1] = TextureGl::createForAttach(winWidth, winHeight, GL_FLOAT);
	pingPongBlurFb[1]->create({ pingPongBlurTex[1] });

	gbufferFb = std::make_shared<FrameBufferGl>();
	gPositionTex = TextureGl::createForAttach(winWidth, winHeight, GL_FLOAT);
	gPrevPositionTex = TextureGl::createForAttach(winWidth, winHeight, GL_FLOAT);
	gNormalTex = TextureGl::createForAttach(winWidth, winHeight, GL_FLOAT);
	gAlbedoSpecTex = TextureGl::createForAttach(winWidth, winHeight, GL_FLOAT);
	gRoughAOTex = TextureGl::createForAttach(winWidth, winHeight, GL_FLOAT);
	std::vector<std::shared_ptr<TextureGl>> gtexs = std::vector{ gPositionTex, gNormalTex, gAlbedoSpecTex, gRoughAOTex };
	gbufferFb->create(gtexs);

	gbufferGlobalFb = std::make_shared<FrameBufferGl>();
	gPositionGlobalTex = TextureGl::createForAttach(winWidth, winHeight, GL_FLOAT);
	gVelocityGlobalTex = TextureGl::createForAttach(winWidth, winHeight, GL_FLOAT);
	std::vector<std::shared_ptr<TextureGl>> ggtexs = std::vector{ gPositionGlobalTex, gVelocityGlobalTex };
	gbufferGlobalFb->create(ggtexs);

	deferredFb = std::make_shared<FrameBufferGl>();
	deferredResTex = TextureGl::createForAttach(winWidth, winHeight, GL_FLOAT);
	std::vector<std::shared_ptr<TextureGl>> deferredtexs = std::vector{ deferredResTex };
	deferredFb->create(deferredtexs);

	//TODO: update it when mPipeline.mIsPbr change
	mDeferredFb = deferredFb;
	mDeferredTexture = deferredResTex;
	mPrevDeferredTexture = TextureGl::createForAttach(winWidth, winHeight, GL_FLOAT);

	deferredPBRFb = std::make_shared<FrameBufferGl>();
	deferredResPBRTex = TextureGl::createForAttach(winWidth, winHeight, GL_FLOAT);
	std::vector<std::shared_ptr<TextureGl>> deferredPBRtexs = std::vector{ deferredResPBRTex };
	deferredPBRFb->create(deferredPBRtexs);

	mTextures["brightTexture"] = TextureGl::createForAttach(winWidth, winHeight, GL_FLOAT);
	mFramebuffers["brightTexture"] = std::make_shared<FrameBufferGl>();
	mFramebuffers["brightTexture"]->create({ mTextures["brightTexture"] });

	mTextures["godRaysTexture"] = TextureGl::createForAttach(winWidth, winHeight, GL_FLOAT);
	mFramebuffers["godRaysTexture"] = std::make_shared<FrameBufferGl>();
	mFramebuffers["godRaysTexture"]->create({ mTextures["godRaysTexture"] });

	mTextures["dirShadowMap"] = TextureGl::createDepthForAttach2DArray(mPipeline.mDirShadowMap.mDirShadowMapResolution, mPipeline.mDirShadowMap.mDirShadowMapResolution, mPipeline.mDirShadowMap.mShadowCascadeLevels.size() + 1);
	mFramebuffers["dirShadowMap"] = std::make_shared<FrameBufferGl>();
	mFramebuffers["dirShadowMap"]->create({ }, mTextures["dirShadowMap"]);

	mTextures["dirShadowMapBaked"] = TextureGl::createDepthForAttach2DArray(mPipeline.mDirShadowMap.mDirShadowMapResolution, mPipeline.mDirShadowMap.mDirShadowMapResolution, mPipeline.mDirShadowMap.mShadowCascadeLevels.size() + 1);
	mFramebuffers["dirShadowMapBaked"] = std::make_shared<FrameBufferGl>();
	mFramebuffers["dirShadowMapBaked"]->create({ }, mTextures["dirShadowMapBaked"]);

	mTextures["spotShadowMap"] = TextureGl::createDepthForAttach(mPipeline.mDirShadowMap.mSpotShadowMapResolution, mPipeline.mDirShadowMap.mSpotShadowMapResolution);
	mFramebuffers["spotShadowMap"] = std::make_shared<FrameBufferGl>();
	mFramebuffers["spotShadowMap"]->create({ }, mTextures["spotShadowMap"]);


	mTextures["deferredTextureSave"] = TextureGl::createForAttach(winWidth, winHeight, GL_FLOAT);
	mTextures["blur"] = TextureGl::createForAttach(winWidth, winHeight, GL_FLOAT);

	mTextures["ssao"] = TextureGl::createForAttach(winWidth, winHeight, GL_FLOAT);
	mFramebuffers["ssao"] = std::make_shared<FrameBufferGl>();
	mFramebuffers["ssao"]->create({ mTextures["ssao"] });

	mTextures["ssaoBlur"] = TextureGl::createForAttach(winWidth, winHeight, GL_FLOAT);
	mFramebuffers["ssaoBlur"] = std::make_shared<FrameBufferGl>();
	mFramebuffers["ssaoBlur"]->create({ mTextures["ssaoBlur"] });

	//mFramebuffers["equirectangularToCubemap"] = std::make_shared<FrameBufferGl>();
	//mFramebuffers["equirectangularToCubemap"]->create();
	//mFramebuffers["ssaoequirectangularToCubemapBlur"]->create({ mTextures["hdrSkybox"] });

	// Генерируем ядро выборки
	std::uniform_real_distribution<GLfloat> randomFloats(0.0, 1.0); // генерируем случайное число типа float в диапазоне между 0.0 и 1.0
	std::default_random_engine generator;

	auto lerp = [](float a, float b, float f) {
		return a + f * (b - a);
	};
	for (unsigned int i = 0; i < 64; ++i) {
		MATHGL::Vector3 sample(randomFloats(generator) * 2.0 - 1.0, randomFloats(generator) * 2.0 - 1.0, randomFloats(generator));
		sample = MATHGL::Vector3::Normalize(sample);
		sample *= randomFloats(generator);
		float scale = float(i) / 64.0;
		// Масштабируем точки выборки, чтобы они распологались ближе к центру ядра
		scale = lerp(0.1f, 1.0f, scale * scale);
		sample *= scale;
		mPipeline.mSSAO.mSSAOKernel.push_back(sample);
	}

	// Генерируем текстуру шума
	std::vector<uint8_t> ssaoNoise;
	for (unsigned int i = 0; i < 16; i++) {
		//glm::vec3 noise(randomFloats(generator) * 2.0 - 1.0, randomFloats(generator) * 2.0 - 1.0, 0.0f); // поворот вокруг z-оси (в касательном пространстве)
		ssaoNoise.push_back(randomFloats(generator) * 2.0 - 1.0);
		ssaoNoise.push_back(randomFloats(generator) * 2.0 - 1.0);
		ssaoNoise.push_back(0.0);
	}

	mTextures["noiseTexture"] = TextureGl::CreateFromMemory(&ssaoNoise[0], 4, 4, false);

	
	//mTextures["pointShadowMap"] = TextureGl::createDepthForAttachCubemap(mPipeline.mDirShadowMap.mPointShadowMapResolution, mPipeline.mDirShadowMap.mPointShadowMapResolution, GL_FLOAT);
	//mFramebuffers["pointShadowMap"] = std::make_shared<FrameBufferGl>();
	//mFramebuffers["pointShadowMap"]->create({ }, mTextures["pointShadowMap"]);


	mTextures["ssr"] = TextureGl::createForAttach(winWidth, winHeight, GL_FLOAT);
	mFramebuffers["ssr"] = std::make_shared<FrameBufferGl>();
	mFramebuffers["ssr"]->create({ mTextures["ssr"] });

	mTextures["ssgi"] = TextureGl::createForAttach(winWidth, winHeight, GL_FLOAT);
	mFramebuffers["ssgi"] = std::make_shared<FrameBufferGl>();
	mFramebuffers["ssgi"]->create({ mTextures["ssgi"] });

	mTextures["sss"] = TextureGl::createForAttach(winWidth, winHeight, GL_FLOAT);
	mFramebuffers["sss"] = std::make_shared<FrameBufferGl>();
	mFramebuffers["sss"]->create({ mTextures["sss"] });

	mTextures["noiseTexture"] = TextureGl::create(KUMA::UTILS::getRealPath("Textures/color-noise.png"));
}


void GameRendererGl::applySSS() {
	mFramebuffers["sss"]->bind();
	mShaders["sss"]->bind();
	//gAlbedoSpecTex->bind(0);
	//gNormalTex->bind(0);
	gPositionTex->bind(0);
	//mHDRSkyBoxTexture->bind(2);
	//gAlbedoSpecTex->bind(2);
	mDriver->clear(true, true, false);
	renderQuad();
	mShaders["sss"]->unbind();
	mFramebuffers["sss"]->unbind();
	
	//TextureGl::CopyTexture(*mDeferredTexture, *mTextures["deferredTextureSave"]);

	//appy
	pingPongFb[pingPong]->bind();
	mShaders["sssApply"]->bind();
	//mTextures["deferredTextureSave"]->bind(0);
	//gAlbedoSpecTex->bind(0);
	mTextures["sss"]->bind(0);
	//mHDRSkyBoxTexture->bind(2);
	//gRoughAOTex->bind(3);
	pingPongTex[!pingPong]->bind(1);
	mDriver->clear(true, true, false);
	renderQuad();
	mShaders["sssApply"]->unbind();
	pingPongFb[pingPong]->unbind();
	pingPong = !pingPong;
}

void GameRendererGl::applySSR() {
	mFramebuffers["ssr"]->bind();
	mShaders["ssr"]->bind();
	//gAlbedoSpecTex->bind(0);
	gNormalTex->bind(0);
	gPositionTex->bind(1);
	//mHDRSkyBoxTexture->bind(2);
	gAlbedoSpecTex->bind(2);
	mDriver->clear(true, true, false);
	renderQuad();
	mShaders["ssr"]->unbind();
	mFramebuffers["ssr"]->unbind();

	//TextureGl::CopyTexture(*mDeferredTexture, *mTextures["deferredTextureSave"]);

	//appy
	pingPongFb[pingPong]->bind();
	mShaders["ssrApply"]->bind();
	//mTextures["deferredTextureSave"]->bind(0);
	gAlbedoSpecTex->bind(0);
	mTextures["ssr"]->bind(1);
	mHDRSkyBoxTexture->bind(2);
	gRoughAOTex->bind(3);
	pingPongTex[!pingPong]->bind(4);
	mDriver->clear(true, true, false);
	renderQuad();
	mShaders["ssrApply"]->unbind();
	pingPongFb[pingPong]->unbind();
	pingPong = !pingPong;
}

void GameRendererGl::applySSGI() {
	mFramebuffers["ssgi"]->bind();
	mShaders["ssgi"]->bind();
	gPositionTex->bind(0);
	mTextures["deferredTextureSave"]->bind(1);
	//gNormalTex->bind(2);
	//gPositionTex->bind(3);
	mDriver->clear(true, true, false);
	renderQuad();
	mShaders["ssgi"]->unbind();
	mFramebuffers["ssgi"]->unbind();


	//TextureGl::CopyTexture(*mDeferredTexture, *mTextures["deferredTextureSave"]);

	//appy
	pingPongFb[pingPong]->bind();
	mShaders["ssgiApply"]->bind();
	pingPongTex[!pingPong]->bind(0);
	mTextures["ssgi"]->bind(1);
	gAlbedoSpecTex->bind(2);
	mDriver->clear(true, true, false);
	renderQuad();
	mShaders["ssgiApply"]->unbind();
	pingPongFb[pingPong]->unbind();
	pingPong = !pingPong;
}


void GameRendererGl::renderSkybox() {

	glEnable(GL_DEPTH_TEST);
	mDeferredFb->bind();
	//if (isHDRSkybox) {
		mShaders["hdrSkyboxShader"]->bind();

		GLint OldCullFaceMode;
		glGetIntegerv(GL_CULL_FACE_MODE, &OldCullFaceMode);
		GLint OldDepthFuncMode;
		glGetIntegerv(GL_DEPTH_FUNC, &OldDepthFuncMode);
		glCullFace(GL_FRONT);
		glDepthFunc(GL_LEQUAL);


		mShaders["hdrSkyboxShader"]->setInt("environmentMap", 0);
		mTextures["hdrSkybox"]->bind(0);
		//glBindTexture(GL_TEXTURE_CUBE_MAP, irradianceMap); // display irradiance map
		//glBindTexture(GL_TEXTURE_CUBE_MAP, prefilterMap); // display prefilter map
		renderCube();
		glCullFace(OldCullFaceMode);
		glDepthFunc(OldDepthFuncMode);
		mShaders["hdrSkyboxShader"]->unbind();
	//}
	mDeferredFb->unbind();

}

void GameRendererGl::prepareIBL() {//ibl
	unsigned int captureFBO;
	unsigned int captureRBO;
	glGenFramebuffers(1, &captureFBO);
	glGenRenderbuffers(1, &captureRBO);

	glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
	glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 512, 512);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, captureRBO);


	// PBR: настройка кубической карты для рендеринга и прикрепления к фреймбуферу
	//envCubemap
	mTextures["hdrSkybox"] = TextureGl::CreateHDREmptyCubemap(512, 512);

	// PBR: установка матриц проекции и вида для захвата данных по всем 6 направлениям граней кубической карты
	auto captureProjection = MATHGL::Matrix4::CreatePerspective(90.0f, 1.0f, 0.1f, 10.0f);
	std::vector<MATHGL::Matrix4> captureViews = {
		MATHGL::Matrix4::CreateView(MATHGL::Vector3(0.0f, 0.0f, 0.0f), MATHGL::Vector3(1.0f,  0.0f,  0.0f), MATHGL::Vector3(0.0f, -1.0f,  0.0f)),
		MATHGL::Matrix4::CreateView(MATHGL::Vector3(0.0f, 0.0f, 0.0f), MATHGL::Vector3(-1.0f,  0.0f,  0.0f),MATHGL::Vector3(0.0f, -1.0f,  0.0f)),
		MATHGL::Matrix4::CreateView(MATHGL::Vector3(0.0f, 0.0f, 0.0f), MATHGL::Vector3(0.0f,  1.0f,  0.0f), MATHGL::Vector3(0.0f,  0.0f,  1.0f)),
		MATHGL::Matrix4::CreateView(MATHGL::Vector3(0.0f, 0.0f, 0.0f), MATHGL::Vector3(0.0f, -1.0f,  0.0f), MATHGL::Vector3(0.0f,  0.0f, -1.0f)),
		MATHGL::Matrix4::CreateView(MATHGL::Vector3(0.0f, 0.0f, 0.0f), MATHGL::Vector3(0.0f,  0.0f,  1.0f), MATHGL::Vector3(0.0f, -1.0f,  0.0f)),
		MATHGL::Matrix4::CreateView(MATHGL::Vector3(0.0f, 0.0f, 0.0f), MATHGL::Vector3(0.0f,  0.0f, -1.0f), MATHGL::Vector3(0.0f, -1.0f,  0.0f))
	};

	// PBR: конвертирование равнопромежуточной HDR-карты окружения в кубическую
	mShaders["equirectangularToCubemap"]->bind();
	mShaders["equirectangularToCubemap"]->setInt("equirectangularMap", 0);
	mShaders["equirectangularToCubemap"]->setMat4("projection", captureProjection);

	mHDRSkyBoxTexture->bind(0);

	mDriver->setViewPort(0, 0, 512, 512); // не забудьте настроить видовой экран в соответствии с размерами захвата
	//mFramebuffers["equirectangularToCubemap"]->bind();
	glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
	for (unsigned int i = 0; i < 6; ++i) {
		mShaders["equirectangularToCubemap"]->setMat4("view", captureViews[i]);
		//mFramebuffers["equirectangularToCubemap"]->attachCubeMapSide(mTextures["hdrSkybox"], i);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, mTextures["hdrSkybox"]->id, 0);
		mDriver->clear(true, true, false);

		renderCube();
	}
	//mFramebuffers["equirectangularToCubemap"]->unbind();
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// Далее позволим OpenGL сгенерировать мипмап-карты (для борьбы с артефактами в виде визуальных точек)
	mTextures["hdrSkybox"]->generateMipmaps();

	// PBR: создаем кубическую карту облученности, и приводим размеры захвата FBO к размерам карты облученности
	mTextures["irradianceMap"] = TextureGl::CreateHDREmptyCubemap(32, 32);
	glBindTexture(GL_TEXTURE_CUBE_MAP, mTextures["irradianceMap"]->id);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		//captureFBO.bind();
		//captureRBO.init(32, 32, DepthBuffer::Format::DEPTH_COMPONENT24);
		

		// PBR: решаем диффузный интеграл, применяя операцию свертки для создания кубической карты облученности
		mShaders["irradianceConvolution"]->bind();
		mShaders["irradianceConvolution"]->setInt("environmentMap", 0);
		mShaders["irradianceConvolution"]->setMat4("projection", captureProjection);
		mTextures["hdrSkybox"]->bind(0);

		mDriver->setViewPort(0, 0, 32, 32); // не забудьте настроить видовой экран на размеры захвата
		//captureFBO.bind();
		glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
		for (unsigned int i = 0; i < 6; ++i) {
			mShaders["irradianceConvolution"]->setMat4("view", captureViews[i]);
			//captureFBO.attachCubeMapSide(mTextures["irradianceMap"], i);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, mTextures["irradianceMap"]->id, 0);

			mDriver->clear(true, true, false);
			renderCube();
		}
		//captureFBO.unbind();
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		// PBR: создаем префильтрованную кубическую карту, и приводим размеры захвата FBO к размерам префильтрованной карты
		mTextures["prefilterMap"] = TextureGl::CreateHDREmptyCubemap(128, 128);
		glBindTexture(GL_TEXTURE_CUBE_MAP, mTextures["prefilterMap"]->id);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR); // be sure to set minification filter to mip_linear 
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		// Генерируем мипмап-карты для кубической карты, OpenGL автоматически выделит нужное количество памяти
		mTextures["prefilterMap"]->generateMipmaps();
		
		// PBR: применяем симуляцию квази Монте-Карло для освещения окружающей среды, чтобы создать префильтрованную (кубическую)карту
		mShaders["prefilterShader"]->bind();
		mShaders["prefilterShader"]->setInt("environmentMap", 0);
		mShaders["prefilterShader"]->setMat4("projection", captureProjection);
		mTextures["hdrSkybox"]->bind(0);

		//captureFBO.bind();
		glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
		unsigned int maxMipLevels = 5;
		for (unsigned int mip = 0; mip < maxMipLevels; ++mip) {
			// Изменяем размеры фреймбуфера в соответствии с размерами мипмап-карты
			unsigned int mipWidth = 128 * std::pow(0.5, mip);
			unsigned int mipHeight = 128 * std::pow(0.5, mip);
			//captureRBO.init(mipWidth, mipHeight, DepthBuffer::Format::DEPTH_COMPONENT24);
			glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
			glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, mipWidth, mipHeight);
			mDriver->setViewPort(0, 0, mipWidth, mipHeight);

			float roughness = (float)mip / (float)(maxMipLevels - 1);
			mShaders["prefilterShader"]->setFloat("roughness", roughness);
			for (unsigned int i = 0; i < 6; ++i) {
				mShaders["prefilterShader"]->setMat4("view", captureViews[i]);
				//captureRBO.attachCubeMapSide(*pipeline.ibl.prefilterMap, i, mip);
				glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, mTextures["prefilterMap"]->id, mip);
				mDriver->clear(true, true, false);
				renderCube();
			}
		}
		//captureFBO.unbind();
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		// PBR: генерируем 2D LUT-текстуру при помощи используемых уравнений BRDF
		mTextures["brdfLUTTexture"] = TextureGl::createForAttach(512, 512, GL_FLOAT);

		// Убеждаемся, что режим наложения задан как GL_CLAMP_TO_EDGE
		//pipeline.ibl.brdfLUTTexture->setFilter(RESOURCES::TextureFiltering::LINEAR, RESOURCES::TextureFiltering::LINEAR);
		//pipeline.ibl.brdfLUTTexture->setWrapType(RESOURCES::TextureWrap::CLAMP_TO_EDGE, RESOURCES::TextureWrap::CLAMP_TO_EDGE);
		glBindTexture(GL_TEXTURE_CUBE_MAP, mTextures["brdfLUTTexture"]->id);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		// Затем переконфигурируем захват объекта фреймбуфера и рендерим экранный прямоугольник с использованием BRDF-шейдера
		//captureFBO.bind();
		//captureRBO.init(512, 512, DepthBuffer::Format::DEPTH_COMPONENT24);
		//captureFBO.attachTexture(*pipeline.ibl.brdfLUTTexture, Attachment::COLOR_ATTACHMENT0);
		glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
		glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 512, 512);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mTextures["brdfLUTTexture"]->id, 0);

		mDriver->setViewPort(0, 0, 512, 512);
		mShaders["brdf"]->bind();
		mDriver->clear(true, true, false);
		renderQuad();
		mShaders["brdf"]->unbind();
		//captureFBO.unbind();
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		auto [winWidth, winHeight] = mContext.window->getSize();
		mDriver->setViewPort(0, 0, winWidth, winHeight);
}


void GameRendererGl::applySSAO() {
	//TODO: add support fb without depth for mDriver->clear(true, false, false);
	// 2. Генерируем текстуру для SSAO
	mFramebuffers["ssao"]->bind();
	mDriver->clear(true, true, false);
	mShaders["ssao"]->bind();

	// Посылаем ядро + поворот 
	for (unsigned int i = 0; i < 64; ++i) {
		mShaders["ssao"]->setVec3("samples[" + std::to_string(i) + "]", mPipeline.mSSAO.mSSAOKernel[i]);
	}
	gPositionTex->bind(0);
	gNormalTex->bind(1);
	mTextures["noiseTexture"]->bind(2);
	renderQuad();
	mShaders["ssao"]->unbind();
	mFramebuffers["ssao"]->unbind();

	// 3. Размываем SSAO-текстуру, чтобы убрать шум
	mFramebuffers["ssaoBlur"]->bind();
	mDriver->clear(true, true, false);
	mShaders["ssaoBlur"]->bind();
	mTextures["ssao"]->bind(0);
	renderQuad();
	mFramebuffers["ssaoBlur"]->unbind();
}
MATHGL::Matrix4 PrevView;
MATHGL::Matrix4 View;
void GameRendererGl::sendEngineUBO() {
	auto [winWidth, winHeight] = mContext.window->getSize();
	const auto& cameraPosition = mainCameraComponent.value()->obj->getTransform()->getWorldPosition();
	EngineUBO data;
	data.Projection = MATHGL::Matrix4::Transpose(mainCameraComponent.value()->getCamera().getProjectionMatrix());
	data.View = MATHGL::Matrix4::Transpose(mainCameraComponent.value()->getCamera().getViewMatrix());
	data.ViewPos = cameraPosition;
	data.ViewportSize = MATHGL::Vector2f(winWidth, winHeight);

	data.Time = static_cast<float>(KUMA::TIME::Timer::GetInstance().getTimeSinceStart().count());
	data.FPS = static_cast<float>(KUMA::TIME::Timer::GetInstance().getFPS());
	data.FrameCount = frameCount;
	mEngineUbo->set(data);
	View = data.View;
}

EngineShadowDataUBO mEngineShadowData;
void GameRendererGl::sendEngineShadowUBO(std::shared_ptr<ShaderGl> shader) {
		//dir light
		int i = 0;
		for (auto e : mPipeline.mDirShadowMap.mShadowCascadeLevels) {
			mEngineShadowData.dirCascadePlaneDistances[i] = e;
			i++;
		}
		mEngineShadowData.dirCascadeCount = mPipeline.mDirShadowMap.mShadowCascadeLevels.size();
		mEngineShadowData.dirFarPlane = mPipeline.mDirShadowMap.mDirFarPlane;
		//PCSS light
		mEngineShadowData.dirBLOCKER_SEARCH_NUM_SAMPLES = 64;
		mEngineShadowData.dirPCF_NUM_SAMPLES = 64;
		mEngineShadowData.dirNEAR_PLANE = 0.22f;
		mEngineShadowData.dirLIGHT_WORLD_SIZE = 1;
		mEngineShadowData.dirLIGHT_FRUSTUM_WIDTH = 5.75f;
		mEngineShadowData.dirLIGHT_SIZE = (mEngineShadowData.dirLIGHT_WORLD_SIZE / mEngineShadowData.dirLIGHT_FRUSTUM_WIDTH);

		mEngineShadowData.useDirLightShadow = true;
		mEngineShadowData.useDirBakedLightShadow = false;
		mEngineShadowData.useSpotLightShadow = false;
		mEngineShadowData.usePointLightShadow = false;
	//mEngineShadowDataUBO->set(mEngineShadowData);

		for (auto& e : mEngineShadowData.dirMatrices) {
			shader->setMat4("engine_ShadowUBO.dirMatrices[" + std::to_string(i) + "]", e);
			i++;
		}
		shader->setVec3("engine_ShadowUBO.dirLightPos", mEngineShadowData.dirLightPos);
		shader->setVec3("engine_ShadowUBO.dirLightDir", mEngineShadowData.dirLightDir);
		for (auto e : mPipeline.mDirShadowMap.mShadowCascadeLevels) {
			shader->setMat4("engine_ShadowUBO.dirCascadePlaneDistances[" + std::to_string(i) + "]", mEngineShadowData.dirCascadePlaneDistances[i]);
			i++;
		}
		shader->setInt("engine_ShadowUBO.dirCascadeCount", mEngineShadowData.dirCascadeCount);
		shader->setFloat("engine_ShadowUBO.dirFarPlane", mEngineShadowData.dirFarPlane);
		shader->setInt("engine_ShadowUBO.dirBLOCKER_SEARCH_NUM_SAMPLES", mEngineShadowData.dirBLOCKER_SEARCH_NUM_SAMPLES);
		shader->setInt("engine_ShadowUBO.dirPCF_NUM_SAMPLES", mEngineShadowData.dirPCF_NUM_SAMPLES);
		shader->setFloat("engine_ShadowUBO.dirNEAR_PLANE", mEngineShadowData.dirNEAR_PLANE);
		shader->setFloat("engine_ShadowUBO.dirLIGHT_WORLD_SIZE", mEngineShadowData.dirLIGHT_WORLD_SIZE);
		shader->setFloat("engine_ShadowUBO.dirLIGHT_FRUSTUM_WIDTH", mEngineShadowData.dirLIGHT_FRUSTUM_WIDTH);
		shader->setFloat("engine_ShadowUBO.dirLIGHT_SIZE", mEngineShadowData.dirLIGHT_SIZE);
		shader->setBool("engine_ShadowUBO.useDirLightShadow", mEngineShadowData.useDirLightShadow);
		shader->setBool("engine_ShadowUBO.useDirBakedLightShadow", mEngineShadowData.useDirBakedLightShadow);
		shader->setBool("engine_ShadowUBO.useSpotLightShadow", mEngineShadowData.useSpotLightShadow);
		shader->setBool("engine_ShadowUBO.usePointLightShadow", mEngineShadowData.usePointLightShadow);

		//shader->setInt("engine_dirShadowMap", 4);
		//mTextures["dirShadowMap"]->bind(4);

		shader->setInt("engine_dirShadowMap", 6);
		mTextures["spotShadowMap"]->bind(6);
		shader->setMat4("engine_ShadowUBO.dirLightSpaceMatrix", mPipeline.mDirShadowMap.dirLightSpaceMatrix);

}

void GameRendererGl::renderScene() {
	mainCameraComponent = std::nullopt;
	if (mContext.sceneManager->hasCurrentScene()) {
		mainCameraComponent = mContext.sceneManager->getCurrentScene().findMainCamera();
	}
	if (mainCameraComponent) {
		auto [winWidth, winHeight] = mContext.window->getSize();
		const auto& cameraPosition = mainCameraComponent.value()->obj->getTransform()->getWorldPosition();
		const auto& cameraRotation = mainCameraComponent.value()->obj->getTransform()->getWorldRotation();
		mainCameraComponent->getPtr()->getCamera().cacheMatrices(winWidth, winHeight, cameraPosition, cameraRotation);

		PrevView = View;
		//Save current state
		auto glState = mDriver->fetchGLState();

		sendEngineUBO();
		//sendEngineShadowUBO(nullptr);

		renderScene(mainCameraComponent.value());

		//Return state back
		mDriver->applyStateMask(glState);
	}
	else {
		mDriver->setClearColor(mPipeline.mClearColor.x, mPipeline.mClearColor.y, mPipeline.mClearColor.z, mPipeline.mClearColor.w);
		mDriver->clear(true, true, false);
	}
	frameCount++;
	frameCount %= 10000;
}

std::vector<MATHGL::Vector4> getFrustumCornersWorldSpace(const MATHGL::Matrix4& projview) {
	const auto inv = MATHGL::Matrix4::Inverse(projview);
	std::vector<MATHGL::Vector4> frustumCorners;
	for (unsigned int x = 0; x < 2; ++x) {
		for (unsigned int y = 0; y < 2; ++y) {
			for (unsigned int z = 0; z < 2; ++z) {
				const auto pt = inv * MATHGL::Vector4(2.0f * x - 1.0f, 2.0f * y - 1.0f, 2.0f * z - 1.0f, 1.0f);
				frustumCorners.push_back(pt / pt.w);
			}
		}
	}
	return frustumCorners;
}


std::vector<MATHGL::Vector4> getFrustumCornersWorldSpace(const MATHGL::Matrix4& proj, const MATHGL::Matrix4& view) {
	return getFrustumCornersWorldSpace(proj * view);
}

MATHGL::Matrix4 GameRendererGl::getLightSpaceMatrix(float nearPlane, float farPlane, const MATHGL::Vector3& lightDir, const MATHGL::Vector3& lightPos) {
	auto [winWidth, winHeight] = mContext.window->getSize();
	const auto proj = MATHGL::Matrix4::CreatePerspective(
		45.0f, (float)winWidth / (float)winHeight, nearPlane,
		farPlane);
	const auto corners = getFrustumCornersWorldSpace(proj, mainCameraComponent.value()->getCamera().getViewMatrix());

	auto center = lightPos;// MATHGL::Vector3(0, 0, 0);
	for (const auto& v : corners) {
		center += MATHGL::Vector3(v.x, v.y, v.z);
	}
	center /= corners.size();

	const auto lightView = MATHGL::Matrix4::CreateView(center, lightDir /*MATHGL::Vector3(20, 50, 20)*/, MATHGL::Vector3(0.0f, 1.0f, 0.0f));

	float minX = std::numeric_limits<float>::max();
	float maxX = std::numeric_limits<float>::lowest();
	float minY = std::numeric_limits<float>::max();
	float maxY = std::numeric_limits<float>::lowest();
	float minZ = std::numeric_limits<float>::max();
	float maxZ = std::numeric_limits<float>::lowest();
	for (const auto& v : corners) {
		const auto trf = lightView * v;
		minX = std::min(minX, trf.x);
		maxX = std::max(maxX, trf.x);
		minY = std::min(minY, trf.y);
		maxY = std::max(maxY, trf.y);
		minZ = std::min(minZ, trf.z);
		maxZ = std::max(maxZ, trf.z);
	}

	// Tune this parameter according to the scene
	constexpr float zMult = 10.0f;
	if (minZ < 0) {
		minZ *= zMult;
	}
	else {
		minZ /= zMult;
	}
	if (maxZ < 0) {
		maxZ /= zMult;
	}
	else {
		maxZ *= zMult;
	}

	const auto lightProjection = MATHGL::Matrix4::CreateOrthographic(minX, maxX, minY, maxY, minZ, maxZ);
	return lightProjection * lightView;
}

GameRendererGl::EngineDirShadowUBO GameRendererGl::getLightSpaceMatrices(const MATHGL::Vector3& lightDir, const MATHGL::Vector3& lightPos) {
	EngineDirShadowUBO ret;
	for (size_t i = 0; i < mPipeline.mDirShadowMap.mShadowCascadeLevels.size() + 1; ++i) {
		if (i == 0) {
			ret.lightSpaceMatrices[i] = (getLightSpaceMatrix(mPipeline.mDirShadowMap.mDirNearPlane, 
				mPipeline.mDirShadowMap.mShadowCascadeLevels[i], lightDir, lightPos));
		}
		else if (i < mPipeline.mDirShadowMap.mShadowCascadeLevels.size()) {
			ret.lightSpaceMatrices[i] = (getLightSpaceMatrix(mPipeline.mDirShadowMap.mShadowCascadeLevels[i - 1], 
				mPipeline.mDirShadowMap.mShadowCascadeLevels[i], lightDir, lightPos));
		}
		else {
			ret.lightSpaceMatrices[i] = (getLightSpaceMatrix(mPipeline.mDirShadowMap.mShadowCascadeLevels[i - 1], 
				mPipeline.mDirShadowMap.mDirFarPlane, lightDir, lightPos));
		}
	}

	for (auto& e : ret.lightSpaceMatrices) {
		e = MATHGL::Matrix4::Transpose(e);
	}

	return ret;
}

bool GameRendererGl::prepareDirShadowMap(const std::string& id) {
	for (auto& light : ECS::ComponentManager::getInstance()->getComponentArrayRef<ECS::DirectionalLight>()) {
		auto lightDir = light.obj->getTransform()->getWorldForward();
		auto lightPos = light.obj->getTransform()->getWorldPosition();
		//const auto lightMat = getLightSpaceMatrices(lightDir, lightPos);
		//int i = 0;
		//for (auto& e : mEngineShadowData.dirMatrices) {
		//	e = lightMat.lightSpaceMatrices[i];
		//	i++;
		//}
		mEngineShadowData.dirLightDir = lightDir;
		mEngineShadowData.dirLightPos = lightPos;
		//mEngineDirShadowUBO->set(lightMat);
		glEnable(GL_DEPTH_TEST);
		mDriver->setViewport(*mShaders["dirShadow"], 0, 0, mPipeline.mDirShadowMap.mDirShadowMapResolution/4, mPipeline.mDirShadowMap.mDirShadowMapResolution/4);
		mFramebuffers[id]->bind();
		mShaders[id]->bind();

		//set uniform
		float nearPlane = 1.0f, farPlane = 100.0f;
		auto lightProjection = MATHGL::Matrix4::CreateOrthographic(-50, 50, -50, 50, nearPlane, farPlane);
		//std::cout << light.obj->getTransform()->getWorldPosition().x << " "
		//	<< light.obj->getTransform()->getWorldPosition().y << " "
		//	<< light.obj->getTransform()->getWorldPosition().z << "\n";
		auto lightView = MATHGL::Matrix4::CreateView(light.obj->getTransform()->getWorldPosition(), MATHGL::Vector3(0.0f, 0.0f, 0.0f), MATHGL::Vector3(0.0, 1.0, 0.0));
		auto lightSpaceMatrix = lightProjection * lightView;
		mPipeline.mDirShadowMap.dirLightSpaceMatrix = lightSpaceMatrix;
		mShaders[id]->setMat4("LightSpaceMatrix", lightSpaceMatrix);

		//i = 0;
		//for (auto& e : mEngineShadowData.dirMatrices) {
		//	mShaders[id]->setMat4("engine_DirShadowUBO.lightSpaceMatrices[" + std::to_string(i) + "]", lightMat.lightSpaceMatrices[i]);
		//	i++;
		//}
		mDriver->clear(false, true, false);

		//glCullFace(GL_FRONT);

		bool isDrawSmth = false;
		for (const auto& [distance, drawable] : mOpaqueMeshesForward) {
			if (drawable.material->isCastShadow()) {
				isDrawSmth = true;
				mShaders[id]->setMat4("engine_Model.model", drawable.world);
				drawDrawableWithShader(mShaders[id], drawable);
			}
		}
		for (const auto& [distance, drawable] : mOpaqueMeshesDeferred) {
			if (drawable.material->isCastShadow()) {
				isDrawSmth = true;
				mShaders[id]->setMat4("engine_Model.model", drawable.world);
				drawDrawableWithShader(mShaders[id], drawable);
			}
		}
		//for (const auto& [distance, drawable] : mTransparentMeshesForward) {
		//	if (drawable.material->isCastShadow())
		//		drawDrawableWithShader(mShaders[id], drawable);
		//}
		//for (const auto& [distance, drawable] : mTransparentMeshesDeferred) {
		//	if (drawable.material->isCastShadow())
		//		drawDrawableWithShader(mShaders[id], drawable);
		//}

		//glCullFace(GL_BACK);
		mShaders[id]->unbind();
		mFramebuffers[id]->unbind();
		auto [winWidth, winHeight] = mContext.window->getSize();
		mDriver->setViewport(*mShaders[id], 0, 0, winWidth, winHeight);
		return isDrawSmth;
	}
}

void GameRendererGl::prepareSpotShadow() {
	//TODO: create 4 fb
	for (auto& light : ECS::ComponentManager::getInstance()->getComponentArrayRef<ECS::SpotLight>()) {
		auto lightPos = light.obj->getTransform()->getWorldPosition();

		float near_plane = mPipeline.mDirShadowMap.mSpotNearPlane;
		float far_plane = mPipeline.mDirShadowMap.mSpotFarPlane;
		auto lightProjection = MATHGL::Matrix4::CreatePerspective(45.0f, (GLfloat)mPipeline.mDirShadowMap.mSpotShadowMapResolution /
			(GLfloat)mPipeline.mDirShadowMap.mSpotShadowMapResolution, near_plane, far_plane);
		auto lightView = MATHGL::Matrix4::CreateView(lightPos, MATHGL::Vector3(0.0f), MATHGL::Vector3(0.0, 1.0, 0.0));
		auto lightSpaceMatrix = lightProjection * lightView;
		EngineSpotShadowUBO ubo;
		ubo.LightSpaceMatrix = MATHGL::Matrix4::Transpose(lightSpaceMatrix);
		mEngineSpotShadowUBO->set(ubo);

		mFramebuffers["spotShadow"]->bind();
		mShaders["spotShadow"]->bind();

		mDriver->setViewport(*mShaders["spotShadow"], 0, 0, mPipeline.mDirShadowMap.mSpotShadowMapResolution, mPipeline.mDirShadowMap.mSpotShadowMapResolution);
		mDriver->clear(false, true, false);
		glCullFace(GL_FRONT);

		bool isDrawSmth = false;
		for (const auto& [distance, drawable] : mOpaqueMeshesForward) {
			if (drawable.material->isCastShadow()) {
				isDrawSmth = true;
				drawDrawableWithShader(mShaders["spotShadow"], drawable);
			}
		}
		for (const auto& [distance, drawable] : mOpaqueMeshesDeferred) {
			if (drawable.material->isCastShadow()) {
				isDrawSmth = true;
				drawDrawableWithShader(mShaders["spotShadow"], drawable);
			}
		}
		//for (const auto& [distance, drawable] : mTransparentMeshesForward) {
		//	if (drawable.material->isCastShadow())
		//		drawDrawableWithShader(mShaders["spotShadow"], drawable);
		//}
		//for (const auto& [distance, drawable] : mTransparentMeshesDeferred) {
		//	if (drawable.material->isCastShadow())
		//		drawDrawableWithShader(mShaders["spotShadow"], drawable);
		//}

		glCullFace(GL_BACK);
		mShaders["spotShadow"]->unbind();
		mFramebuffers["spotShadow"]->unbind();
	}

	auto [winWidth, winHeight] = mContext.window->getSize();
	mDriver->setViewport(*mShaders["spotShadow"], 0, 0, winWidth, winHeight);
}

void GameRendererGl::preparePointShadow() {
	//TODO: create 4 fb
	for (auto& light : ECS::ComponentManager::getInstance()->getComponentArrayRef<ECS::PointLight>()) {
		auto lightPos = light.obj->getTransform()->getWorldPosition();

		float near_plane = mPipeline.mDirShadowMap.mPointNearPlane;
		float far_plane = mPipeline.mDirShadowMap.mPointFarPlane;

		auto shadowProj = MATHGL::Matrix4::CreatePerspective(90.0f, (float)mPipeline.mDirShadowMap.mPointShadowMapResolution /
			(float)mPipeline.mDirShadowMap.mPointShadowMapResolution, near_plane, far_plane);
		std::vector<MATHGL::Matrix4> shadowTransforms;
		shadowTransforms.push_back(shadowProj * MATHGL::Matrix4::CreateView(lightPos, lightPos + MATHGL::Vector3(1.0f, 0.0f, 0.0f), MATHGL::Vector3(0.0f, -1.0f, 0.0f)));
		shadowTransforms.push_back(shadowProj * MATHGL::Matrix4::CreateView(lightPos, lightPos + MATHGL::Vector3(-1.0f, 0.0f, 0.0f), MATHGL::Vector3(0.0f, -1.0f, 0.0f)));
		shadowTransforms.push_back(shadowProj * MATHGL::Matrix4::CreateView(lightPos, lightPos + MATHGL::Vector3(0.0f, 1.0f, 0.0f), MATHGL::Vector3(0.0f, 0.0f, 1.0f)));
		shadowTransforms.push_back(shadowProj * MATHGL::Matrix4::CreateView(lightPos, lightPos + MATHGL::Vector3(0.0f, -1.0f, 0.0f), MATHGL::Vector3(0.0f, 0.0f, -1.0f)));
		shadowTransforms.push_back(shadowProj * MATHGL::Matrix4::CreateView(lightPos, lightPos + MATHGL::Vector3(0.0f, 0.0f, 1.0f), MATHGL::Vector3(0.0f, -1.0f, 0.0f)));
		shadowTransforms.push_back(shadowProj * MATHGL::Matrix4::CreateView(lightPos, lightPos + MATHGL::Vector3(0.0f, 0.0f, -1.0f), MATHGL::Vector3(0.0f, -1.0f, 0.0f)));

		EnginePointShadowUBO ubo;
		ubo.LightPos = lightPos;
		ubo.FarPlane = far_plane;
		for (unsigned int i = 0; i < 6; ++i) {
			ubo.Matricies[i] = MATHGL::Matrix4::Transpose(shadowTransforms[i]);
		}
		mEnginePointShadowUBO->set(ubo);
		// 1. render scene to depth cubemap
		// --------------------------------
		mDriver->setViewPort(0, 0, mPipeline.mDirShadowMap.mPointShadowMapResolution, mPipeline.mDirShadowMap.mPointShadowMapResolution);
		mFramebuffers["pointShadowMap"]->bind();
		mDriver->clear(false, true, false);
		mShaders["pointShadow"]->bind();

		bool isDrawSmth = false;
		for (const auto& [distance, drawable] : mOpaqueMeshesForward) {
			if (drawable.material->isCastShadow()) {
				isDrawSmth = true;
				drawDrawableWithShader(mShaders["pointShadow"], drawable);
			}
		}
		for (const auto& [distance, drawable] : mOpaqueMeshesDeferred) {
			if (drawable.material->isCastShadow()) {
				isDrawSmth = true;
				drawDrawableWithShader(mShaders["pointShadow"], drawable);
			}
		}

		mShaders["pointShadow"]->unbind();
		mFramebuffers["pointShadowMap"]->unbind();
	}

	auto [winWidth, winHeight] = mContext.window->getSize();
	mDriver->setViewport(*mShaders["spotShadow"], 0, 0, winWidth, winHeight);
}

unsigned int lightFBO;
unsigned int lightDepthMaps;
float cameraNearPlane = 0.1f;
float cameraFarPlane = 500.0f;
std::vector<float> shadowCascadeLevels{ cameraFarPlane / 50.0f, cameraFarPlane / 25.0f, cameraFarPlane / 10.0f, cameraFarPlane / 2.0f };


void GameRendererGl::renderScene(KUMA::Ref<KUMA::ECS::CameraComponent> mainCameraComponent) {
	auto& currentScene = mContext.sceneManager->getCurrentScene();
	
	auto& camera = mainCameraComponent->getCamera();
	if (mainCameraComponent->isFrustumLightCulling()) {
		updateLightsInFrustum(currentScene, mainCameraComponent->getCamera().getFrustum());
	}
	else {
		updateLights(currentScene);
	}

	const auto& cameraPosition = mainCameraComponent->obj->getTransform()->getWorldPosition();
	std::tie(mOpaqueMeshesForward, mTransparentMeshesForward, mOpaqueMeshesDeferred, mTransparentMeshesDeferred) =
		currentScene.findDrawables(cameraPosition, camera, nullptr, nullptr);

	mDriver->setClearColor(mPipeline.mClearColor.x, mPipeline.mClearColor.y, mPipeline.mClearColor.z, mPipeline.mClearColor.w);
	mDriver->clear(true, true, false);

	if (!mPipeline.mDirShadowMap.mIsShadowBakedInit) {
		//mPipeline.mDirShadowMap.mUseShadowBaked = prepareDirShadowMap("dirShadowMapBaked");
		mPipeline.mDirShadowMap.mIsShadowBakedInit = true;
	}
	mPipeline.mDirShadowMap.mUseShadowBaked = prepareDirShadowMap("spotShadowMap");
	//prepareSpotShadow();
	//preparePointShadow();
	pingPong = false;

	{
		//save pos tex
		TextureGl::CopyTexture(*gPositionGlobalTex, *gPrevPositionTex);
		TextureGl::CopyTexture(*mTextures["deferredTextureSave"], *mPrevDeferredTexture);

		//GLOBAL BUFFER PASS
		gbufferGlobalFb->bind();
		mDriver->clear(true, true, false);
		mShaders["GBuffer"]->bind();
		mShaders["GBuffer"]->setMat4("engine_TempData.previousView", MATHGL::Matrix4::Transpose(PrevView));
		for (const auto& [distance, drawable] : mOpaqueMeshesDeferred) {
			drawDrawableWithShader(mShaders["GBuffer"], drawable);
		}
		for (const auto& [distance, drawable] : mTransparentMeshesDeferred) {
			drawDrawableWithShader(mShaders["GBuffer"], drawable);
		}
		for (const auto& [distance, drawable] : mOpaqueMeshesForward) {
			drawDrawableWithShader(mShaders["GBuffer"], drawable);
		}
		for (const auto& [distance, drawable] : mTransparentMeshesForward) {
			drawDrawableWithShader(mShaders["GBuffer"], drawable);
		}
		mShaders["GBuffer"]->unbind();
		gbufferGlobalFb->unbind();
	}



	drawDeferredGBuffer();

	applySSR();
	//applySSGI();
	//applySSS();

	TextureGl::CopyTexture(*pingPongTex[!pingPong], *mDeferredTexture);
	auto [winWidth, winHeight] = mContext.window->getSize();
	FrameBufferGl::CopyDepth(*gbufferFb, *mDeferredFb, winWidth, winHeight);
	
	drawForward();
	
	renderSkybox();

	pingPong = false;
	TextureGl::CopyTexture(*mDeferredTexture, *pingPongTex[1]);
	TextureGl::CopyTexture(*mDeferredTexture, *mTextures["deferredTextureSave"]);

	//applyTAA();
	//applyMotionBlur();

	prepareBrightTexture();
	auto horizontal = prepareBlurTexture(mTextures["deferredTextureSave"]);
	TextureGl::CopyTexture(*pingPongBlurTex[!horizontal], *mTextures["blur"]);
	prepareGodRaysTexture();

	applyBloom();

	//applyGoodRays();
	//applyFXAA();
	//applyHDR();
	//applyColorGrading();
	//applyVignette();
	//applyDepthOfField();
	//applyOutline();
	//applyChromaticAbberation();
	//applyPosterize();
	//applyPixelize();
	//applySharpen();
	//applyDilation();
	//applyFilmGrain();

	{//render to screen
		//mShaders["renderToScreen"]->bind();
		////mTextures["spotShadowMap"]->bind(0);
		////mTextures["ssaoBlur"]->bind(0);
		////mRenderToScreenTexture->bind(0);
		////mDeferredTexture->bind(0);
		//pingPongTex[!pingPong]->bind(0);
		//renderQuad();
		//mShaders["renderToScreen"]->unbind();
	}
	//drawGUI();
}

void GameRendererGl::sendIBLData() {
	mDeferredShader->bind();
	mDeferredShader->setInt("u_engine_irradianceMap", 14);
	mDeferredShader->setInt("u_engine_prefilterMap", 15);
	mDeferredShader->setInt("u_engine_brdfLUT", 16);
	mDeferredShader->setInt("u_UseIbl", mPipeline.ibl.useIBL);

	mTextures["irradianceMap"]->bind(14);
	mTextures["prefilterMap"]->bind(15);
	mTextures["brdfLUTTexture"]->bind(16);
	mDeferredShader->unbind();
}

void GameRendererGl::prepareBrightTexture() {
	mFramebuffers["brightTexture"]->bind();
	mDriver->clear(true, true, false);
	mShaders["brightTexture"]->bind();
	mTextures["deferredTextureSave"]->bind(0);
	renderQuad();
	mShaders["brightTexture"]->unbind();
	mFramebuffers["brightTexture"]->unbind();
}

bool GameRendererGl::prepareBlurTexture(std::shared_ptr<TextureGl> tex) {
	bool horizontal = true;
	bool firstIteration = true;
	unsigned int amount = 10;
	mShaders["blurTexture"]->bind();
	for (unsigned int i = 0; i < amount; i++) {
		pingPongBlurFb[horizontal]->bind();
		mDriver->clear(true, true, false);
		mShaders["blurTexture"]->setInt("horizontal", horizontal);
		firstIteration ? tex->bind(0) : pingPongBlurTex[!horizontal]->bind(0);
		renderQuad();
		if (firstIteration) {
			firstIteration = false;
		}
		pingPongBlurFb[horizontal]->unbind();
		horizontal = !horizontal;
	}
	mShaders["blurTexture"]->unbind();
	//mTextures["blur"] = pingPongBlurTex[!horizontal];
	return horizontal;
}

void GameRendererGl::prepareGodRaysTexture() {
	mFramebuffers["godRaysTexture"]->bind();
	mDriver->clear(true, true, false);
	mShaders["godRaysTexture"]->bind();
	mShaders["godRaysTexture"]->setVec3("u_Color", MATHGL::Vector3(0.0f, 0.0f, 0.0f));

	for (const auto& [distance, drawable] : mOpaqueMeshesForward) {
		drawDrawableWithShader(mShaders["godRaysTexture"], drawable);
	}
	for (const auto& [distance, drawable] : mOpaqueMeshesDeferred) {
		drawDrawableWithShader(mShaders["godRaysTexture"], drawable);
	}
	mShaders["godRaysTexture"]->setVec3("u_Color", MATHGL::Vector3(1.0f, 1.0f, 1.0f));
	for (auto& light : ECS::ComponentManager::getInstance()->getComponentArrayRef<ECS::DirectionalLight>()) {
		Drawable d;
		d.mesh = sphere->getMeshes()[0];
		d.material = emptyMaterial;
		d.world = light.obj->getTransform()->getTransform().getWorldMatrix();
		drawDrawableWithShader(mShaders["godRaysTexture"], d);
	}
	mFramebuffers["godRaysTexture"]->unbind();
}

void GameRendererGl::applyGoodRays() {
	auto& dirLights = ECS::ComponentManager::getInstance()->getComponentArrayRef<ECS::DirectionalLight>();
	if (dirLights.getSize() > 0) {
		pingPongFb[pingPong]->bind();
		mDriver->clear(true, true, false);
		mShaders["godRays"]->bind();
		pingPongTex[!pingPong]->bind(0);
		mTextures["godRaysTexture"]->bind(1);
		mShaders["godRays"]->setInt("u_UseGodRays", true);
		mShaders["godRays"]->setVec3("u_SunPos", dirLights.begin()->obj->getTransform()->getWorldPosition());
		renderQuad();
		mShaders["godRays"]->unbind();

		pingPongFb[pingPong]->unbind();
		pingPong = !pingPong;
	}
}

void GameRendererGl::applyBloom() {
	auto horizontal = prepareBlurTexture(mTextures["brightTexture"]);
	pingPongFb[pingPong]->bind();
	mDriver->clear(true, true, false);
	mShaders["bloom"]->bind();
	pingPongTex[!pingPong]->bind(0);
	pingPongBlurTex[!horizontal]->bind(1);
	mShaders["bloom"]->setInt("u_UseBloom", true);
	renderQuad();
	mShaders["bloom"]->unbind();
	pingPongFb[pingPong]->unbind();
	pingPong = !pingPong;
}

void GameRendererGl::applyFXAA() {
	pingPongFb[pingPong]->bind();
	mDriver->clear(true, true, false);
	mShaders["fxaa"]->bind();
	pingPongTex[!pingPong]->bind(0);
	mShaders["fxaa"]->setInt("u_UseFXAA", true);
	renderQuad();
	mShaders["fxaa"]->unbind();
	pingPongFb[pingPong]->unbind();
	pingPong = !pingPong;
}

void GameRendererGl::applyHDR() {
	pingPongFb[pingPong]->bind();
	mDriver->clear(true, true, false);
	mShaders["hdr"]->bind();
	pingPongTex[!pingPong]->bind(0);
	

	mShaders["hdr"]->setInt("u_UseHDR", mPipeline.hdr.isEnabled);
	mShaders["hdr"]->setFloat("u_Exposure", mPipeline.hdr.exposure);
	mShaders["hdr"]->setFloat("u_Gamma", mPipeline.hdr.gamma);

	renderQuad();
	mShaders["hdr"]->unbind();

	pingPongFb[pingPong]->unbind();
	pingPong = !pingPong;
}

void GameRendererGl::applyColorGrading() {
	pingPongFb[pingPong]->bind();
	mDriver->clear(true, true, false);
	mShaders["colorGrading"]->bind();
	pingPongTex[!pingPong]->bind(0);

	mShaders["colorGrading"]->setVec3("u_ChannelR", mPipeline.colorGrading.r);
	mShaders["colorGrading"]->setVec3("u_ChannelG", mPipeline.colorGrading.g);
	mShaders["colorGrading"]->setVec3("u_ChannelB", mPipeline.colorGrading.b);
	renderQuad();
	mShaders["colorGrading"]->unbind();

	pingPongFb[pingPong]->unbind();
	pingPong = !pingPong;
}

void GameRendererGl::applyVignette() {
	pingPongFb[pingPong]->bind();
	mDriver->clear(true, true, false);
	mShaders["vignette"]->bind();
	pingPongTex[!pingPong]->bind(0);

	mShaders["vignette"]->setFloat("u_Radius", mPipeline.vignette.radius);
	mShaders["vignette"]->setFloat("u_Intensity", mPipeline.vignette.intensity);
	renderQuad();
	mShaders["vignette"]->unbind();

	pingPongFb[pingPong]->unbind();
	pingPong = !pingPong;
}

void GameRendererGl::applyDepthOfField() {
	pingPongFb[pingPong]->bind();
	mDriver->clear(true, true, false);
	mShaders["depthOfField"]->bind();
	pingPongTex[!pingPong]->bind(0);

	gPositionGlobalTex->bind(1);
	mTextures["noiseTexture"]->bind(2);
	mTextures["blur"]->bind(3);
	mShaders["depthOfField"]->setVec2("nearFar", MATHGL::Vector2{ mainCameraComponent.value()->getNear(), mainCameraComponent.value()->getFar() });
	mShaders["depthOfField"]->setVec3("focusPoint", MATHGL::Vector3(400.0f, 300.0f, 0.0f));
	renderQuad();
	mShaders["depthOfField"]->unbind();

	pingPongFb[pingPong]->unbind();
	pingPong = !pingPong;
}

void GameRendererGl::applyOutline() {
	pingPongFb[pingPong]->bind();
	mDriver->clear(true, true, false);
	mShaders["outline"]->bind();
	pingPongTex[!pingPong]->bind(0);

	gPositionGlobalTex->bind(1);
	mTextures["noiseTexture"]->bind(2);
	mShaders["outline"]->setVec2("nearFar", MATHGL::Vector2{ mainCameraComponent.value()->getNear(), mainCameraComponent.value()->getFar() });
	mShaders["outline"]->setVec2("gamma", MATHGL::Vector2{ 2.2f, 1.0f / 2.2f });
	renderQuad();
	mShaders["outline"]->unbind();

	pingPongFb[pingPong]->unbind();
	pingPong = !pingPong;
}

void GameRendererGl::applyChromaticAbberation() {
	pingPongFb[pingPong]->bind();
	mDriver->clear(true, true, false);
	mShaders["chromaticAbberation"]->bind();
	pingPongTex[!pingPong]->bind(0);
	mShaders["chromaticAbberation"]->setVec3("u_ChromaticAbberationParams", mPipeline.chromaticAbberation.params);
	renderQuad();
	mShaders["chromaticAbberation"]->unbind();

	pingPongFb[pingPong]->unbind();
	pingPong = !pingPong;
}

void GameRendererGl::applyPosterize() {
	pingPongFb[pingPong]->bind();

	mShaders["posterize"]->bind();
	pingPongTex[!pingPong]->bind(0);
	mDriver->clear(true, true, false);
	gPositionGlobalTex->bind(1);
	renderQuad();
	mShaders["posterize"]->unbind();

	//set to default
	pingPongFb[pingPong]->unbind();
	pingPong = !pingPong;
}

void GameRendererGl::applyPixelize() {
	pingPongFb[pingPong]->bind();
	mDriver->clear(true, true, false);

	mShaders["pixelize"]->bind();
	pingPongTex[!pingPong]->bind(0);
	gPositionGlobalTex->bind(1);
	mShaders["pixelize"]->setVec2("parameters", MATHGL::Vector2{ 5.0f, 0.0f });
	renderQuad();
	mShaders["pixelize"]->unbind();

	//set to default
	pingPongFb[pingPong]->unbind();
	pingPong = !pingPong;
}

void GameRendererGl::applySharpen() {
	pingPongFb[pingPong]->bind();

	mShaders["sharpen"]->bind();
	pingPongTex[!pingPong]->bind(0);
	mDriver->clear(true, true, false);
	renderQuad();
	mShaders["sharpen"]->unbind();

	//set to default
	pingPongFb[pingPong]->unbind();
	pingPong = !pingPong;
}

void GameRendererGl::applyDilation() {
	pingPongFb[pingPong]->bind();

	mShaders["dilation"]->bind();
	pingPongTex[!pingPong]->bind(0);
	mDriver->clear(true, true, false);
	mShaders["dilation"]->setVec2("parameters", MATHGL::Vector2{ 4.0f, 2.0f });
	renderQuad();
	mShaders["dilation"]->unbind();

	//set to default
	pingPongFb[pingPong]->unbind();
	pingPong = !pingPong;
}


void GameRendererGl::applyFilmGrain() {
	pingPongFb[pingPong]->bind();

	mShaders["filmGrain"]->bind();
	pingPongTex[!pingPong]->bind(0);
	mDriver->clear(true, true, false);
	renderQuad();
	mShaders["filmGrain"]->unbind();

	//set to default
	pingPongFb[pingPong]->unbind();
	pingPong = !pingPong;
}

void GameRendererGl::sendSSAOData() {
	//mDeferredShader->bind();
	mDeferredShader->setInt("engine_SSAOMap", 13);
	mDeferredShader->setInt("engine_UseSSAO", mPipeline.mSSAO.mUseSSAO);
	mTextures["ssaoBlur"]->bind(13);
	//mDeferredShader->unbind();
}

void GameRendererGl::sendBounseDataToShader(std::shared_ptr<MaterialGl> material, ECS::Skeletal& animator, std::shared_ptr<ShaderGl> shader) {
	//shader.bind();
	bool useBones = false;
	if (material->mUniformData.count("u_UseBone")) {
		useBones = std::get<bool>(material->mUniformData["u_UseBone"]);
		shader->setInt("u_UseBone", useBones);
	}

	if (useBones) {
		auto transforms = animator.animator->GetFinalBoneMatrices();
		for (int i = 0; i < transforms.size(); ++i) {
			shader->setMat4("u_engine_FinalBonesMatrices[" + std::to_string(i) + "]", transforms[i]);
		}
	}
	//shader.unbind();
}

#include "../gui/components/spriteComponent.h"
#include "../gui/guiObject.h"
#include "../inputManager/inputManager.h"

void GameRendererGl::drawGUISubtree(Ref<ECS::Object> obj) {
	static bool initLblBfrs = false;
	static unsigned VAO = 0;
	static unsigned VBO = 0;
	if (!initLblBfrs) {
		initLblBfrs = true;
		glGenVertexArrays(1, &VAO);
		glGenBuffers(1, &VBO);
		glBindVertexArray(VAO);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
	}

	auto transform = obj->getComponent<ECS::TransformComponent>().value();
	MATHGL::Matrix4 projection = MATHGL::Matrix4::CreateOrthographic(0.0f, static_cast<float>(800), static_cast<float>(600), 0.0f, -1, 1);
	auto [winWidth, winHeight] = mContext.window->getSize();
	if (auto component = obj->getComponent<ECS::SpriteComponent>()) {
		auto _component = component.value();
		glDisable(GL_DEPTH_TEST);
		glEnable(GL_CULL_FACE);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		mShaders["sprite"]->bind();
		static_cast<TextureGl*>(_component->mTexture.get())->bind(0);
		mShaders["sprite"]->setVec4("spriteColor", _component->mColor);
		mShaders["sprite"]->setMat4("u_engine_model", 
			transform->getWorldMatrix() * 
			MATHGL::Matrix4::Scaling(MATHGL::Vector3(transform->getTransform().getLocalSize().x, transform->getTransform().getLocalSize().y, 1.0f)));
		mShaders["sprite"]->setMat4("u_engine_projection", projection);

		renderQuadGUI();
		mShaders["sprite"]->unbind();
	}

	if (auto component = obj->getComponent<ECS::LabelComponent>()) {
		auto _component = component.value();
		glDisable(GL_DEPTH_TEST);
		glEnable(GL_CULL_FACE);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		
		mShaders["label"]->bind();
		mShaders["label"]->setVec3("textColor", { _component->color.x, _component->color.y, _component->color.z});
		mShaders["label"]->setMat4("u_engine_projection", projection);
		//_component->font->texture->bind(0);
		glBindTexture(GL_TEXTURE_2D, _component->font->texture);

		std::string::const_iterator c;
		mShaders["label"]->setMat4("u_engine_model", transform->getWorldMatrix());

		glBindVertexArray(VAO);

		auto model = transform->getWorldMatrix();
		float x = 0.0f;//model(0, 3);
		float y = 0.0f;//model(1, 3);
		for (auto c : _component->mLabel) {
			auto scaleX = transform->getWorldScale().x; /// transform->getTransform().getLocalSize().x;
			auto scaleY = transform->getWorldScale().y; /// transform->getTransform().getLocalSize().y;
			GUI::Character ch = _component->font->Characters[c];
		
			float xpos = x + ch.Bearing.x * scaleX;
			float ypos = y - (ch.Size.y - ch.Bearing.y) * scaleY;
		
			float w = ch.Size.x * scaleX;
			float h = ch.Size.y * scaleY;

			float vertices[6][4] = {
				{ xpos,     ypos + h, ch.Start.x / 1024.0,               (ch.Start.y + ch.Size.y) / 1024.0 },
				{ xpos + w, ypos,     (ch.Start.x + ch.Size.x) / 1024.0,  ch.Start.y / 1024.0},
				{ xpos,     ypos,     ch.Start.x / 1024.0,               ch.Start.y / 1024.0 },
				{ xpos,     ypos + h, ch.Start.x / 1024.0,               (ch.Start.y + ch.Size.y) / 1024.0  },
				{ xpos + w, ypos + h, (ch.Start.x + ch.Size.x) / 1024.0, (ch.Start.y + ch.Size.y) / 1024.0 },
				{ xpos + w, ypos,     (ch.Start.x + ch.Size.x) / 1024.0, ch.Start.y / 1024.0 }
			};

			glBindBuffer(GL_ARRAY_BUFFER, VBO);
			glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
			glBindBuffer(GL_ARRAY_BUFFER, 0);
			glDrawArrays(GL_TRIANGLES, 0, 6);
			//renderQuadGUI();
		
			x += (ch.Advance >> 6) * scaleX;
		}
		glBindVertexArray(0);
		mShaders["label"]->unbind();
	}
	if (auto component = obj->getComponent<ECS::LayoutComponent>()) {
		auto _component = component.value();

		auto model = transform->getWorldMatrix();

		float width = 0.0f;
		float height = 0.0f;
		auto startX = _component->mHorizontalOffset;
		auto startY = _component->mVerticalOffset;
		for (auto& child : obj->getChildren()) {
			auto& childTransform = child->getTransform()->getTransform();
			if (_component->mType == ECS::LayoutComponent::Type::HORIZONTAL) {
				child->getTransform()->setLocalPosition(
				{
					 startX,
					 startY,
					 child->getTransform()->getLocalPosition().z
					}
				);
				startX += _component->mHorizontalOffset + childTransform.getLocalSize().x * childTransform.getWorldScale().x;
				width += childTransform.getLocalSize().x * childTransform.getWorldScale().x + _component->mHorizontalOffset;
				height = std::max<float>(height, childTransform.getLocalSize().y * childTransform.getWorldScale().y);
			}
			else {
				child->getTransform()->setLocalPosition(
					{
						 startX,
						 startY,
						 child->getTransform()->getLocalPosition().z
					}
				);
				startY += _component->mVerticalOffset + childTransform.getLocalSize().y * childTransform.getWorldScale().y;
				height += childTransform.getLocalSize().y * childTransform.getWorldScale().y + _component->mVerticalOffset;
				width = std::max<float>(width, childTransform.getLocalSize().x * childTransform.getWorldScale().x);
			}
		}
		transform->getTransform().setLocalSize(MATHGL::Vector2{ width, height });
	}
	if (auto component = obj->getComponent<ECS::ClipComponent>()) {
		auto _component = component.value();
		glEnable(GL_SCISSOR_TEST);
		glScissor(
			transform->getWorldPosition().x, 
			winHeight - transform->getWorldPosition().y - _component->mHeight,
			_component->mWidth * transform->getWorldScale().x, 
			_component->mHeight * transform->getWorldScale().y);

		for (auto child : obj->getChildren()) {
			drawGUISubtree(*child);
		}

		glScissor(0, 0, winWidth, winHeight);
		glDisable(GL_SCISSOR_TEST);
		return;
	}
	if (auto component = obj->getComponent<ECS::InteractionComponent>()) {
		auto _component = component.value();
		auto ev = ECS::GuiEventType::NONE;
		auto mpos = RESOURCES::ServiceManager::Get<INPUT_SYSTEM::InputManager>().getMousePosition();
		if (_component->contains(mpos.x, mpos.y)) {
			if (RESOURCES::ServiceManager::Get<INPUT_SYSTEM::InputManager>().isMouseButtonPressed(INPUT_SYSTEM::EMouseButton::MOUSE_BUTTON_1)) {
				ev = ECS::GuiEventType::PRESS;
			}
			else if (RESOURCES::ServiceManager::Get<INPUT_SYSTEM::InputManager>().isMouseButtonReleased(INPUT_SYSTEM::EMouseButton::MOUSE_BUTTON_1)) {
				ev = ECS::GuiEventType::RELEASE;
			}
			else {
				ev = ECS::GuiEventType::COVER;
			}
		}
		else {
			ev = ECS::GuiEventType::UNCOVER;
		}
		
		if (_component->mCurEvent == ev && (_component->mCurEvent == ECS::GuiEventType::PRESS || _component->mCurEvent == ECS::GuiEventType::PRESS_CONTINUE)) {
			if (_component->mOnPressContinue) _component->mOnPressContinue();
		}
		
		if (_component->mCurEvent != ev) {
			_component->mCurEvent = ev;
			switch (ev) {
			case ECS::GuiEventType::COVER: if (_component->mOnCover) _component->mOnCover();  break;
			case ECS::GuiEventType::PRESS: if (_component->mOnPress) _component->mOnPress();  break;
			case ECS::GuiEventType::RELEASE: if (_component->mOnRelease) _component->mOnRelease();  break;
			case ECS::GuiEventType::UNCOVER: if (_component->mOnUncover) _component->mOnUncover();  break;
			default: break;
			}
		}
	}
	if (auto component = obj->getComponent<ECS::ScrollComponent>()) {
		auto _component = component.value();
		
	}

	for (auto child : obj->getChildren()) {
		drawGUISubtree(*child);
	}
}

void GameRendererGl::drawGUI() {
	for (auto& guiRoot : ECS::ComponentManager::getInstance()->getComponentArrayRef<ECS::RootGuiComponent>()) {
		auto obj = guiRoot.obj;
		drawGUISubtree(obj);
	}
}

void GameRendererGl::drawDeferredGBuffer() {
	//save pos tex
	//TextureGl::CopyTexture(*gPositionTex, *gPrevPositionTex);
	//TextureGl::CopyTexture(*mDeferredTexture, *mPrevDeferredTexture);

	//GBUFFER PASS
	gbufferFb->bind();
	mDriver->clear(true, true, false);
	mShaders["deferredGBuffer"]->bind();
	mShaders["deferredGBuffer"]->setMat4("engine_TempData.previousView", MATHGL::Matrix4::Transpose(PrevView));
	for (const auto& [distance, drawable] : mOpaqueMeshesDeferred) {
		drawDrawableDeferred(drawable);
	}
	for (const auto& [distance, drawable] : mTransparentMeshesDeferred) {
		drawDrawableDeferred(drawable);
	}

	mShaders["deferredGBuffer"]->unbind();
	gbufferFb->unbind();

	//LIGHT PASS
	if (mPipeline.mSSAO.mUseSSAO) {
		applySSAO();
	}
	mDeferredFb->bind();
	mDriver->clear(true, true, false);
	mDeferredShader->bind();

	sendSSAOData();
	if (mPipeline.mIsPBR) {
		sendIBLData();
	}
	//sendShadowDirData(*shaderStorage["deferredLightning"]);
	//sendShadowPointData(*shaderStorage["deferredLightning"]);
	//sendShadowSpotData(*shaderStorage["deferredLightning"]);
	//sendFogData(*shaderStorage["deferredLightning"]);

	gPositionTex->bind(0);
	gNormalTex->bind(1);
	gAlbedoSpecTex->bind(2);
	gRoughAOTex->bind(3);
	
	sendEngineShadowUBO(mDeferredShader);


	if (mOpaqueMeshesDeferred.size() + mTransparentMeshesDeferred.size() > 0) {
		renderQuad();
	}
	
	mDeferredShader->unbind();
	mDeferredFb->unbind();

	TextureGl::CopyTexture(*mDeferredTexture, *pingPongTex[1]);
}

void GameRendererGl::applyMotionBlur() {
	pingPongFb[pingPong]->bind();
	mShaders["motionBlur"]->bind();
	gVelocityGlobalTex->bind(0);
	pingPongTex[!pingPong]->bind(1);
	
	mDriver->clear(true, true, false);
	renderQuad();
	mShaders["motionBlur"]->unbind();
	pingPongFb[pingPong]->unbind();
	pingPong = !pingPong;
}

void GameRendererGl::applyTAA() {
	pingPongFb[pingPong]->bind();
	mShaders["taa"]->bind();
	//gAlbedoSpecTex->bind(0);
	//gNormalTex->bind(0);
	
	pingPongTex[!pingPong]->bind(0);
	gPositionGlobalTex->bind(1);
	mPrevDeferredTexture->bind(2);
	gPrevPositionTex->bind(3);
	gVelocityGlobalTex->bind(4);
	//mHDRSkyBoxTexture->bind(2);
	//gAlbedoSpecTex->bind(2);
	mDriver->clear(true, true, false);
	renderQuad();
	mShaders["taa"]->unbind();
	pingPongFb[pingPong]->unbind();
	pingPong = !pingPong;

	//TODO: use ping pong
	//TextureGl::CopyTexture(*mDeferredTexture, *mTextures["deferredTextureSave"]);

}

void GameRendererGl::drawForward() {
	mDeferredFb->bind();
	for (const auto& [distance, drawable] : mOpaqueMeshesForward) {
		drawDrawable(drawable);
	}
	for (const auto& [distance, drawable] : mTransparentMeshesForward) {
		drawDrawable(drawable);
	}
	mDeferredFb->unbind();

	mRenderToScreenTexture = mDeferredTexture;
}

void GameRendererGl::updateLights(SCENE_SYSTEM::Scene& scene) {
	auto lightMatrices = scene.findLightData();
	mLightSSBO->SendBlocks(lightMatrices.data(), lightMatrices.size() * sizeof(LightOGL));
}

void GameRendererGl::updateLightsInFrustum(SCENE_SYSTEM::Scene& scene, const Frustum& frustum) {
	auto lightMatrices = scene.findLightDataInFrustum(frustum);
	mLightSSBO->SendBlocks(lightMatrices.data(), lightMatrices.size() * sizeof(LightOGL));
}

void GameRendererGl::sendEngineUBO(ShaderGl& shader, const MATHGL::Matrix4& world) {
	shader.setUniform(*mEngineUbo);
	shader.setMat4("engine_Model.model", world);
}

void GameRendererGl::sendEngineShadowData(ShaderGl& shader) {
	//shader.setUniform(*mEngineShadowDataUBO);
}

void GameRendererGl::drawDrawable(const Drawable& p_toDraw) {
	if (p_toDraw.material->hasShader() && p_toDraw.material->getGPUInstances() > 0) {
		uint8_t stateMask = reinterpret_cast<MaterialGl*>(p_toDraw.material.get())->generateStateMask();
		mDriver->applyStateMask(stateMask);

		p_toDraw.material->bind(mEmptyTexture, true);

		sendEngineShadowUBO(std::static_pointer_cast<ShaderGl>(p_toDraw.material->getShader()));
		sendEngineUBO(*reinterpret_cast<ShaderGl*>(p_toDraw.material->getShader().get()), p_toDraw.world);
		if (p_toDraw.animator) {
			sendBounseDataToShader(std::static_pointer_cast<MaterialGl>(p_toDraw.material), 
				p_toDraw.animator.value(), 
				std::static_pointer_cast<ShaderGl>(std::static_pointer_cast<MaterialGl>(p_toDraw.material)->getShader()));
		}
		else {
			mShaders["deferredGBuffer"]->setInt("u_UseBone", false);
		}
		mDriver->draw(*p_toDraw.mesh, PrimitiveMode::TRIANGLES, p_toDraw.material->getGPUInstances());

		p_toDraw.material->unbind();
	}
}

//use external shader
void GameRendererGl::drawMesh(const Drawable& p_toDraw) {
	if (p_toDraw.material->getGPUInstances() > 0) {
		//uint8_t stateMask = reinterpret_cast<MaterialGl*>(p_toDraw.material.get())->generateStateMask();
		//mDriver->applyStateMask(stateMask);
		//p_toDraw.material->bind(mEmptyTexture, true);

		//data from engine
		//sendEngineUBO(*reinterpret_cast<ShaderGl*>(p_toDraw.material->getShader().get()), p_toDraw.world);

		mDriver->draw(*p_toDraw.mesh, PrimitiveMode::TRIANGLES, p_toDraw.material->getGPUInstances());

		//p_toDraw.material->unbind();
	}
}

void GameRendererGl::drawDrawableDeferred(const Drawable& p_toDraw) {
	if (p_toDraw.material->getGPUInstances() > 0) {
		uint8_t stateMask = reinterpret_cast<MaterialGl*>(p_toDraw.material.get())->generateStateMask();
		mDriver->applyStateMask(stateMask);

		reinterpret_cast<MaterialGl*>(p_toDraw.material.get())->fillUniformsWithShader(mShaders["deferredGBuffer"], mEmptyTexture, true);
		sendEngineUBO(*mShaders["deferredGBuffer"], p_toDraw.world);
		if (p_toDraw.animator) {
			sendBounseDataToShader(std::static_pointer_cast<MaterialGl>(p_toDraw.material), p_toDraw.animator.value(), mShaders["deferredGBuffer"]);
		}
		else {
			mShaders["deferredGBuffer"]->setInt("u_UseBone", false);
		}

		mShaders["deferredGBuffer"]->setMat4("engine_TempData.previousModel", p_toDraw.prevWorld);

		mDriver->draw(*p_toDraw.mesh, PrimitiveMode::TRIANGLES, p_toDraw.material->getGPUInstances());
		
	}
}

void GameRendererGl::drawDrawableWithShader(std::shared_ptr<ShaderGl> shader, const Drawable& p_toDraw) {
	if (p_toDraw.material->getGPUInstances() > 0) {
		//uint8_t stateMask = reinterpret_cast<MaterialGl*>(p_toDraw.material.get())->generateStateMask();
		//mDriver->applyStateMask(stateMask);

		//reinterpret_cast<MaterialGl*>(p_toDraw.material.get())->fillUniformsWithShader(mShaders["deferredGBuffer"], mEmptyTexture, true);
		sendEngineUBO(*shader, p_toDraw.world);
		shader->setMat4("engine_TempData.previousModel", p_toDraw.prevWorld);
		if (p_toDraw.animator) {
			sendBounseDataToShader(std::static_pointer_cast<MaterialGl>(p_toDraw.material), p_toDraw.animator.value(), shader);
		}
		else {
			shader->setInt("u_UseBone", false);
		}
		mDriver->draw(*p_toDraw.mesh, PrimitiveMode::TRIANGLES, p_toDraw.material->getGPUInstances());

	}
}

void GameRendererGl::drawDrawableBatching(const Drawable& p_toDraw) {
	//TODO: add to Drawable model and if model!=null => call this and bind VAO in model
}


unsigned int quadVAO = 0;
unsigned int quadVBO;
void renderQuad() {
	if (quadVAO == 0) {
		float quadVertices[] = {
			// координаты      // текстурные координаты
		   -1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
		   -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
			1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
			1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
		};

		// Установка VAO плоскости
		glGenVertexArrays(1, &quadVAO);
		glGenBuffers(1, &quadVBO);
		glBindVertexArray(quadVAO);
		glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	}
	glBindVertexArray(quadVAO);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glBindVertexArray(0);
}

unsigned int quadVAOGUI = 0;
unsigned int quadVBOGUI;
void renderQuadGUI() {
	if (quadVAOGUI == 0) {
		float quadVertices[] = {
			// координаты      // текстурные координаты
			1.0f, 1.0f, 0.0f, 1.0f, 1.0f,
			1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
			0.0f, 1.0f, 0.0f, 0.0f, 1.0f,

			0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
		};

		// Установка VAO плоскости
		glGenVertexArrays(1, &quadVAOGUI);
		glGenBuffers(1, &quadVBOGUI);
		glBindVertexArray(quadVAOGUI);
		glBindBuffer(GL_ARRAY_BUFFER, quadVBOGUI);
		glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	}
	glBindVertexArray(quadVAOGUI);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glBindVertexArray(0);
}


unsigned int cubeVAO = 0;
unsigned int cubeVBO = 0;
void renderCube() {
	// Инициализация (если необходимо)
	if (cubeVAO == 0) {
		float vertices[] = {
			// задняя грань
		   -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // нижняя-левая
			1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // верхняя-правая
			1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 0.0f, // нижняя-правая         
			1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // верхняя-правая
		   -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // нижняя-левая
		   -1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 1.0f, // верхняя-левая

		   // передняя грань
		  -1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // нижняя-левая
		   1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 0.0f, // нижняя-правая
		   1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // верхняя-правая
		   1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // верхняя-правая
		  -1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 1.0f, // верхняя-левая
		  -1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // нижняя-левая

		  // грань слева
		 -1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // верхняя-правая
		 -1.0f,  1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // верхняя-левая
		 -1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // нижняя-левая
		 -1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // нижняя-левая
		 -1.0f, -1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // нижняя-правая
		 -1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // верхняя-правая

		 // грань справа
		 1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // верхняя-левая
		 1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // нижняя-правая
		 1.0f,  1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // верхняя-правая         
		 1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // нижняя-правая
		 1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // верхняя-левая
		 1.0f, -1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // нижняя-левая     

		 // нижняя грань
		-1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // верхняя-правая
		 1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 1.0f, // верхняя-левая
		 1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // нижняя-левая
		 1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // нижняя-левая
		-1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 0.0f, // нижняя-правая
		-1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // верхняя-правая

		// верхняя грань
	   -1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // верхняя-левая
		1.0f,  1.0f , 1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // нижняя-правая
		1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 1.0f, // верхняя-правая     
		1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // нижняя-правая
	   -1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // верхняя-левая
	   -1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 0.0f  // нижняя-левая        
		};
		glGenVertexArrays(1, &cubeVAO);
		glGenBuffers(1, &cubeVBO);

		// Заполняем буфер
		glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

		// Связываем вершинные атрибуты
		glBindVertexArray(cubeVAO);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
	}

	// Рендер куба
	glBindVertexArray(cubeVAO);
	glDrawArrays(GL_TRIANGLES, 0, 36);
	glBindVertexArray(0);
}
#endif
