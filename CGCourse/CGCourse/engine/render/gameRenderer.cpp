﻿#include "gameRenderer.h"

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
#include "../resourceManager/textureManager.h"
#include "../utils/time/time.h"
#include "../resourceManager/resource/bone.h"

using namespace KUMA;
using namespace KUMA::RENDER;

void renderQuad();
void renderCube();

MATHGL::Vector2f Renderer::getShadowMapResolution() {
	switch (pipeline.shadowLightData.resolution) {
		case ShadowMapResolution::LOW: return MATHGL::Vector2f(512, 512);
		case ShadowMapResolution::MEDIUM: return MATHGL::Vector2f(1024, 1024);
		case ShadowMapResolution::HIGH: return MATHGL::Vector2f(2048, 2048);
	}
}

void Renderer::initShaders() {
	//shadows
	shadersMap["simpleDepthShader"] =              RESOURCES::ShaderLoader::CreateFromFile("Shaders/dirShadow.glsl");
	shadersMap["pointShadowShader"] =              RESOURCES::ShaderLoader::CreateFromFile("Shaders/pointShadow.glsl");

	//deferred
	shadersMap["deferredGBuffer"] =                RESOURCES::ShaderLoader::CreateFromFile("Shaders/deferredGBuffer.glsl");
	shadersMap["deferredLightning"] =              RESOURCES::ShaderLoader::CreateFromFile("Shaders/deferredLightning.glsl");

	//ssao
	shadersMap["ssao"] =                           RESOURCES::ShaderLoader::CreateFromFile("Shaders/ssao.glsl");
	shadersMap["ssaoBlur"] =                       RESOURCES::ShaderLoader::CreateFromFile("Shaders/ssaoBlur.glsl");

	//ibl
	shadersMap["equirectangularToCubemapShader"] = RESOURCES::ShaderLoader::CreateFromFile("Shaders/equirectangular_to_cubemap.glsl");
	shadersMap["irradianceShader"] =               RESOURCES::ShaderLoader::CreateFromFile("Shaders/irradiance_convolution.glsl");
	shadersMap["prefilterShader"] =                RESOURCES::ShaderLoader::CreateFromFile("Shaders/prefilter.glsl");
	shadersMap["brdfShader"] =                     RESOURCES::ShaderLoader::CreateFromFile("Shaders/brdf.glsl");

	//blum
	shadersMap["blur"] =                           RESOURCES::ShaderLoader::CreateFromFile("Shaders/blur.glsl");
	shadersMap["bloom"] =                          RESOURCES::ShaderLoader::CreateFromFile("Shaders/bloom.glsl");

	//motion blur
	shadersMap["motionBlur"] =                     RESOURCES::ShaderLoader::CreateFromFile("Shaders/motionBlur.glsl");

	//fxaa
	shadersMap["fxaa"] =                           RESOURCES::ShaderLoader::CreateFromFile("Shaders/fxaa.glsl");

	//good rays
	shadersMap["bright"] =                         RESOURCES::ShaderLoader::CreateFromFile("Shaders/bright.glsl");
	shadersMap["godRaysTexture"] =                 RESOURCES::ShaderLoader::CreateFromFile("Shaders/godRaysTexture.glsl");
	shadersMap["godRays"] =                        RESOURCES::ShaderLoader::CreateFromFile("Shaders/godRays.glsl");

	//hdr
	shadersMap["hdr"] =                            RESOURCES::ShaderLoader::CreateFromFile("Shaders/hdr.glsl");
}


