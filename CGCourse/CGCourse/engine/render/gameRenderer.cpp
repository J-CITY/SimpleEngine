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

using namespace KUMA;
using namespace KUMA::RENDER;

//unsigned int depthMapFBO;
//unsigned int depthMap;
glm::vec3 lightPos = glm::vec3(0.0f, 15.0f, 15.0f);
const unsigned int SHADOW_WIDTH = 1024, SHADOW_HEIGHT = 1024;


const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

GLuint depthBufferTexture;

//hdr
unsigned int hdrFBO;
std::shared_ptr<RESOURCES::Shader> hdrShader;
unsigned int colorBuffers[2];
bool hdr = true;
float exposure = 1.0f;

//defered render
unsigned int gBuffer;
unsigned int gPosition, gNormal, gAlbedoSpec;

//ssao
unsigned int ssaoFBO, ssaoBlurFBO;
unsigned int ssaoColorBuffer, ssaoColorBufferBlur;
std::vector<MATHGL::Vector3> ssaoKernel;
unsigned int noiseTexture;

//motion blur
bool motionBlurIsInit = false;
std::vector<unsigned> motionBlurTextures;

//bloom
unsigned int pingpongFBO[2];
unsigned int pingpongColorbuffers[2];

//fog
std::shared_ptr<RESOURCES::Texture> tex3;

void renderQuad();

bool currentBuffer = 0;
std::array<FrameBuffer, 2> swapBuffers;
std::array<RESOURCES::Texture, 2> swapTextures;

