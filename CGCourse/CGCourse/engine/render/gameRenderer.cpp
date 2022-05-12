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
#include "../resourceManager/textureManager.h"
#include "../utils/time/time.h"
#include "../resourceManager/resource/bone.h"

using namespace KUMA;
using namespace KUMA::RENDER;


GLuint depthBufferTexture;

//ssao
unsigned int ssaoFBO, ssaoBlurFBO;
unsigned int ssaoColorBuffer, ssaoColorBufferBlur;
std::vector<MATHGL::Vector3> ssaoKernel;
unsigned int noiseTexture;

//bloom
unsigned int pingpongFBO[2];
unsigned int pingpongColorbuffers[2];

//fog
std::shared_ptr<RESOURCES::Texture> tex3;

void renderQuad();
void renderCube();

//ibl
unsigned int irradianceMap;
unsigned int prefilterMap;
unsigned int brdfLUTTexture;


MATHGL::Vector2f Renderer::getShadowMapResolution() {
	switch (pipeline.shadowLightData.resolution) {
		case ShadowMapResolution::LOW: return MATHGL::Vector2f(512, 512);
		case ShadowMapResolution::MEDIUM: return MATHGL::Vector2f(1024, 1024);
		case ShadowMapResolution::HIGH: return MATHGL::Vector2f(2048, 2048);
	}
}

void Renderer::initShaders() {
	auto shaderLoader = RESOURCES::ServiceManager::Get<RESOURCES::ShaderLoader>();
	//shadows
	shadersMap["simpleDepthShader"]              = shaderLoader.createResource("Shaders/dirShadow.glsl");
	shadersMap["pointShadowShader"]              = shaderLoader.createResource("Shaders/pointShadow.glsl");

	//deferred
	shadersMap["deferredGBuffer"]                = shaderLoader.createResource("Shaders/deferredGBuffer.glsl");
	shadersMap["deferredLightning"]              = shaderLoader.createResource("Shaders/deferredLightning.glsl");

	//ssao
	shadersMap["ssao"]                           = shaderLoader.createResource("Shaders/ssao.glsl");
	shadersMap["ssaoBlur"]                       = shaderLoader.createResource("Shaders/ssaoBlur.glsl");

	//ibl
	shadersMap["equirectangularToCubemapShader"] = shaderLoader.createResource("Shaders/equirectangular_to_cubemap.glsl");
	shadersMap["irradianceShader"]               = shaderLoader.createResource("Shaders/irradiance_convolution.glsl");
	shadersMap["prefilterShader"]                = shaderLoader.createResource("Shaders/prefilter.glsl");
	shadersMap["brdfShader"]                     = shaderLoader.createResource("Shaders/brdf.glsl");

	//blum
	shadersMap["blur"]                           = shaderLoader.createResource("Shaders/blur.glsl");
	shadersMap["bloom"]                          = shaderLoader.createResource("Shaders/bloom.glsl");

	//motion blur
	shadersMap["motionBlur"]                     = shaderLoader.createResource("Shaders/motionBlur.glsl");

	//fxaa
	shadersMap["fxaa"]                           = shaderLoader.createResource("Shaders/fxaa.glsl");

	//good rays
	shadersMap["bright"]                         = shaderLoader.createResource("Shaders/bright.glsl");
	shadersMap["godRaysTexture"]                 = shaderLoader.createResource("Shaders/godRaysTexture.glsl");
	shadersMap["godRays"]                        = shaderLoader.createResource("Shaders/godRays.glsl");

	//hdr
	shadersMap["hdr"] = shaderLoader.createResource("Shaders/hdr.glsl");
}