void Renderer::init() {
	emptyMaterial = std::make_shared<RENDER::Material>();
	emptyMaterial->setShader(context.shaderManager.CreateFromFile("Shaders\\Unlit.glsl"));
	emptyMaterial->set("u_Diffuse", MATHGL::Vector3(1.f, 0.f, 1.f));
	emptyMaterial->set<RESOURCES::Texture*>("u_DiffuseMap", nullptr);

	KUMA::RESOURCES::ModelParserFlags flags = KUMA::RESOURCES::ModelParserFlags::TRIANGULATE;
	flags |= KUMA::RESOURCES::ModelParserFlags::GEN_SMOOTH_NORMALS;
	flags |= KUMA::RESOURCES::ModelParserFlags::FLIP_UVS;
	flags |= KUMA::RESOURCES::ModelParserFlags::GEN_UV_COORDS;
	flags |= KUMA::RESOURCES::ModelParserFlags::CALC_TANGENT_SPACE;
	sphere = KUMA::RESOURCES::ModelLoader::Create("C:\\Projects\\SimpleEngine\\CGCourse\\CGCourse\\Assets\\Engine\\Models\\Sphere.fbx", flags);


	registerModelMatrixSender([this](const MATHGL::Matrix4& p_modelMatrix) {
		engineUBO->setSubData(MATHGL::Matrix4::Transpose(p_modelMatrix), 0);
	});

	//GUI test (remove later)
	guiObjs.push_back(std::make_shared<GUI::GuiButton>());
	auto l = std::make_shared<GUI::GuiLayout>();
	for (int i = 0; i < 5; i++) {
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

	initShaders();

	auto screenRes = RESOURCES::ServiceManager::Get<WINDOW_SYSTEM::Window>().getSize();
	{//create swap buffers
		int i = 0;
		for (auto& buffer : swapBuffers) {
			buffer.bind();
			swapTextures[i] = std::make_shared<RESOURCES::Texture>(screenRes.x, screenRes.y);
			swapTextures[i]->setFilter(RESOURCES::TextureFiltering::NEAREST, RESOURCES::TextureFiltering::NEAREST);
			buffer.attachTexture(*swapTextures[i], Attachment::COLOR_ATTACHMENT0);
			i++;
		}

		pipeline.godRays.godRaysTexture = RESOURCES::TextureLoader::CreateEmpty(screenRes.x, screenRes.y);
		pipeline.godRays.godRaysTexture->setFilter(RESOURCES::TextureFiltering::NEAREST, RESOURCES::TextureFiltering::NEAREST);
	}

	{//defered render
		// Конфигурирование g-буфера фреймбуфера
		pipeline.deferredRender.gBuffer.bind();

		// Цветовой буфер позиций
		pipeline.deferredRender.gPosition = RESOURCES::TextureLoader::CreateEmpty(screenRes.x, screenRes.y);
		pipeline.deferredRender.gPosition->setFilter(RESOURCES::TextureFiltering::NEAREST, RESOURCES::TextureFiltering::NEAREST);
		pipeline.deferredRender.gBuffer.attachTexture(*pipeline.deferredRender.gPosition, Attachment::COLOR_ATTACHMENT0);

		// Цветовой буфер нормалей
		pipeline.deferredRender.gNormal = RESOURCES::TextureLoader::CreateEmpty(screenRes.x, screenRes.y);
		pipeline.deferredRender.gNormal->setFilter(RESOURCES::TextureFiltering::NEAREST, RESOURCES::TextureFiltering::NEAREST);
		pipeline.deferredRender.gBuffer.attachTexture(*pipeline.deferredRender.gNormal, Attachment::COLOR_ATTACHMENT1);

		// Цветовой буфер значений цвета + отраженной составляющей
		pipeline.deferredRender.gAlbedoSpec = RESOURCES::TextureLoader::CreateEmpty(screenRes.x, screenRes.y, false, 4, RESOURCES::TextureFormat::RGBA);
		pipeline.deferredRender.gAlbedoSpec->setFilter(RESOURCES::TextureFiltering::NEAREST, RESOURCES::TextureFiltering::NEAREST);
		pipeline.deferredRender.gBuffer.attachTexture(*pipeline.deferredRender.gAlbedoSpec, Attachment::COLOR_ATTACHMENT2);

		// Указываем OpenGL на то, в какой прикрепленный цветовой буфер (заданного фреймбуфера) мы собираемся выполнять рендеринг 
		pipeline.deferredRender.gBuffer.setOupbutBuffers({Attachment::COLOR_ATTACHMENT0, Attachment::COLOR_ATTACHMENT1, Attachment::COLOR_ATTACHMENT2});

		// Создаем и прикрепляем буфер глубины (рендербуфер)
		DepthBuffer rboDepth(screenRes.x, screenRes.y);
		pipeline.deferredRender.gBuffer.attachDepth(rboDepth);

		// Проверяем готовность фреймбуфера
		if (pipeline.deferredRender.gBuffer.getStatus() != FrameBufferStatus::FRAMEBUFFER_COMPLETE) {
			LOG_ERROR("Deferred Framebuffer not complete!");
		}
		pipeline.deferredRender.gBuffer.unbind();
		//glBindFramebuffer(GL_FRAMEBUFFER, 0);

		shadersMap["deferredLightning"]->bind();
		shadersMap["deferredLightning"]->setUniformInt("u_PositionMap", 0);
		shadersMap["deferredLightning"]->setUniformInt("u_NormalMap", 1);
		shadersMap["deferredLightning"]->setUniformInt("u_DiffuseMap", 2);
		shadersMap["deferredLightning"]->setUniformInt("u_SSAO", 3);
		shadersMap["deferredLightning"]->unbind();
	}

	{//bloom
		shadersMap["bloom"]->bind();
		shadersMap["bloom"]->setUniformInt("u_Scene", 0);
		shadersMap["bloom"]->setUniformInt("u_BloomBlur", 1);
		shadersMap["bloom"]->bind();
	}

	{//godRays
		shadersMap["godRays"]->bind();
		shadersMap["godRays"]->setUniformInt("u_Scene", 0);
		shadersMap["godRays"]->setUniformInt("u_BinaryScene", 1);
		shadersMap["godRays"]->bind();
	}


	{//ssao
		pipeline.ssao.ssaoFBO.bind();
		pipeline.ssao.ssaoColorBuffer = RESOURCES::TextureLoader::CreateEmpty(screenRes.x, screenRes.y);
		pipeline.ssao.ssaoColorBuffer->setFilter(RESOURCES::TextureFiltering::NEAREST, RESOURCES::TextureFiltering::NEAREST);
		pipeline.ssao.ssaoFBO.attachTexture(*pipeline.ssao.ssaoColorBuffer);
		if (pipeline.ssao.ssaoFBO.getStatus() != FrameBufferStatus::FRAMEBUFFER_COMPLETE) {
			LOG_ERROR("SSAO Framebuffer not complete!");
		}

		// И стадия размытия
		pipeline.ssao.ssaoBlurFBO.bind();
		pipeline.ssao.ssaoColorBufferBlur = RESOURCES::TextureLoader::CreateEmpty(screenRes.x, screenRes.y);
		pipeline.ssao.ssaoColorBufferBlur->setFilter(RESOURCES::TextureFiltering::NEAREST, RESOURCES::TextureFiltering::NEAREST);
		pipeline.ssao.ssaoBlurFBO.attachTexture(*pipeline.ssao.ssaoColorBufferBlur);
		if (pipeline.ssao.ssaoBlurFBO.getStatus() != FrameBufferStatus::FRAMEBUFFER_COMPLETE) {
			LOG_ERROR("SSAO Framebuffer not complete!");
		}
		pipeline.ssao.ssaoBlurFBO.unbind();

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
			pipeline.ssao.ssaoKernel.push_back(sample);
		}

		// Генерируем текстуру шума
		std::vector<uint8_t> ssaoNoise;
		for (unsigned int i = 0; i < 16; i++) {
			//glm::vec3 noise(randomFloats(generator) * 2.0 - 1.0, randomFloats(generator) * 2.0 - 1.0, 0.0f); // поворот вокруг z-оси (в касательном пространстве)
			ssaoNoise.push_back(randomFloats(generator) * 2.0 - 1.0);
			ssaoNoise.push_back(randomFloats(generator) * 2.0 - 1.0);
			ssaoNoise.push_back(0.0);
		}

		pipeline.ssao.noiseTexture = RESOURCES::TextureLoader::CreateFromMemory(&ssaoNoise[0], 4, 4, RESOURCES::TextureFiltering::NEAREST, RESOURCES::TextureFiltering::NEAREST, false);
		pipeline.ssao.noiseTexture->setWrapType(RESOURCES::TextureWrap::REPEAT, RESOURCES::TextureWrap::REPEAT);

		shadersMap["ssao"]->bind();
		shadersMap["ssao"]->setUniformInt("gPosition", 0);
		shadersMap["ssao"]->setUniformInt("gNormal", 1);
		shadersMap["ssao"]->setUniformInt("texNoise", 2);
		shadersMap["ssaoBlur"]->bind();
		shadersMap["ssaoBlur"]->setUniformInt("ssaoInput", 0);
		shadersMap["ssaoBlur"]->unbind();
		shadersMap["ssao"]->unbind();
	}

	{//ibl
		// PBR: настройка фреймбуфера
		FrameBuffer captureFBO;
		DepthBuffer captureRBO(512, 512, DepthBuffer::Format::DEPTH_COMPONENT24);
		captureFBO.bind();
		captureFBO.attachDepth(captureRBO);

		RESOURCES::stbiSetFlipVerticallyOnLoad(true);
		int width, height, nrComponents;
		float* data = RESOURCES::stbiLoadf("C:\\Projects\\SimpleEngine\\CGCourse\\CGCourse\\Assets\\Engine\\textures\\hdr\\newport_loft.hdr", &width, &height, &nrComponents, 0);

		auto hdrTexture = RESOURCES::TextureLoader::CreateFromFileFloat("textures\\hdr\\newport_loft.hdr");

		// PBR: настройка кубической карты для рендеринга и прикрепления к фреймбуферу
		auto envCubemap = RESOURCES::TextureLoader::CreateCubeMap(512, 512, RESOURCES::TextureFiltering::LINEAR_MIPMAP_LINEAR, RESOURCES::TextureFiltering::LINEAR);

		// PBR: установка матриц проекции и вида для захвата данных по всем 6 направлениям граней кубической карты
		auto captureProjection = MATHGL::Matrix4::CreatePerspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
		std::vector<MATHGL::Matrix4> captureViews =
		{
			MATHGL::Matrix4::CreateView(MATHGL::Vector3(0.0f, 0.0f, 0.0f), MATHGL::Vector3(1.0f,  0.0f,  0.0f), MATHGL::Vector3(0.0f, -1.0f,  0.0f)),
			MATHGL::Matrix4::CreateView(MATHGL::Vector3(0.0f, 0.0f, 0.0f), MATHGL::Vector3(-1.0f,  0.0f,  0.0f),MATHGL::Vector3(0.0f, -1.0f,  0.0f)),
			MATHGL::Matrix4::CreateView(MATHGL::Vector3(0.0f, 0.0f, 0.0f), MATHGL::Vector3(0.0f,  1.0f,  0.0f), MATHGL::Vector3(0.0f,  0.0f,  1.0f)),
			MATHGL::Matrix4::CreateView(MATHGL::Vector3(0.0f, 0.0f, 0.0f), MATHGL::Vector3(0.0f, -1.0f,  0.0f), MATHGL::Vector3(0.0f,  0.0f, -1.0f)),
			MATHGL::Matrix4::CreateView(MATHGL::Vector3(0.0f, 0.0f, 0.0f), MATHGL::Vector3(0.0f,  0.0f,  1.0f), MATHGL::Vector3(0.0f, -1.0f,  0.0f)),
			MATHGL::Matrix4::CreateView(MATHGL::Vector3(0.0f, 0.0f, 0.0f), MATHGL::Vector3(0.0f,  0.0f, -1.0f), MATHGL::Vector3(0.0f, -1.0f,  0.0f))
		};

		// PBR: конвертирование равнопромежуточной HDR-карты окружения в кубическую
		shadersMap["equirectangularToCubemapShader"]->bind();
		shadersMap["equirectangularToCubemapShader"]->setUniformInt("equirectangularMap", 0);
		shadersMap["equirectangularToCubemapShader"]->setUniformMat4("projection", captureProjection);
		hdrTexture->bind(0);

		setViewPort(0, 0, 512, 512); // не забудьте настроить видовой экран в соответствии с размерами захвата
		captureFBO.bind();
		for (unsigned int i = 0; i < 6; ++i) {
			shadersMap["equirectangularToCubemapShader"]->setUniformMat4("view", captureViews[i]);
			captureFBO.attachCubeMapSide(*envCubemap, i);
			//glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, envCubemap, 0);
			BaseRender::clear(true, true, false);

			renderCube();
		}
		captureFBO.unbind();

		// Далее позволим OpenGL сгенерировать мипмап-карты (для борьбы с артефактами в виде визуальных точек)
		envCubemap->generateMipmaps();
	
		// PBR: создаем кубическую карту облученности, и приводим размеры захвата FBO к размерам карты облученности

		pipeline.ibl.irradianceMap = RESOURCES::TextureLoader::CreateCubeMap(32, 32);

		captureFBO.bind();
		captureRBO.init(32, 32, DepthBuffer::Format::DEPTH_COMPONENT24);

		// PBR: решаем диффузный интеграл, применяя операцию свертки для создания кубической карты облученности
		shadersMap["irradianceShader"]->bind();
		shadersMap["irradianceShader"]->setUniformInt("environmentMap", 0);
		shadersMap["irradianceShader"]->setUniformMat4("projection", captureProjection);
		envCubemap->bind(0);

		setViewPort(0, 0, 32, 32); // не забудьте настроить видовой экран на размеры захвата
		captureFBO.bind();
		for (unsigned int i = 0; i < 6; ++i) {
			shadersMap["irradianceShader"]->setUniformMat4("view", captureViews[i]);
			captureFBO.attachCubeMapSide(*pipeline.ibl.irradianceMap, i);
			//glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, irradianceMap, 0);
			BaseRender::clear(true, true, false);
			renderCube();
		}
		captureFBO.unbind();

		// PBR: создаем префильтрованную кубическую карту, и приводим размеры захвата FBO к размерам префильтрованной карты
		pipeline.ibl.prefilterMap = RESOURCES::TextureLoader::CreateCubeMap(128, 128, RESOURCES::TextureFiltering::LINEAR_MIPMAP_LINEAR, RESOURCES::TextureFiltering::LINEAR);

		// Генерируем мипмап-карты для кубической карты, OpenGL автоматически выделит нужное количество памяти
		pipeline.ibl.prefilterMap->generateMipmaps();
		//glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

		// PBR: применяем симуляцию квази Монте-Карло для освещения окружающей среды, чтобы создать префильтрованную (кубическую)карту
		shadersMap["prefilterShader"]->bind();
		shadersMap["prefilterShader"]->setUniformInt("environmentMap", 0);
		shadersMap["prefilterShader"]->setUniformMat4("projection", captureProjection);
		envCubemap->bind(0);

		captureFBO.bind();
		unsigned int maxMipLevels = 5;
		for (unsigned int mip = 0; mip < maxMipLevels; ++mip) {
			// Изменяем размеры фреймбуфера в соответствии с размерами мипмап-карты
			unsigned int mipWidth = 128 * std::pow(0.5, mip);
			unsigned int mipHeight = 128 * std::pow(0.5, mip);
			captureRBO.init(mipWidth, mipHeight, DepthBuffer::Format::DEPTH_COMPONENT24);
			setViewPort(0, 0, mipWidth, mipHeight);

			float roughness = (float)mip / (float)(maxMipLevels - 1);
			shadersMap["prefilterShader"]->setUniformFloat("roughness", roughness);
			for (unsigned int i = 0; i < 6; ++i) {
				shadersMap["prefilterShader"]->setUniformMat4("view", captureViews[i]);
				captureRBO.attachCubeMapSide(*pipeline.ibl.prefilterMap, i, mip);

				BaseRender::clear(true, true, false);
				renderCube();
			}
		}
		captureFBO.unbind();

		// PBR: генерируем 2D LUT-текстуру при помощи используемых уравнений BRDF
		pipeline.ibl.brdfLUTTexture = RESOURCES::TextureLoader::CreateEmpty(512, 512);

		// Убеждаемся, что режим наложения задан как GL_CLAMP_TO_EDGE
		pipeline.ibl.brdfLUTTexture->setFilter(RESOURCES::TextureFiltering::LINEAR, RESOURCES::TextureFiltering::LINEAR);
		pipeline.ibl.brdfLUTTexture->setWrapType(RESOURCES::TextureWrap::CLAMP_TO_EDGE, RESOURCES::TextureWrap::CLAMP_TO_EDGE);


		// Затем переконфигурируем захват объекта фреймбуфера и рендерим экранный прямоугольник с использованием BRDF-шейдера
		captureFBO.bind();
		captureRBO.init(512, 512, DepthBuffer::Format::DEPTH_COMPONENT24);
		captureFBO.attachTexture(*pipeline.ibl.brdfLUTTexture, Attachment::COLOR_ATTACHMENT0);

		setViewPort(0, 0, 512, 512);
		shadersMap["brdfShader"]->bind();
		BaseRender::clear(true, true, false);
		renderQuad();
		
		captureFBO.unbind();

		glViewport(0, 0, screenRes.x, screenRes.y);
	}


	{//motion blur
		pipeline.motionBlur.motionBlurTextures.resize(4);
		for (auto i = 0; i < 4; i++) {
			pipeline.motionBlur.motionBlurTextures[i] = RESOURCES::TextureLoader::CreateEmpty(screenRes.x, screenRes.y);
			pipeline.motionBlur.motionBlurTextures[i]->setFilter(RESOURCES::TextureFiltering::LINEAR, RESOURCES::TextureFiltering::LINEAR);
		}
	}
	
	//hdr
	// Конфигурируем фреймбуфер типа с плавающей точкой

	pipeline.finalFBOBeforePostprocessing.bind();
	// Создаем 2 цветовых фреймбуфера типа с плавающей точкой (первый - для обычного рендеринга, другой - для граничных значений яркости)

	pipeline.finalTextureBeforePostprocessing = std::make_shared<RESOURCES::Texture>(screenRes.x, screenRes.y); //colorBuffer0
	pipeline.bloom.brightTexture = std::make_shared<RESOURCES::Texture>(screenRes.x, screenRes.y);  //colorBuffer1
	pipeline.finalFBOBeforePostprocessing.attachTexture(*pipeline.finalTextureBeforePostprocessing, Attachment::COLOR_ATTACHMENT0);
	
	// Создание буфера глубины (рендербуфер)
	DepthBuffer rboDepth(screenRes.x, screenRes.y);
	pipeline.finalFBOBeforePostprocessing.attachDepth(rboDepth);
	//glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboDepth);

	// Сообщаем OpenGL, какой прикрепленный цветовой буфер мы будем использовать для рендеринга
	pipeline.finalFBOBeforePostprocessing.setOupbutBuffers({Attachment::COLOR_ATTACHMENT0, Attachment::COLOR_ATTACHMENT1});

	// Проверяем готовность фреймбуфера
	if (pipeline.finalFBOBeforePostprocessing.getStatus() != FrameBufferStatus::FRAMEBUFFER_COMPLETE) {
		LOG_ERROR("SSAO Framebuffer not complete!");
	}
	pipeline.finalFBOBeforePostprocessing.unbind();

	// ping-pong-фреймбуфер для размытия
	
	for (unsigned int i = 0; i < 2; i++) {
		//glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBO[i]);
		pipeline.bloom.pingpongFBO[i].bind();
		pipeline.bloom.pingpongColorbuffers[i] = RESOURCES::TextureLoader::CreateEmpty(screenRes.x, screenRes.y);
		pipeline.bloom.pingpongColorbuffers[i]->setFilter(RESOURCES::TextureFiltering::LINEAR, RESOURCES::TextureFiltering::LINEAR);
		pipeline.bloom.pingpongColorbuffers[i]->setWrapType(RESOURCES::TextureWrap::CLAMP_TO_EDGE, RESOURCES::TextureWrap::CLAMP_TO_EDGE);
		pipeline.bloom.pingpongFBO[i].attachTexture(*pipeline.bloom.pingpongColorbuffers[i]);
		if (pipeline.bloom.pingpongFBO[i].getStatus() != FrameBufferStatus::FRAMEBUFFER_COMPLETE) {
			LOG_ERROR("SSAO Framebuffer not complete!");
		}
	}
	

	shadersMap["hdr"]->bind();
	shadersMap["hdr"]->setUniformInt("u_Scene", 0);
	shadersMap["hdr"]->unbind();


	shadersMap["blur"]->bind();
	shadersMap["blur"]->setUniformInt("image", 0);

	shadersMap["motionBlur"]->bind();
	shadersMap["motionBlur"]->setUniformInt("u_SceneBuffer0", 0);
	shadersMap["motionBlur"]->setUniformInt("u_SceneBuffer1", 1);
	shadersMap["motionBlur"]->setUniformInt("u_SceneBuffer2", 2);
	shadersMap["motionBlur"]->setUniformInt("u_SceneBuffer3", 3);

	shadersMap["fxaa"]->bind();
	shadersMap["fxaa"]->setUniformInt("u_Scene", 0);

	shadersMap["bright"]->bind();
	shadersMap["bright"]->setUniformInt("u_Scene", 0);



}

