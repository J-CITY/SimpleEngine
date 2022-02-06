#include "gameRenderer.h"

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>

#include "../ecs/components/cameraComponent.h"
#include "../ecs/components/materialRenderer.h"
#include "../ecs/ComponentManager.h"
#include "../resourceManager/textureManager.h"
#include "../core/core.h"
#include "light.h"
#include "../ecs/components/directionalLight.h"
#include "../ecs/components/spotLight.h"
#include "../resourceManager/ServiceManager.h"

using namespace KUMA;
using namespace KUMA::RENDER;

//unsigned int depthMapFBO;
//unsigned int depthMap;
glm::vec3 lightPos = glm::vec3(0.0f, 15.0f, 15.0f);
const unsigned int SHADOW_WIDTH = 1024, SHADOW_HEIGHT = 1024;

GameRenderer::GameRenderer(CORE_SYSTEM::Core& _context) :
	context(_context) {
	
	emptyMaterial.setShader(context.shaderManager.createResource("Shaders\\Unlit.glsl"));
	emptyMaterial.set("u_Diffuse", MATHGL::Vector3(1.f, 0.f, 1.f));
	emptyMaterial.set<RESOURCES::Texture*>("u_DiffuseMap", nullptr);

	context.renderer->registerModelMatrixSender([this](const MATHGL::Matrix4& p_modelMatrix) {
		context.engineUBO->setSubData(MATHGL::Matrix4::Transpose(p_modelMatrix), 0);
	});

	context.renderer->registerUserMatrixSender([this](const MATHGL::Matrix4& p_userMatrix) {
		context.engineUBO->setSubData(
			p_userMatrix,
			// UBO layout offset
			sizeof(MATHGL::Matrix4) +
			sizeof(MATHGL::Matrix4) +
			sizeof(MATHGL::Matrix4) +
			sizeof(MATHGL::Vector3) +
			sizeof(float)
		);
	});

	//GUI test (remove later)
	guiObjs.push_back(std::make_shared<GUI::GuiButton>());
	auto l = std::make_shared<GUI::GuiLayout>();
	for (int i =0;i<5;i++) {
		l->addChild(std::make_shared<GUI::GuiImage>());
	}
	l->transform->haligh = GUI::EAlign::CENTER;
	l->transform->valigh = GUI::EAlign::CENTER;
	l->calculateTransform();
	
	auto scroll = std::make_shared<GUI::GuiScroll>(l->getWidht() * 1.5f, 100.0f);
	scroll->transform->haligh = GUI::EAlign::CENTER;
	scroll->transform->valigh = GUI::EAlign::CENTER;
	scroll->scroll->isScrollVertical = false;
	scroll->addChild(l);
	//guiObjs.push_back(scroll);
	//scroll->calculateTransform();
	auto clip = std::make_shared<GUI::GuiClip>(320.0f, 70.0f);
	clip->transform->haligh = GUI::EAlign::CENTER;
	clip->transform->valigh = GUI::EAlign::BOTTOM;
	clip->addChild(scroll);
	guiObjs.push_back(clip);
	clip->calculateTransform();
	//GUI test end

	//shadow direction light
	// Ќастраиваем карту глубины FBO

	shadersMap["simpleDepthShader"] = KUMA::RESOURCES::ShaderLoader::Create("C:\\Projects\\SimpleEngine\\CGCourse\\CGCourse\\Assets\\Engine\\Shaders\\dirShadow.glsl");

	//glGenFramebuffers(1, &depthMapFBO);

	// —оздаем текстуры глубины
	//glGenTextures(1, &depthMap);
	//glBindTexture(GL_TEXTURE_2D, depthMap);
	//glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	//float borderColor [] = {1.0, 1.0, 1.0, 1.0};
	//glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

	// ѕрикрепл€ем текстуру глубины в качестве буфера глубины дл€ FBO
	//glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
	//glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
	//glDrawBuffer(GL_NONE);
	//glReadBuffer(GL_NONE);
	//glBindFramebuffer(GL_FRAMEBUFFER, 0);

}

