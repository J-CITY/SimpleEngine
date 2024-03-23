#include "gameRendererVk.h"

#ifdef VULKAN_BACKEND

#include "backends/imgui_impl_vulkan.h"
//#include "backends/vk/rt/rt.h"
#include "backends/vk/materialVk.h"
#include "backends/vk/frameBufferVk.h"
#include "backends/vk/storageBufferVk.h"
#include "backends/vk/uniformBufferVk.h"
#include "backends/vk/shaderVk.h"
#include <coreModule/core/core.h>
#include <sceneModule/sceneManager.h>
#include <windowModule/window/window.h>

using namespace IKIGAI;
using namespace IKIGAI::RENDER;

std::optional<IKIGAI::UTILS::Ref<IKIGAI::ECS::CameraComponent>> mainCameraComponentVk = std::nullopt;


GameRendererVk::GameRendererVk(IKIGAI::CORE::Core& context) : mContext(context) {

	mEmptyTexture = TextureVk::create(IKIGAI::UTILS::GetRealPath("textures/snow.png"));

	mDriver = dynamic_cast<DriverVk*>(context.driver.get());

	VkPhysicalDeviceProperties p;
	vkGetPhysicalDeviceProperties(mDriver->m_MainDevice.PhysicalDevice, &p);


	//createVkResources();
}

#if defined(USE_EDITOR) || defined(USE_CHEATS)
static ImGui_ImplVulkanH_Window g_MainWindowData;
static VkAllocationCallbacks* g_Allocator = nullptr;
static int                      g_MinImageCount = 2;


static void SetupVulkanWindow(ImGui_ImplVulkanH_Window* wd, VkSurfaceKHR surface, int width, int height) {
	auto render = reinterpret_cast<IKIGAI::RENDER::GameRendererVk&>(IKIGAI::RESOURCES::ServiceManager::Get<IKIGAI::RENDER::GameRendererInterface>()).getDriver();

	wd->Surface = surface;

	// Check for WSI support
	VkBool32 res;
	vkGetPhysicalDeviceSurfaceSupportKHR(render->m_MainDevice.PhysicalDevice, render->m_QueueFamilyIndices.GraphicsFamily, wd->Surface, &res);
	if (res != VK_TRUE) {
		fprintf(stderr, "Error no WSI support on physical device 0\n");
		exit(-1);
	}

	// Select Surface Format
	const VkFormat requestSurfaceImageFormat[] = {VK_FORMAT_B8G8R8A8_UNORM, VK_FORMAT_R8G8B8A8_UNORM, VK_FORMAT_B8G8R8_UNORM, VK_FORMAT_R8G8B8_UNORM};
	const VkColorSpaceKHR requestSurfaceColorSpace = VK_COLORSPACE_SRGB_NONLINEAR_KHR;
	wd->SurfaceFormat = ImGui_ImplVulkanH_SelectSurfaceFormat(render->m_MainDevice.PhysicalDevice, wd->Surface, requestSurfaceImageFormat, (size_t)IM_ARRAYSIZE(requestSurfaceImageFormat), requestSurfaceColorSpace);

	// Select Present Mode
#ifdef IMGUI_UNLIMITED_FRAME_RATE
	VkPresentModeKHR present_modes[] = {VK_PRESENT_MODE_MAILBOX_KHR, VK_PRESENT_MODE_IMMEDIATE_KHR, VK_PRESENT_MODE_FIFO_KHR};
#else
	VkPresentModeKHR present_modes[] = {VK_PRESENT_MODE_FIFO_KHR};
#endif
	wd->PresentMode = ImGui_ImplVulkanH_SelectPresentMode(render->m_MainDevice.PhysicalDevice, wd->Surface, &present_modes[0], IM_ARRAYSIZE(present_modes));
	//printf("[vulkan] Selected PresentMode = %d\n", wd->PresentMode);

	// Create SwapChain, RenderPass, Framebuffer, etc.
	//IM_ASSERT(g_MinImageCount >= 2);
	ImGui_ImplVulkanH_CreateOrResizeWindow(render->m_VulkanInstance, render->m_MainDevice.PhysicalDevice, 
		render->m_MainDevice.LogicalDevice, wd, render->m_QueueFamilyIndices.GraphicsFamily, g_Allocator, width, height, g_MinImageCount);
}