void Renderer::renderSkybox() {
	//SKYBOX
	auto currentScene = context.sceneManager->getCurrentScene();
	auto& skyboxObj = currentScene->getSkybox();

	auto skyboxMat = skyboxObj.getComponent<ECS::MaterialRenderer>()->getMaterials()[0];
	skyboxMat->getShader()->bind();
	GLint OldCullFaceMode;
	glGetIntegerv(GL_CULL_FACE_MODE, &OldCullFaceMode);
	GLint OldDepthFuncMode;
	glGetIntegerv(GL_DEPTH_FUNC, &OldDepthFuncMode);
	glCullFace(GL_FRONT);
	glDepthFunc(GL_LEQUAL);

	auto mainCameraComponent = currentScene->findMainCamera();
	auto& camera = mainCameraComponent->getCamera();
	auto v = camera.getViewMatrix();
	auto p = camera.getProjectionMatrix();

	auto pos = mainCameraComponent->obj.transform->getLocalPosition();
	auto rot = MATHGL::Quaternion(MATHGL::Vector3(0, 0, 0));
	auto scl = MATHGL::Vector3(1, 1, 1);

	auto m = MATHGL::Matrix4::Translation(pos) * MATHGL::Quaternion::ToMatrix4(MATHGL::Quaternion::Normalize(rot)) * MATHGL::Matrix4::Scaling(scl);

	skyboxMat->getShader()->setUniformMat4("gWVP", p * v * m);
	//currentScene->getSkyboxTexture().bind(GL_TEXTURE0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, currentScene->getSkyboxTexture().id);

	auto mr = skyboxObj.getComponent<ECS::ModelRenderer>();
	auto mm = mr->getModel()->getMeshes();
	for (auto m : mm)
		draw(*m, PrimitiveMode::TRIANGLES, 1);

	glCullFace(OldCullFaceMode);
	glDepthFunc(OldDepthFuncMode);
	skyboxMat->getShader()->unbind();
}