void Renderer::init() {
	emptyMaterial = std::make_shared<RENDER::Material>();
	emptyMaterial->setShader(context.shaderManager.createResource("Shaders\\Unlit.glsl"));
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

	glGenTextures(1, &depthBufferTexture);
	glBindTexture(GL_TEXTURE_2D, depthBufferTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, 800, 600, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_INT, NULL);

	initShaders();

	auto screenRes = RESOURCES::ServiceManager::Get<WINDOW_SYSTEM::Window>().getSize();
	{//create swap buffers
		int i = 0;
		for (auto& buffer : swapBuffers) {
			buffer.bind();
			swapTextures[i] = std::make_shared<RESOURCES::Texture>(screenRes.x, screenRes.y);
			//texture.bindWithoutAttach();
			//texture.load(nullptr, screenRes.x, screenRes.y, 4, true, RESOURCES::TextureFormat::RGBA16F);
			swapTextures[i]->setFilter(RESOURCES::TextureFiltering::NEAREST, RESOURCES::TextureFiltering::NEAREST);
			//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			buffer.AttachTexture(*swapTextures[i], Attachment::COLOR_ATTACHMENT0);
			i++;
		}


		textureForGodRays.bindWithoutAttach();
		textureForGodRays.load(nullptr, screenRes.x, screenRes.y, 4, true, RESOURCES::TextureFormat::RGBA16F);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	}

	{//defered render
		// Конфигурирование g-буфера фреймбуфера

		glGenFramebuffers(1, &gBuffer);
		glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);

		// Цветовой буфер позиций
		glGenTextures(1, &gPosition);
		glBindTexture(GL_TEXTURE_2D, gPosition);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, screenRes.x, screenRes.y, 0, GL_RGBA, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, gPosition, 0);

		// Цветовой буфер нормалей
		glGenTextures(1, &gNormal);
		glBindTexture(GL_TEXTURE_2D, gNormal);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, screenRes.x, screenRes.y, 0, GL_RGBA, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, gNormal, 0);

		// Цветовой буфер значений цвета + отраженной составляющей
		glGenTextures(1, &gAlbedoSpec);
		glBindTexture(GL_TEXTURE_2D, gAlbedoSpec);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, screenRes.x, screenRes.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, gAlbedoSpec, 0);

		// Указываем OpenGL на то, в какой прикрепленный цветовой буфер (заданного фреймбуфера) мы собираемся выполнять рендеринг 
		unsigned int attachments[3] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2};
		glDrawBuffers(3, attachments);

		// Создаем и прикрепляем буфер глубины (рендербуфер)
		unsigned int rboDepth;
		glGenRenderbuffers(1, &rboDepth);
		glBindRenderbuffer(GL_RENDERBUFFER, rboDepth);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, screenRes.x, screenRes.y);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboDepth);

		// Проверяем готовность фреймбуфера
		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
			std::cout << "Framebuffer not complete!" << std::endl;
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

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
		glGenFramebuffers(1, &ssaoFBO);  glGenFramebuffers(1, &ssaoBlurFBO);
		glBindFramebuffer(GL_FRAMEBUFFER, ssaoFBO);
		// Цветовой буфер SSAO 
		glGenTextures(1, &ssaoColorBuffer);
		glBindTexture(GL_TEXTURE_2D, ssaoColorBuffer);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, screenRes.x, screenRes.y, 0, GL_RED, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ssaoColorBuffer, 0);
		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
			std::cout << "SSAO Framebuffer not complete!" << std::endl;

		// И стадия размытия
		glBindFramebuffer(GL_FRAMEBUFFER, ssaoBlurFBO);
		glGenTextures(1, &ssaoColorBufferBlur);
		glBindTexture(GL_TEXTURE_2D, ssaoColorBufferBlur);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, screenRes.x, screenRes.y, 0, GL_RED, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ssaoColorBufferBlur, 0);
		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
			std::cout << "SSAO Blur Framebuffer not complete!" << std::endl;
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

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
			ssaoKernel.push_back(sample);
		}

		// Генерируем текстуру шума
		std::vector<glm::vec3> ssaoNoise;
		for (unsigned int i = 0; i < 16; i++) {
			glm::vec3 noise(randomFloats(generator) * 2.0 - 1.0, randomFloats(generator) * 2.0 - 1.0, 0.0f); // поворот вокруг z-оси (в касательном пространстве)
			ssaoNoise.push_back(noise);
		}
		glGenTextures(1, &noiseTexture);
		glBindTexture(GL_TEXTURE_2D, noiseTexture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, 4, 4, 0, GL_RGB, GL_FLOAT, &ssaoNoise[0]);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

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
		unsigned int captureFBO;
		unsigned int captureRBO;
		glGenFramebuffers(1, &captureFBO);
		glGenRenderbuffers(1, &captureRBO);

		glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
		glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 512, 512);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, captureRBO);

		RESOURCES::stbiSetFlipVerticallyOnLoad(true);
		int width, height, nrComponents;
		float* data = RESOURCES::stbiLoadf("C:\\Projects\\SimpleEngine\\CGCourse\\CGCourse\\Assets\\Engine\\textures\\hdr\\newport_loft.hdr", &width, &height, &nrComponents, 0);
		unsigned int hdrTexture;
		if (data) {
			glGenTextures(1, &hdrTexture);
			glBindTexture(GL_TEXTURE_2D, hdrTexture);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, data); // обратите внимание, что мы указываем значение данных текстуры как float

			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

			RESOURCES::stbiImageFree(data);
		}

		// PBR: настройка кубической карты для рендеринга и прикрепления к фреймбуферу
		unsigned int envCubemap;
		glGenTextures(1, &envCubemap);
		glBindTexture(GL_TEXTURE_CUBE_MAP, envCubemap);
		for (unsigned int i = 0; i < 6; ++i) {
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, 512, 512, 0, GL_RGB, GL_FLOAT, nullptr);
		}
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR); // включаем сэмплирование префильтрованной мипмап-карты (для борьбы с артефактами в виде визуальных точек)
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

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
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, hdrTexture);

		glViewport(0, 0, 512, 512); // не забудьте настроить видовой экран в соответствии с размерами захвата
		glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
		for (unsigned int i = 0; i < 6; ++i) {
			shadersMap["equirectangularToCubemapShader"]->setUniformMat4("view", captureViews[i]);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, envCubemap, 0);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			renderCube();
		}
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		// Далее позволим OpenGL сгенерировать мипмап-карты (для борьбы с артефактами в виде визуальных точек)
		glBindTexture(GL_TEXTURE_CUBE_MAP, envCubemap);
		glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

		// PBR: создаем кубическую карту облученности, и приводим размеры захвата FBO к размерам карты облученности

		glGenTextures(1, &irradianceMap);
		glBindTexture(GL_TEXTURE_CUBE_MAP, irradianceMap);
		for (unsigned int i = 0; i < 6; ++i) {
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, 32, 32, 0, GL_RGB, GL_FLOAT, nullptr);
		}
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
		glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 32, 32);

		// PBR: решаем диффузный интеграл, применяя операцию свертки для создания кубической карты облученности
		shadersMap["irradianceShader"]->bind();
		shadersMap["irradianceShader"]->setUniformInt("environmentMap", 0);
		shadersMap["irradianceShader"]->setUniformMat4("projection", captureProjection);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, envCubemap);

		glViewport(0, 0, 32, 32); // не забудьте настроить видовой экран на размеры захвата
		glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
		for (unsigned int i = 0; i < 6; ++i) {
			shadersMap["irradianceShader"]->setUniformMat4("view", captureViews[i]);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, irradianceMap, 0);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			renderCube();
		}
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		// PBR: создаем префильтрованную кубическую карту, и приводим размеры захвата FBO к размерам префильтрованной карты

		glGenTextures(1, &prefilterMap);
		glBindTexture(GL_TEXTURE_CUBE_MAP, prefilterMap);
		for (unsigned int i = 0; i < 6; ++i) {
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, 128, 128, 0, GL_RGB, GL_FLOAT, nullptr);
		}
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR); // убеждаемся, что фильтр уменьшения задан как mip_linear 
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		// Генерируем мипмап-карты для кубической карты, OpenGL автоматически выделит нужное количество памяти
		glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

		// PBR: применяем симуляцию квази Монте-Карло для освещения окружающей среды, чтобы создать префильтрованную (кубическую)карту
		shadersMap["prefilterShader"]->bind();
		shadersMap["prefilterShader"]->setUniformInt("environmentMap", 0);
		shadersMap["prefilterShader"]->setUniformMat4("projection", captureProjection);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, envCubemap);

		glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
		unsigned int maxMipLevels = 5;
		for (unsigned int mip = 0; mip < maxMipLevels; ++mip) {
			// Изменяем размеры фреймбуфера в соответствии с размерами мипмап-карты
			unsigned int mipWidth = 128 * std::pow(0.5, mip);
			unsigned int mipHeight = 128 * std::pow(0.5, mip);
			glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
			glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, mipWidth, mipHeight);
			glViewport(0, 0, mipWidth, mipHeight);

			float roughness = (float)mip / (float)(maxMipLevels - 1);
			shadersMap["prefilterShader"]->setUniformFloat("roughness", roughness);
			for (unsigned int i = 0; i < 6; ++i) {
				shadersMap["prefilterShader"]->setUniformMat4("view", captureViews[i]);
				glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, prefilterMap, mip);

				glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
				renderCube();
			}
		}
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		// PBR: генерируем 2D LUT-текстуру при помощи используемых уравнений BRDF

		glGenTextures(1, &brdfLUTTexture);

		// Выделяем необходимое количество памяти для LUT-текстуры
		glBindTexture(GL_TEXTURE_2D, brdfLUTTexture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RG16F, 512, 512, 0, GL_RG, GL_FLOAT, 0);

		// Убеждаемся, что режим наложения задан как GL_CLAMP_TO_EDGE
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		// Затем переконфигурируем захват объекта фреймбуфера и рендерим экранный прямоугольник с использованием BRDF-шейдера
		glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
		glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 512, 512);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, brdfLUTTexture, 0);

		glViewport(0, 0, 512, 512);
		shadersMap["brdfShader"]->bind();
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		renderQuad();

		glBindFramebuffer(GL_FRAMEBUFFER, 0);


		glViewport(0, 0, 800, 600);
	}


	{//motion blur
		pipeline.motionBlur.motionBlurTextures.resize(4);
		for (auto i = 0; i < 4; i++) {
			pipeline.motionBlur.motionBlurTextures[i] = RESOURCES::TextureLoader::Create(screenRes.x, screenRes.y);
			pipeline.motionBlur.motionBlurTextures[i]->setFilter(RESOURCES::TextureFiltering::LINEAR, RESOURCES::TextureFiltering::LINEAR);
		}
	}

	//glGenFramebuffers(1, &depthMapFBO);

	// Создаем текстуры глубины
	//glGenTextures(1, &depthMap);
	//glBindTexture(GL_TEXTURE_2D, depthMap);
	//glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	//float borderColor [] = {1.0, 1.0, 1.0, 1.0};
	//glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

	// Прикрепляем текстуру глубины в качестве буфера глубины для FBO
	//glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
	//glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
	//glDrawBuffer(GL_NONE);
	//glReadBuffer(GL_NONE);
	//glBindFramebuffer(GL_FRAMEBUFFER, 0);


	//hdr
	// Конфигурируем фреймбуфер типа с плавающей точкой

	pipeline.hdr.hdrFBO.bind();
	//glGenFramebuffers(1, &hdrFBO);
	//glBindFramebuffer(GL_FRAMEBUFFER, hdrFBO);
	// Создаем 2 цветовых фреймбуфера типа с плавающей точкой (первый - для обычного рендеринга, другой - для граничных значений яркости)

	pipeline.hdr.finalTexture = std::make_shared<RESOURCES::Texture>(screenRes.x, screenRes.y); //colorBuffer0
	pipeline.bloom.blurTexture = std::make_shared<RESOURCES::Texture>(screenRes.x, screenRes.y);  //colorBuffer1
	pipeline.hdr.hdrFBO.AttachTexture(*pipeline.hdr.finalTexture, Attachment::COLOR_ATTACHMENT0);
	//glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorBuffers[0], 0);

	// Создание буфера глубины (рендербуфер)
	unsigned int rboDepth;
	glGenRenderbuffers(1, &rboDepth);
	glBindRenderbuffer(GL_RENDERBUFFER, rboDepth);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, 800, 600);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboDepth);

	// Сообщаем OpenGL, какой прикрепленный цветовой буфер мы будем использовать для рендеринга
	unsigned int attachments[2] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1};
	glDrawBuffers(2, attachments);

	// Проверяем готовность фреймбуфера
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		std::cout << "Framebuffer not complete!" << std::endl;
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// ping-pong-фреймбуфер для размытия

	glGenFramebuffers(2, pingpongFBO);
	glGenTextures(2, pingpongColorbuffers);
	for (unsigned int i = 0; i < 2; i++) {
		glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBO[i]);
		glBindTexture(GL_TEXTURE_2D, pingpongColorbuffers[i]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, screenRes.x, screenRes.y, 0, GL_RGBA, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); // используем режим GL_CLAMP_TO_EDGE, т.к. в противном случае фильтр размытия производил бы выборку повторяющихся значений текстуры!
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, pingpongColorbuffers[i], 0);

		// Также проверяем, готовы ли фреймбуферы (буфер глубины нам не нужен)
		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
			std::cout << "Framebuffer not complete!" << std::endl;
	}

	tex3 = KUMA::RESOURCES::TextureLoader().createResource("textures\\noiseTexture.png");


	shadersMap["hdr"]->bind();
	shadersMap["hdr"]->setUniformInt("u_Scene", 0);
	//hdrShader->setUniformInt("hdrBuffer0", 0);
	//hdrShader->setUniformInt("hdrBuffer1", 1);
	//hdrShader->setUniformInt("hdrBuffer2", 2);
	//hdrShader->setUniformInt("hdrBuffer3", 3);
	//hdrShader->setUniformInt("bloomBlur",  4);
	//hdrShader->setUniformInt("u_Noise",  5);
	//hdrShader->setUniformInt("u_Depth",  6);
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
	//shadersMap["bloom"]->bind();
	//shadersMap["bloom"]->setUniformInt("scene", 0);
	//shadersMap["bloom"]->setUniformInt("bloomBlur", 1);



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


			pipeline.hdr.hdrFBO.bind();
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);



			//TODO: где-то тут прокинуть текстуры для IBL

			uint8_t glState = fetchGLState();
			applyStateMask(glState);
			//glEnable(GL_BLEND);
			//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			{//deferred
				// 1. Геометрический проход: выполняем рендеринг геометрических/цветовых данных сцены в g-буфер
				glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);
				glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
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
					glBindFramebuffer(GL_FRAMEBUFFER, ssaoFBO);
					glClear(GL_COLOR_BUFFER_BIT);
					shadersMap["ssao"]->bind();

					// Посылаем ядро + поворот 
					for (unsigned int i = 0; i < 64; ++i)
						shadersMap["ssao"]->setUniformVec3("samples[" + std::to_string(i) + "]", ssaoKernel[i]);
					//shaderSSAO.setMat4("projection", projection);
					glActiveTexture(GL_TEXTURE0);
					glBindTexture(GL_TEXTURE_2D, gPosition);
					glActiveTexture(GL_TEXTURE1);
					glBindTexture(GL_TEXTURE_2D, gNormal);
					glActiveTexture(GL_TEXTURE2);
					glBindTexture(GL_TEXTURE_2D, noiseTexture);
					renderQuad();
					glBindFramebuffer(GL_FRAMEBUFFER, 0);


					// 3. Размываем SSAO-текстуру, чтобы убрать шум
					glBindFramebuffer(GL_FRAMEBUFFER, ssaoBlurFBO);
					glClear(GL_COLOR_BUFFER_BIT);
					shadersMap["ssaoBlur"]->bind();
					glActiveTexture(GL_TEXTURE0);
					glBindTexture(GL_TEXTURE_2D, ssaoColorBuffer);
					renderQuad();
					glBindFramebuffer(GL_FRAMEBUFFER, 0);
				}


				pipeline.hdr.hdrFBO.bind();

				// 2. Проход освещения: вычисление освещение, перебирая попиксельно экранный прямоугольник, используя содержимое g-буфера
				glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
				shadersMap["deferredLightning"]->bind();
				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, gPosition);
				glActiveTexture(GL_TEXTURE1);
				glBindTexture(GL_TEXTURE_2D, gNormal);
				glActiveTexture(GL_TEXTURE2);
				glBindTexture(GL_TEXTURE_2D, gAlbedoSpec);

				{//ibl TODO: (send to forward shaders too)
					shadersMap["deferredLightning"]->setUniformInt("irradianceMap", 7);
					shadersMap["deferredLightning"]->setUniformInt("prefilterMap", 8);
					shadersMap["deferredLightning"]->setUniformInt("brdfLUT", 9);
					// Связываем предварительно вычисленные IBL-данные
					glActiveTexture(GL_TEXTURE7);
					glBindTexture(GL_TEXTURE_CUBE_MAP, irradianceMap);
					glActiveTexture(GL_TEXTURE8);
					glBindTexture(GL_TEXTURE_CUBE_MAP, prefilterMap);
					glActiveTexture(GL_TEXTURE9);
					glBindTexture(GL_TEXTURE_2D, brdfLUTTexture);
				}

				//// Рендерим прямоугольник
				renderQuad();
				shadersMap["deferredLightning"]->unbind();
			}
			// 2.5. Копируем содержимое буфера глубины (геометрический проход) в буфер глубины заданного по умолчанию фреймбуфера
			glBindFramebuffer(GL_READ_FRAMEBUFFER, gBuffer);
			glBindFramebuffer(GL_DRAW_FRAMEBUFFER, pipeline.hdr.hdrFBO.id); // пишем в заданный по умолчанию фреймбуфер
			glBlitFramebuffer(0, 0, screenRes.x, screenRes.y, 0, 0, screenRes.x, screenRes.y, GL_DEPTH_BUFFER_BIT, GL_NEAREST);
			pipeline.hdr.hdrFBO.bind();

			renderSkybox();

			renderScene(nullptr);
			/* {//prepare textures for post processing
				swapBuffers[0].bind();
				glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorBuffers[1], 0);
				shadersMap["bright"]->bind();
				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, colorBuffers[0]);
				renderQuad();
				shadersMap["bright"]->unbind();
				glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, swapTextures[0].id, 0);
				swapBuffers[0].unbind();

				swapBuffers[0].bind();
				glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureForGodRays.id, 0);
				glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
				shadersMap["godRaysTexture"]->bind();
				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, colorBuffers[0]);
				shadersMap["godRaysTexture"]->setUniformVec3("u_Color", MATHGL::Vector3(0.0f, 0.0f, 0.0f));
				//renderScene(shadersMap["godRaysTexture"]);

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
				glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, swapTextures[0].id, 0);
				swapBuffers[0].unbind();
			}*/
			//applyStateMask(glState);

			//glBindFramebuffer(GL_READ_FRAMEBUFFER, hdrFBO);
			//glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0); // пишем в заданный по умолчанию фреймбуфер
			//glBlitFramebuffer(0, 0, SCR_WIDTH, SCR_HEIGHT, 0, 0, SCR_WIDTH, SCR_HEIGHT, GL_DEPTH_BUFFER_BIT, GL_NEAREST);
			//copy depth buffer to texture

			{//bloom
				// 2. Размываем яркие фрагменты с помощью двухпроходного размытия по Гауссу
				bool horizontal = true, firstIteration = true;
				unsigned int amount = 10;
				shadersMap["blur"]->bind();
				for (unsigned int i = 0; i < amount; i++) {
					glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBO[horizontal]);
					shadersMap["blur"]->setUniformInt("horizontal", horizontal);
					glActiveTexture(GL_TEXTURE0);
					glBindTexture(GL_TEXTURE_2D, firstIteration ? pipeline.bloom.blurTexture->getId() : pingpongColorbuffers[!horizontal]);  // привязка текстуры другого фреймбуфера (или сцены, если это - первая итерация)
					renderQuad();
					horizontal = !horizontal;
					if (firstIteration) {
						firstIteration = false;
					}
				}

				swapBuffers[currentSwapBuffer].bind();
				glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
				shadersMap["bloom"]->bind();
				glActiveTexture(GL_TEXTURE0);
				pipeline.hdr.finalTexture->bind();
				//glBindTexture(GL_TEXTURE_2D, colorBuffers[0]);
				glActiveTexture(GL_TEXTURE1);
				glBindTexture(GL_TEXTURE_2D, pingpongColorbuffers[!horizontal]);
				shadersMap["bloom"]->setUniformInt("u_UseBloom", true);
				renderQuad();
				shadersMap["bloom"]->unbind();
				swapBuffers[currentSwapBuffer].unbind();
				currentSwapBuffer = !currentSwapBuffer;
			}

			//{//god rays
			//	auto dirLights = ECS::ComponentManager::getInstance()->getAllDirectionalLights();
			//	if (dirLights.size() > 0) {
			//		swapBuffers[currentSwapBuffer].bind();
			//
			//		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			//		shadersMap["godRays"]->bind();
			//		glActiveTexture(GL_TEXTURE0);
			//		glBindTexture(GL_TEXTURE_2D, swapTextures[!currentSwapBuffer].id);
			//		glActiveTexture(GL_TEXTURE1);
			//		glBindTexture(GL_TEXTURE_2D, textureForGodRays.id);
			//		shadersMap["godRays"]->setUniformInt("u_UseGodRays", true);
			//		shadersMap["godRays"]->setUniformVec3("u_SunPos", dirLights[0]->obj.transform->getLocalPosition());
			//		renderQuad();
			//		shadersMap["godRays"]->unbind();
			//
			//		swapBuffers[currentSwapBuffer].unbind();
			//		currentSwapBuffer = !currentSwapBuffer;
			//	}
			//}

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
				//glActiveTexture(GL_TEXTURE0);
				//glBindTexture(GL_TEXTURE_2D, motionBlurTextures[0]);
				//glActiveTexture(GL_TEXTURE1);
				//glBindTexture(GL_TEXTURE_2D, motionBlurTextures[1]);
				//glActiveTexture(GL_TEXTURE2);
				//glBindTexture(GL_TEXTURE_2D, motionBlurTextures[2]);
				//glActiveTexture(GL_TEXTURE3);
				//glBindTexture(GL_TEXTURE_2D, motionBlurTextures[3]);
			
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


			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			shadersMap["hdr"]->bind();
			//glActiveTexture(GL_TEXTURE0);
			//glBindTexture(GL_TEXTURE_2D, motionBlurTextures[0]);
			//glActiveTexture(GL_TEXTURE1);
			//glBindTexture(GL_TEXTURE_2D, motionBlurTextures[1]);
			//glActiveTexture(GL_TEXTURE2);
			//glBindTexture(GL_TEXTURE_2D, motionBlurTextures[2]);
			//glActiveTexture(GL_TEXTURE3);
			//glBindTexture(GL_TEXTURE_2D, motionBlurTextures[3]);
			//
			//glActiveTexture(GL_TEXTURE5);
			//glBindTexture(GL_TEXTURE_2D, tex3->id);
			//glActiveTexture(GL_TEXTURE6);
			//glBindTexture(GL_TEXTURE_2D, depthBufferTexture);
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
			swapTextures[!currentSwapBuffer]->bind();
			//textureForGodRays.bind();
			shadersMap["hdr"]->setUniformInt("u_UseHDR", pipeline.hdr.isEnabled);
			shadersMap["hdr"]->setUniformFloat("u_Exposure", pipeline.hdr.exposure);
			shadersMap["hdr"]->setUniformFloat("u_Gamma", pipeline.hdr.gamma);
			//hdrShader->setUniformVec3("sunPos", {-20.0f, 40.0f, 10.0f});
			renderQuad();
			shadersMap["hdr"]->unbind();

			applyStateMask(glState);

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

