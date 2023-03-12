#include "gameRendererVk.h"
#ifdef VULKAN_BACKEND

#include "backends/vk/materialVk.h"
#include "backends/vk/frameBufferVk.h"
#include "backends/vk/storageBufferVk.h"
#include "backends/vk/uniformBufferVk.h"
#include "backends/vk/shaderVk.h"
#include "../../../core/core.h"
#include "../../../scene/sceneManager.h"
#include "../../../window/window.h"

using namespace KUMA;
using namespace KUMA::RENDER;

std::optional<KUMA::Ref<KUMA::ECS::CameraComponent>> mainCameraComponentVk = std::nullopt;


GameRendererVk::GameRendererVk(KUMA::CORE_SYSTEM::Core& context) : mContext(context) {

	mEmptyTexture = TextureVk::create(KUMA::UTILS::getRealPath("Textures/snow.png"));

	mDriver = dynamic_cast<DriverVk*>(context.driver.get());

	VkPhysicalDeviceProperties p;
	vkGetPhysicalDeviceProperties(mDriver->m_MainDevice.PhysicalDevice, &p);


	//createVkResources();
}
int NUM_LIGHTS = 20;
void GameRendererVk::createVkResources() {
	auto [winWidth, winHeight] = mContext.window->getSize();

	mFramebuffers["gbufferFb"] = std::make_shared<FrameBufferVk>();
	mTextures["gPositionTex"] = TextureVk::createForAttach(winWidth, winHeight);
	mTextures["gNormalTex"] = TextureVk::createForAttach(winWidth, winHeight);
	mTextures["gAlbedoSpecTex"] = TextureVk::createForAttach(winWidth, winHeight);
	mTextures["gRoughAOTex"] = TextureVk::createForAttach(winWidth, winHeight);
	mFramebuffers["gbufferFb"]->create(std::vector{
		mTextures["gPositionTex"], mTextures["gNormalTex"], mTextures["gAlbedoSpecTex"], mTextures["gRoughAOTex"]
		}, mDriver->m_DepthBufferImage);

	mFramebuffers["deferredFb"] = std::make_shared<FrameBufferVk>();
	mTextures["deferredResult"] = TextureVk::createForAttach(winWidth, winHeight);
	mFramebuffers["deferredFb"]->create(std::vector{
		mTextures["deferredResult"] }, mDriver->m_DepthBufferImage);




	ShaderVk::push_constant = mDriver->m_PushCostantRange;
	ShaderVk::depthWriteEnable = VK_TRUE;
	ShaderVk::cullMode = VK_CULL_MODE_BACK_BIT;
	ShaderVk::frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
	mShaders["deferredRender"] = std::make_shared<ShaderVk>(
		mFramebuffers["gbufferFb"]->m_RenderPass,
		"./Shaders/vk/deferredGBuffer.vert.spv", "./Shaders/vk/deferredGBuffer.frag.spv");
	
	ShaderVk::push_constant = std::nullopt;
	ShaderVk::depthWriteEnable = VK_FALSE;
	ShaderVk::cullMode = VK_CULL_MODE_FRONT_BIT;
	ShaderVk::frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
	mShaders["deferredLightning"] = std::make_shared<ShaderVk>(mFramebuffers["deferredFb"]->m_RenderPass,
		"./Shaders/vk/deferredLightningPbr.vert.spv", "./Shaders/vk/deferredLightningPbr.frag.spv");


	mShaders["renderToScreen"] = std::make_shared<ShaderVk>(mDriver->defaultFb->m_RenderPass,
		"./Shaders/vk/renderToScreen.vert.spv", "./Shaders/vk/renderToScreen.frag.spv");


	// we could send it to different shaders if it set in same bind and set
	mLightSSBO = std::make_shared<ShaderStorageBufferVk<LightOGL>>(mShaders["deferredLightning"], "ssbo_Lights");
	//mLightUBO = std::make_shared<UniformBufferVk<RENDER::LightOGL>>(mShaders["deferredLightning"], "ssbo_Lights", NUM_LIGHTS);
	mEngineUbo = std::make_shared<UniformBufferVk<EngineUBO>>(mShaders["deferredRender"], "Engine_UBO");


	mDeferredTexturesUniform = std::make_shared<UniformTexturesVk>(mShaders["deferredLightning"], 1,
		std::vector<std::shared_ptr<TextureInterface>>{ mTextures["gPositionTex"], mTextures["gNormalTex"], mTextures["gAlbedoSpecTex"], mTextures["gRoughAOTex"] });

	mRenderToScreenTexturesUniform = std::make_shared<UniformTexturesVk>(mShaders["renderToScreen"], 0,
		std::vector<std::shared_ptr<TextureInterface>>{ mTextures["deferredResult"] });

}