void Renderer::renderScene() {
	auto screenRes = RESOURCES::ServiceManager::Get<WINDOW_SYSTEM::Window>().getSize();
	if (auto currentScene = context.sceneManager->getCurrentScene()) {
		//grass
		auto grass = currentScene->findObjectByName("Grass");
		auto m = grass->getComponent<KUMA::ECS::MaterialRenderer>();
		m->getMaterials()[0]->getUniformsData()["fTimePassed"] = KUMA::TIME::Timer::instance()->getTimeSinceStart();

		if (auto mainCameraComponent = currentScene->findMainCamera()) {
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
			camera.cacheMatrices(winWidth, winHeight, cameraPosition, cameraRotation);

			updateEngineUBO(*mainCameraComponent);

			BaseRender::clear(true, true, false);
			std::tie(opaqueMeshesForward, transparentMeshesForward, opaqueMeshesDeferred, transparentMeshesDeferred) =
				currentScene->findDrawables(cameraPosition, camera, nullptr, emptyMaterial);

			prepareDirLightShadowMap();
			//prepareSpotLightShadowMap();
			//preparePointLightShadowMap();


			pipeline.finalFBOBeforePostprocessing.bind();
			BaseRender::clear(true, true, false);

			//TODO: где-то тут прокинуть текстуры для IBL

			uint8_t glState = fetchGLState();
			applyStateMask(glState);
			{//deferred
				// 1. Геометрический проход: выполняем рендеринг геометрических/цветовых данных сцены в g-буфер
				pipeline.deferredRender.gBuffer.bind();
				BaseRender::clear(true, true, false);
				shadersMap["deferredGBuffer"]->bind();

				for (const auto& [distance, p_toDraw] : opaqueMeshesDeferred) {
					drawGBuffer(*p_toDraw.mesh, *p_toDraw.material, &p_toDraw.world, shadersMap["deferredGBuffer"]);
				}
				for (const auto& [distance, p_toDraw] : transparentMeshesDeferred) {
					drawGBuffer(*p_toDraw.mesh, *p_toDraw.material, &p_toDraw.world, shadersMap["deferredGBuffer"]);
				}

				shadersMap["deferredGBuffer"]->unbind();

				{//ssao
					// 2. Генерируем текстуру для SSAO
					pipeline.ssao.ssaoFBO.bind();
					BaseRender::clear(true, false, false);
					shadersMap["ssao"]->bind();

					// Посылаем ядро + поворот 
					for (unsigned int i = 0; i < 64; ++i)
						shadersMap["ssao"]->setUniformVec3("samples[" + std::to_string(i) + "]", pipeline.ssao.ssaoKernel[i]);
					pipeline.deferredRender.gPosition->bind(0);
					pipeline.deferredRender.gNormal->bind(1);
					pipeline.ssao.noiseTexture->bind(2);
					renderQuad();
					pipeline.ssao.ssaoFBO.unbind();

					// 3. Размываем SSAO-текстуру, чтобы убрать шум
					pipeline.ssao.ssaoBlurFBO.bind();
					BaseRender::clear(true, false, false);
					shadersMap["ssaoBlur"]->bind();
					pipeline.ssao.ssaoColorBuffer->bind(0);
					renderQuad();
					pipeline.ssao.ssaoBlurFBO.unbind();
				}


				pipeline.finalFBOBeforePostprocessing.bind();

				// 2. Проход освещения: вычисление освещение, перебирая попиксельно экранный прямоугольник, используя содержимое g-буфера
				BaseRender::clear(true, true, false);
				shadersMap["deferredLightning"]->bind();

				pipeline.deferredRender.gPosition->bind(0);
				pipeline.deferredRender.gNormal->bind(1);
				pipeline.deferredRender.gAlbedoSpec->bind(2);

				{//ibl TODO: (send to forward shaders too)
					shadersMap["deferredLightning"]->setUniformInt("irradianceMap", 7);
					shadersMap["deferredLightning"]->setUniformInt("prefilterMap", 8);
					shadersMap["deferredLightning"]->setUniformInt("brdfLUT", 9);
					// Связываем предварительно вычисленные IBL-данные
					pipeline.ibl.irradianceMap->bind(7);
					pipeline.ibl.prefilterMap->bind(8);
					pipeline.ibl.brdfLUTTexture->bind(9);
				}

				//// Рендерим прямоугольник
				renderQuad();
				shadersMap["deferredLightning"]->unbind();
			}
			// 2.5. Копируем содержимое буфера глубины (геометрический проход) в буфер глубины заданного по умолчанию фреймбуфера
			FrameBuffer::CopyDepth(pipeline.deferredRender.gBuffer, pipeline.finalFBOBeforePostprocessing, screenRes.x, screenRes.y);
			pipeline.finalFBOBeforePostprocessing.bind();

			renderSkybox();

			renderScene(nullptr);
			{//prepare textures for post processing
				//bright texture for bloom
				swapBuffers[0].bind();
				swapBuffers[0].attachTexture(*pipeline.bloom.brightTexture);
				shadersMap["bright"]->bind();
				pipeline.finalTextureBeforePostprocessing->bind(0);
				renderQuad();
				shadersMap["bright"]->unbind();
				swapBuffers[0].attachTexture(*swapTextures[0]);
				swapBuffers[0].unbind();

				//texture for god rays
				swapBuffers[0].bind();
				swapBuffers[0].attachTexture(*pipeline.godRays.godRaysTexture);
				BaseRender::clear(true, true, false);
				shadersMap["godRaysTexture"]->bind();
				pipeline.finalTextureBeforePostprocessing->bind(0);
				shadersMap["godRaysTexture"]->setUniformVec3("u_Color", MATHGL::Vector3(0.0f, 0.0f, 0.0f));

				for (const auto& [distance, drawable] : opaqueMeshesForward) {
					drawDrawable(drawable, shadersMap["godRaysTexture"]);
				}
				for (const auto& [distance, drawable] : opaqueMeshesDeferred) {
					drawDrawable(drawable, shadersMap["godRaysTexture"]);
				}
				shadersMap["godRaysTexture"]->setUniformVec3("u_Color", MATHGL::Vector3(1.0f, 1.0f, 1.0f));
				for (auto& light : ECS::ComponentManager::getInstance()->getAllDirectionalLights()) {
					Drawable d;
					d.mesh = sphere->meshes[0];
					d.material = emptyMaterial;
					d.material->getUniformsData()["u_UseBone"] = false;
					d.world = light->obj.transform->getTransform().getWorldMatrix();
					drawDrawable(d, shadersMap["godRaysTexture"]);
				}
				shadersMap["godRaysTexture"]->unbind();
				swapBuffers[0].attachTexture(*swapTextures[0]);
				swapBuffers[0].unbind();
			}

			{//bloom
				// 2. Размываем яркие фрагменты с помощью двухпроходного размытия по Гауссу
				bool horizontal = true, firstIteration = true;
				unsigned int amount = 10;
				shadersMap["blur"]->bind();
				for (unsigned int i = 0; i < amount; i++) {
					pipeline.bloom.pingpongFBO[horizontal].bind();
					shadersMap["blur"]->setUniformInt("horizontal", horizontal);
					firstIteration ? pipeline.bloom.brightTexture->bind(0) : pipeline.bloom.pingpongColorbuffers[!horizontal]->bind(0);

					renderQuad();
					horizontal = !horizontal;
					if (firstIteration) {
						firstIteration = false;
					}
				}

				swapBuffers[currentSwapBuffer].bind();
				BaseRender::clear(true, true, false);
				shadersMap["bloom"]->bind();
				pipeline.finalTextureBeforePostprocessing->bind(0);
				pipeline.bloom.pingpongColorbuffers[!horizontal]->bind(1);
				shadersMap["bloom"]->setUniformInt("u_UseBloom", true);
				renderQuad();
				shadersMap["bloom"]->unbind();
				swapBuffers[currentSwapBuffer].unbind();
				currentSwapBuffer = !currentSwapBuffer;
			}

			{//god rays
				auto dirLights = ECS::ComponentManager::getInstance()->getAllDirectionalLights();
				if (dirLights.size() > 0) {
					swapBuffers[currentSwapBuffer].bind();
			
					BaseRender::clear(true, true, false);
					shadersMap["godRays"]->bind();
					swapTextures[!currentSwapBuffer]->bind(0);
					pipeline.godRays.godRaysTexture->bind(1);
					shadersMap["godRays"]->setUniformInt("u_UseGodRays", true);
					shadersMap["godRays"]->setUniformVec3("u_SunPos", dirLights[0]->obj.transform->getLocalPosition());
					renderQuad();
					shadersMap["godRays"]->unbind();
			
					swapBuffers[currentSwapBuffer].unbind();
					currentSwapBuffer = !currentSwapBuffer;
				}
			}

			{//motion blur
				swapBuffers[currentSwapBuffer].bind();

				//update textures
				if (!pipeline.motionBlur.isInit) {
					pipeline.motionBlur.isInit = true;
					for (auto i = 0; i < pipeline.motionBlur.motionBlurTextures.size(); i++) {
						RESOURCES::Texture::CopyTexture(*swapTextures[!currentSwapBuffer], *pipeline.motionBlur.motionBlurTextures[i]);
					}
				}
				else {
					auto front = pipeline.motionBlur.motionBlurTextures[0];
					pipeline.motionBlur.motionBlurTextures.erase(pipeline.motionBlur.motionBlurTextures.begin());
					pipeline.motionBlur.motionBlurTextures.push_back(front);
					RESOURCES::Texture::CopyTexture(*swapTextures[!currentSwapBuffer],
						*pipeline.motionBlur.motionBlurTextures[pipeline.motionBlur.motionBlurTextures.size() - 1]);
				}
				shadersMap["motionBlur"]->bind();
				pipeline.motionBlur.motionBlurTextures[0]->bind(0);
				pipeline.motionBlur.motionBlurTextures[1]->bind(1);
				pipeline.motionBlur.motionBlurTextures[2]->bind(2);
				pipeline.motionBlur.motionBlurTextures[3]->bind(3);

				shadersMap["motionBlur"]->setUniformInt("u_UseMotionBlur", true);
				renderQuad();
				shadersMap["motionBlur"]->unbind();

				swapBuffers[currentSwapBuffer].unbind();
				currentSwapBuffer = !currentSwapBuffer;
			}


			{//fxaa
				swapBuffers[currentSwapBuffer].bind();

				shadersMap["fxaa"]->bind();
				swapTextures[!currentSwapBuffer]->bind();

				shadersMap["fxaa"]->setUniformInt("u_UseFXAA", true);
				renderQuad();
				shadersMap["fxaa"]->unbind();

				swapBuffers[currentSwapBuffer].unbind();
				currentSwapBuffer = !currentSwapBuffer;
			}

			
			BaseRender::clear(true, true, false);
			shadersMap["hdr"]->bind();

			FrameBuffer::Unbind();
			BaseRender::clear(true, true, false);
			swapTextures[!currentSwapBuffer]->bind();
			//textureForGodRays.bind();
			shadersMap["hdr"]->setUniformInt("u_UseHDR", pipeline.hdr.isEnabled);
			shadersMap["hdr"]->setUniformFloat("u_Exposure", pipeline.hdr.exposure);
			shadersMap["hdr"]->setUniformFloat("u_Gamma", pipeline.hdr.gamma);
			//hdrShader->setUniformVec3("sunPos", {-20.0f, 40.0f, 10.0f});
			renderQuad();
			shadersMap["hdr"]->unbind();

			applyStateMask(glState);
		}
		else {
			setClearColor(0.0f, 0.0f, 0.0f);
			BaseRender::clear(true, true, false);
		}
	}
}