void GameRendererVk::initForVk() {
	auto render = reinterpret_cast<IKIGAI::RENDER::GameRendererVk&>(IKIGAI::RESOURCES::ServiceManager::Get<IKIGAI::RENDER::GameRendererInterface>()).getDriver();

	//ImGui_ImplVulkanH_Window* wd = &g_MainWindowData;
	//auto size = RESOURCES::ServiceManager::Get<WINDOW_SYSTEM::Window>().getSize();
	//SetupVulkanWindow(wd, render->m_Surface, size.x, size.y);


	auto rd = render->GetRenderData();

	ImGui_ImplVulkan_InitInfo init_info = {};
	init_info.Instance = render->m_VulkanInstance;
	init_info.PhysicalDevice = render->m_MainDevice.PhysicalDevice;
	init_info.Device = render->m_MainDevice.LogicalDevice;
	init_info.PipelineCache = VK_NULL_HANDLE;
	init_info.Allocator = nullptr;
	init_info.QueueFamily = rd.graphic_queue_index;
	init_info.Queue = rd.graphic_queue;
	init_info.DescriptorPool = rd.imgui_descriptor_pool;
	init_info.MinImageCount = rd.min_image_count;
	init_info.ImageCount = rd.image_count;
	init_info.CheckVkResultFn = nullptr;


	ImGui_ImplVulkan_Init(&init_info, render->defaultFb->m_RenderPass);

	VkCommandPool command_pool = render->m_CommandHandler.GetCommandPool();

	VkCommandBuffer command_buffer = render->m_CommandHandler.GetCommandBuffer(0);

	vkResetCommandPool(rd.device, command_pool, 0);
	VkCommandBufferBeginInfo begin_info = {};
	begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	begin_info.flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	vkBeginCommandBuffer(command_buffer, &begin_info);

	ImGui_ImplVulkan_CreateFontsTexture(command_buffer);

	VkSubmitInfo end_info = {};
	end_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	end_info.commandBufferCount = 1;
	end_info.pCommandBuffers = &command_buffer;
	vkEndCommandBuffer(command_buffer);
	vkQueueSubmit(rd.graphic_queue, 1, &end_info, VK_NULL_HANDLE);
	vkDeviceWaitIdle(rd.device);
	ImGui_ImplVulkan_DestroyFontUploadObjects();
}
#endif