void GameRendererVk::renderScene() {
	mainCameraComponentVk = std::nullopt;
	if (mContext.sceneManager->hasCurrentScene()) {
		mainCameraComponentVk = mContext.sceneManager->getCurrentScene().findMainCamera();
	}
	if (mainCameraComponentVk) {
		auto [winWidth, winHeight] = mContext.window->getSize();
		const auto& cameraPosition = mainCameraComponentVk.value()->obj->getTransform()->getWorldPosition();
		const auto& cameraRotation = mainCameraComponentVk.value()->obj->getTransform()->getWorldRotation();
		mainCameraComponentVk->getPtr()->getCamera().cacheMatrices(winWidth, winHeight, cameraPosition, cameraRotation);

		//sendEngineUBO();
		renderScene(mainCameraComponentVk.value());
	}
}

void GameRendererVk::sendEngineUBO() {
	auto [winWidth, winHeight] = mContext.window->getSize();
	const auto& cameraPosition = mainCameraComponentVk.value()->obj->getTransform()->getWorldPosition();
	EngineUBO data;
	auto proj = mainCameraComponentVk.value()->getCamera().getProjectionMatrix();
	proj(1, 1) *= -1.0f;
	data.Projection = MATHGL::Matrix4::Transpose(proj);
	data.View = MATHGL::Matrix4::Transpose(mainCameraComponentVk.value()->getCamera().getViewMatrix());
	data.ViewPos = cameraPosition;
	data.ViewportSize = MATHGL::Vector2f(winWidth, winHeight);
	mEngineUbo->set(data);
}

void GameRendererVk::updateLights(SCENE_SYSTEM::Scene& scene) {
	auto lightMatrices = scene.findLightData();
	LightOGL end;
	end.type = -1;
	lightMatrices.push_back(end);
	mLightSSBO->SendBlocks(lightMatrices);
	//mLightUBO->set(lightMatrices);
}

void GameRendererVk::updateLightsInFrustum(SCENE_SYSTEM::Scene& scene, const Frustum& frustum) {
	auto lightMatrices = scene.findLightDataInFrustum(frustum);
	LightOGL end;
	end.type = -1;
	lightMatrices.push_back(end);
	mLightSSBO->SendBlocks(lightMatrices);
	//mLightUBO->set(lightMatrices);
}