void Renderer::updateEngineUBO(ECS::CameraComponent& p_mainCamera) {
	size_t offset = sizeof(MATHGL::Matrix4); // We skip the model matrix (Which is a special case, modified every draw calls)
	auto& camera = p_mainCamera.getCamera();

	engineUBO->setSubData(MATHGL::Matrix4::Transpose(camera.getViewMatrix()), std::ref(offset));
	engineUBO->setSubData(MATHGL::Matrix4::Transpose(camera.getProjectionMatrix()), std::ref(offset));
	engineUBO->setSubData(p_mainCamera.obj.transform->getWorldPosition(), std::ref(offset));
}

void Renderer::updateLights(SCENE_SYSTEM::Scene& scene) {
	auto lightMatrices = scene.findLightData();
	lightSSBO->SendBlocks<LightOGL>(lightMatrices.data(), lightMatrices.size() * sizeof(LightOGL));
}

void Renderer::updateLightsInFrustum(SCENE_SYSTEM::Scene& scene, const Frustum& frustum) {
	auto lightMatrices = scene.findLightDataInFrustum(frustum);
	lightSSBO->SendBlocks<LightOGL>(lightMatrices.data(), lightMatrices.size() * sizeof(LightOGL));
}
Renderer::Renderer(GL_SYSTEM::GlManager& _driver, CORE_SYSTEM::Core& context) :
	BaseRender(_driver), context(context),
	emptyTexture(RESOURCES::TextureLoader::CreateColor(
		(255 << 24) | (255 << 16) | (255 << 8) | 255,
		RESOURCES::TextureFiltering::NEAREST,
		RESOURCES::TextureFiltering::NEAREST,
		false
	)) {
	engineUBO = std::make_unique<RENDER::UniformBuffer>(
		sizeof(MATHGL::Matrix4) +
		sizeof(MATHGL::Matrix4) +
		sizeof(MATHGL::Matrix4) +
		sizeof(MATHGL::Vector3) +
		sizeof(float) +
		sizeof(MATHGL::Matrix4),
		0,
		0,
		RENDER::AccessSpecifier::STREAM_DRAW
		);

	lightSSBO = std::make_unique<RENDER::ShaderStorageBuffer>(RENDER::AccessSpecifier::STREAM_DRAW);
	lightSSBO->bind(0);

	init();

}