// renderCube() рендерит 1x1 3D-ящик в NDC
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

	glGenTextures(1, &depthBufferTexture);
	glBindTexture(GL_TEXTURE_2D, depthBufferTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, 800, 600, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_INT, NULL);

	//shadow direction light
	// Настраиваем карту глубины FBO

	context.renderer->shadersMap["simpleDepthShader"] = KUMA::RESOURCES::ShaderLoader::Create("C:\\Projects\\SimpleEngine\\CGCourse\\CGCourse\\Assets\\Engine\\Shaders\\dirShadow.glsl");
	context.renderer->shadersMap["pointShadowShader"] = KUMA::RESOURCES::ShaderLoader::Create("C:\\Projects\\SimpleEngine\\CGCourse\\CGCourse\\Assets\\Engine\\Shaders\\pointShadow.glsl");

	context.renderer->shadersMap["deferredGBuffer"] = KUMA::RESOURCES::ShaderLoader::Create("C:\\Projects\\SimpleEngine\\CGCourse\\CGCourse\\Assets\\Engine\\Shaders\\deferredGBuffer.glsl");
	context.renderer->shadersMap["deferredLightning"] = KUMA::RESOURCES::ShaderLoader::Create("C:\\Projects\\SimpleEngine\\CGCourse\\CGCourse\\Assets\\Engine\\Shaders\\deferredLightning.glsl");

	context.renderer->shadersMap["ssao"] = KUMA::RESOURCES::ShaderLoader::Create("C:\\Projects\\SimpleEngine\\CGCourse\\CGCourse\\Assets\\Engine\\Shaders\\ssao.glsl");
	context.renderer->shadersMap["ssaoBlur"] = KUMA::RESOURCES::ShaderLoader::Create("C:\\Projects\\SimpleEngine\\CGCourse\\CGCourse\\Assets\\Engine\\Shaders\\ssaoBlur.glsl");

	//ibl
	context.renderer->shadersMap["equirectangularToCubemapShader"] = KUMA::RESOURCES::ShaderLoader::Create("C:\\Projects\\SimpleEngine\\CGCourse\\CGCourse\\Assets\\Engine\\Shaders\\equirectangular_to_cubemap.glsl");
	context.renderer->shadersMap["irradianceShader"] =               KUMA::RESOURCES::ShaderLoader::Create("C:\\Projects\\SimpleEngine\\CGCourse\\CGCourse\\Assets\\Engine\\Shaders\\irradiance_convolution.glsl");
	context.renderer->shadersMap["prefilterShader"] =                KUMA::RESOURCES::ShaderLoader::Create("C:\\Projects\\SimpleEngine\\CGCourse\\CGCourse\\Assets\\Engine\\Shaders\\prefilter.glsl");
	context.renderer->shadersMap["brdfShader"] =                     KUMA::RESOURCES::ShaderLoader::Create("C:\\Projects\\SimpleEngine\\CGCourse\\CGCourse\\Assets\\Engine\\Shaders\\brdf.glsl");

	//blum
	context.renderer->shadersMap["blur"] = KUMA::RESOURCES::ShaderLoader::Create("C:\\Projects\\SimpleEngine\\CGCourse\\CGCourse\\Assets\\Engine\\Shaders\\blur.glsl");
	context.renderer->shadersMap["bloom"] = KUMA::RESOURCES::ShaderLoader::Create("C:\\Projects\\SimpleEngine\\CGCourse\\CGCourse\\Assets\\Engine\\Shaders\\bloom.glsl");

	
	{//create swap buffers
		int i = 0;
		for (auto& buffer : swapBuffers) {
			buffer.bind();
			auto& texture = swapTextures[i];
			texture.bindWithoutAttach();
			texture.Load(nullptr, SCR_WIDTH, SCR_HEIGHT, 4, true, RESOURCES::TextureFormat::RGBA16F);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			buffer.AttachTexture(texture, Attachment::COLOR_ATTACHMENT0);
			i++;
		}
	}

	{//defered render
		// Конфигурирование g-буфера фреймбуфера
		
		glGenFramebuffers(1, &gBuffer);
		glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);

		// Цветовой буфер позиций
		glGenTextures(1, &gPosition);
		glBindTexture(GL_TEXTURE_2D, gPosition);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, gPosition, 0);

		// Цветовой буфер нормалей
		glGenTextures(1, &gNormal);
		glBindTexture(GL_TEXTURE_2D, gNormal);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, gNormal, 0);

		// Цветовой буфер значений цвета + отраженной составляющей
		glGenTextures(1, &gAlbedoSpec);
		glBindTexture(GL_TEXTURE_2D, gAlbedoSpec);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
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
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, SCR_WIDTH, SCR_HEIGHT);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboDepth);

		// Проверяем готовность фреймбуфера
		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
			std::cout << "Framebuffer not complete!" << std::endl;
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		context.renderer->shadersMap["deferredLightning"]->bind();
		context.renderer->shadersMap["deferredLightning"]->setUniformInt("u_PositionMap", 0);
		context.renderer->shadersMap["deferredLightning"]->setUniformInt("u_NormalMap", 1);
		context.renderer->shadersMap["deferredLightning"]->setUniformInt("u_DiffuseMap", 2);
		context.renderer->shadersMap["deferredLightning"]->setUniformInt("u_SSAO", 3);
		context.renderer->shadersMap["deferredLightning"]->unbind();
	}

	{//blum
		
	}


	/* {//ssao
		glGenFramebuffers(1, &ssaoFBO);  glGenFramebuffers(1, &ssaoBlurFBO);
		glBindFramebuffer(GL_FRAMEBUFFER, ssaoFBO);
		// Цветовой буфер SSAO 
		glGenTextures(1, &ssaoColorBuffer);
		glBindTexture(GL_TEXTURE_2D, ssaoColorBuffer);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, SCR_WIDTH, SCR_HEIGHT, 0, GL_RED, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ssaoColorBuffer, 0);
		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
			std::cout << "SSAO Framebuffer not complete!" << std::endl;

		// И стадия размытия
		glBindFramebuffer(GL_FRAMEBUFFER, ssaoBlurFBO);
		glGenTextures(1, &ssaoColorBufferBlur);
		glBindTexture(GL_TEXTURE_2D, ssaoColorBufferBlur);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, SCR_WIDTH, SCR_HEIGHT, 0, GL_RED, GL_FLOAT, NULL);
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

		context.renderer->shadersMap["ssao"]->bind();
		context.renderer->shadersMap["ssao"]->setUniformInt("gPosition", 0);
		context.renderer->shadersMap["ssao"]->setUniformInt("gNormal", 1);
		context.renderer->shadersMap["ssao"]->setUniformInt("texNoise", 2);
		context.renderer->shadersMap["ssaoBlur"]->bind();
		context.renderer->shadersMap["ssaoBlur"]->setUniformInt("ssaoInput", 0);
		context.renderer->shadersMap["ssaoBlur"]->unbind();
		context.renderer->shadersMap["ssao"]->unbind();
	}
	*/
	/*{//ibl
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
		float* data = RESOURCES::stbiLoadf("../resources/textures/hdr/newport_loft.hdr", &width, &height, &nrComponents, 0);
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
		context.renderer->shadersMap["equirectangularToCubemapShader"]->bind();
		context.renderer->shadersMap["equirectangularToCubemapShader"]->setUniformInt("equirectangularMap", 0);
		context.renderer->shadersMap["equirectangularToCubemapShader"]->setUniformMat4("projection", captureProjection);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, hdrTexture);

		glViewport(0, 0, 512, 512); // не забудьте настроить видовой экран в соответствии с размерами захвата
		glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
		for (unsigned int i = 0; i < 6; ++i) {
			context.renderer->shadersMap["equirectangularToCubemapShader"]->setUniformMat4("view", captureViews[i]);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, envCubemap, 0);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			renderCube();
		}
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		// Далее позволим OpenGL сгенерировать мипмап-карты (для борьбы с артефактами в виде визуальных точек)
		glBindTexture(GL_TEXTURE_CUBE_MAP, envCubemap);
		glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

		// PBR: создаем кубическую карту облученности, и приводим размеры захвата FBO к размерам карты облученности
		unsigned int irradianceMap;
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
		context.renderer->shadersMap["irradianceShader"]->bind();
		context.renderer->shadersMap["irradianceShader"]->setUniformInt("environmentMap", 0);
		context.renderer->shadersMap["irradianceShader"]->setUniformMat4("projection", captureProjection);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, envCubemap);

		glViewport(0, 0, 32, 32); // не забудьте настроить видовой экран на размеры захвата
		glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
		for (unsigned int i = 0; i < 6; ++i) {
			context.renderer->shadersMap["irradianceShader"]->setUniformMat4("view", captureViews[i]);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, irradianceMap, 0);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			renderCube();
		}
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		// PBR: создаем префильтрованную кубическую карту, и приводим размеры захвата FBO к размерам префильтрованной карты
		unsigned int prefilterMap;
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
		context.renderer->shadersMap["prefilterShader"]->bind();
		context.renderer->shadersMap["prefilterShader"]->setUniformInt("environmentMap", 0);
		context.renderer->shadersMap["prefilterShader"]->setUniformMat4("projection", captureProjection);
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
			context.renderer->shadersMap["prefilterShader"]->setUniformFloat("roughness", roughness);
			for (unsigned int i = 0; i < 6; ++i) {
				context.renderer->shadersMap["prefilterShader"]->setUniformMat4("view", captureViews[i]);
				glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, prefilterMap, mip);

				glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
				renderCube();
			}
		}
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		// PBR: генерируем 2D LUT-текстуру при помощи используемых уравнений BRDF
		unsigned int brdfLUTTexture;
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
		context.renderer->shadersMap["brdfShader"]->bind();
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		renderQuad();

		glBindFramebuffer(GL_FRAMEBUFFER, 0);


		glViewport(0, 0, 800, 600);
	}*/


	{//motion blur
		for (auto i = 0; i < 4; i++) {
			// Цветовой буфер позиций
			unsigned int id;
			glGenTextures(1, &id);
			glBindTexture(GL_TEXTURE_2D, id);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, 800, 600, 0, GL_RGBA, GL_FLOAT, NULL);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			motionBlurTextures.push_back(id);
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
	hdrShader = KUMA::RESOURCES::ShaderLoader::Create("C:\\Projects\\SimpleEngine\\CGCourse\\CGCourse\\Assets\\Engine\\Shaders\\hdr.glsl");
	// Конфигурируем фреймбуфер типа с плавающей точкой
	
	glGenFramebuffers(1, &hdrFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, hdrFBO);
	// Создаем 2 цветовых фреймбуфера типа с плавающей точкой (первый - для обычного рендеринга, другой - для граничных значений яркости)
	
	glGenTextures(2, colorBuffers);
	for (unsigned int i = 0; i < 2; i++) {
		glBindTexture(GL_TEXTURE_2D, colorBuffers[i]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);  //используем режим GL_CLAMP_TO_EDGE, т.к. в противном случае фильтр размытия производил бы выборку повторяющихся значений текстуры!
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		// Прикрепляем текстуру к фреймбуферу
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, colorBuffers[i], 0);
	}

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
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);
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
	

	hdrShader->bind();
	hdrShader->setUniformInt("hdrBuffer0", 0);
	hdrShader->setUniformInt("hdrBuffer1", 1);
	hdrShader->setUniformInt("hdrBuffer2", 2);
	hdrShader->setUniformInt("hdrBuffer3", 3);
	hdrShader->setUniformInt("bloomBlur",  4);
	hdrShader->setUniformInt("u_Noise",  5);
	hdrShader->setUniformInt("u_Depth",  6);
	hdrShader->unbind();


	context.renderer->shadersMap["blur"]->bind();
	context.renderer->shadersMap["blur"]->setUniformInt("image", 0);
	//context.renderer->shadersMap["bloom"]->bind();
	//context.renderer->shadersMap["bloom"]->setUniformInt("scene", 0);
	//context.renderer->shadersMap["bloom"]->setUniformInt("bloomBlur", 1);



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