void GameRenderer::renderSkybox() {
	//SKYBOX
	auto currentScene = context.sceneManager.getCurrentScene();
	auto& skyboxObj = currentScene->getSkybox();

	auto skyboxMat = skyboxObj.getComponent<ECS::MaterialRenderer>()->getMaterials()[0];
	skyboxMat->getShader()->bind();
	GLint OldCullFaceMode;
	glGetIntegerv(GL_CULL_FACE_MODE, &OldCullFaceMode);
	GLint OldDepthFuncMode;
	glGetIntegerv(GL_DEPTH_FUNC, &OldDepthFuncMode);
	glCullFace(GL_FRONT);
	glDepthFunc(GL_LEQUAL);
	
	auto mainCameraComponent = context.renderer->findMainCamera(*currentScene);
	auto& camera = mainCameraComponent->getCamera();
	auto v = camera.getViewMatrix();
	auto p = camera.getProjectionMatrix();

	auto pos = mainCameraComponent->obj.transform->getLocalPosition();
	auto rot = MATHGL::Quaternion(MATHGL::Vector3(0, 0, 0));
	auto scl = MATHGL::Vector3(1, 1, 1);

	auto m = MATHGL::Matrix4::Translation(pos) * MATHGL::Quaternion::ToMatrix4(MATHGL::Quaternion::Normalize(rot)) * MATHGL::Matrix4::Scaling(scl);

	skyboxMat->getShader()->setUniformMat4("gWVP", p * v * m);
	currentScene->getSkyboxTexture().bind(GL_TEXTURE0);

	auto mr = skyboxObj.getComponent<ECS::ModelRenderer>();
	auto mm = mr->getModel()->getMeshes();
	for (auto m : mm)
		context.renderer->draw(*m, PrimitiveMode::TRIANGLES, 1);

	glCullFace(OldCullFaceMode);
	glDepthFunc(OldDepthFuncMode);
	skyboxMat->getShader()->unbind();
}

bool isShadowDir = false;
glm::mat4 lightSpaceMatrix;

void GameRenderer::prepareShadowMap() {
	context.renderer->dirShadowsMapIDs.clear();
	for (auto& light : ECS::ComponentManager::getInstance()->getAllDirectionalLights()) {
		depthMapFBO.AttachTexture(*light->shadowMap, Attachment::DEPTH_ATTACHMENT);
		//glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO.id);
		//glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, light->shadowMap->id, 0);
		glDrawBuffer(GL_NONE);
		glReadBuffer(GL_NONE);
		depthMapFBO.unbind();
		context.renderer->dirShadowsMapIDs.push_back(light->shadowMap->id);

		lightPos = glm::vec3(light->obj.transform->getLocalPosition().x,
			light->obj.transform->getLocalPosition().y,
			light->obj.transform->getLocalPosition().z);

		glm::mat4 lightProjection, lightView;
		float near_plane = 1.0f, far_plane = 100.0f;
		// lightProjection = glm::perspective(glm::radians(45.0f), (GLfloat)SHADOW_WIDTH / (GLfloat)SHADOW_HEIGHT, near_plane, far_plane); // обратите внимание, что если вы используете матрицу перспективной проекции, то вам придетс€ изменить положение света, так как текущего положени€ света недостаточно дл€ отображени€ всей сцены
		lightProjection = glm::ortho(-50.0f, 50.0f, -50.0f, 50.0f, near_plane, far_plane);
		lightView = glm::lookAt(lightPos, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0, 1.0, 0.0));
		lightSpaceMatrix = lightProjection * lightView;

		MATHGL::Matrix4 m(
			lightSpaceMatrix[0][0], lightSpaceMatrix[0][1], lightSpaceMatrix[0][2], lightSpaceMatrix[0][3],
			lightSpaceMatrix[1][0], lightSpaceMatrix[1][1], lightSpaceMatrix[1][2], lightSpaceMatrix[1][3],
			lightSpaceMatrix[2][0], lightSpaceMatrix[2][1], lightSpaceMatrix[2][2], lightSpaceMatrix[2][3],
			lightSpaceMatrix[3][0], lightSpaceMatrix[3][1], lightSpaceMatrix[3][2], lightSpaceMatrix[3][3]
		);


		// –ендеринг сцены глазами источника света
		shadersMap["simpleDepthShader"]->bind();
		shadersMap["simpleDepthShader"]->setUniformMat4("lightSpaceMatrix", m.Transpose(m));
		//glUniformMatrix4fv(glGetUniformLocation(shadersMap["simpleDepthShader"]->id, "lightSpaceMatrix"), 1, GL_FALSE, &lightSpaceMatrix[0][0]);

		context.renderer->setViewPort(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
		//glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
		//glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
		depthMapFBO.bind();
		context.renderer->ClearDepth();
		//glClear(GL_DEPTH_BUFFER_BIT);
		isShadowDir = true;
		context.renderer->renderScene(shadersMap["simpleDepthShader"]);
		depthMapFBO.unbind();
		//glBindFramebuffer(GL_FRAMEBUFFER, 0);
		isShadowDir = false;

		// —брос настроек области просмотра
		glViewport(0, 0, 800, 600);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}
}