Renderer::~Renderer() {
	//RESOURCES::TextureLoader::Destroy(emptyTexture);
}

void Renderer::prepareDirLightShadowMap() {
	pipeline.dirLightsData.clear();
	for (auto& light : ECS::ComponentManager::getInstance()->getAllDirectionalLights()) {
		pipeline.depthMapFBO.attachTexture(*light->shadowMap, Attachment::DEPTH_ATTACHMENT);
		glDrawBuffer(GL_NONE);
		glReadBuffer(GL_NONE);
		pipeline.depthMapFBO.unbind();


		float nearPlane = 1.0f, farPlane = 100.0f;
		auto lightProjection = MATHGL::Matrix4::CreateOrthographic(-50.0f, 50.0f, -50.0f, 50.0f, nearPlane, farPlane);
		auto lightView = MATHGL::Matrix4::CreateView(light->obj.transform->getLocalPosition(), MATHGL::Vector3(0.0f, 0.0f, 0.0f), MATHGL::Vector3(0.0, 1.0, 0.0));
		auto lightSpaceMatrix = lightProjection * lightView;

		pipeline.dirLightsData.push_back(DirLightData{light->shadowMap->getId(), lightSpaceMatrix, light->obj.transform->getLocalPosition()});

		shadersMap["simpleDepthShader"]->bind();
		shadersMap["simpleDepthShader"]->setUniformMat4("u_LightSpaceMatrix", lightSpaceMatrix);

		auto res = getShadowMapResolution();
		setViewPort(0, 0, static_cast<unsigned>(res.x), static_cast<unsigned>(res.y));

		pipeline.depthMapFBO.bind();
		clearDepth();
		renderDirShadowMap();
		pipeline.depthMapFBO.unbind();

		auto screenRes = RESOURCES::ServiceManager::Get<WINDOW_SYSTEM::Window>().getSize();
		setViewPort(0, 0, screenRes.x, screenRes.y);
		BaseRender::clear(true, true, false);

		//Support only one shadow map for dir light
		break;
	}
}