void GameRendererVk::renderScene(KUMA::Ref<KUMA::ECS::CameraComponent> mainCameraComponent) {
	auto [winWidth, winHeight] = mContext.window->getSize();
	auto& currentScene = mContext.sceneManager->getCurrentScene();

	mDriver->begin();


	auto& camera = mainCameraComponent->getCamera();
	const auto& cameraPosition = mainCameraComponent->obj->getTransform()->getWorldPosition();
	std::tie(mOpaqueMeshesForward, mTransparentMeshesForward, mOpaqueMeshesDeferred, mTransparentMeshesDeferred) =
		currentScene.findDrawables(cameraPosition, camera, nullptr, nullptr);

	
	//TODO: move it to shader bind() func
	mDriver->callShaderSequence.push(std::dynamic_pointer_cast<ShaderVk>(mShaders["deferredRender"]));
	mDriver->callShaderSequence.push(std::dynamic_pointer_cast<ShaderVk>(mShaders["deferredLightning"]));
	mDriver->callShaderSequence.push(std::dynamic_pointer_cast<ShaderVk>(mShaders["renderToScreen"]));

	auto imageIndex = mDriver->imageIndex;


	sendEngineUBO();
	if (mainCameraComponent->isFrustumLightCulling()) {
		updateLightsInFrustum(currentScene, mainCameraComponent->getCamera().getFrustum());
	}
	else {
		updateLights(currentScene);
	}

	drawDeferredGBuffer();
	{//render to screen
		mShaders["renderToScreen"]->bind();
		mDriver->defaultFb->bind(*mShaders["renderToScreen"]);
		mDriver->setViewport(*mShaders["renderToScreen"], 0.0, 0.0, winWidth, winHeight);
		mDriver->setScissor(*mShaders["renderToScreen"], 0, 0, winWidth, winHeight);
		{
			mShaders["renderToScreen"]->setUniform(*mRenderToScreenTexturesUniform);
			mShaders["renderToScreen"]->bindDescriptorSets();
		}
		mDriver->draw(mShaders["renderToScreen"], 3);
		//ImGui_ImplVulkan_RenderDrawData(draw_data, std::dynamic_pointer_cast<ShaderVk>(renderToScreenShader)->m_CommandHandler.m_CommandBuffers[imageIndex]);
		mDriver->defaultFb->unbind(*mShaders["renderToScreen"]);
		mShaders["renderToScreen"]->unbind();
	}

	mDriver->submit();
	mDriver->end();
}

void GameRendererVk::drawDeferredGBuffer() {
	auto [winWidth, winHeight] = mContext.window->getSize();

	mShaders["deferredRender"]->bind();
	mFramebuffers["gbufferFb"]->bind(*mShaders["deferredRender"]);
	mDriver->setViewport(*mShaders["deferredRender"], 0.0, 0.0, winWidth, winHeight);
	mDriver->setScissor(*mShaders["deferredRender"], 0, 0, winWidth, winHeight);
	for (const auto& [distance, drawable] : mOpaqueMeshesDeferred) {
		drawDrawableDeferred(drawable);
	}
	for (const auto& [distance, drawable] : mTransparentMeshesDeferred) {
		drawDrawableDeferred(drawable);
	}
	mFramebuffers["gbufferFb"]->unbind(*mShaders["deferredRender"]);
	mShaders["deferredRender"]->unbind();

	mShaders["deferredLightning"]->bind();
	//mDriver->defaultFb->bind(*mShaders["deferredLightning"]);
	mFramebuffers["deferredFb"]->bind(*mShaders["deferredLightning"]);
	mDriver->setViewport(*mShaders["deferredLightning"], 0.0, 0.0, winWidth, winHeight);
	mDriver->setScissor(*mShaders["deferredLightning"], 0, 0, winWidth, winHeight);
	{
		mShaders["deferredLightning"]->setUniform(*mEngineUbo);
		mShaders["deferredLightning"]->setUniform(*mDeferredTexturesUniform);
		mShaders["deferredLightning"]->setDescriptorSet(mLightSSBO->setId, mLightSSBO->descriptorSets);
		//mShaders["deferredLightning"]->setUniform(*mLightUBO);
		mShaders["deferredLightning"]->bindDescriptorSets();
	}
	mDriver->draw(mShaders["deferredLightning"], 3);
	mFramebuffers["deferredFb"]->unbind(*mShaders["deferredLightning"]);
	//mDriver->defaultFb->unbind(*mShaders["deferredLightning"]);
	mShaders["deferredLightning"]->unbind();
}

void GameRendererVk::drawDrawableDeferred(const Drawable& p_toDraw) {
	if (p_toDraw.material->getGPUInstances() > 0) {
		reinterpret_cast<MaterialVk*>(p_toDraw.material.get())->fillUniformsWithShader(mShaders["deferredRender"], mEmptyTexture, true);
		mShaders["deferredRender"]->setUniform(*mEngineUbo);
		mShaders["deferredRender"]->setPushConstant(MATHGL::Matrix4::Transpose(p_toDraw.world));
		mShaders["deferredRender"]->bindDescriptorSets();
		mDriver->draw(mShaders["deferredRender"] , *p_toDraw.mesh, p_toDraw.material->getGPUInstances());
	}
}

#endif