void GameRenderer::renderScene() {
	
	if (auto currentScene = context.sceneManager.getCurrentScene()) {
		//grass
		auto grass = currentScene->findActorByName("Grass");
		auto m = grass->getComponent<KUMA::ECS::MaterialRenderer>();
		m->getMaterials()[0]->getUniformsData()["fTimePassed"] = KUMA::TIME::Timer::instance()->getTimeSinceStart();

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

			context.renderer->prepareDirLightShadowMap();
			//context.renderer->prepareSpotLightShadowMap();
			//context.renderer->preparePointLightShadowMap();


			glBindFramebuffer(GL_FRAMEBUFFER, hdrFBO);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			renderSkybox();

			//TODO: где-то тут прокинуть текстуры для IBL

			uint8_t glState = context.renderer->fetchGLState();
			context.renderer->applyStateMask(glState);

			/* {//deferred
				// 1. Геометрический проход: выполняем рендеринг геометрических/цветовых данных сцены в g-буфер
				glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);
				glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
				context.renderer->shadersMap["deferredGBuffer"]->bind();
				
				for (const auto& [distance, p_toDraw] : context.renderer->opaqueMeshes) {
					context.renderer->userMatrixSender(p_toDraw.userMatrix);
					context.renderer->drawGBuffer(*p_toDraw.mesh, *p_toDraw.material, &p_toDraw.world, context.renderer->shadersMap["deferredGBuffer"]);
				}
				for (const auto& [distance, p_toDraw] : context.renderer->transparentMeshes) {
					context.renderer->userMatrixSender(p_toDraw.userMatrix);
					context.renderer->drawGBuffer(*p_toDraw.mesh, *p_toDraw.material, &p_toDraw.world, context.renderer->shadersMap["deferredGBuffer"]);
				}

				context.renderer->shadersMap["deferredGBuffer"]->unbind();

				{//ssao
					// 2. Генерируем текстуру для SSAO
					glBindFramebuffer(GL_FRAMEBUFFER, ssaoFBO);
					glClear(GL_COLOR_BUFFER_BIT);
					context.renderer->shadersMap["ssao"]->bind();

					// Посылаем ядро + поворот 
					for (unsigned int i = 0; i < 64; ++i)
						context.renderer->shadersMap["ssao"]->setUniformVec3("samples[" + std::to_string(i) + "]", ssaoKernel[i]);
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
					context.renderer->shadersMap["ssaoBlur"]->bind();
					glActiveTexture(GL_TEXTURE0);
					glBindTexture(GL_TEXTURE_2D, ssaoColorBuffer);
					renderQuad();
					glBindFramebuffer(GL_FRAMEBUFFER, 0);
				}


				glBindFramebuffer(GL_FRAMEBUFFER, hdrFBO);

				// 2. Проход освещения: вычисление освещение, перебирая попиксельно экранный прямоугольник, используя содержимое g-буфера
				glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
				context.renderer->shadersMap["deferredLightning"]->bind();
				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, gPosition);
				glActiveTexture(GL_TEXTURE1);
				glBindTexture(GL_TEXTURE_2D, gNormal);
				glActiveTexture(GL_TEXTURE2);
				glBindTexture(GL_TEXTURE_2D, gAlbedoSpec);

				// Рендерим прямоугольник
				renderQuad();

				context.renderer->shadersMap["deferredLightning"]->unbind();
			}*/
			// 2.5. Копируем содержимое буфера глубины (геометрический проход) в буфер глубины заданного по умолчанию фреймбуфера
			//glBindFramebuffer(GL_READ_FRAMEBUFFER, gBuffer);
			//glBindFramebuffer(GL_DRAW_FRAMEBUFFER, hdrFBO); // пишем в заданный по умолчанию фреймбуфер
			//glBlitFramebuffer(0, 0, SCR_WIDTH, SCR_HEIGHT, 0, 0, SCR_WIDTH, SCR_HEIGHT, GL_DEPTH_BUFFER_BIT, GL_NEAREST);
			//glBindFramebuffer(GL_FRAMEBUFFER, hdrFBO);


			context.renderer->renderScene(nullptr);
			//context.renderer->applyStateMask(glState);

			//glBindFramebuffer(GL_READ_FRAMEBUFFER, hdrFBO);
			//glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0); // пишем в заданный по умолчанию фреймбуфер
			//glBlitFramebuffer(0, 0, SCR_WIDTH, SCR_HEIGHT, 0, 0, SCR_WIDTH, SCR_HEIGHT, GL_DEPTH_BUFFER_BIT, GL_NEAREST);
			//copy depth buffer to texture
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthBufferTexture, 0);

			glBindFramebuffer(GL_FRAMEBUFFER, 0);


			//glState = context.renderer->fetchGLState();
			//context.renderer->applyStateMask(GL_DEPTH_TEST);

			// 2. Размываем яркие фрагменты с помощью двухпроходного размытия по Гауссу
			bool horizontal = true, first_iteration = true;
			unsigned int amount = 10;
			context.renderer->shadersMap["blur"]->bind();
			for (unsigned int i = 0; i < amount; i++) {
				glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBO[horizontal]);
				context.renderer->shadersMap["blur"]->setUniformInt("horizontal", horizontal);
				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, first_iteration ? colorBuffers[1] : pingpongColorbuffers[!horizontal]);  // привязка текстуры другого фреймбуфера (или сцены, если это - первая итерация)
				renderQuad();
				horizontal = !horizontal;
				if (first_iteration)
					first_iteration = false;
			}

			//glBindFramebuffer(GL_FRAMEBUFFER, hdrFBO);

			// 3. Теперь рендерим цветовой буфер (типа с плавающей точкой) на 2D-прямоугольник и сужаем диапазон значений HDR-цветов к цветовому диапазону значений заданного по умолчанию фреймбуфера
			//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			//context.renderer->shadersMap["bloom"]->bind();
			//glActiveTexture(GL_TEXTURE0);
			//glBindTexture(GL_TEXTURE_2D, colorBuffers[0]);
			//glActiveTexture(GL_TEXTURE1);
			//glBindTexture(GL_TEXTURE_2D, pingpongColorbuffers[!horizontal]);
			//context.renderer->shadersMap["bloom"]->setUniformInt("bloom", 1);
			//context.renderer->shadersMap["bloom"]->setUniformFloat("exposure", exposure);
			//renderQuad();


			glBindFramebuffer(GL_FRAMEBUFFER, 0);
			if (!motionBlurIsInit) {
				motionBlurIsInit = true;
				for (auto i = 0; i < motionBlurTextures.size(); i++) {
					glCopyImageSubData(colorBuffers[0], GL_TEXTURE_2D, 0, 0, 0, 0,
						motionBlurTextures[i], GL_TEXTURE_2D, 0, 0, 0, 0,
						800, 600, 1);
				}
			}
			else {
				auto front = motionBlurTextures[0];
				motionBlurTextures.erase(motionBlurTextures.begin());
				motionBlurTextures.push_back(front);
				glCopyImageSubData(colorBuffers[0], GL_TEXTURE_2D, 0, 0, 0, 0,
					motionBlurTextures[motionBlurTextures.size()-1], GL_TEXTURE_2D, 0, 0, 0, 0,
					800, 600, 1);
			}
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			hdrShader->bind();
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, motionBlurTextures[0]);
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, motionBlurTextures[1]);
			glActiveTexture(GL_TEXTURE2);
			glBindTexture(GL_TEXTURE_2D, motionBlurTextures[2]);
			glActiveTexture(GL_TEXTURE3);
			glBindTexture(GL_TEXTURE_2D, motionBlurTextures[3]);
			glActiveTexture(GL_TEXTURE4);
			glBindTexture(GL_TEXTURE_2D, pingpongColorbuffers[!horizontal]);
			glActiveTexture(GL_TEXTURE5);
			glBindTexture(GL_TEXTURE_2D, tex3->id);
			glActiveTexture(GL_TEXTURE6);
			glBindTexture(GL_TEXTURE_2D, depthBufferTexture);
			hdrShader->setUniformInt("hdr", hdr);
			hdrShader->setUniformFloat("exposure", exposure);
			hdrShader->setUniformVec3("sunPos", {-20.0f, 40.0f, 10.0f});
			renderQuad();
			hdrShader->unbind();

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