void Renderer::prepareSpotLightShadowMap() {
	pipeline.spotLightsData.clear();
	for (auto& light : ECS::ComponentManager::getInstance()->getAllSpotLights()) {
		pipeline.depthMapFBO.attachTexture(*light->shadowMap, Attachment::DEPTH_ATTACHMENT);
		glDrawBuffer(GL_NONE);
		glReadBuffer(GL_NONE);
		pipeline.depthMapFBO.unbind();


		float nearPlane = 1.0f, farPlane = 100.0f;
		auto res = getShadowMapResolution();
		auto lightProjection = MATHGL::Matrix4::CreatePerspective(TO_RADIANS(45.0f), res.x / res.y, nearPlane, farPlane);
		auto lightView = MATHGL::Matrix4::CreateView(light->obj.transform->getLocalPosition(), MATHGL::Vector3(0.0f, 0.0f, 0.0f), MATHGL::Vector3(0.0, 1.0, 0.0));

		auto lightSpaceMatrix = lightProjection * lightView;

		pipeline.spotLightsData.push_back(SpotLightData{light->shadowMap->getId(), lightSpaceMatrix});

		// Рендеринг сцены глазами источника света
		shadersMap["simpleDepthShader"]->bind();
		shadersMap["simpleDepthShader"]->setUniformMat4("u_LightSpaceMatrix", lightSpaceMatrix);

		setViewPort(0, 0, static_cast<unsigned>(res.x), static_cast<unsigned>(res.y));
		pipeline.depthMapFBO.bind();
		clearDepth();
		renderScene(shadersMap["simpleDepthShader"]);
		pipeline.depthMapFBO.unbind();

		// Сброс настроек области просмотра
		auto screenRes = RESOURCES::ServiceManager::Get<WINDOW_SYSTEM::Window>().getSize();
		setViewPort(0, 0, screenRes.x, screenRes.y);
		BaseRender::clear(true, true, false);
	}
}

void Renderer::preparePointLightShadowMap() {
	pipeline.pointLightsData.clear();
	auto res = getShadowMapResolution();
	for (auto& light : ECS::ComponentManager::getInstance()->getAllPointLights()) {
		pipeline.depthMapFBO.attachCubeMap(*light->DepthMap, Attachment::DEPTH_ATTACHMENT);
		glDrawBuffer(GL_NONE);
		glReadBuffer(GL_NONE);
		pipeline.depthMapFBO.unbind();

		// Рендер
		setClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		BaseRender::clear(true, true, false);

		// 0. Создаем матрицы трансформации кубической карты глубины
		float nearPlane = 1.0f;
		float farPlane = 25.0f;
		auto shadowProj = MATHGL::Matrix4::CreatePerspective(TO_RADIANS(90.0f), res.x / res.y, nearPlane, farPlane);

		std::vector<MATHGL::Matrix4> shadowTransforms;
		shadowTransforms.push_back(shadowProj * MATHGL::Matrix4::CreateView(light->obj.transform->getLocalPosition(), light->obj.transform->getLocalPosition() + MATHGL::Vector3(1.0f, 0.0f, 0.0f), MATHGL::Vector3(0.0f, -1.0f, 0.0f)));
		shadowTransforms.push_back(shadowProj * MATHGL::Matrix4::CreateView(light->obj.transform->getLocalPosition(), light->obj.transform->getLocalPosition() + MATHGL::Vector3(-1.0f, 0.0f, 0.0f), MATHGL::Vector3(0.0f, -1.0f, 0.0f)));
		shadowTransforms.push_back(shadowProj * MATHGL::Matrix4::CreateView(light->obj.transform->getLocalPosition(), light->obj.transform->getLocalPosition() + MATHGL::Vector3(0.0f, 1.0f, 0.0f), MATHGL::Vector3(0.0f, 0.0f, 1.0f)));
		shadowTransforms.push_back(shadowProj * MATHGL::Matrix4::CreateView(light->obj.transform->getLocalPosition(), light->obj.transform->getLocalPosition() + MATHGL::Vector3(0.0f, -1.0f, 0.0f), MATHGL::Vector3(0.0f, 0.0f, -1.0f)));
		shadowTransforms.push_back(shadowProj * MATHGL::Matrix4::CreateView(light->obj.transform->getLocalPosition(), light->obj.transform->getLocalPosition() + MATHGL::Vector3(0.0f, 0.0f, 1.0f), MATHGL::Vector3(0.0f, -1.0f, 0.0f)));
		shadowTransforms.push_back(shadowProj * MATHGL::Matrix4::CreateView(light->obj.transform->getLocalPosition(), light->obj.transform->getLocalPosition() + MATHGL::Vector3(0.0f, 0.0f, -1.0f), MATHGL::Vector3(0.0f, -1.0f, 0.0f)));

		// 1. Рендерим сцену в кубическую карту глубины
		setViewPort(0, 0, static_cast<unsigned>(res.x), static_cast<unsigned>(res.y));
		pipeline.depthMapFBO.bind();
		BaseRender::clear(false, true, false);
		shadersMap["pointShadowShader"]->bind();
		for (unsigned int i = 0; i < 6; ++i) {
			shadersMap["pointShadowShader"]->setUniformMat4("shadowMatrices[" + std::to_string(i) + "]", shadowTransforms[i]);
		}
		shadersMap["pointShadowShader"]->setUniformFloat("far_plane", farPlane);
		shadersMap["pointShadowShader"]->setUniformVec3("lightPos", light->obj.transform->getLocalPosition());

		//renderScene(shadersMap["pointShadowShader"]);
		for (const auto& [distance, drawable] : opaqueMeshesForward) {
			drawDirShadowMap(*drawable.mesh, *drawable.material, &drawable.world, shadersMap["pointShadowShader"]);
		}
		for (const auto& [distance, drawable] : opaqueMeshesDeferred) {
			drawDirShadowMap(*drawable.mesh, *drawable.material, &drawable.world, shadersMap["pointShadowShader"]);
		}
		pipeline.pointLightsData.push_back(PointLightData{light->DepthMap->id});
		pipeline.depthMapFBO.unbind();

		// Сброс настроек области просмотра
		auto screenRes = RESOURCES::ServiceManager::Get<WINDOW_SYSTEM::Window>().getSize();
		setViewPort(0, 0, screenRes.x, screenRes.y);
		BaseRender::clear(true, true, false);
	}
}


void Renderer::clear() const {
	glClear(clearMask);
}

void Renderer::clearDepth() const {
	glClear(GL_DEPTH_BUFFER_BIT);
}


void Renderer::renderScene(std::shared_ptr<RESOURCES::Shader> shader) {
	for (const auto& [distance, drawable] : opaqueMeshesForward) {
		if (shader)
			drawDrawable(drawable, shader);
		else
			drawDrawable(drawable);
	}
	for (const auto& [distance, drawable] : transparentMeshesForward) {
		if (shader)
			drawDrawable(drawable, shader);
		else
			drawDrawable(drawable);
	}
}


void Renderer::renderDirShadowMap() {
	for (const auto& [distance, drawable] : opaqueMeshesForward) {
		drawDirShadowMap(*drawable.mesh, *drawable.material, &drawable.world, shadersMap["simpleDepthShader"]);
	}
	for (const auto& [distance, drawable] : opaqueMeshesDeferred) {
		drawDirShadowMap(*drawable.mesh, *drawable.material, &drawable.world, shadersMap["simpleDepthShader"]);
	}
}



void Renderer::drawDrawable(const Drawable& p_toDraw) {
	drawMesh(*p_toDraw.mesh, *p_toDraw.material, &p_toDraw.world);
}