void GameRenderer::renderScene() {
	if (auto currentScene = context.sceneManager.getCurrentScene()) {
		if (auto mainCameraComponent = context.renderer->findMainCamera(*currentScene)) {
			auto& camera = mainCameraComponent->getCamera();
			if (mainCameraComponent->isFrustumLightCulling()) {
				updateLightsInFrustum(*currentScene, mainCameraComponent->getCamera().getFrustum());
			}
			else {
				updateLights(*currentScene);
			}

			auto [winWidth, winHeight] = context.window->getSize();
			const auto& cameraPosition = mainCameraComponent->obj.transform->getWorldPosition();
			const auto& cameraRotation = mainCameraComponent->obj.transform->getWorldRotation();
			//auto& camera = mainCameraComponent->GetCamera();

			camera.cacheMatrices(winWidth, winHeight, cameraPosition, cameraRotation);

			updateEngineUBO(*mainCameraComponent);

			context.renderer->clear(true, true, false);
			context.renderer->FindDrawables(cameraPosition, camera, nullptr, &emptyMaterial);

			prepareShadowMap();


			renderSkybox();

			uint8_t glState = context.renderer->fetchGLState();
			context.renderer->applyStateMask(glState);
			context.renderer->renderScene(nullptr);
			context.renderer->applyStateMask(glState);

			

			//glEnable(GL_DEPTH_TEST);
			//glDepthMask(GL_TRUE);
			//glEnable(GL_BLEND);
			//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


			//world->OnUpdate(context.window.get());
			//world->RenderWorld();


			//context.window->getSFMLContext()->pushGLStates();
			//for (auto& e : guiObjs) {
			//	e->onUpdate(context, 0.0f);
			//}
			//context.window->getSFMLContext()->popGLStates();
		}
		else {
			context.renderer->setClearColor(0.0f, 0.0f, 0.0f);
			context.renderer->clear(true, true, false);
		}
	}
}

void GameRenderer::updateEngineUBO(ECS::CameraComponent& p_mainCamera) {
	size_t offset = sizeof(MATHGL::Matrix4); // We skip the model matrix (Which is a special case, modified every draw calls)
	auto& camera = p_mainCamera.getCamera();

	context.engineUBO->setSubData(MATHGL::Matrix4::Transpose(camera.getViewMatrix()), std::ref(offset));
	context.engineUBO->setSubData(MATHGL::Matrix4::Transpose(camera.getProjectionMatrix()), std::ref(offset));
	context.engineUBO->setSubData(p_mainCamera.obj.transform->getWorldPosition(), std::ref(offset));
}