// renderQuad() рендерит 1x1 XY-прямоугольник в NDC
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

void Renderer::prepareDirLightShadowMap() {
	pipeline.dirLightsData.clear();
	for (auto& light : ECS::ComponentManager::getInstance()->getAllDirectionalLights()) {
		pipeline.depthMapFBO.AttachTexture(*light->shadowMap, Attachment::DEPTH_ATTACHMENT);
		//glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO.id);
		//glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, light->shadowMap->id, 0);
		glDrawBuffer(GL_NONE);
		glReadBuffer(GL_NONE);
		pipeline.depthMapFBO.unbind();

		lightPos = glm::vec3(light->obj.transform->getLocalPosition().x,
			light->obj.transform->getLocalPosition().y,
			light->obj.transform->getLocalPosition().z);

		glm::mat4 lightProjection, lightView;
		float near_plane = 1.0f, far_plane = 100.0f;
		// lightProjection = glm::perspective(glm::radians(45.0f), (GLfloat)SHADOW_WIDTH / (GLfloat)SHADOW_HEIGHT, near_plane, far_plane); // обратите внимание, что если вы используете матрицу перспективной проекции, то вам придется изменить положение света, так как текущего положения света недостаточно для отображения всей сцены
		lightProjection = glm::ortho(-50.0f, 50.0f, -50.0f, 50.0f, near_plane, far_plane);
		lightView = glm::lookAt(lightPos, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0, 1.0, 0.0));
		auto lightSpaceMatrix = lightProjection * lightView;

		MATHGL::Matrix4 m(
			lightSpaceMatrix[0][0], lightSpaceMatrix[0][1], lightSpaceMatrix[0][2], lightSpaceMatrix[0][3],
			lightSpaceMatrix[1][0], lightSpaceMatrix[1][1], lightSpaceMatrix[1][2], lightSpaceMatrix[1][3],
			lightSpaceMatrix[2][0], lightSpaceMatrix[2][1], lightSpaceMatrix[2][2], lightSpaceMatrix[2][3],
			lightSpaceMatrix[3][0], lightSpaceMatrix[3][1], lightSpaceMatrix[3][2], lightSpaceMatrix[3][3]
		);


		pipeline.dirLightsData.push_back(DirLightData{light->shadowMap->id, m.Transpose(m)});

		// Рендеринг сцены глазами источника света
		shadersMap["simpleDepthShader"]->bind();
		shadersMap["simpleDepthShader"]->setUniformMat4("lightSpaceMatrix", m.Transpose(m));
		//glUniformMatrix4fv(glGetUniformLocation(shadersMap["simpleDepthShader"]->id, "lightSpaceMatrix"), 1, GL_FALSE, &lightSpaceMatrix[0][0]);

		setViewPort(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
		//glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
		//glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
		pipeline.depthMapFBO.bind();
		ClearDepth();
		//glClear(GL_DEPTH_BUFFER_BIT);
		isShadowDir = true;
		renderScene(shadersMap["simpleDepthShader"]);
		pipeline.depthMapFBO.unbind();
		//glBindFramebuffer(GL_FRAMEBUFFER, 0);
		isShadowDir = false;

		// Сброс настроек области просмотра
		glViewport(0, 0, 800, 600);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}
}