void Renderer::prepareDirLightShadowMap() {
	pipeline.dirLightsData.clear();
	for (auto& light : ECS::ComponentManager::getInstance()->getAllDirectionalLights()) {
		pipeline.depthMapFBO.AttachTexture(*light->shadowMap, Attachment::DEPTH_ATTACHMENT);
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
		glViewport(0, 0, screenRes.x, screenRes.y);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		//Support only one shadow map for dir light
		break;
	}
}


void Renderer::prepareSpotLightShadowMap() {
	pipeline.spotLightsData.clear();
	for (auto& light : ECS::ComponentManager::getInstance()->getAllSpotLights()) {
		pipeline.depthMapFBO.AttachTexture(*light->shadowMap, Attachment::DEPTH_ATTACHMENT);
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
		glViewport(0, 0, 800, 600);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}
}

void Renderer::preparePointLightShadowMap() {
	pipeline.pointLightsData.clear();
	auto res = getShadowMapResolution();
	for (auto& light : ECS::ComponentManager::getInstance()->getAllPointLights()) {
		pipeline.depthMapFBO.AttachCubeMap(*light->DepthMap, Attachment::DEPTH_ATTACHMENT);
		//glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO.id);
		//glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, light->shadowMap->id, 0);
		glDrawBuffer(GL_NONE);
		glReadBuffer(GL_NONE);
		pipeline.depthMapFBO.unbind();

		// Рендер
		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

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
		glViewport(0, 0, static_cast<unsigned>(res.x), static_cast<unsigned>(res.y));
		pipeline.depthMapFBO.bind();
		glClear(GL_DEPTH_BUFFER_BIT);
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
		glViewport(0, 0, 800, 600);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
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