void GameRenderer::updateLights(SCENE_SYSTEM::Scene& p_scene) {
	auto lightMatrices = context.renderer->findLightMatrices(p_scene);
	context.lightSSBO->SendBlocks<LightOGL>(lightMatrices.data(), lightMatrices.size() * sizeof(LightOGL));
}

void GameRenderer::updateLightsInFrustum(SCENE_SYSTEM::Scene& p_scene, const Frustum& p_frustum) {
	auto lightMatrices = context.renderer->findLightMatricesInFrustum(p_scene, p_frustum);
	context.lightSSBO->SendBlocks<LightOGL>(lightMatrices.data(), lightMatrices.size() * sizeof(LightOGL));
}
/// 
Renderer::Renderer(GL_SYSTEM::GlManager& _driver) :
	BaseRender(_driver),
	emptyTexture(RESOURCES::TextureLoader::CreateColor(
		(255 << 24) | (255 << 16) | (255 << 8) | 255,
		RESOURCES::ETextureFilteringMode::NEAREST,
		RESOURCES::ETextureFilteringMode::NEAREST,
		false
	)) {

	//multiSampledFBO.setupFrameBuffer(true);
	//shaderAtlas[0] = new Shader("basicShader.vert", "basicShader.frag");
	//shaderAtlas[1] = new Shader("screenShader.vert", "screenShader.frag");
	//shaderAtlas[2] = new Shader("skyboxShader.vert", "skyboxShader.frag");
	//shaderAtlas[PipelineRenderShaderType::DIR_SHADOW] = KUMA::RESOURCES::ShaderLoader().createResource("Shaders\\shadowShader.glsl");
	//shaderAtlas[PipelineRenderShaderType::POINT_SHADOW] = KUMA::RESOURCES::ShaderLoader().createResource("Shaders\\pointShadowShader.glsl");
	//new Shader("pointShadowShader.vert", "pointShadowShader.frag", "pointShadowShader.geom");
	//shaderAtlas[5] = new Shader("splitHighShader.vert", "splitHighShader.frag");
	//shaderAtlas[6] = new Shader("blurShader.vert", "blurShader.frag");

}

Renderer::~Renderer() {
	//RESOURCES::TextureLoader::Destroy(emptyTexture);
}

std::shared_ptr<ECS::CameraComponent> Renderer::findMainCamera(const SCENE_SYSTEM::Scene& p_scene) {
	for (auto& camera : ECS::ComponentManager::getInstance()->cameraComponents)
		if (camera.second->obj.getIsActive())
			return camera.second;

	return nullptr;
}

std::vector<LightOGL> Renderer::findLightMatrices(const SCENE_SYSTEM::Scene& p_scene)
{
	std::vector<LightOGL> result;

	const auto facs = ECS::ComponentManager::getInstance();

	for (auto light : facs->lightComponents) {
		if (light.second->obj.getIsActive()) {
			auto ldata = light.second->getData().generateOGLStruct();
			result.push_back(ldata);
		}
	}

	return result;
}

std::vector<LightOGL> Renderer::findLightMatricesInFrustum(const SCENE_SYSTEM::Scene& p_scene, const Frustum& p_frustum) {
	std::vector<LightOGL> result;

	const auto& facs = ECS::ComponentManager::getInstance();

	for (auto& light : facs->lightComponents) {
		if (light.second->obj.getIsActive()) {
			const auto& lightData = light.second->getData();
			const auto& position = lightData.getTransform().getWorldPosition();
			auto effectRange = lightData.getEffectRange();

			// We always consider lights that have an +inf range (Not necessary to test if they are in frustum)
			if (std::isinf(effectRange) || p_frustum.sphereInFrustum(position.x, position.y, position.z, lightData.getEffectRange())) {
				auto ldata = lightData.generateOGLStruct();
				result.push_back(ldata);
			}
		}
	}

	return result;
}