void Renderer::prepareSpotLightShadowMap() {
	pipeline.spotLightsData.clear();
	for (auto& light : ECS::ComponentManager::getInstance()->getAllSpotLights()) {
		pipeline.depthMapFBO.AttachTexture(*light->shadowMap, Attachment::DEPTH_ATTACHMENT);
		//glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO.id);
		//glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, light->shadowMap->id, 0);
		glDrawBuffer(GL_NONE);
		glReadBuffer(GL_NONE);
		pipeline.depthMapFBO.unbind();

		lightPos = glm::vec3(light->obj.transform->getLocalPosition().x,
			light->obj.transform->getLocalPosition().y,
			light->obj.transform->getLocalPosition().z);

		glm::mat4 lightProjection, lightView;
		float near_plane = 1.0f, far_plane = 100.0f;
		lightProjection = glm::perspective(glm::radians(45.0f), (GLfloat)SHADOW_WIDTH / (GLfloat)SHADOW_HEIGHT, near_plane, far_plane); // обратите внимание, что если вы используете матрицу перспективной проекции, то вам придется изменить положение света, так как текущего положения света недостаточно для отображения всей сцены
		//lightProjection = glm::ortho(-50.0f, 50.0f, -50.0f, 50.0f, near_plane, far_plane);
		lightView = glm::lookAt(lightPos, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0, 1.0, 0.0));
		auto lightSpaceMatrix = lightProjection * lightView;

		MATHGL::Matrix4 m(
			lightSpaceMatrix[0][0], lightSpaceMatrix[0][1], lightSpaceMatrix[0][2], lightSpaceMatrix[0][3],
			lightSpaceMatrix[1][0], lightSpaceMatrix[1][1], lightSpaceMatrix[1][2], lightSpaceMatrix[1][3],
			lightSpaceMatrix[2][0], lightSpaceMatrix[2][1], lightSpaceMatrix[2][2], lightSpaceMatrix[2][3],
			lightSpaceMatrix[3][0], lightSpaceMatrix[3][1], lightSpaceMatrix[3][2], lightSpaceMatrix[3][3]
		);


		pipeline.spotLightsData.push_back(SpotLightData{light->shadowMap->id, m});

		// Рендеринг сцены глазами источника света
		shadersMap["simpleDepthShader"]->bind();
		shadersMap["simpleDepthShader"]->setUniformMat4("lightSpaceMatrix", m.Transpose(m));
		//glUniformMatrix4fv(glGetUniformLocation(shadersMap["simpleDepthShader"]->id, "lightSpaceMatrix"), 1, GL_FALSE, &lightSpaceMatrix[0][0]);

		setViewPort(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
		//glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
		//glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
		pipeline.depthMapFBO.bind();
		ClearDepth();
		//glClear(GL_DEPTH_BUFFER_BIT);
		isShadowDir = true;
		renderScene(shadersMap["simpleDepthShader"]);
		pipeline.depthMapFBO.unbind();
		//glBindFramebuffer(GL_FRAMEBUFFER, 0);
		isShadowDir = false;

		// Сброс настроек области просмотра
		glViewport(0, 0, 800, 600);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}
}