void Renderer::drawDrawable(const Drawable& p_toDraw, std::shared_ptr<RESOURCES::Shader> shader) {
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
		if (p_modelMatrix)
			modelMatrixSender(*p_modelMatrix);

		uint8_t stateMask = p_material.generateStateMask();
		applyStateMask(stateMask);

		{
			p_material.bind(emptyTexture, useTextures);
			p_material.getShader()->setUniformVec3("lightPos", pipeline.dirLightsData[0].pos);
			p_material.getShader()->setUniformMat4("u_LightSpaceMatrix", pipeline.dirLightsData[0].projMap);
			p_material.getShader()->setUniformInt("shadowMap", 5);
			glActiveTexture(GL_TEXTURE5);
			glBindTexture(GL_TEXTURE_2D, pipeline.dirLightsData[0].id);

			//point lights
			//auto i = 6u;
			//int cnt = 0;
			//for (auto& data : pipeline.pointLightsData) {
			//	if (cnt == 4) {
			//		break;
			//	}
			//	p_material.getShader()->setUniformInt("shadowMap", i);
			//	glActiveTexture(GL_TEXTURE0 + i);
			//	glBindTexture(GL_TEXTURE_CUBE_MAP, data.id);
			//}
		}
		if (animator) {
			auto transforms = animator->GetFinalBoneMatrices();
			for (int i = 0; i < transforms.size(); ++i) {
				MATHGL::Matrix4 m = MATHGL::Matrix4(
					transforms[i][0][0], transforms[i][0][1], transforms[i][0][2], transforms[i][0][3],
					transforms[i][1][0], transforms[i][1][1], transforms[i][1][2], transforms[i][1][3],
					transforms[i][2][0], transforms[i][2][1], transforms[i][2][2], transforms[i][2][3],
					transforms[i][3][0], transforms[i][3][1], transforms[i][3][2], transforms[i][3][3]
				);
				//material.getShader()->setUniformMat4("finalBonesMatrices[" + std::to_string(i) + "]", MATHGL::Matrix4::Transpose(m));
				glUniformMatrix4fv(glGetUniformLocation(p_material.getShader()->getId(), std::string("u_FinalBonesMatrices[" + std::to_string(i) + "]").c_str()), 1, GL_FALSE, &transforms[i][0][0]);
			}
		}
		draw(p_mesh, p_material, PrimitiveMode::TRIANGLES, p_material.getGPUInstances());
		p_material.unbind();
	}
}

void Renderer::drawDirShadowMap(RESOURCES::Mesh& p_mesh, Material& p_material, MATHGL::Matrix4 const* p_modelMatrix, std::shared_ptr<RESOURCES::Shader> shader) {
	if (p_material.getGPUInstances() > 0) {
		if (p_material.getUniformsData().count("castShadow") && !std::get<bool>(p_material.getUniformsData()["castShadow"])) {
			return;
		}
		if (p_modelMatrix) {
			modelMatrixSender(*p_modelMatrix);
		}
		uint8_t stateMask = p_material.generateStateMask();
		applyStateMask(stateMask);

		if (p_material.getUniformsData().count("u_UseBone")) {
			shader->setUniformInt("u_UseBone", std::get<bool>(p_material.getUniformsData()["u_UseBone"]));
		}
		if (animator) {
			auto transforms = animator->GetFinalBoneMatrices();
			for (int i = 0; i < transforms.size(); ++i) {
				MATHGL::Matrix4 m = MATHGL::Matrix4(
					transforms[i][0][0], transforms[i][0][1], transforms[i][0][2], transforms[i][0][3],
					transforms[i][1][0], transforms[i][1][1], transforms[i][1][2], transforms[i][1][3],
					transforms[i][2][0], transforms[i][2][1], transforms[i][2][2], transforms[i][2][3],
					transforms[i][3][0], transforms[i][3][1], transforms[i][3][2], transforms[i][3][3]
				);
				//material.getShader()->setUniformMat4("finalBonesMatrices[" + std::to_string(i) + "]", MATHGL::Matrix4::Transpose(m));
				glUniformMatrix4fv(glGetUniformLocation(shader ? shader->getId() : p_material.getShader()->getId(), std::string("u_FinalBonesMatrices[" + std::to_string(i) + "]").c_str()), 1, GL_FALSE, &transforms[i][0][0]);
			}
		}
		draw(p_mesh, p_material, PrimitiveMode::TRIANGLES, p_material.getGPUInstances());
	}
}

void Renderer::drawGBuffer(RESOURCES::Mesh& p_mesh, Material& p_material, MATHGL::Matrix4 const* p_modelMatrix, std::shared_ptr<RESOURCES::Shader> shader) {
	//if (p_material.hasShader() && p_material.getGPUInstances() > 0) {

	//if (p_material.getUniformsData().count("castShadow") && !std::get<bool>(p_material.getUniformsData()["u_UseBone"])) {
	//	return;
	//}
	if (p_modelMatrix) {
		modelMatrixSender(*p_modelMatrix);
	}
	uint8_t stateMask = p_material.generateStateMask();
	applyStateMask(stateMask);

	bool useTextures = true;
	//p_material.setShader(shader);
	p_material.bind(shader, emptyTexture, useTextures);

	if (p_material.getUniformsData().count("u_UseBone")) {
		shader->setUniformInt("u_UseBone", std::get<bool>(p_material.getUniformsData()["u_UseBone"]));
	}
	if (animator) {
		auto transforms = animator->GetFinalBoneMatrices();
		for (int i = 0; i < transforms.size(); ++i) {
			MATHGL::Matrix4 m = MATHGL::Matrix4(
				transforms[i][0][0], transforms[i][0][1], transforms[i][0][2], transforms[i][0][3],
				transforms[i][1][0], transforms[i][1][1], transforms[i][1][2], transforms[i][1][3],
				transforms[i][2][0], transforms[i][2][1], transforms[i][2][2], transforms[i][2][3],
				transforms[i][3][0], transforms[i][3][1], transforms[i][3][2], transforms[i][3][3]
			);
			//material.getShader()->setUniformMat4("finalBonesMatrices[" + std::to_string(i) + "]", MATHGL::Matrix4::Transpose(m));
			glUniformMatrix4fv(glGetUniformLocation(shader ? shader->getId() : p_material.getShader()->getId(), std::string("u_FinalBonesMatrices[" + std::to_string(i) + "]").c_str()), 1, GL_FALSE, &transforms[i][0][0]);
		}
	}
	draw(p_mesh, p_material, PrimitiveMode::TRIANGLES, p_material.getGPUInstances());

	//p_material.setShader(nullptr);
	//if (!isShadowDir)
	//	p_material.unbind();
	//}
}

void Renderer::registerModelMatrixSender(std::function<void(MATHGL::Matrix4)> p_modelMatrixSender) {
	modelMatrixSender = p_modelMatrixSender;
}

unsigned int cubeVAO = 0;
unsigned int cubeVBO = 0;
void renderCube() {
	// Инициализация (если необходимо)
	if (cubeVAO == 0) {
		float vertices [] = {
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

unsigned int quadVAO = 0;
unsigned int quadVBO;
void renderQuad() {
	if (quadVAO == 0) {
		float quadVertices [] = {
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