void Renderer::Clear() const {
	glClear(clearMask);
}

void Renderer::ClearDepth() const {
	glClear(GL_DEPTH_BUFFER_BIT);
}

void Renderer::FindDrawables(
	const MATHGL::Vector3& p_cameraPosition,
	const Camera& p_camera,
	const Frustum* p_customFrustum,
	Material* p_defaultMaterial
) {
	opaqueMeshes.clear();
	transparentMeshes.clear();

	if (p_camera.isFrustumGeometryCulling()) {
		const auto& frustum = p_customFrustum ? *p_customFrustum : p_camera.getFrustum();
		std::tie(opaqueMeshes, transparentMeshes) = findAndSortFrustumCulledDrawables(p_cameraPosition, frustum, p_defaultMaterial);
	}
	else {
		std::tie(opaqueMeshes, transparentMeshes) = findAndSortDrawables(p_cameraPosition, p_defaultMaterial);
	}
}

void Renderer::renderScene(std::shared_ptr<RESOURCES::Shader> shader) {
	for (const auto& [distance, drawable] : opaqueMeshes) {
		if (isShadowDir)
			drawDrawable(drawable, shader);
		else
			drawDrawable(drawable);
	}
	for (const auto& [distance, drawable] : transparentMeshes) {
		if (isShadowDir)
			drawDrawable(drawable, shader);
		else
			drawDrawable(drawable);
	}
}

void findAndSortDrawables(
	Renderer::OpaqueDrawables& p_opaques,
	Renderer::TransparentDrawables& p_transparents,
	const KUMA::MATHGL::Vector3& p_cameraPosition,
	std::shared_ptr<RENDER::Material> p_defaultMaterial
)
{
	for (auto& modelRenderer : ECS::ComponentManager::getInstance()->modelComponents) {
		if (modelRenderer.second->obj.getIsActive()) {
			if (auto model = modelRenderer.second->getModel()) {
				float distanceToActor = KUMA::MATHGL::Vector3::Distance(modelRenderer.second->obj.transform->getWorldPosition(), p_cameraPosition);

				if (auto materialRenderer = modelRenderer.second->obj.getComponent<ECS::MaterialRenderer>()) {
					const auto& transform = modelRenderer.second->obj.transform->getTransform();

					const ECS::MaterialRenderer::MaterialList& materials = materialRenderer->getMaterials();

					for (auto mesh : model->getMeshes()) {
						std::shared_ptr<RENDER::Material> material;

						if (mesh->getMaterialIndex() < MAX_MATERIAL_COUNT) {
							material = materials.at(mesh->getMaterialIndex());
							if (!material || !material->getShader())
								material = p_defaultMaterial;
						}

						if (material) {
							Renderer::Drawable element = {
								transform.getWorldMatrix(),
								mesh, material.get(), materialRenderer->getUserMatrix() };

							if (material->isBlendable())
								p_transparents.emplace(distanceToActor, element);
							else
								p_opaques.emplace(distanceToActor, element);
						}
					}
				}
			}
		}
	}
}