int NUM_LIGHTS = 20;
void GameRendererVk::createVkResources() {
	const auto sz = mContext.window->getSize();
	const auto winWidth = sz.x;
	const auto winHeight = sz.y;
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
		"./shaders/vk/deferredGBuffer.vert.spv", "./shaders/vk/deferredGBuffer.frag.spv");
	
	ShaderVk::push_constant = std::nullopt;
	ShaderVk::depthWriteEnable = VK_FALSE;
	ShaderVk::cullMode = VK_CULL_MODE_FRONT_BIT;
	ShaderVk::frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
	mShaders["deferredLightning"] = std::make_shared<ShaderVk>(mFramebuffers["deferredFb"]->m_RenderPass,
		"./shaders/vk/deferredLightningPbr.vert.spv", "./shaders/vk/deferredLightningPbr.frag.spv");


	mShaders["renderToScreen"] = std::make_shared<ShaderVk>(mDriver->defaultFb->m_RenderPass,
		"./shaders/vk/renderToScreen.vert.spv", "./shaders/vk/renderToScreen.frag.spv");


	// we could send it to different shaders if it set in same bind and set
	mLightSSBO = std::make_shared<ShaderStorageBufferVk<LightOGL>>(mShaders["deferredLightning"], "ssbo_Lights");
	//mLightUBO = std::make_shared<UniformBufferVk<RENDER::LightOGL>>(mShaders["deferredLightning"], "ssbo_Lights", NUM_LIGHTS);
	mEngineUbo = std::make_shared<UniformBufferVk<EngineUBO>>(mShaders["deferredRender"], "Engine_UBO");


	mDeferredTexturesUniform = std::make_shared<UniformTexturesVk>(mShaders["deferredLightning"], 1,
		std::vector<std::shared_ptr<TextureInterface>>{ mTextures["gPositionTex"], mTextures["gNormalTex"], mTextures["gAlbedoSpecTex"], mTextures["gRoughAOTex"] });

	mRenderToScreenTexturesUniform = std::make_shared<UniformTexturesVk>(mShaders["renderToScreen"], 0,
		std::vector<std::shared_ptr<TextureInterface>>{ mTextures["deferredResult"] });

}
//HybridRendering* hr = nullptr;
void GameRendererVk::renderScene() {
	//if (!hr)
	//{
	//	hr = new HybridRendering();
	//	if (!hr->init_base())
	//		return;
	//}
	//hr->update_base(TIME::Timer::GetInstance().getDeltaTime().count());
	//return;
	mainCameraComponentVk = std::nullopt;
	if (mContext.sceneManager->hasCurrentScene()) {
		mainCameraComponentVk = mContext.sceneManager->getCurrentScene().findMainCamera();
	}
	if (mainCameraComponentVk) {
		const auto sz = mContext.window->getSize();
		const auto winWidth = sz.x;
		const auto winHeight = sz.y;
		const auto& cameraPosition = mainCameraComponentVk.value()->obj->getTransform()->getWorldPosition();
		const auto& cameraRotation = mainCameraComponentVk.value()->obj->getTransform()->getWorldRotation();
		mainCameraComponentVk->getPtr()->getCamera().cacheMatrices(winWidth, winHeight, cameraPosition, cameraRotation);

		//sendEngineUBO();
		renderScene(mainCameraComponentVk.value());
	}
}

void GameRendererVk::sendEngineUBO() {
	const auto sz = mContext.window->getSize();
	const auto winWidth = sz.x;
	const auto winHeight = sz.y;
	const auto& cameraPosition = mainCameraComponentVk.value()->obj->getTransform()->getWorldPosition();
	EngineUBO data;
	auto proj = mainCameraComponentVk.value()->getCamera().getProjectionMatrix();
	proj(1, 1) *= -1.0f;
	data.Projection = MATH::Matrix4f::Transpose(proj);
	data.View = MATH::Matrix4f::Transpose(mainCameraComponentVk.value()->getCamera().getViewMatrix());
	data.ViewPos = cameraPosition;
	data.ViewportSize = MATH::Vector2f(winWidth, winHeight);
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

void GameRendererVk::renderScene(IKIGAI::UTILS::Ref<IKIGAI::ECS::CameraComponent> mainCameraComponent) {
	const auto sz = mContext.window->getSize();
	const auto winWidth = sz.x;
	const auto winHeight = sz.y;
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
		ImDrawData* draw_data = ImGui::GetDrawData();
		ImGui_ImplVulkan_RenderDrawData(draw_data, std::dynamic_pointer_cast<ShaderVk>(mShaders["renderToScreen"])->m_CommandHandler.m_CommandBuffers[imageIndex]);

		mDriver->defaultFb->unbind(*mShaders["renderToScreen"]);
		mShaders["renderToScreen"]->unbind();
	}

	mDriver->submit();
	mDriver->end();
}

void GameRendererVk::drawDeferredGBuffer() {
	const auto sz = mContext.window->getSize();
	const auto winWidth = sz.x;
	const auto winHeight = sz.y;

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
		mShaders["deferredRender"]->setPushConstant(p_toDraw.world);
		mShaders["deferredRender"]->bindDescriptorSets();
		mDriver->draw(mShaders["deferredRender"] , *p_toDraw.mesh, p_toDraw.material->getGPUInstances());
	}
}

#endif