void Renderer::preparePointLightShadowMap() {
	pipeline.pointLightsData.clear();
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
		float near_plane = 1.0f;
		float far_plane = 25.0f;
		glm::mat4 shadowProj = glm::perspective(glm::radians(90.0f), (float)SHADOW_WIDTH / (float)SHADOW_HEIGHT, near_plane, far_plane);
		std::vector<glm::mat4> shadowTransforms;
		shadowTransforms.push_back(shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)));
		shadowTransforms.push_back(shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)));
		shadowTransforms.push_back(shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f)));
		shadowTransforms.push_back(shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f)));
		shadowTransforms.push_back(shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f)));
		shadowTransforms.push_back(shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, -1.0f, 0.0f)));

		// 1. Рендерим сцену в кубическую карту глубины
		glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
		pipeline.depthMapFBO.bind();
		glClear(GL_DEPTH_BUFFER_BIT);
		shadersMap["pointShadowShader"]->bind();
		for (unsigned int i = 0; i < 6; ++i) {
			MATHGL::Matrix4 m(
				shadowTransforms[i][0][0], shadowTransforms[i][0][1], shadowTransforms[i][0][2], shadowTransforms[i][0][3],
				shadowTransforms[i][1][0], shadowTransforms[i][1][1], shadowTransforms[i][1][2], shadowTransforms[i][1][3],
				shadowTransforms[i][2][0], shadowTransforms[i][2][1], shadowTransforms[i][2][2], shadowTransforms[i][2][3],
				shadowTransforms[i][3][0], shadowTransforms[i][3][1], shadowTransforms[i][3][2], shadowTransforms[i][3][3]
			);
			shadersMap["pointShadowShader"]->setUniformMat4("shadowMatrices[" + std::to_string(i) + "]", m.Transpose(m));
		}
		shadersMap["pointShadowShader"]->setUniformFloat("far_plane", far_plane);
		shadersMap["pointShadowShader"]->setUniformVec3("lightPos", MATHGL::Vector3(lightPos.x, lightPos.y, lightPos.z));
		isShadowDir = true;
		renderScene(shadersMap["pointShadowShader"]);
		isShadowDir = false;
		pipeline.depthMapFBO.unbind();

		// Сброс настроек области просмотра
		glViewport(0, 0, 800, 600);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}
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
			p_material.getShader()->setUniformMat4("lightSpaceMatrix", pipeline.dirLightsData[0].projMap);
			p_material.getShader()->setUniformInt("shadowMap", 5);
			glActiveTexture(GL_TEXTURE5);
			glBindTexture(GL_TEXTURE_2D, pipeline.dirLightsData[0].id);
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

void Renderer::drawGBuffer(RESOURCES::Mesh& p_mesh, Material& p_material, MATHGL::Matrix4 const* p_modelMatrix, std::shared_ptr<RESOURCES::Shader> shader) {
	//if (p_material.hasShader() && p_material.getGPUInstances() > 0) {
		
	//if (p_material.getUniformsData().count("castShadow") && !std::get<bool>(p_material.getUniformsData()["useBone"])) {
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

	if (p_material.getUniformsData().count("useBone")) {
		shader->setUniformInt("useBone", std::get<bool>(p_material.getUniformsData()["useBone"]));
	}
	shader->setUniformMat4("model", *p_modelMatrix);

	draw(p_mesh, p_material, PrimitiveMode::TRIANGLES, p_material.getGPUInstances());

	//p_material.setShader(nullptr);
	//if (!isShadowDir)
	//	p_material.unbind();
	//}
}

void Renderer::registerModelMatrixSender(std::function<void(MATHGL::Matrix4)> p_modelMatrixSender) {
	modelMatrixSender = p_modelMatrixSender;
}

void Renderer::registerUserMatrixSender(std::function<void(MATHGL::Matrix4)> p_userMatrixSender) {
	userMatrixSender = p_userMatrixSender;
}