std::pair<Renderer::OpaqueDrawables, Renderer::TransparentDrawables> Renderer::findAndSortFrustumCulledDrawables
(
	const MATHGL::Vector3& p_cameraPosition,
	const Frustum& p_frustum,
	Material* p_defaultMaterial
) {
	
	Renderer::OpaqueDrawables opaqueDrawables;
	Renderer::TransparentDrawables transparentDrawables;

	for (auto& modelRenderer : ECS::ComponentManager::getInstance()->modelComponents) {
		auto& owner = modelRenderer.second->obj;

		if (owner.getIsActive()) {
			if (auto model = modelRenderer.second->getModel()) {
				if (auto materialRenderer = modelRenderer.second->obj.getComponent<ECS::MaterialRenderer>()) {
					auto& transform = owner.transform->getTransform();

					CullingOptions cullingOptions = CullingOptions::NONE;

					if (modelRenderer.second->getFrustumBehaviour() != ECS::ModelRenderer::EFrustumBehaviour::DISABLED) {
						cullingOptions |= CullingOptions::FRUSTUM_PER_MODEL;
					}

					if (modelRenderer.second->getFrustumBehaviour() == ECS::ModelRenderer::EFrustumBehaviour::CULL_MESHES) {
						cullingOptions |= CullingOptions::FRUSTUM_PER_MESH;
					}

					const auto& modelBoundingSphere = modelRenderer.second->getFrustumBehaviour() == ECS::ModelRenderer::EFrustumBehaviour::CULL_CUSTOM ? modelRenderer.second->getCustomBoundingSphere() : model->getBoundingSphere();

					std::vector<std::reference_wrapper<RESOURCES::Mesh>> meshes;
					{
						meshes = getMeshesInFrustum(*model, modelBoundingSphere, transform, p_frustum, cullingOptions);
					}

					if (!meshes.empty()) {
						float distanceToActor = MATHGL::Vector3::Distance(transform.getWorldPosition(), p_cameraPosition);
						const ECS::MaterialRenderer::MaterialList& materials = materialRenderer->getMaterials();

						for (const auto& mesh : meshes)
						{
							Material* material = nullptr;

							if (mesh.get().getMaterialIndex() < MAX_MATERIAL_COUNT)
							{
								material = materials.at(mesh.get().getMaterialIndex()).get();
								if (!material || !material->getShader())
									material = p_defaultMaterial;
							}

							if (material) {
								Renderer::Drawable element = { transform.getWorldMatrix(), &mesh.get(), material, materialRenderer->getUserMatrix() };

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

	return { opaqueDrawables, transparentDrawables };
}

std::pair<Renderer::OpaqueDrawables, Renderer::TransparentDrawables> Renderer::findAndSortDrawables
(
	const MATHGL::Vector3& p_cameraPosition,
	Material* p_defaultMaterial
)
{
	OpaqueDrawables opaqueDrawables;
	TransparentDrawables transparentDrawables;

	for (auto& modelRenderer : ECS::ComponentManager::getInstance()->modelComponents) {
		if (modelRenderer.second->obj.getIsActive() && modelRenderer.second->obj.getName() != "Skybox") {
			if (auto model = modelRenderer.second->getModel()) {
				float distanceToActor = MATHGL::Vector3::Distance(modelRenderer.second->obj.transform->getWorldPosition(), p_cameraPosition);

				if (auto materialRenderer = modelRenderer.second->obj.getComponent<ECS::MaterialRenderer>()) {
					const auto& transform = modelRenderer.second->obj.transform->getTransform();

					const ECS::MaterialRenderer::MaterialList& materials = materialRenderer->getMaterials();

					for (auto mesh : model->getMeshes()) {
						Material* material = nullptr;

						if (mesh->getMaterialIndex() < MAX_MATERIAL_COUNT) {
							material = materials.at(mesh->getMaterialIndex()).get();
							if (!material || !material->getShader())
								material = p_defaultMaterial;
						}

						if (material) {
							Renderer::Drawable element = { transform.getWorldMatrix(), mesh, material, materialRenderer->getUserMatrix() };

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

	return { opaqueDrawables, transparentDrawables };
}

void Renderer::drawDrawable(const Drawable& p_toDraw) {
	userMatrixSender(p_toDraw.userMatrix);
	drawMesh(*p_toDraw.mesh, *p_toDraw.material, &p_toDraw.world);
}

void Renderer::drawDrawable(const Drawable& p_toDraw, std::shared_ptr<RESOURCES::Shader> shader) {
	userMatrixSender(p_toDraw.userMatrix);
	drawDirShadowMap(*p_toDraw.mesh, *p_toDraw.material, &p_toDraw.world, shader);
}

void Renderer::drawModelWithSingleMaterial(Model& p_model, Material& p_material, MATHGL::Matrix4 const* p_modelMatrix, Material* p_defaultMaterial) {
	if (p_modelMatrix)
		modelMatrixSender(*p_modelMatrix);

	for (auto mesh : p_model.getMeshes()) {
		Material* material = p_material.getShader() ? &p_material : p_defaultMaterial;

		if (material)
			drawMesh(*mesh, *material, nullptr);
	}
}

void Renderer::drawModelWithMaterials(Model& p_model, std::vector<Material*> p_materials, MATHGL::Matrix4 const* p_modelMatrix, Material* p_defaultMaterial) {
	if (p_modelMatrix)
		modelMatrixSender(*p_modelMatrix);

	for (auto mesh : p_model.getMeshes()) {
		Material* material = p_materials.size() > mesh->getMaterialIndex() ? p_materials[mesh->getMaterialIndex()] : p_defaultMaterial;
		if (material)
			drawMesh(*mesh, *material, nullptr);
	}
}

void Renderer::drawMesh(RESOURCES::Mesh& p_mesh, Material& p_material, MATHGL::Matrix4 const* p_modelMatrix, bool useTextures) {
	if (p_material.hasShader() && p_material.getGPUInstances() > 0) {
		if (isShadowDir) {
			if (p_material.getUniformsData().count("castShadow") && !std::get<bool>(p_material.getUniformsData()["useBone"])) {
				return;
			}
		}

		if (p_modelMatrix)
			modelMatrixSender(*p_modelMatrix);

		uint8_t stateMask = p_material.generateStateMask();
		applyStateMask(stateMask);

		{
			p_material.bind(emptyTexture, useTextures);
			p_material.getShader()->setUniformVec3("lightPos", MATHGL::Vector3(lightPos.x, lightPos.y, lightPos.z));
			//p_material.getShader()->setUniformMat4("lightSpaceMatrix", lightSpaceMatrix);
			glUniformMatrix4fv(glGetUniformLocation(p_material.getShader()->id, "lightSpaceMatrix"), 1, GL_FALSE, &lightSpaceMatrix[0][0]);
			p_material.getShader()->setUniformInt("shadowMap", 5);
			glActiveTexture(GL_TEXTURE5);
			glBindTexture(GL_TEXTURE_2D, dirShadowsMapIDs[0]);
		}
		draw(p_mesh, p_material, PrimitiveMode::TRIANGLES, p_material.getGPUInstances());
		if (!isShadowDir)
			p_material.unbind();
	}
}

void Renderer::drawDirShadowMap(RESOURCES::Mesh& p_mesh, Material& p_material, MATHGL::Matrix4 const* p_modelMatrix, std::shared_ptr<RESOURCES::Shader> shader) {
	if (p_material.hasShader() && p_material.getGPUInstances() > 0) {
		if (isShadowDir) {
			if (p_material.getUniformsData().count("castShadow") && !std::get<bool>(p_material.getUniformsData()["useBone"])) {
				return;
			}
		}

		if (p_modelMatrix)
			modelMatrixSender(*p_modelMatrix);

		uint8_t stateMask = p_material.generateStateMask();
		applyStateMask(stateMask);

		if (p_material.getUniformsData().count("useBone")) {
			shader->setUniformInt("useBone", std::get<bool>(p_material.getUniformsData()["useBone"]));
		}
		shader->setUniformMat4("model", *p_modelMatrix);

		draw(p_mesh, p_material, PrimitiveMode::TRIANGLES, p_material.getGPUInstances());
		if (!isShadowDir)
			p_material.unbind();
	}
}

void Renderer::registerModelMatrixSender(std::function<void(MATHGL::Matrix4)> p_modelMatrixSender) {
	modelMatrixSender = p_modelMatrixSender;
}

void Renderer::registerUserMatrixSender(std::function<void(MATHGL::Matrix4)> p_userMatrixSender) {
	userMatrixSender = p_userMatrixSender;
}
