﻿//#include "CameraRT.h"
////#include "DiffuseLightRT.h"
//#include "ObjectRT.h"
//#include "RenderRT.h"
//#include "engine/ecs/components/ambientSphereLight.h"
//#include "engine/ecs/components/directionalLight.h"
//#include "engine/ecs/components/directionalLight.h"
//#include "engine/ecs/components/cameraComponent.h"
//#include "engine/core/app.h"
//#include "engine/ecs/components/inputComponent.h"
//#include "engine/ecs/components/physicsComponent.h"
//#include "engine/ecs/components/pointLight.h"
//#include "engine/ecs/components/skeletal.h"
//#include "engine/gui/guiObject.h"
//#include "engine/inputManager/inputManager.h"
//#include "engine/physics/body.h"
//#include "engine/physics/narrowPhase.h"
//#include "engine/physics/PhysicWorld.h"
//#include "engine/physics/cloth/Cloth.h"
////#include "engine/physics/particel/opencl/loadKernel.h"
//#include "engine/resourceManager/materialManager.h"
//#include "engine/resourceManager/resource/bone.h"
//#include "engine/resourceManager/modelManager.h"
//#include "engine/resourceManager/shaderManager.h"
//#include "engine/scene/sceneManager.h"
//#include "engine/utils/meshGenerator.h"
//#include "game/World.h"
//#include "game/World.h"

#include "coreModule/core/app.h"
#include "coreModule/gui/guiObject.h"
#include "sceneModule/sceneManager.h"

//using namespace std;
extern "C" {
	_declspec(dllexport) unsigned long NvOptimusEnablement = 0x00000001;
}
//Game::World* world = nullptr;

//std::shared_ptr<IKIGAI::RESOURCES::Animation> danceAnimation;
//std::shared_ptr<IKIGAI::RESOURCES::Animator> animator;

float liveTime = 20.0f;
float liveTime2 = 20.0f;

namespace IKIGAI::MATHGL {
	class Vector3;
};

//namespace Game {
//	Game::ChunkDataTypePtr _GetChunkDataForMeshing(int cx, int cz) {
//		if (world && world->ChunkExistsInMap(cx, cz)) {
//			Game::Chunk* chunk = world->RetrieveChunkFromMap(cx, cz);
//			return &chunk->p_ChunkContents;
//		}
//
//		return nullptr;
//	}
//
//	Game::ChunkLightDataTypePtr _GetChunkLightDataForMeshing(int cx, int cz) {
//		if (world && world->ChunkExistsInMap(cx, cz)) {
//			Game::Chunk* chunk = world->RetrieveChunkFromMap(cx, cz);
//			return &chunk->p_ChunkLightInformation;
//		}
//
//		return nullptr;
//	}
//
//	Game::Block* GetWorldBlock(const IKIGAI::MATHGL::Vector3& block_pos) {
//		std::pair<Game::Block*, Game::Chunk*> block = world->GetBlockFromPosition(block_pos);
//		return block.first;
//	}
//}

#ifdef DX12_BACKEND
#include <Windows.h>
#include <iostream>
#include <dxgi1_4.h>
#include <d3d12.h>
#include <D3Dcompiler.h>
#include <DirectXMath.h>
#include <wrl/client.h>
#include <DirectXPackedVector.h>
using Microsoft::WRL::ComPtr;
using namespace DirectX;
using namespace DirectX::PackedVector;

#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "D3D12.lib")

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE prevInstance, PSTR cmdLine, int showCmd) {

#if defined(DEBUG) | defined(_DEBUG)
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif
	IKIGAI::CORE_SYSTEM::App::hInstance = hInstance;
	IKIGAI::CORE_SYSTEM::App app;
	app.getCore()->sceneManager->loadFromFile("Scenes/scene0Vk.json");

	app.run();
	return 0;
}
#endif


#if defined(VULKAN_BACKEND) || defined(OPENGL_BACKEND)

#include <rttr/registration>
#include <utilsModule/jsonParser/jsonParser.h>
enum class color
{
	red,
	green,
	blue
};

struct point2d
{
	point2d() {}
	point2d(int x_, int y_) : x(x_), y(y_) {}
	int x = 0;
	int y = 0;
};

struct shape
{
	shape(){}
	shape(std::string n) : name(n) {}

	void set_visible(bool v) { visible = v; }
	bool get_visible() const { return visible; }

	color color_ = color::blue;
	std::string name = "";
	point2d position;
	std::map<color, point2d> dictionary;

	RTTR_ENABLE()
private:
	bool visible = false;
};

struct circle : shape
{
	circle(){}
	circle(std::string n) : shape(n) {}

	double radius = 5.2;
	std::vector<point2d> points;

	int no_serialize = 100;

	RTTR_ENABLE(shape)
};

RTTR_REGISTRATION
{
	rttr::registration::class_<shape>("shape")
		.property("visible", &shape::get_visible, &shape::set_visible)
		.property("color", &shape::color_)
		.property("name", &shape::name)
		.property("position", &shape::position)
		.property("dictionary", &shape::dictionary)
	;

	rttr::registration::class_<circle>("circle")
		.property("radius", &circle::radius)
		.property("points", &circle::points)
		.property("no_serialize", &circle::no_serialize)
		(
			rttr::metadata("NO_SERIALIZE", true)
		)
		;

	rttr::registration::class_<point2d>("point2d")
		.constructor()(rttr::detail::as_object{})
		.property("x", &point2d::x)
		.property("y", &point2d::y)
		;


	rttr::registration::enumeration<color>("color")
		(
			rttr::value("red", color::red),
			rttr::value("blue", color::blue),
			rttr::value("green", color::green)
		);
}
#include "soloud.h"
#include "soloud_thread.h"
#include "soloud_wavstream.h"
int main() {
	

	/*
	std::string json_string;
	
	{
		circle c_1("Circle #1");
		shape& my_shape = c_1;
	
		c_1.set_visible(true);
		c_1.points = std::vector<point2d>(2, point2d(1, 1));
		c_1.points[1].x = 23;
		c_1.points[1].y = 42;
	
		c_1.position.x = 12;
		c_1.position.y = 66;
	
		c_1.radius = 5.123;
		c_1.color_ = color::red;
	
		c_1.dictionary = { { {color::green, {1, 2} }, {color::blue, {3, 4} }, {color::red, {5, 6} } } };
	
		c_1.no_serialize = 12345;
	
		json_string = IKIGAI::UTILS::ToJson(my_shape); // serialize the circle to 'json_string'
	}
	
	std::cout << "Circle: c_1:\n" << json_string << std::endl;
	
	circle c_2("Circle #2"); // create a new empty circle
	
	c_2 = IKIGAI::UTILS::FromJsonStr<circle>(json_string).unwrap<circle>(); // deserialize it with the content of 'c_1'
	std::cout << "\n############################################\n" << std::endl;
	
	std::cout << "Circle c_2:\n" << IKIGAI::UTILS::ToJson(c_2) << std::endl;
	
	return 0;
	*/

	IKIGAI::CORE_SYSTEM::App app;
	app.getCore()->sceneManager->loadFromFile("Scenes/scene0.json");
	auto& scene = app.getCore()->sceneManager->getCurrentScene();

	auto sprite1 = IKIGAI::GUI::GuiHelper::CreateSprite("sprite1", std::string("Textures/btn.png"), true);
	auto btn1 = IKIGAI::GUI::GuiHelper::CreateButton("btn1", true);
	auto label1 = IKIGAI::GUI::GuiHelper::CreateLabel("label1", std::string("HELLO"), false);
	label1->setParent(sprite1);

	auto clip = IKIGAI::GUI::GuiHelper::CreateClip("c", 150, 55, true);
	auto scroll = IKIGAI::GUI::GuiHelper::CreateScroll("scroll", 250, 55, false);
	scroll->setParent(clip);
	auto layout = IKIGAI::GUI::GuiHelper::CreateLayout("l", IKIGAI::ECS::LayoutComponent::Type::HORIZONTAL, false);
	layout->setParent(scroll);
	for (int i = 0; i < 5 ; i++) {
		auto s = IKIGAI::GUI::GuiHelper::CreateSprite("s" + std::to_string(i), std::string("Textures/btn.png"), false);
		s->setParent(layout);
	}

	//auto shader = std::make_shared<ShaderGl>("Shaders/Standard_.vs", "Shaders/Standard_.fs");

	//{
	//	auto obj = scene.createObject("DirLight");
	//	auto light = obj->addComponent<IKIGAI::ECS::DirectionalLight>();
	//	light->orthoBoxSize = 100;
	//	light->distance = 100;
	//	light->setIntensity(1.0f);
	//	light->setColor({ 1.0f, 1.0f, 1.0f });
	//	obj->getTransform()->setLocalPosition({ -20.0f, 40.0f, 10.0f });
	//	obj->getTransform()->setLocalRotation({ 0.81379771, -0.17101006, 0.29619816, 0.46984628 });
	//	obj->getTransform()->setLocalScale({ 1.5f, 1.5f, 1.5f });
	//}
	//{
	//	auto obj = scene.createObject("Camera");
	//	auto cam = obj->addComponent<IKIGAI::ECS::CameraComponent>();
	//	cam->setFov(45.0f);
	//	cam->setSize(5.0f);
	//	cam->setNear(0.1f);
	//	cam->setFar(1000.0f);
	//	cam->setFrustumGeometryCulling(false);
	//	cam->setFrustumLightCulling(false);
	//	cam->setProjectionMode(IKIGAI::RENDER::Camera::ProjectionMode::PERSPECTIVE);
	//	obj->getTransform()->setLocalPosition({ 0.f, 140.0f, 0.0f });
	//	obj->getTransform()->setLocalRotation({ 0.0f, 0.98480773f, -0.17364819f, 0.0f });
	//	obj->getTransform()->setLocalScale({ 1.0f, 1.0f, 1.0f });
	//	obj->addComponent<IKIGAI::ECS::ScriptComponent>("Controller");
	//
	//	//auto cam = obj->addComponent<IKIGAI::ECS::ArCameraComponent>();
	//
	//}
	//{
	//	auto& obj = scene.createObject("Box3");
	//
	//	auto model = obj->addComponent<IKIGAI::ECS::ModelRenderer>();
	//	IKIGAI::RESOURCES::ModelParserFlags flags = IKIGAI::RESOURCES::ModelParserFlags::TRIANGULATE;
	//	flags |= IKIGAI::RESOURCES::ModelParserFlags::GEN_SMOOTH_NORMALS;
	//	flags |= IKIGAI::RESOURCES::ModelParserFlags::FLIP_UVS;
	//	flags |= IKIGAI::RESOURCES::ModelParserFlags::GEN_UV_COORDS;
	//	flags |= IKIGAI::RESOURCES::ModelParserFlags::CALC_TANGENT_SPACE;
	//
	//	//auto m = IKIGAI::RESOURCES::ModelLoader::Create("C:\\Projects\\SimpleEngine\\CGCourse\\CGCourse\\Assets\\User\\cottage_fbx.fbx", flags);
	//	auto m = IKIGAI::RESOURCES::ModelLoader::Create("C:\\Projects\\SimpleEngine\\CGCourse\\CGCourse\\Assets\\Engine\\Models\\Cube.fbx", flags);
	//	model->setModel(m);
	//	model->setFrustumBehaviour(IKIGAI::ECS::ModelRenderer::EFrustumBehaviour::CULL_MODEL);
	//	auto bs = IKIGAI::RENDER::BoundingSphere();
	//	bs.position = { 0.0f, 0.0f, 0.0f };
	//	bs.radius = 1.0f;
	//	model->setCustomBoundingSphere(bs);
	//	obj->getTransform()->setLocalPosition({ 0.0f, 0.0f, 0.0f });
	//	obj->getTransform()->setLocalRotation({ 0.0f, 0.0f, 0.0f, 1.0f });
	//	//obj.getTransform()->setLocalScale({100.01f, 100.01f, 100.01f});
	//	obj->getTransform()->setLocalScale({ 1.f, 1.f, 1.f });
	//
	//	auto s = IKIGAI::RESOURCES::ShaderLoader::CreateFromFile("Shaders\\Base.glsl");
	//
	//	auto mat = obj->addComponent<IKIGAI::ECS::MaterialRenderer>();
	//	auto _m = IKIGAI::RESOURCES::MaterialLoader::Create("");
	//	//_m->isDeferred = true;
	//	_m->setShader(s);
	//	_m->setBlendable(false);
	//	_m->setBackfaceCulling(true);
	//	_m->setFrontfaceCulling(false);
	//	_m->setDepthTest(true);
	//	_m->setDepthWriting(true);
	//	_m->setColorWriting(true);
	//	_m->setGPUInstances(1);
	//	auto& data = _m->getUniformsData();
	//	//data["u_Albedo"] = IKIGAI::MATHGL::Vector4{ 1.0f, 1.0f, 1.0f, 1.0f };
	//
	//	auto tex1 = IKIGAI::RESOURCES::TextureLoader().CreateFromFile("textures\\brick_albedo.jpg");
	//	data["u_DiffuseMap"] = tex1;
	//
	//	//data["u_EnableNormalMapping"] = true;
	//	//data["u_HeightScale"] = 0.0f;
	//
	//	//auto tex2 = IKIGAI::RESOURCES::TextureLoader().CreateFromFile("textures\\brick_normal.jpg");
	//	//data["u_NormalMap"] = tex2;
	//	//auto tex3 = IKIGAI::RESOURCES::TextureLoader().CreateFromFile("textures\\noiseTexture.png");
	//	//data["u_Noise"] = tex3;
	//	//data["fogScaleBias"] = IKIGAI::MATHGL::Vector4(0, -0.06f, 0, 0.0008f);
	//	//data["u_Shininess"] = 100;
	//	//data["u_Specular"] = IKIGAI::MATHGL::Vector3{ 1.0f, 1.0f, 1.0f };
	//	//data["u_TextureOffset"] = IKIGAI::MATHGL::Vector2f{ 0.0f, 0.0f };
	//	//data["u_TextureTiling"] = IKIGAI::MATHGL::Vector2f{ 1.0f, 1.0f };
	//	//data["u_UseBone"] = false;
	//	mat->fillWithMaterial(_m);
	//}
	app.run();
	return 0;
}
/*
int main() {

	//example();

	IKIGAI::CORE_SYSTEM::App app;
	auto& scene = app.getCore()->sceneManager->getCurrentScene();
	{
		auto rpos = []() {
			auto LO = -100.0f;
			auto HI = 100.0f;
			return  LO + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (HI - LO)));
		};
		auto rc = []() {
			auto LO = 0.3f;
			auto HI = 1.0f;
			return  255.0f * (LO + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (HI - LO))));
		};
		for (int i = 0; i < 1; i++) {
			auto obj = scene.createObject("PointLight");
			auto light = obj->addComponent<IKIGAI::ECS::PointLight>();
			light->setIntensity(0.90f);
			light->setQuadratic(0.90f);
			light->setColor({255.0f, 200.0f, 255.0f});
			light->Radius = 25.0f;
			//obj.transform->setLocalPosition({rpos(), rpos(), rpos()});
			obj->getTransform()->setLocalPosition({0, 50, 0});
			obj->getTransform()->setLocalRotation({0.0f, 0.0f, 0.0f, 1.0f});
			obj->getTransform()->setLocalScale({1.0f, 1.0f, 1.0f});
		}
	}

	{


		for (auto i = 0; i < 1; i++) {
			auto obj = scene.createObject("DirLight");
			auto light = obj->addComponent<IKIGAI::ECS::DirectionalLight>();
			//light->shadowRes = 2048;
			light->orthoBoxSize = 100;
			//light->zNear = 1.0f;
			//light->zFar = 700.0f;
			//light->strength = 1.0f;
			light->distance = 100;

			light->setIntensity(1.0f);

			light->setColor({1.0f, 1.0f, 1.0f});
			obj->getTransform()->setLocalPosition({ -20.0f, 40.0f, 10.0f });
			obj->getTransform()->setLocalRotation({ 0.81379771, -0.17101006, 0.29619816, 0.46984628 });
			obj->getTransform()->setLocalScale({1.5f, 1.5f, 1.5f});
			//obj->addScript("ControllerDirLight");
		}
	}

	{
		auto obj = scene.createObject("Point2Light");
		auto light = obj->addComponent<IKIGAI::ECS::PointLight>();
		//light->shadowRes = 2048;
		//light->zNear = 1.0f;
		//light->zFar = 2000.0f;
		//light->strength = 1.0f;
		//light->aspect = 1.0f;
	
		light->setIntensity(1.0f);
		light->setQuadratic(1.0f);
		light->setLinear(1.0f);
		light->setColor({1.0f, 1.0f, 1.0f});
		obj->getTransform()->setLocalPosition({0.0f, 14.0f, -45.0f});
		obj->getTransform()->setLocalRotation({0.81379771f, -0.17101006f, 0.29619816f, 0.46984628f});
		obj->getTransform()->setLocalScale({1.0f, 1.0f, 1.0f});
	}
	{
		auto obj = scene.createObject("Point3Light");
		auto light = obj->addComponent<IKIGAI::ECS::PointLight>();
		//light->shadowRes = 2048;
		//light->zNear = 1.0f;
		//light->zFar = 2000.0f;
		//light->strength = 1.0f;
		//light->aspect = 1.0f;
	
		light->setIntensity(1.0f);
		light->setQuadratic(1.0f);
		light->setLinear(1.0f);
		light->setColor({255.0f, 255.0f, 255.0f});
		obj->getTransform()->setLocalPosition({-20.0f, 40.0f, 10.0f});
		obj->getTransform()->setLocalRotation({0.81379771f, -0.17101006f, 0.29619816f, 0.46984628f});
		obj->getTransform()->setLocalScale({1.0f, 1.0f, 1.0f});
	}
	{
		auto obj = scene.createObject("Point3Light");
		auto light = obj->addComponent<IKIGAI::ECS::PointLight>();
		//light->shadowRes = 2048;
		//light->zNear = 1.0f;
		//light->zFar = 2000.0f;
		//light->strength = 1.0f;
		//light->aspect = 1.0f;
	
		light->setIntensity(1.0f);
		light->setQuadratic(1.0f);
		light->setLinear(1.0f);
		light->setColor({255.0f, 255.0f, 255.0f});
		obj->getTransform()->setLocalPosition({-20.0f, 35.0f, 15.0f});
		obj->getTransform()->setLocalRotation({0.81379771f, -0.17101006f, 0.29619816f, 0.46984628f});
		obj->getTransform()->setLocalScale({1.0f, 1.0f, 1.0f});
	}

	{
		auto obj = scene.createObject("AmbLight");

		nlohmann::json j;
		obj->getTransform()->onSerialize(j);
		obj->getTransform()->onDeserialize(j);
		auto s = j.dump(2);

		auto light = obj->addComponent<IKIGAI::ECS::AmbientSphereLight>();
		light->setIntensity(1.0f);
		light->setColor({1.0f, 0.7f, 0.8f});
		light->setRadius(1.0f);
		obj->getTransform()->setLocalPosition({0.0f, 0.0f, 0.0f});
		obj->getTransform()->setLocalRotation({0.0f, 0.0f, 0.0f, 1.0f});
		obj->getTransform()->setLocalScale({1.0f, 1.0f, 1.0f});
	}
	{
		auto obj = scene.createObject("Camera");
		auto cam = obj->addComponent<IKIGAI::ECS::CameraComponent>();
		cam->setFov(45.0f);
		cam->setSize(5.0f);
		cam->setNear(0.1f);
		cam->setFar(1000.0f);
		cam->setFrustumGeometryCulling(false);
		cam->setFrustumLightCulling(false);
		cam->setProjectionMode(IKIGAI::RENDER::Camera::ProjectionMode::PERSPECTIVE);
		obj->getTransform()->setLocalPosition({ 0.f, 140.0f, 0.0f });
		obj->getTransform()->setLocalRotation({ 0.0f, 0.98480773f, -0.17364819f, 0.0f });
		obj->getTransform()->setLocalScale({1.0f, 1.0f, 1.0f});
		obj->addComponent<IKIGAI::ECS::ScriptComponent>("Controller");


		//VR test
		//auto vrCam = obj->addComponent<IKIGAI::ECS::VrCameraComponent>();
		//
		//{
		//	auto camLeft = scene.createObject("CameraLeft");
		//	auto cam = camLeft->addComponent<IKIGAI::ECS::CameraComponent>();
		//	cam->setFov(45.0f);
		//	cam->setSize(5.0f);
		//	cam->setNear(0.1f);
		//	cam->setFar(1000.0f);
		//	cam->setFrustumGeometryCulling(false);
		//	cam->setFrustumLightCulling(false);
		//	cam->setProjectionMode(IKIGAI::RENDER::Camera::ProjectionMode::PERSPECTIVE);
		//	vrCam->left = camLeft;
		//}
		//{
		//	auto camRight = scene.createObject("CameraRight");
		//	auto cam = camRight->addComponent<IKIGAI::ECS::CameraComponent>();
		//	cam->setFov(45.0f);
		//	cam->setSize(5.0f);
		//	cam->setNear(0.1f);
		//	cam->setFar(1000.0f);
		//	cam->setFrustumGeometryCulling(false);
		//	cam->setFrustumLightCulling(false);
		//	cam->setProjectionMode(IKIGAI::RENDER::Camera::ProjectionMode::PERSPECTIVE);
		//	vrCam->right = camRight;
		//}
		//auto inp = obj.addComponent<IKIGAI::ECS::InputComponent>([&app](float dt) {
		//	auto& core = app.getCore();
		//	if (core.inputManager->isKeyPressed(IKIGAI::INPUT_SYSTEM::EKey::KEY_S)) {
		//		auto pos = core.sceneManager.getCurrentScene()->findObjectByName("Camera")->transform->getLocalPosition();
		//		pos.z += 1.0f;
		//		core.sceneManager.getCurrentScene()->findObjectByName("Camera")->transform->setLocalPosition(pos);
		//	}
		//	if (core.inputManager->isKeyPressed(IKIGAI::INPUT_SYSTEM::EKey::KEY_W)) {
		//		auto pos = core.sceneManager.getCurrentScene()->findObjectByName("Camera")->transform->getLocalPosition();
		//		pos.z -= 1.0f;
		//		core.sceneManager.getCurrentScene()->findObjectByName("Camera")->transform->setLocalPosition(pos);
		//	}
		//
		//	if (core.inputManager->isKeyPressed(IKIGAI::INPUT_SYSTEM::EKey::KEY_A)) {
		//		auto pos = core.sceneManager.getCurrentScene()->findObjectByName("Camera")->transform->getLocalPosition();
		//		pos.x -= 1.0f;
		//		core.sceneManager.getCurrentScene()->findObjectByName("Camera")->transform->setLocalPosition(pos);
		//	}
		//	if (core.inputManager->isKeyPressed(IKIGAI::INPUT_SYSTEM::EKey::KEY_D)) {
		//		auto pos = core.sceneManager.getCurrentScene()->findObjectByName("Camera")->transform->getLocalPosition();
		//		pos.x += 1.0f;
		//		core.sceneManager.getCurrentScene()->findObjectByName("Camera")->transform->setLocalPosition(pos);
		//	}
		//
		//	if (core.inputManager->isKeyPressed(IKIGAI::INPUT_SYSTEM::EKey::KEY_Q)) {
		//		auto pos = core.sceneManager.getCurrentScene()->findObjectByName("Camera")->transform->getLocalPosition();
		//		pos.y -= 1.0f;
		//		core.sceneManager.getCurrentScene()->findObjectByName("Camera")->transform->setLocalPosition(pos);
		//	}
		//	if (core.inputManager->isKeyPressed(IKIGAI::INPUT_SYSTEM::EKey::KEY_E)) {
		//		auto pos = core.sceneManager.getCurrentScene()->findObjectByName("Camera")->transform->getLocalPosition();
		//		pos.y += 1.0f;
		//		core.sceneManager.getCurrentScene()->findObjectByName("Camera")->transform->setLocalPosition(pos);
		//	}
		//
		//		static bool isFirst = true;
		//	static std::pair<int, int> prev;
		//
		//	auto mpos = core.inputManager->getMousePosition();
		//	if (isFirst) {
		//		prev = mpos;
		//		isFirst = false;
		//	}
		//	static IKIGAI::MATHGL::Vector2f offset(0.0f, 0.0f);
		//	static IKIGAI::MATHGL::Vector2f look(0.0f, 0.0f);
		//
		//	offset.x = mpos.first - prev.first;
		//	offset.y = mpos.second - prev.second;
		//
		//	prev = mpos;
		//
		//	offset = offset * 0.5f;
		//
		//	look = look + offset;
		//
		//	if (look.y > 89.0f) look.y = 89;
		//	if (look.y < -89.0f) look.y = -89;
		//
		//	if (core.inputManager->isKeyPressed(IKIGAI::INPUT_SYSTEM::EKey::KEY_Z)) {
		//		core.sceneManager.getCurrentScene()->findObjectByName("Camera")->transform->setLocalRotation(
		//			IKIGAI::MATHGL::Quaternion(IKIGAI::MATHGL::Vector3(look.y, -look.x, 0)));
		//	}
		//	auto pos = core.sceneManager.getCurrentScene()->findObjectByName("Camera")->transform->getLocalPosition();
		//
		//});

		//world = new Game::World(42, IKIGAI::MATHGL::Vector2f(800, 600), "test", Game::Generation_Normal, &obj);
		//app->renderer.world = world;
	}
	//{
	//	auto& obj = scene->createObject("A");
	//
	//	auto model = obj.addComponent<IKIGAI::ECS::ModelRenderer>();
	//	IKIGAI::RESOURCES::ModelParserFlags flags = IKIGAI::RESOURCES::ModelParserFlags::TRIANGULATE;
	//	flags |= IKIGAI::RESOURCES::ModelParserFlags::GEN_SMOOTH_NORMALS;
	//	flags |= IKIGAI::RESOURCES::ModelParserFlags::FLIP_UVS;
	//
	//	//flags |= IKIGAI::RESOURCES::ModelParserFlags::OPTIMIZE_MESHES;
	//	//flags |= IKIGAI::RESOURCES::ModelParserFlags::OPTIMIZE_GRAPH;
	//	//flags |= IKIGAI::RESOURCES::ModelParserFlags::FIND_INSTANCES;
	//	//flags |= IKIGAI::RESOURCES::ModelParserFlags::CALC_TANGENT_SPACE;
	//	//flags |= IKIGAI::RESOURCES::ModelParserFlags::JOIN_IDENTICAL_VERTICES;
	//	//flags |= IKIGAI::RESOURCES::ModelParserFlags::DEBONE;
	//	//flags |= IKIGAI::RESOURCES::ModelParserFlags::FIND_INVALID_DATA;
	//	//flags |= IKIGAI::RESOURCES::ModelParserFlags::IMPROVE_CACHE_LOCALITY;
	//	flags |= IKIGAI::RESOURCES::ModelParserFlags::GEN_UV_COORDS;
	//	//flags |= IKIGAI::RESOURCES::ModelParserFlags::PRE_TRANSFORM_VERTICES;
	//
	//	//auto m = IKIGAI::RESOURCES::ModelLoader::Create("C:\\Projects\\SimpleEngine\\CGCourse\\CGCourse\\Assets\\guard\\boblampclean.md5mesh", flags);
	//	auto m = IKIGAI::RESOURCES::ModelLoader::Create("C:\\Projects\\SimpleEngine\\CGCourse\\CGCourse\\Assets\\Game\\textures\\dancing_vampire.dae", flags);
	//	//auto m = IKIGAI::RESOURCES::ModelLoader::Create("C:\\Projects\\SimpleEngine\\CGCourse\\CGCourse\\Assets\\User\\cottage_fbx.fbx", flags);
	//	//auto m = IKIGAI::RESOURCES::ModelLoader::Create("C:\\Projects\\SimpleEngine\\CGCourse\\CGCourse\\Assets\\User\\stone\\Stonefbx.fbx", flags);
	//	//auto m = IKIGAI::RESOURCES::ModelLoader::Create("C:\\Projects\\SimpleEngine\\CGCourse\\CGCourse\\Assets\\astroBoy_walk_Maya.dae", flags);
	//	//auto m = IKIGAI::RESOURCES::ModelLoader::Create("C:\\Projects\\SimpleEngine\\CGCourse\\CGCourse\\Assets\\Drone.fbx", flags);
	//
	//	danceAnimation = std::make_shared<IKIGAI::RESOURCES::Animation>("C:\\Projects\\SimpleEngine\\CGCourse\\CGCourse\\Assets\\Game\\textures\\dancing_vampire.dae", m.get());
	//	animator = std::make_shared<IKIGAI::RESOURCES::Animator>(danceAnimation.get());
	//	IKIGAI::RENDER::BaseRender::animator = animator;
	//	obj.addComponent<IKIGAI::ECS::InputComponent>([](float dt) {
	//		if (animator)
	//		animator->UpdateAnimation(dt);
	//	});
	//	model->setModel(m);
	//	model->setFrustumBehaviour(IKIGAI::ECS::ModelRenderer::EFrustumBehaviour::CULL_MODEL);
	//	auto bs = IKIGAI::RENDER::BoundingSphere();
	//	bs.position = {0.0f, 0.0f, 0.0f};
	//	bs.radius = 1.0f;
	//	model->setCustomBoundingSphere(bs);
	//	obj.transform->setLocalPosition({0.0f, 0.0f, 0.0f});
	//	obj.transform->setLocalRotation({0.0f, 0.0f, 0.0f, 1.0f});
	//	//obj.transform->setLocalScale({100.01f, 100.01f, 100.01f});
	//	obj.transform->setLocalScale({1.0f, 1.0f, 1.0f});
	//
	//	auto s = IKIGAI::RESOURCES::ShaderLoader::Create("C:\\Projects\\SimpleEngine\\CGCourse\\CGCourse\\Assets\\Engine\\Shaders\\Standard.glsl");
	//
	//	auto mat = obj.addComponent<IKIGAI::ECS::MaterialRenderer>();
	//	auto _m = IKIGAI::RESOURCES::MaterialLoader::Create("");
	//
	//	_m->setShader(s);
	//	_m->setBlendable(false);
	//	_m->setBackfaceCulling(true);
	//	_m->setFrontfaceCulling(false);
	//	_m->setDepthTest(true);
	//	_m->setDepthWriting(true);
	//	_m->setColorWriting(true);
	//	_m->setGPUInstances(1);
	//	auto& data = _m->getdata["sData();
	//	data["u_Diffuse"] = IKIGAI::MATHGL::Vector4{1.0f, 1.0f, 1.0f, 1.0f};
	//
	//	auto tex1 = IKIGAI::RESOURCES::TextureLoader().createResource("textures\\Vampire_diffuse.png");
	//	data["u_DiffuseMap"] = tex1;
	//
	//	data["u_EnableNormalMapping"] = 1;
	//	data["u_HeightScale"] = 0;
	//
	//	auto tex2 = IKIGAI::RESOURCES::TextureLoader().createResource("textures\\Vampire_normal.png");
	//	data["u_NormalMap"] = tex2;
	//	data["u_Shininess"] = 100;
	//	data["u_Specular"] = IKIGAI::MATHGL::Vector3{1.0f, 1.0f, 1.0f};
	//	data["u_TextureOffset"] = IKIGAI::MATHGL::Vector2f{0.0f, 0.0f};
	//	data["u_TextureTiling"] = IKIGAI::MATHGL::Vector2f{1.0f, 1.0f};
	//	mat->fillWithMaterial(_m);
	//
	//
	//}

	//{
	//	for (int i = 0; i < 0; i++) {
	//		auto& obj = scene->createObject("AA"+std::to_string(i));
	//
	//		auto model = obj.addComponent<IKIGAI::ECS::ModelRenderer>();
	//		IKIGAI::RESOURCES::ModelParserFlags flags = IKIGAI::RESOURCES::ModelParserFlags::TRIANGULATE;
	//		flags |= IKIGAI::RESOURCES::ModelParserFlags::GEN_SMOOTH_NORMALS;
	//		flags |= IKIGAI::RESOURCES::ModelParserFlags::FLIP_UVS;
	//
	//		//flags |= IKIGAI::RESOURCES::ModelParserFlags::OPTIMIZE_MESHES;
	//		//flags |= IKIGAI::RESOURCES::ModelParserFlags::OPTIMIZE_GRAPH;
	//		//flags |= IKIGAI::RESOURCES::ModelParserFlags::FIND_INSTANCES;
	//		//flags |= IKIGAI::RESOURCES::ModelParserFlags::CALC_TANGENT_SPACE;
	//		//flags |= IKIGAI::RESOURCES::ModelParserFlags::JOIN_IDENTICAL_VERTICES;
	//		//flags |= IKIGAI::RESOURCES::ModelParserFlags::DEBONE;
	//		//flags |= IKIGAI::RESOURCES::ModelParserFlags::FIND_INVALID_DATA;
	//		//flags |= IKIGAI::RESOURCES::ModelParserFlags::IMPROVE_CACHE_LOCALITY;
	//		flags |= IKIGAI::RESOURCES::ModelParserFlags::GEN_UV_COORDS;
	//		//flags |= IKIGAI::RESOURCES::ModelParserFlags::PRE_TRANSFORM_VERTICES;
	//
	//		//auto m = IKIGAI::RESOURCES::ModelLoader::Create("C:\\Projects\\SimpleEngine\\CGCourse\\CGCourse\\Assets\\guard\\boblampclean.md5mesh", flags);
	//		//auto m = IKIGAI::RESOURCES::ModelLoader::Create("C:\\Projects\\SimpleEngine\\CGCourse\\CGCourse\\Assets\\Game\\textures\\dancing_vampire.dae", flags);
	//		//auto m = IKIGAI::RESOURCES::ModelLoader::Create("C:\\Projects\\SimpleEngine\\CGCourse\\CGCourse\\Assets\\Engine\Models\\Cube.fbx", flags);
	//		auto m = IKIGAI::RESOURCES::ModelLoader::Create("C:\\Projects\\SimpleEngine\\CGCourse\\CGCourse\\Assets\\User\\cottage_fbx.fbx", flags);
	//		//auto m = IKIGAI::RESOURCES::ModelLoader::Create("C:\\Projects\\SimpleEngine\\CGCourse\\CGCourse\\Assets\\User\\stone\\Stonefbx.fbx", flags);
	//		//auto m = IKIGAI::RESOURCES::ModelLoader::Create("C:\\Projects\\SimpleEngine\\CGCourse\\CGCourse\\Assets\\astroBoy_walk_Maya.dae", flags);
	//		//auto m = IKIGAI::RESOURCES::ModelLoader::Create("C:\\Projects\\SimpleEngine\\CGCourse\\CGCourse\\Assets\\Drone.fbx", flags);
	//
	//		//danceAnimation = std::make_shared<IKIGAI::RESOURCES::Animation>("C:\\Projects\\SimpleEngine\\CGCourse\\CGCourse\\Assets\\Game\\textures\\dancing_vampire.dae", m.get());
	//		//animator = std::make_shared<IKIGAI::RESOURCES::Animator>(danceAnimation.get());
	//		//IKIGAI::RENDER::BaseRender::animator = animator;
	//		//obj.addComponent<IKIGAI::ECS::InputComponent>([](float dt) {
	//		//	if (animator)
	//		//		animator->UpdateAnimation(dt);
	//		//});
	//		model->setModel(m);
	//		model->setFrustumBehaviour(IKIGAI::ECS::ModelRenderer::EFrustumBehaviour::CULL_MODEL);
	//		auto bs = IKIGAI::RENDER::BoundingSphere();
	//		bs.position = {0.0f, 0.0f, 0.0f};
	//		bs.radius = 1.0f;
	//		model->setCustomBoundingSphere(bs);
	//
	//		auto rpos = []() {
	//			auto LO = -100.0f;
	//			auto HI = 100.0f;
	//			return  LO + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (HI - LO)));
	//		};
	//
	//		obj.transform->setLocalPosition({rpos(), rpos(), rpos()});
	//		obj.transform->setLocalRotation({0.0f, 0.0f, 0.0f, 1.0f});
	//		//obj.transform->setLocalScale({100.01f, 100.01f, 100.01f});
	//		obj.transform->setLocalScale({0.020f, 0.020f, 0.020f});
	//
	//		//auto s = IKIGAI::RESOURCES::ShaderLoader::Create("C:\\Projects\\SimpleEngine\\CGCourse\\CGCourse\\Assets\\Engine\\Shaders\\Standard.glsl");
	//
	//		auto mat = obj.addComponent<IKIGAI::ECS::MaterialRenderer>();
	//		auto _m = IKIGAI::RESOURCES::MaterialLoader::Create("");
	//
	//		//_m->setShader(s);
	//		_m->setBlendable(false);
	//		_m->setBackfaceCulling(true);
	//		_m->setFrontfaceCulling(false);
	//		_m->setDepthTest(true);
	//		_m->setDepthWriting(true);
	//		_m->setColorWriting(true);
	//		_m->setGPUInstances(1);
	//		auto& data = _m->getdata["sData();
	//		//data["u_Diffuse"] = IKIGAI::MATHGL::Vector4{1.0f, 1.0f, 1.0f, 1.0f};
	//		//auto tex1 = IKIGAI::RESOURCES::TextureLoader().createResource("textures\\cottage_diffuse.png");
	//		//data["u_DiffuseMap"] = tex1;
	//		//data["u_EnableNormalMapping"] = 1;
	//		//data["u_HeightScale"] = 0;
	//		//auto tex2 = IKIGAI::RESOURCES::TextureLoader().createResource("textures\\cottage_normal.png");
	//		//data["u_NormalMap"] = tex2;
	//		//data["u_Shininess"] = 100;
	//		//data["u_Specular"] = IKIGAI::MATHGL::Vector3{1.0f, 1.0f, 1.0f};
	//		//data["u_TextureOffset"] = IKIGAI::MATHGL::Vector2f{0.0f, 0.0f};
	//		//data["u_TextureTiling"] = IKIGAI::MATHGL::Vector2f{1.0f, 1.0f};
	//		//mat->fillWithMaterial(_m);
	//
	//		auto tex1 =  IKIGAI::RESOURCES::TextureLoader().createResource("textures\\brick_albedo.jpg");
	//		//uint8_t buffer [] = {0, 200, 0};
	//		//tex1->Load(buffer, 1, 1, 3, false, IKIGAI::RESOURCES::TextureFormat::RGB);
	//		data["AlbedoMap"] = tex1;
	//		//_m->albedoMap = tex1;
	//
	//		auto texM = IKIGAI::RESOURCES::TextureLoader().createResource("textures\\brick_metalic.png");
	//		data["MetallicMap"] = texM;
	//
	//		auto texE = IKIGAI::RESOURCES::TextureLoader().createResource("textures\\brick_metalic.png");
	//		data["EmissiveMap"] = texE;
	//		auto tex2 = IKIGAI::RESOURCES::TextureLoader().createResource("textures\\brick_ao.jpg");
	//		data["AmbientOcclusionMap"] = tex2;
	//		data["BaseColor"] = IKIGAI::MATHGL::Vector3(1.0f, 1.0f, 1.0f);
	//		data["Displacement"] = 0.025f;
	//		data["Emission"] = 0.0f;
	//		auto tex3 = IKIGAI::RESOURCES::TextureLoader().createResource("textures\\brick_height.jpg");
	//		data["HeightMap"] = tex3;
	//		//data["Height_map"] = 20;
	//		data["id"] = 1;
	//		data["MetallicFactor"] = 0.0f;
	//		data["UVMultipliers"] = IKIGAI::MATHGL::Vector2f(1.0f, 1.0f);
	//		//data["metallic_map"] : 18446744073709551615,
	//		data["Name"] = "DefaultMaterial";
	//		auto texN = IKIGAI::RESOURCES::TextureLoader().createResource("textures\\brick_normal.jpg");
	//		data["NormalMap"] = texN;
	//		data["RoughnessRactor"] = 0.75f;
	//		auto tex4 = IKIGAI::RESOURCES::TextureLoader().createResource("textures\\brick_roughness.jpg");
	//		data["RoughnessMap"] = tex4;
	//		data["Transparency"] = 1.0f;
	//		data["RoughnessFactor"] = 0.75f;
	//		mat->fillWithMaterial(_m);
	//	}
	//
	//	for (int i = 0; i < 2; i++) {
	//		auto& obj = scene->createObject("AA" + std::to_string(i));
	//
	//		auto model = obj.addComponent<IKIGAI::ECS::ModelRenderer>();
	//		IKIGAI::RESOURCES::ModelParserFlags flags = IKIGAI::RESOURCES::ModelParserFlags::TRIANGULATE;
	//		//flags |= IKIGAI::RESOURCES::ModelParserFlags::GEN_SMOOTH_NORMALS;
	//		//flags |= IKIGAI::RESOURCES::ModelParserFlags::FLIP_UVS;
	//		//flags |= IKIGAI::RESOURCES::ModelParserFlags::GEN_UV_COORDS;
	//		auto m = IKIGAI::RESOURCES::ModelLoader::Create("C:\\Projects\\SimpleEngine\\CGCourse\\CGCourse\\Assets\\Engine\\Models\\Cube.fbx", flags);
	//
	//		model->setModel(m);
	//		model->setFrustumBehaviour(IKIGAI::ECS::ModelRenderer::EFrustumBehaviour::CULL_MODEL);
	//		auto bs = IKIGAI::RENDER::BoundingSphere();
	//		bs.position = {0.0f, 0.0f, 0.0f};
	//		bs.radius = 1.0f;
	//		model->setCustomBoundingSphere(bs);
	//
	//		auto rpos = []() {
	//			auto LO = -100.0f;
	//			auto HI = 100.0f;
	//			return  LO + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (HI - LO)));
	//		};
	//
	//		if (i == 0) {
	//			obj.transform->setLocalPosition({0.0f, 0.0f, 0.0f});
	//			obj.transform->setLocalRotation({0.0f, 0.0f, 0.0f, 1.0f});
	//			obj.transform->setLocalScale({100.70f, 0.01f, 100.70f});
	//		}
	//		else
	//		{
	//			obj.transform->setLocalPosition({0.0f, 5.0f, 0.0f});
	//			obj.transform->setLocalRotation({0.0f, 0.0f, 0.0f, 1.0f});
	//			obj.transform->setLocalScale({10.0f, 10.0f, 10.0f});
	//		}
	//		auto mat = obj.addComponent<IKIGAI::ECS::MaterialRenderer>();
	//		auto _m = IKIGAI::RESOURCES::MaterialLoader::Create("");
	//		_m->setBlendable(false);
	//		_m->setBackfaceCulling(true);
	//		_m->setFrontfaceCulling(false);
	//		_m->setDepthTest(true);
	//		_m->setDepthWriting(true);
	//		_m->setColorWriting(true);
	//		_m->setGPUInstances(1);
	//		auto& data = _m->getdata["sData();
	//
	//		auto tex1 = IKIGAI::RESOURCES::TextureLoader().createResource("textures\\brick_albedo.jpg");
	//		data["AlbedoMap"] = tex1;
	//
	//		auto texM = IKIGAI::RESOURCES::TextureLoader().createResource("textures\\brick_metalic.png");
	//		data["MetallicMap"] = texM;
	//
	//		auto texE = IKIGAI::RESOURCES::TextureLoader().createResource("textures\\brick_metalic.png");
	//		data["EmissiveMap"] = texE;
	//		auto tex2 = IKIGAI::RESOURCES::TextureLoader().createResource("textures\\brick_ao.jpg");
	//		data["AmbientOcclusionMap"] = tex2;
	//		data["BaseColor"] = IKIGAI::MATHGL::Vector3(1.0f, 1.0f, 1.0f);
	//		data["Displacement"] = 0.025f;
	//		data["Emission"] = 0.0f;
	//		auto tex3 = IKIGAI::RESOURCES::TextureLoader().createResource("textures\\brick_height.jpg");
	//		data["HeightMap"] = tex3;
	//		//data["Height_map"] = 20;
	//		data["id"] = 1;
	//		data["MetallicFactor"] = 0.0f;
	//		data["UVMultipliers"] = IKIGAI::MATHGL::Vector2f(1.0f, 1.0f);
	//		//data["metallic_map"] : 18446744073709551615,
	//		data["Name"] = "DefaultMaterial";
	//		auto texN = IKIGAI::RESOURCES::TextureLoader().createResource("textures\\brick_normal.jpg");
	//		data["NormalMap"] = texN;
	//		data["RoughnessRactor"] = 0.75f;
	//		auto tex4 = IKIGAI::RESOURCES::TextureLoader().createResource("textures\\brick_roughness.jpg");
	//		data["RoughnessMap"] = tex4;
	//		data["Transparency"] = 1.0f;
	//		data["RoughnessFactor"] = 0.75f;
	//		mat->fillWithMaterial(_m);
	//	}
	//}
	//{
	//	auto& obj = scene.createObject("AAA");
	//
	//	auto model = obj->addComponent<IKIGAI::ECS::ModelRenderer>();
	//	IKIGAI::RESOURCES::ModelParserFlags flags = IKIGAI::RESOURCES::ModelParserFlags::TRIANGULATE;
	//	flags |= IKIGAI::RESOURCES::ModelParserFlags::GEN_SMOOTH_NORMALS;
	//	
	//	flags |= IKIGAI::RESOURCES::ModelParserFlags::CALC_TANGENT_SPACE;
	//	auto m = IKIGAI::RESOURCES::ModelLoader::Create("C:\\Projects\\SimpleEngine\\CGCourse\\CGCourse\\Assets\\Game\\textures\\dancing_vampire.dae", flags);
	//	
	//	model->setModel(m);
	//	model->setFrustumBehaviour(IKIGAI::ECS::ModelRenderer::EFrustumBehaviour::CULL_MODEL);
	//	auto bs = IKIGAI::RENDER::BoundingSphere();
	//	bs.position = {0.0f, 0.0f, 0.0f};
	//	bs.radius = 1.0f;
	//	model->setCustomBoundingSphere(bs);
	//	obj->transform->setLocalPosition({0.0f, 0.0f, -40.0f});
	//	obj->transform->setLocalRotation({0.0f, 0.0f, 0.0f, 1.0f});
	//	//obj.transform->setLocalScale({100.01f, 100.01f, 100.01f});
	//	obj->transform->setLocalScale({10.20f, 10.20f, 10.20f});
	//
	//	obj->addComponent<IKIGAI::ECS::Skeletal>("C:\\Projects\\SimpleEngine\\CGCourse\\CGCourse\\Assets\\Game\\textures\\dancing_vampire.dae", "");
	//
	//
	//	auto s = IKIGAI::RESOURCES::ShaderLoader::CreateFromFile("Shaders\\Standard.glsl");
	//
	//	auto mat = obj->addComponent<IKIGAI::ECS::MaterialRenderer>();
	//
	//	auto mm = IKIGAI::RESOURCES::MaterialLoader::CreateFromFile("Materials/vampire.mat");
	//	mat->fillWithMaterial(mm);
	//	
	//}


	{
		auto obj = scene.createObject("Plane");

		auto model = obj->addComponent<IKIGAI::ECS::ModelRenderer>();
		IKIGAI::RESOURCES::ModelParserFlags flags = IKIGAI::RESOURCES::ModelParserFlags::TRIANGULATE;
		flags |= IKIGAI::RESOURCES::ModelParserFlags::GEN_SMOOTH_NORMALS;
		flags |= IKIGAI::RESOURCES::ModelParserFlags::FLIP_UVS;
		flags |= IKIGAI::RESOURCES::ModelParserFlags::GEN_UV_COORDS;
		flags |= IKIGAI::RESOURCES::ModelParserFlags::CALC_TANGENT_SPACE;

		//auto m = IKIGAI::RESOURCES::ModelLoader::Create("C:\\Projects\\SimpleEngine\\CGCourse\\CGCourse\\Assets\\User\\cottage_fbx.fbx", flags);
		auto m = IKIGAI::RESOURCES::ModelLoader::Create("C:\\Projects\\SimpleEngine\\CGCourse\\CGCourse\\Assets\\Engine\\Models\\Cube.fbx", flags);
		model->setModel(m);
		model->setFrustumBehaviour(IKIGAI::ECS::ModelRenderer::EFrustumBehaviour::CULL_MODEL);
		auto bs = IKIGAI::RENDER::BoundingSphere();
		bs.position = {0.0f, 0.0f, 0.0f};
		bs.radius = 100.0f;
		model->setCustomBoundingSphere(bs);
		obj->getTransform()->setLocalPosition({0.0f, -5.0f, 10.0f});
		obj->getTransform()->setLocalRotation({0.0f, 0.0f, 0.0f, 1.0f});
		//obj.transform->setLocalScale({100.01f, 100.01f, 100.01f});
		obj->getTransform()->setLocalScale({300.0f, 1.0f, 300.0f});

		auto s = IKIGAI::RESOURCES::ShaderLoader::CreateFromFile("Shaders\\Standard.glsl");

		auto mat = obj->addComponent<IKIGAI::ECS::MaterialRenderer>();
		auto _m = IKIGAI::RESOURCES::MaterialLoader::Create("");

		//s = app.renderer.context.renderer->shadersMap["deferredGBuffer"];
		_m->setShader(s);
		_m->setBlendable(false);
		_m->setBackfaceCulling(true);
		_m->setFrontfaceCulling(false);
		_m->setDepthTest(true);
		_m->setDepthWriting(true);
		_m->setColorWriting(true);
		_m->setGPUInstances(1);
		_m->isDeferred = true;
		auto& data = _m->getUniformsData();
		data["u_Diffuse"] = IKIGAI::MATHGL::Vector4{1.0f, 1.0f, 1.0f, 1.0f};

		auto tex1 = IKIGAI::RESOURCES::TextureLoader().CreateFromFile("textures\\brick_albedo.jpg");
		data["u_DiffuseMap"] = tex1;

		data["u_EnableNormalMapping"] = true;
		data["u_HeightScale"] = 0.0f;

		auto tex2 = IKIGAI::RESOURCES::TextureLoader().CreateFromFile("textures\\brick_normal.jpg");
		data["u_NormalMap"] = tex2;
		auto tex3 = IKIGAI::RESOURCES::TextureLoader().CreateFromFile("textures\\noiseTexture.png");
		data["u_Noise"] = tex3;
		data["fogScaleBias"] = IKIGAI::MATHGL::Vector4(0, -0.06f, 0, 0.0008f);
		data["u_Shininess"] = 100;
		data["u_Specular"] = IKIGAI::MATHGL::Vector3{1.0f, 1.0f, 1.0f};
		data["u_TextureOffset"] = IKIGAI::MATHGL::Vector2f{0.0f, 0.0f};
		data["u_TextureTiling"] = IKIGAI::MATHGL::Vector2f{1.0f, 1.0f};
		data["u_UseBone"] = false;
		mat->fillWithMaterial(_m);


		//auto pc = obj->addComponent<IKIGAI::ECS::PhysicsComponent>();
		auto body = std::make_shared<IKIGAI::PHYSICS::RigidBody>();
		body->setPosition(obj->getTransform()->getLocalPosition().x, obj->getTransform()->getLocalPosition().y, obj->getTransform()->getLocalPosition().z);
		body->setOrientation(1, 0, 0, 0);
		body->setVelocity(0, 0, 0);
		body->setRotation(0, 0, 0);

		body->setInverseMass(0);

		IKIGAI::MATHGL::Matrix3 tensor = IKIGAI::MATHGL::Matrix3(0.0f);
		tensor.setBlockInertiaTensor(IKIGAI::MATHGL::Vector3{30, 1.f, 30} *0.5f, 0);
		body->setInertiaTensor(tensor);

		body->setLinearDamping(0.95f);
		body->setAngularDamping(0.8f);
		body->clearAccumulators();
		body->setAcceleration(0, 0, 0);

		body->setCanSleep(false);
		body->setAwake();

		body->calculateDerivedData();
		auto colider = std::make_unique<IKIGAI::PHYSICS::CollisionBox>();
		colider->halfSize = {30.0f, 1.0f, 30.0f} * 0.5f;
		//colider->transform = obj->transform->getLocalMatrix();
		colider->body = body.get();
		colider->calculateInternals();

		auto pc = obj->addComponent<IKIGAI::ECS::PhysicsComponent>();
		pc->body = body;
		pc->collider = std::move(colider);
		pc->boundingSphere = bs;
		pc->collisionType = IKIGAI::ECS::CollisionType::OBB;

		//app.getCore().physicsManger->bodies.push_back(body);
		//app.getCore().physicsManger->bodiesCollide.push_back(colider);
		app.getCore()->physicsManger->objects.Insert(obj, pc->boundingSphere);

		auto inp = obj->addComponent<IKIGAI::ECS::InputComponent>([body, obj](std::chrono::duration<double> dt) {
			//if (pc->body)
			obj->getTransform()->setLocalPosition(IKIGAI::MATHGL::Vector3(body->getPosition().x, body->getPosition().y, body->getPosition().z));
			obj->getTransform()->setLocalRotation(IKIGAI::MATHGL::Quaternion(
				body->getOrientation().x, body->getOrientation().y, body->getOrientation().z, body->getOrientation().w));

			//pc->collider->transform = pc->obj.transform->getLocalMatrix();
			//pc->boundingSphere.position = pc->obj.transform->getLocalPosition();
			//if (pc->obj.transform->getLocalPosition().y < -500.0f) {
			//	pc->obj.transform->setLocalPosition(IKIGAI::MATHGL::Vector3(obj->transform->getLocalPosition().x, 500.0f, obj->transform->getLocalPosition().z));
			//}
		});
	}

	{
		auto obj = scene.createObject("Box1");

		auto model = obj->addComponent<IKIGAI::ECS::ModelRenderer>();
		IKIGAI::RESOURCES::ModelParserFlags flags = IKIGAI::RESOURCES::ModelParserFlags::TRIANGULATE;
		//flags |= IKIGAI::RESOURCES::ModelParserFlags::GEN_SMOOTH_NORMALS;
		flags |= IKIGAI::RESOURCES::ModelParserFlags::FLIP_UVS;
		flags |= IKIGAI::RESOURCES::ModelParserFlags::GEN_UV_COORDS;
		flags |= IKIGAI::RESOURCES::ModelParserFlags::CALC_TANGENT_SPACE;

		//auto m = IKIGAI::RESOURCES::ModelLoader::Create("C:\\Projects\\SimpleEngine\\CGCourse\\CGCourse\\Assets\\User\\cottage_fbx.fbx", flags);
		auto m = IKIGAI::RESOURCES::ModelLoader::Create("C:\\Projects\\SimpleEngine\\CGCourse\\CGCourse\\Assets\\Engine\\Models\\Cube.fbx", flags);
		model->setModel(m);
		model->setFrustumBehaviour(IKIGAI::ECS::ModelRenderer::EFrustumBehaviour::CULL_MODEL);
		auto bs = IKIGAI::RENDER::BoundingSphere();
		bs.position = {0.0f, 0.0f, 0.0f};
		bs.radius = 10.50f;
		model->setCustomBoundingSphere(bs);
		obj->getTransform()->setLocalPosition({0.0f, 55.0f, 0.0f});
		obj->getTransform()->setLocalRotation({0.0f, 0.0f, 0.0f, 1.0f});
		//obj.transform->setLocalScale({100.01f, 100.01f, 100.01f});
		obj->getTransform()->setLocalScale({10.5f, 10.5f, 10.5f});

		auto s = IKIGAI::RESOURCES::ShaderLoader::CreateFromFile("Shaders\\Standard.glsl");

		auto mat = obj->addComponent<IKIGAI::ECS::MaterialRenderer>();
		auto _m = IKIGAI::RESOURCES::MaterialLoader::Create("");

		_m->setShader(s);
		_m->setBlendable(false);
		_m->setBackfaceCulling(true);
		_m->setFrontfaceCulling(false);
		_m->setDepthTest(true);
		_m->setDepthWriting(true);
		_m->setColorWriting(true);
		_m->setGPUInstances(1);
		auto& data = _m->getUniformsData();
		data["u_Diffuse"] = IKIGAI::MATHGL::Vector4{1.0f, 1.0f, 1.0f, 1.0f};

		auto tex1 = IKIGAI::RESOURCES::TextureLoader().CreateFromFile("textures\\brick_albedo.jpg");
		data["u_DiffuseMap"] = tex1;

		data["u_EnableNormalMapping"] = true;
		data["u_HeightScale"] = 0.0f;

		auto tex2 = IKIGAI::RESOURCES::TextureLoader().CreateFromFile("textures\\brick_normal.jpg");
		data["u_NormalMap"] = tex2;
		auto tex3 = IKIGAI::RESOURCES::TextureLoader().CreateFromFile("textures\\noiseTexture.png");
		data["u_Noise"] = tex3;
		data["fogScaleBias"] = IKIGAI::MATHGL::Vector4(0, -0.06f, 0, 0.0008f);
		data["u_Shininess"] = 100;
		data["u_Specular"] = IKIGAI::MATHGL::Vector3{1.0f, 1.0f, 1.0f};
		data["u_TextureOffset"] = IKIGAI::MATHGL::Vector2f{0.0f, 0.0f};
		data["u_TextureTiling"] = IKIGAI::MATHGL::Vector2f{1.0f, 1.0f};
		data["u_UseBone"] = false;
		mat->fillWithMaterial(_m);

		auto pc = obj->addComponent<IKIGAI::ECS::PhysicsComponent>();
		auto body = std::make_shared<IKIGAI::PHYSICS::RigidBody>();
		body->setPosition(obj->getTransform()->getLocalPosition().x, obj->getTransform()->getLocalPosition().y, obj->getTransform()->getLocalPosition().z);
		body->setOrientation(1, 0, 0, 0);
		body->setVelocity(0, 0, 0);
		body->setRotation(0, 0, 0);
		//halfSize = cyclone::Vector3(1, 1, 1);
		float mass = 5555.0f;
		body->setMass(mass);

		IKIGAI::MATHGL::Matrix3 tensor = IKIGAI::MATHGL::Matrix3(0.0f);
		tensor.setBlockInertiaTensor(IKIGAI::MATHGL::Vector3{10.5f, 10.5f, 10.5f} *0.5f, mass);
		body->setInertiaTensor(tensor);

		body->setLinearDamping(0.95f);
		body->setAngularDamping(0.8f);
		body->clearAccumulators();
		body->setAcceleration(0, -10.0f, 0);

		body->setCanSleep(false);
		body->setAwake();

		body->calculateDerivedData();

		auto colider =  std::make_unique<IKIGAI::PHYSICS::CollisionBox>();
		colider->halfSize = IKIGAI::MATHGL::Vector3{10.5f, 10.5f, 10.5f} *0.5f;
		//colider->transform = obj->transform->getLocalMatrix();
		colider->body = body.get();
		colider->calculateInternals();
		pc->body = body;
		pc->collider = std::move(colider);
		pc->boundingSphere = bs;
		pc->collisionType = IKIGAI::ECS::CollisionType::OBB;

		//app.getCore().physicsManger->bodies.push_back(body);
		//app.getCore().physicsManger->bodiesCollide.push_back(colider);
		app.getCore()->physicsManger->objects.Insert(obj, pc->boundingSphere);

		auto inp = obj->addComponent<IKIGAI::ECS::InputComponent>([body, obj](std::chrono::duration<double> dt) {
			//if (pc->body)
			obj->getTransform()->setLocalPosition(IKIGAI::MATHGL::Vector3(body->getPosition().x, body->getPosition().y, body->getPosition().z));
			//obj->transform->setLocalRotation(IKIGAI::MATHGL::Quaternion(
			//	IKIGAI::MATHGL::Vector3(TO_DEGREES(body->getRotation().x), TO_DEGREES(body->getRotation().y), TO_DEGREES(body->getRotation().z))));
			obj->getTransform()->setLocalRotation(IKIGAI::MATHGL::Quaternion(
				body->getOrientation().x, body->getOrientation().y, body->getOrientation().z, body->getOrientation().w));


			//pc->collider->transform = pc->obj.transform->getLocalMatrix();
			//pc->boundingSphere.position = pc->obj.transform->getLocalPosition();
			//if (pc->obj.transform->getLocalPosition().y < -500.0f) {
			//	pc->obj.transform->setLocalPosition(IKIGAI::MATHGL::Vector3(obj->transform->getLocalPosition().x, 500.0f, obj->transform->getLocalPosition().z));
			//}
			liveTime -= dt.count();
			if (liveTime < 0) {
				liveTime = 20.0f;
				body->setPosition(0.0f, 55.0f, 0.0f);
				body->setOrientation(1, 0, 0, 0);
				body->setVelocity(0, 0, 0);
				body->setRotation(IKIGAI::MATHGL::Vector3(0, 0, 0));
			}
		});
	}


	{
		auto obj = scene.createObject("BoxN");

		auto model = obj->addComponent<IKIGAI::ECS::ModelRenderer>();
		IKIGAI::RESOURCES::ModelParserFlags flags = IKIGAI::RESOURCES::ModelParserFlags::TRIANGULATE;
		//flags |= IKIGAI::RESOURCES::ModelParserFlags::GEN_SMOOTH_NORMALS;
		flags |= IKIGAI::RESOURCES::ModelParserFlags::FLIP_UVS;
		flags |= IKIGAI::RESOURCES::ModelParserFlags::GEN_UV_COORDS;
		flags |= IKIGAI::RESOURCES::ModelParserFlags::CALC_TANGENT_SPACE;

		//auto m = IKIGAI::RESOURCES::ModelLoader::Create("C:\\Projects\\SimpleEngine\\CGCourse\\CGCourse\\Assets\\User\\cottage_fbx.fbx", flags);
		auto m = IKIGAI::RESOURCES::ModelLoader::Create("C:\\Projects\\SimpleEngine\\CGCourse\\CGCourse\\Assets\\Engine\\Models\\Cube.fbx", flags);
		model->setModel(m);
		model->setFrustumBehaviour(IKIGAI::ECS::ModelRenderer::EFrustumBehaviour::CULL_MODEL);
		auto bs = IKIGAI::RENDER::BoundingSphere();
		bs.position = {0.0f, 0.0f, 0.0f};
		bs.radius = 10.50;
		model->setCustomBoundingSphere(bs);
		obj->getTransform()->setLocalPosition({7.0f, 15.0f, 0.0f});
		obj->getTransform()->setLocalRotation({0.0f, 0.0f, 0.0f, 1.0f});
		//obj.getTransform()->setLocalScale({100.01f, 100.01f, 100.01f});
		obj->getTransform()->setLocalScale({10.5f, 10.5f, 10.5f});

		auto s = IKIGAI::RESOURCES::ShaderLoader::CreateFromFile("Shaders\\Standard.glsl");

		auto mat = obj->addComponent<IKIGAI::ECS::MaterialRenderer>();
		auto _m = IKIGAI::RESOURCES::MaterialLoader::Create("");

		_m->setShader(s);
		_m->setBlendable(false);
		_m->setBackfaceCulling(true);
		_m->setFrontfaceCulling(false);
		_m->setDepthTest(true);
		_m->setDepthWriting(true);
		_m->setColorWriting(true);
		_m->setGPUInstances(1);
		auto& data = _m->getUniformsData();
		data["u_Diffuse"] = IKIGAI::MATHGL::Vector4{1.0f, 1.0f, 1.0f, 1.0f};

		auto tex1 = IKIGAI::RESOURCES::TextureLoader().CreateFromFile("textures\\brick_albedo.jpg");
		data["u_DiffuseMap"] = tex1;

		data["u_EnableNormalMapping"] = true;
		data["u_HeightScale"] = 0.0f;

		auto tex2 = IKIGAI::RESOURCES::TextureLoader().CreateFromFile("textures\\brick_normal.jpg");
		data["u_NormalMap"] = tex2;
		auto tex3 = IKIGAI::RESOURCES::TextureLoader().CreateFromFile("textures\\noiseTexture.png");
		data["u_Noise"] = tex3;
		data["fogScaleBias"] = IKIGAI::MATHGL::Vector4(0, -0.06f, 0, 0.0008f);
		data["u_Shininess"] = 100;
		data["u_Specular"] = IKIGAI::MATHGL::Vector3{1.0f, 1.0f, 1.0f};
		data["u_TextureOffset"] = IKIGAI::MATHGL::Vector2f{0.0f, 0.0f};
		data["u_TextureTiling"] = IKIGAI::MATHGL::Vector2f{1.0f, 1.0f};
		data["u_UseBone"] = false;
		mat->fillWithMaterial(_m);

		auto& pc = obj->addComponent<IKIGAI::ECS::PhysicsComponent>();
		auto body = std::make_shared<IKIGAI::PHYSICS::RigidBody>();
		body->setPosition(obj->getTransform()->getLocalPosition().x, obj->getTransform()->getLocalPosition().y, obj->getTransform()->getLocalPosition().z);
		body->setOrientation(1, 0, 0, 0);
		body->setVelocity(0, 0, 0);
		body->setRotation(0, 0, 0);
		//halfSize = cyclone::Vector3(1, 1, 1);
		float mass = 5555.0f;
		body->setMass(mass);

		IKIGAI::MATHGL::Matrix3 tensor = IKIGAI::MATHGL::Matrix3(0.0f);
		tensor.setBlockInertiaTensor(IKIGAI::MATHGL::Vector3{10.5f, 10.5f, 10.5f} *0.5f, mass);
		body->setInertiaTensor(tensor);

		body->setLinearDamping(0.95f);
		body->setAngularDamping(0.8f);
		body->clearAccumulators();
		body->setAcceleration(0, -10.0f, 0);

		body->setCanSleep(false);
		body->setAwake();

		body->calculateDerivedData();

		auto colider = std::make_unique<IKIGAI::PHYSICS::CollisionBox>();
		colider->halfSize = IKIGAI::MATHGL::Vector3{10.5f, 10.5f, 10.5f} *0.5f;
		//colider->transform = obj->transform->getLocalMatrix();
		colider->body = body.get();

		colider->calculateInternals();

		pc->body = body;
		pc->collider = std::move(colider);
		pc->boundingSphere = bs;
		pc->collisionType = IKIGAI::ECS::CollisionType::OBB;


		//app.getCore().physicsManger->bodies.push_back(body);
		//app.getCore().physicsManger->bodiesCollide.push_back(colider);
		app.getCore()->physicsManger->objects.Insert(obj, pc->boundingSphere);
		auto inp = obj->addComponent<IKIGAI::ECS::InputComponent>([body, obj](std::chrono::duration<double> dt) {
			//if (pc->body)
			obj->getTransform()->setLocalPosition(IKIGAI::MATHGL::Vector3(body->getPosition().x, body->getPosition().y, body->getPosition().z));
			//obj->transform->setLocalRotation(IKIGAI::MATHGL::Quaternion(
			//	IKIGAI::MATHGL::Vector3(TO_DEGREES(body->getRotation().x), TO_DEGREES(body->getRotation().y), TO_DEGREES(body->getRotation().z))));
			obj->getTransform()->setLocalRotation(IKIGAI::MATHGL::Quaternion(
				body->getOrientation().x, body->getOrientation().y, body->getOrientation().z, body->getOrientation().w));

			//pc->collider->transform = pc->obj.transform->getLocalMatrix();
			//pc->boundingSphere.position = pc->obj.transform->getLocalPosition();
			//if (pc->obj.transform->getLocalPosition().y < -500.0f) {
			//	pc->obj.transform->setLocalPosition(IKIGAI::MATHGL::Vector3(obj->transform->getLocalPosition().x, 500.0f, obj->transform->getLocalPosition().z));
			//}
			liveTime2 -= dt.count();
			if (liveTime2 < 0) {
				liveTime2 = 20.0f;
				body->setPosition(7.0f, 155.0f, 0.0f);
				body->setOrientation(1, 0, 0, 0);
				body->setVelocity(0, 0, 0);
				body->setRotation(IKIGAI::MATHGL::Vector3(0, 0, 0));
			}
		});
	}

	{
		auto obj = scene.createObject("Box2");

		auto model = obj->addComponent<IKIGAI::ECS::ModelRenderer>();
		IKIGAI::RESOURCES::ModelParserFlags flags = IKIGAI::RESOURCES::ModelParserFlags::TRIANGULATE;
		flags |= IKIGAI::RESOURCES::ModelParserFlags::GEN_SMOOTH_NORMALS;
		flags |= IKIGAI::RESOURCES::ModelParserFlags::FLIP_UVS;
		flags |= IKIGAI::RESOURCES::ModelParserFlags::GEN_UV_COORDS;
		flags |= IKIGAI::RESOURCES::ModelParserFlags::CALC_TANGENT_SPACE;

		//auto m = IKIGAI::RESOURCES::ModelLoader::Create("C:\\Projects\\SimpleEngine\\CGCourse\\CGCourse\\Assets\\User\\cottage_fbx.fbx", flags);
		auto m = IKIGAI::RESOURCES::ModelLoader::Create("C:\\Projects\\SimpleEngine\\CGCourse\\CGCourse\\Assets\\Engine\\Models\\Cube.fbx", flags);
		model->setModel(m);
		model->setFrustumBehaviour(IKIGAI::ECS::ModelRenderer::EFrustumBehaviour::CULL_MODEL);
		auto bs = IKIGAI::RENDER::BoundingSphere();
		bs.position = {0.0f, 0.0f, 0.0f};
		bs.radius = 1.0f;
		model->setCustomBoundingSphere(bs);
		obj->getTransform()->setLocalPosition({20.0f, 0.0f, 10.0f});
		obj->getTransform()->setLocalRotation({0.0f, 0.0f, 0.0f, 1.0f});
		//obj.getTransform()->setLocalScale({100.01f, 100.01f, 100.01f});
		obj->getTransform()->setLocalScale({10.5f, 10.5f, 10.5f});

		auto s = IKIGAI::RESOURCES::ShaderLoader::CreateFromFile("Shaders\\Standard.glsl");

		auto mat = obj->addComponent<IKIGAI::ECS::MaterialRenderer>();
		auto _m = IKIGAI::RESOURCES::MaterialLoader::Create("");

		_m->setShader(s);
		_m->setBlendable(false);
		_m->setBackfaceCulling(true);
		_m->setFrontfaceCulling(false);
		_m->setDepthTest(true);
		_m->setDepthWriting(true);
		_m->setColorWriting(true);
		_m->setGPUInstances(1);
		auto& data = _m->getUniformsData();
		data["u_Diffuse"] = IKIGAI::MATHGL::Vector4{1.0f, 1.0f, 1.0f, 1.0f};

		auto tex1 = IKIGAI::RESOURCES::TextureLoader().CreateFromFile("textures\\brick_albedo.jpg");
		auto tex2 = IKIGAI::RESOURCES::TextureLoader().CreateFromFile("textures\\brick_normal.jpg");

		data["u_DiffuseMap"] = tex1;

		data["u_EnableNormalMapping"] = true;
		data["u_HeightScale"] = 0.0f;

		data["u_NormalMap"] = tex2;
		data["u_Shininess"] = 100;
		data["u_Specular"] = IKIGAI::MATHGL::Vector3{1.0f, 1.0f, 1.0f};
		data["u_TextureOffset"] = IKIGAI::MATHGL::Vector2f{0.0f, 0.0f};
		data["u_TextureTiling"] = IKIGAI::MATHGL::Vector2f{1.0f, 1.0f};
		data["u_UseBone"] = false;
		mat->fillWithMaterial(_m);
	}
	{
		auto obj = scene.createObject("Box3");

		auto& model = obj->addComponent<IKIGAI::ECS::ModelRenderer>();
		IKIGAI::RESOURCES::ModelParserFlags flags = IKIGAI::RESOURCES::ModelParserFlags::TRIANGULATE;
		flags |= IKIGAI::RESOURCES::ModelParserFlags::GEN_SMOOTH_NORMALS;
		flags |= IKIGAI::RESOURCES::ModelParserFlags::FLIP_UVS;
		flags |= IKIGAI::RESOURCES::ModelParserFlags::GEN_UV_COORDS;
		flags |= IKIGAI::RESOURCES::ModelParserFlags::CALC_TANGENT_SPACE;

		//auto m = IKIGAI::RESOURCES::ModelLoader::Create("C:\\Projects\\SimpleEngine\\CGCourse\\CGCourse\\Assets\\User\\cottage_fbx.fbx", flags);
		auto m = IKIGAI::RESOURCES::ModelLoader::Create("C:\\Projects\\SimpleEngine\\CGCourse\\CGCourse\\Assets\\Engine\\Models\\Cube.fbx", flags);
		model->setModel(m);
		model->setFrustumBehaviour(IKIGAI::ECS::ModelRenderer::EFrustumBehaviour::CULL_MODEL);
		auto bs = IKIGAI::RENDER::BoundingSphere();
		bs.position = {0.0f, 0.0f, 0.0f};
		bs.radius = 1.0f;
		model->setCustomBoundingSphere(bs);
		obj->getTransform()->setLocalPosition({-10.0f, 0.0f, 20.0f});
		obj->getTransform()->setLocalRotation({0.0f, 0.0f, 0.0f, 1.0f});
		obj->getTransform()->setLocalScale({5.25f, 15.25f, 5.25f});

		auto s = IKIGAI::RESOURCES::ShaderLoader::CreateFromFile("Shaders\\StandardPBR.glsl");

		auto mat = obj->addComponent<IKIGAI::ECS::MaterialRenderer>();
		auto _m = IKIGAI::RESOURCES::MaterialLoader::Create("");
		_m->isDeferred = true;
		//_m->setShader(s);
		_m->setBlendable(false);
		_m->setBackfaceCulling(true);
		_m->setFrontfaceCulling(false);
		_m->setDepthTest(true);
		_m->setDepthWriting(true);
		_m->setColorWriting(true);
		_m->setGPUInstances(1);
		auto& data = _m->getUniformsData();
		data["u_Albedo"] = IKIGAI::MATHGL::Vector4{1.0f, 1.0f, 1.0f, 1.0f};

		auto tex1 = IKIGAI::RESOURCES::TextureLoader().CreateFromFile("textures\\brick_albedo.jpg");
		data["u_AlbedoMap"] = tex1;

		data["u_EnableNormalMapping"] = true;
		data["u_HeightScale"] = 0.0f;

		auto tex2 = IKIGAI::RESOURCES::TextureLoader().CreateFromFile("textures\\brick_normal.jpg");
		data["u_NormalMap"] = tex2;
		auto tex3 = IKIGAI::RESOURCES::TextureLoader().CreateFromFile("textures\\noiseTexture.png");
		data["u_Noise"] = tex3;
		data["fogScaleBias"] = IKIGAI::MATHGL::Vector4(0, -0.06f, 0, 0.0008f);
		data["u_Shininess"] = 100;
		data["u_Specular"] = IKIGAI::MATHGL::Vector3{1.0f, 1.0f, 1.0f};
		data["u_TextureOffset"] = IKIGAI::MATHGL::Vector2f{0.0f, 0.0f};
		data["u_TextureTiling"] = IKIGAI::MATHGL::Vector2f{1.0f, 1.0f};
		data["u_UseBone"] = false;
		mat->fillWithMaterial(_m);
	}

	{
		auto& obj = scene.createObject("Box3");

		auto model = obj->addComponent<IKIGAI::ECS::ModelRenderer>();
		IKIGAI::RESOURCES::ModelParserFlags flags = IKIGAI::RESOURCES::ModelParserFlags::TRIANGULATE;
		flags |= IKIGAI::RESOURCES::ModelParserFlags::GEN_SMOOTH_NORMALS;
		flags |= IKIGAI::RESOURCES::ModelParserFlags::FLIP_UVS;
		flags |= IKIGAI::RESOURCES::ModelParserFlags::GEN_UV_COORDS;
		flags |= IKIGAI::RESOURCES::ModelParserFlags::CALC_TANGENT_SPACE;

		//auto m = IKIGAI::RESOURCES::ModelLoader::Create("C:\\Projects\\SimpleEngine\\CGCourse\\CGCourse\\Assets\\User\\cottage_fbx.fbx", flags);
		auto m = IKIGAI::RESOURCES::ModelLoader::Create("C:\\Projects\\SimpleEngine\\CGCourse\\CGCourse\\Assets\\Engine\\Models\\Cube.fbx", flags);
		model->setModel(m);
		model->setFrustumBehaviour(IKIGAI::ECS::ModelRenderer::EFrustumBehaviour::CULL_MODEL);
		auto bs = IKIGAI::RENDER::BoundingSphere();
		bs.position = { 0.0f, 0.0f, 0.0f };
		bs.radius = 1.0f;
		model->setCustomBoundingSphere(bs);
		obj->getTransform()->setLocalPosition({ 40.0f, 0.0f, 30.0f });
		obj->getTransform()->setLocalRotation({ 0.0f, 0.0f, 0.0f, 1.0f });
		//obj.getTransform()->setLocalScale({100.01f, 100.01f, 100.01f});
		obj->getTransform()->setLocalScale({ 5.25f, 15.25f, 5.25f });

		auto s = IKIGAI::RESOURCES::ShaderLoader::CreateFromFile("Shaders\\StandardPBR.glsl");

		auto mat = obj->addComponent<IKIGAI::ECS::MaterialRenderer>();
		auto _m = IKIGAI::RESOURCES::MaterialLoader::Create("");
		_m->isDeferred = true;
		//_m->setShader(s);
		_m->setBlendable(false);
		_m->setBackfaceCulling(true);
		_m->setFrontfaceCulling(false);
		_m->setDepthTest(true);
		_m->setDepthWriting(true);
		_m->setColorWriting(true);
		_m->setGPUInstances(1);
		auto& data = _m->getUniformsData();
		data["u_Albedo"] = IKIGAI::MATHGL::Vector4{ 1.0f, 1.0f, 1.0f, 1.0f };

		auto tex1 = IKIGAI::RESOURCES::TextureLoader().CreateFromFile("textures\\brick_albedo.jpg");
		data["u_AlbedoMap"] = tex1;

		data["u_EnableNormalMapping"] = true;
		data["u_HeightScale"] = 0.0f;

		auto tex2 = IKIGAI::RESOURCES::TextureLoader().CreateFromFile("textures\\brick_normal.jpg");
		data["u_NormalMap"] = tex2;
		auto tex3 = IKIGAI::RESOURCES::TextureLoader().CreateFromFile("textures\\noiseTexture.png");
		data["u_Noise"] = tex3;
		data["fogScaleBias"] = IKIGAI::MATHGL::Vector4(0, -0.06f, 0, 0.0008f);
		data["u_Shininess"] = 100;
		data["u_Specular"] = IKIGAI::MATHGL::Vector3{ 1.0f, 1.0f, 1.0f };
		data["u_TextureOffset"] = IKIGAI::MATHGL::Vector2f{ 0.0f, 0.0f };
		data["u_TextureTiling"] = IKIGAI::MATHGL::Vector2f{ 1.0f, 1.0f };
		data["u_UseBone"] = false;
		mat->fillWithMaterial(_m);
	}


	{
		auto obj = scene.createObject("BoxDirLight");

		auto model = obj->addComponent<IKIGAI::ECS::ModelRenderer>();
		IKIGAI::RESOURCES::ModelParserFlags flags = IKIGAI::RESOURCES::ModelParserFlags::TRIANGULATE;
		flags |= IKIGAI::RESOURCES::ModelParserFlags::GEN_SMOOTH_NORMALS;
		flags |= IKIGAI::RESOURCES::ModelParserFlags::FLIP_UVS;
		flags |= IKIGAI::RESOURCES::ModelParserFlags::GEN_UV_COORDS;
		flags |= IKIGAI::RESOURCES::ModelParserFlags::CALC_TANGENT_SPACE;

		//auto m = IKIGAI::RESOURCES::ModelLoader::Create("C:\\Projects\\SimpleEngine\\CGCourse\\CGCourse\\Assets\\User\\cottage_fbx.fbx", flags);
		auto m = IKIGAI::RESOURCES::ModelLoader::Create("C:\\Projects\\SimpleEngine\\CGCourse\\CGCourse\\Assets\\Engine\\Models\\Sphere.fbx", flags);
		model->setModel(m);
		model->setFrustumBehaviour(IKIGAI::ECS::ModelRenderer::EFrustumBehaviour::CULL_MODEL);
		auto bs = IKIGAI::RENDER::BoundingSphere();
		bs.position = {0.0f, 0.0f, 0.0f};
		bs.radius = 1.0f;
		model->setCustomBoundingSphere(bs);
		obj->getTransform()->setLocalPosition({-20.0f, 35.0f, 10.0f});
		obj->getTransform()->setLocalRotation({0.0f, 0.0f, 0.0f, 1.0f});
		//obj.transform->setLocalScale({100.01f, 100.01f, 100.01f});
		obj->getTransform()->setLocalScale({1.25f, 1.25f, 1.25f});

		auto s = IKIGAI::RESOURCES::ShaderLoader::CreateFromFile("Shaders\\Standard.glsl");

		auto mat = obj->addComponent<IKIGAI::ECS::MaterialRenderer>();
		auto _m = IKIGAI::RESOURCES::MaterialLoader::Create("");

		_m->setShader(s);
		_m->setBlendable(false);
		_m->setBackfaceCulling(true);
		_m->setFrontfaceCulling(false);
		_m->setDepthTest(true);
		_m->setDepthWriting(true);
		_m->setColorWriting(true);
		_m->setGPUInstances(1);
		auto& data = _m->getUniformsData();
		data["u_Diffuse"] = IKIGAI::MATHGL::Vector4{1.0f, 1.0f, 1.0f, 1.0f};

		auto tex1 = IKIGAI::RESOURCES::TextureLoader().CreateFromFile("textures\\brick_metalic.jpg");
		data["u_DiffuseMap"] = tex1;

		data["u_EnableNormalMapping"] = true;
		data["u_HeightScale"] = 0.0f;

		auto tex2 = IKIGAI::RESOURCES::TextureLoader().CreateFromFile("textures\\brick_metalic.jpg");
		data["u_NormalMap"] = tex2;
		data["u_Shininess"] = 100;
		data["u_Specular"] = IKIGAI::MATHGL::Vector3{1.0f, 1.0f, 1.0f};
		data["u_TextureOffset"] = IKIGAI::MATHGL::Vector2f{0.0f, 0.0f};
		data["u_TextureTiling"] = IKIGAI::MATHGL::Vector2f{1.0f, 1.0f};
		data["u_UseBone"] = false;
		mat->fillWithMaterial(_m);



	}

	{
		auto obj = scene.createObject("Center");

		auto model = obj->addComponent<IKIGAI::ECS::ModelRenderer>();
		IKIGAI::RESOURCES::ModelParserFlags flags = IKIGAI::RESOURCES::ModelParserFlags::TRIANGULATE;
		flags |= IKIGAI::RESOURCES::ModelParserFlags::GEN_SMOOTH_NORMALS;
		flags |= IKIGAI::RESOURCES::ModelParserFlags::FLIP_UVS;
		flags |= IKIGAI::RESOURCES::ModelParserFlags::GEN_UV_COORDS;
		flags |= IKIGAI::RESOURCES::ModelParserFlags::CALC_TANGENT_SPACE;

		//auto m = IKIGAI::RESOURCES::ModelLoader::Create("C:\\Projects\\SimpleEngine\\CGCourse\\CGCourse\\Assets\\User\\cottage_fbx.fbx", flags);
		auto m = IKIGAI::RESOURCES::ModelLoader::Create("C:\\Projects\\SimpleEngine\\CGCourse\\CGCourse\\Assets\\Engine\\Models\\Cube.fbx", flags);
		model->setModel(m);
		model->setFrustumBehaviour(IKIGAI::ECS::ModelRenderer::EFrustumBehaviour::CULL_MODEL);
		auto bs = IKIGAI::RENDER::BoundingSphere();
		bs.position = {0.0f, 0.0f, 0.0f};
		bs.radius = 1.0f;
		model->setCustomBoundingSphere(bs);
		obj->getTransform()->setLocalPosition({0.0f, 0.0f, 0.0f});
		obj->getTransform()->setLocalRotation({0.0f, 0.0f, 0.0f, 1.0f});
		//obj.transform->setLocalScale({100.01f, 100.01f, 100.01f});
		obj->getTransform()->setLocalScale({1.25f, 1.25f, 1.25f});

		auto s = IKIGAI::RESOURCES::ShaderLoader::CreateFromFile("Shaders\\Standard.glsl");

		auto mat = obj->addComponent<IKIGAI::ECS::MaterialRenderer>();
		auto _m = IKIGAI::RESOURCES::MaterialLoader::Create("");

		_m->setShader(s);
		_m->setBlendable(false);
		_m->setBackfaceCulling(true);
		_m->setFrontfaceCulling(false);
		_m->setDepthTest(true);
		_m->setDepthWriting(true);
		_m->setColorWriting(true);
		_m->setGPUInstances(1);
		auto& data = _m->getUniformsData();
		data["u_Diffuse"] = IKIGAI::MATHGL::Vector4{1.0f, 1.0f, 1.0f, 1.0f};

		auto tex1 = IKIGAI::RESOURCES::TextureLoader().CreateFromFile("textures\\brick_albedo.jpg");
		data["u_DiffuseMap"] = tex1;

		data["u_EnableNormalMapping"] = 1;
		data["u_HeightScale"] = 0;

		auto tex2 = IKIGAI::RESOURCES::TextureLoader().CreateFromFile("textures\\brick_normal.jpg");
		data["u_NormalMap"] = tex2;
		auto tex3 = IKIGAI::RESOURCES::TextureLoader().CreateFromFile("textures\\noiseTexture.png");
		data["u_Noise"] = tex3;
		data["fogScaleBias"] = IKIGAI::MATHGL::Vector4(0, -0.06f, 0, 0.0008f);
		data["u_Shininess"] = 100;
		data["u_Specular"] = IKIGAI::MATHGL::Vector3{1.0f, 1.0f, 1.0f};
		data["u_TextureOffset"] = IKIGAI::MATHGL::Vector2f{0.0f, 0.0f};
		data["u_TextureTiling"] = IKIGAI::MATHGL::Vector2f{1.0f, 1.0f};
		data["u_UseBone"] = false;
		mat->fillWithMaterial(_m);
	}
	//{
	//	auto& obj = scene.createObject("Start");
	//
	//	auto model = obj.addComponent<IKIGAI::ECS::ModelRenderer>();
	//	IKIGAI::RESOURCES::ModelParserFlags flags = IKIGAI::RESOURCES::ModelParserFlags::TRIANGULATE;
	//	flags |= IKIGAI::RESOURCES::ModelParserFlags::GEN_SMOOTH_NORMALS;
	//	flags |= IKIGAI::RESOURCES::ModelParserFlags::FLIP_UVS;
	//	flags |= IKIGAI::RESOURCES::ModelParserFlags::GEN_UV_COORDS;
	//
	//	//auto m = IKIGAI::RESOURCES::ModelLoader::Create("C:\\Projects\\SimpleEngine\\CGCourse\\CGCourse\\Assets\\User\\cottage_fbx.fbx", flags);
	//	auto m = IKIGAI::RESOURCES::ModelLoader::Create("C:\\Projects\\SimpleEngine\\CGCourse\\CGCourse\\Assets\\Engine\\Models\\Cube.fbx", flags);
	//	model->setModel(m);
	//	model->setFrustumBehaviour(IKIGAI::ECS::ModelRenderer::EFrustumBehaviour::CULL_MODEL);
	//	auto bs = IKIGAI::RENDER::BoundingSphere();
	//	bs.position = {0.0f, 0.0f, 0.0f};
	//	bs.radius = 1.0f;
	//	model->setCustomBoundingSphere(bs);
	//	obj.transform->setLocalPosition({-20.0f, 40.0f, 10.0f});
	//	obj.transform->setLocalRotation({0.0f, 0.0f, 0.0f, 1.0f});
	//	//obj.transform->setLocalScale({100.01f, 100.01f, 100.01f});
	//	obj.transform->setLocalScale({1.25f, 1.25f, 1.25f});
	//
	//	auto s = IKIGAI::RESOURCES::ShaderLoader::Create("C:\\Projects\\SimpleEngine\\CGCourse\\CGCourse\\Assets\\Engine\\Shaders\\Standard.glsl");
	//
	//	auto mat = obj.addComponent<IKIGAI::ECS::MaterialRenderer>();
	//	auto _m = IKIGAI::RESOURCES::MaterialLoader::Create("");
	//
	//	_m->setShader(s);
	//	_m->setBlendable(false);
	//	_m->setBackfaceCulling(true);
	//	_m->setFrontfaceCulling(false);
	//	_m->setDepthTest(true);
	//	_m->setDepthWriting(true);
	//	_m->setColorWriting(true);
	//	_m->setGPUInstances(1);
	//	auto& data = _m->getdata["sData();
	//	data["u_Diffuse"] = IKIGAI::MATHGL::Vector4{1.0f, 1.0f, 1.0f, 1.0f};
	//
	//	auto tex1 = IKIGAI::RESOURCES::TextureLoader().createResource("textures\\brick_albedo.jpg");
	//	data["u_DiffuseMap"] = tex1;
	//
	//	data["u_EnableNormalMapping"] = 1;
	//	data["u_HeightScale"] = 0;
	//
	//	auto tex2 = IKIGAI::RESOURCES::TextureLoader().createResource("textures\\brick_normal.jpg");
	//	data["u_NormalMap"] = tex2;
	//	data["u_Shininess"] = 100;
	//	data["u_Specular"] = IKIGAI::MATHGL::Vector3{1.0f, 1.0f, 1.0f};
	//	data["u_TextureOffset"] = IKIGAI::MATHGL::Vector2f{0.0f, 0.0f};
	//	data["u_TextureTiling"] = IKIGAI::MATHGL::Vector2f{1.0f, 1.0f};
	//	data["u_UseBone"] = false;
	//	mat->fillWithMaterial(_m);
	//}

	{
		auto obj = scene.createObject("Smoke");

		auto model = obj->addComponent<IKIGAI::ECS::ModelRenderer>();
		IKIGAI::RESOURCES::ModelParserFlags flags = IKIGAI::RESOURCES::ModelParserFlags::TRIANGULATE;
		flags |= IKIGAI::RESOURCES::ModelParserFlags::GEN_SMOOTH_NORMALS;
		//flags |= IKIGAI::RESOURCES::ModelParserFlags::OPTIMIZE_MESHES;
		//flags |= IKIGAI::RESOURCES::ModelParserFlags::OPTIMIZE_GRAPH;
		//flags |= IKIGAI::RESOURCES::ModelParserFlags::FIND_INSTANCES;
		//flags |= IKIGAI::RESOURCES::ModelParserFlags::CALC_TANGENT_SPACE;
		flags |= IKIGAI::RESOURCES::ModelParserFlags::FLIP_UVS;
		flags |= IKIGAI::RESOURCES::ModelParserFlags::LIMIT_BONE_WEIGHTS;
		//flags |= IKIGAI::RESOURCES::ModelParserFlags::JOIN_IDENTICAL_VERTICES;
		//flags |= IKIGAI::RESOURCES::ModelParserFlags::DEBONE;
		//flags |= IKIGAI::RESOURCES::ModelParserFlags::FIND_INVALID_DATA;
		//flags |= IKIGAI::RESOURCES::ModelParserFlags::IMPROVE_CACHE_LOCALITY;
		flags |= IKIGAI::RESOURCES::ModelParserFlags::GEN_UV_COORDS;
		//flags |= IKIGAI::RESOURCES::ModelParserFlags::PRE_TRANSFORM_VERTICES;
		auto m = IKIGAI::RESOURCES::ModelLoader::Create("C:\\Projects\\SimpleEngine\\CGCourse\\CGCourse\\Assets\\Engine\\Models\\Sphere.fbx", flags);

		model->setModel(m);
		model->setFrustumBehaviour(IKIGAI::ECS::ModelRenderer::EFrustumBehaviour::CULL_MODEL);
		auto bs = IKIGAI::RENDER::BoundingSphere();
		bs.position = {0.0f, 0.0f, 0.0f};
		bs.radius = 1.0f;
		model->setCustomBoundingSphere(bs);
		obj->getTransform()->setLocalPosition({-128.0f, 30.0f, 0.0f});
		obj->getTransform()->setLocalRotation({0.0f, 0.0f, 0.0f, 1.0f});
		obj->getTransform()->setLocalScale({1.01f, 1.01f, 1.01f});

		auto s = IKIGAI::RESOURCES::ShaderLoader::CreateFromFile("Shaders\\Smoke.glsl");

		auto mat = obj->addComponent<IKIGAI::ECS::MaterialRenderer>();
		auto _m = IKIGAI::RESOURCES::MaterialLoader::Create("");

		_m->setShader(s);
		_m->setBlendable(true);
		_m->setBackfaceCulling(true);
		_m->setFrontfaceCulling(false);
		_m->setDepthTest(true);
		_m->setDepthWriting(false);
		_m->setColorWriting(true);
		_m->setGPUInstances(100);
		auto& data = _m->getUniformsData();
		data["u_Diffuse"] = IKIGAI::MATHGL::Vector4{0.7f, 0.7f, 0.7f, 0.5f};

		auto tex1 = IKIGAI::RESOURCES::TextureLoader().CreateFromFile("textures\\smoke-17.png");
		data["u_DiffuseMap"] = tex1;

		data["u_CosinAmplitude"] = IKIGAI::MATHGL::Vector3{0.5f, 1.0f, 0.9f};
		data["u_CosinFrequency"] = IKIGAI::MATHGL::Vector3{3.5f, 1.0f, 0.5f};

		data["u_MaxHeight"] = 10;
		data["u_Spacing"] = IKIGAI::MATHGL::Vector3{0.3f, 0.0f, 0.5f};
		data["u_TextureOffset"] = IKIGAI::MATHGL::Vector2f{1.0f, 0.0f};
		data["u_TextureTiling"] = IKIGAI::MATHGL::Vector2f{1.0f, 1.0f};
		data["u_Velocity"] = IKIGAI::MATHGL::Vector3{0.0f, 0.5f, 0.0f};
		data["u_UseBone"] = false;
		data["castShadow"] = false;


		mat->fillWithMaterial(_m);
	}

	//{
	//	auto& obj = scene.createObject("Water");
	//
	//	auto model = obj->addComponent<IKIGAI::ECS::ModelRenderer>();
	//	IKIGAI::RESOURCES::ModelParserFlags flags = IKIGAI::RESOURCES::ModelParserFlags::TRIANGULATE;
	//	flags |= IKIGAI::RESOURCES::ModelParserFlags::GEN_SMOOTH_NORMALS;
	//	flags |= IKIGAI::RESOURCES::ModelParserFlags::FLIP_UVS;
	//	flags |= IKIGAI::RESOURCES::ModelParserFlags::GEN_UV_COORDS;
	//	flags |= IKIGAI::RESOURCES::ModelParserFlags::CALC_TANGENT_SPACE;
	//	auto m = IKIGAI::RESOURCES::ModelLoader::Create("C:\\Projects\\SimpleEngine\\CGCourse\\CGCourse\\Assets\\Engine\\Models\\Sphere.fbx", flags);
	//
	//	model->setModel(m);
	//	model->setFrustumBehaviour(IKIGAI::ECS::ModelRenderer::EFrustumBehaviour::CULL_MODEL);
	//	auto bs = IKIGAI::RENDER::BoundingSphere();
	//	bs.position = {0.0f, 0.0f, 0.0f};
	//	bs.radius = 1.0f;
	//	model->setCustomBoundingSphere(bs);
	//	obj->transform->setLocalPosition({-100.0f, 1.0f, 10.0f});
	//	obj->transform->setLocalRotation({0.0f, 0.0f, 0.0f, 1.0f});
	//	obj->transform->setLocalScale({30.0f, 30.0f, 30.0f});
	//
	//	auto s = IKIGAI::RESOURCES::ShaderLoader::CreateFromFile("Shaders\\Fluid.glsl");
	//
	//	auto mat = obj->addComponent<IKIGAI::ECS::MaterialRenderer>();
	//	auto _m = IKIGAI::RESOURCES::MaterialLoader::Create("");
	//
	//	_m->setShader(s);
	//	_m->setBlendable(true);
	//	_m->setBackfaceCulling(true);
	//	_m->setFrontfaceCulling(false);
	//	_m->setDepthTest(true);
	//	_m->setDepthWriting(true);
	//	_m->setColorWriting(true);
	//	_m->setGPUInstances(1);
	//	auto& data = _m->getUniformsData();
	//	data["u_Amplitude"] = 0.1f;
	//	data["u_Diffuse"] = IKIGAI::MATHGL::Vector4{0.1f, 0.6f, 1.0f, 0.9f};
	//
	//	auto tex1 = IKIGAI::RESOURCES::TextureLoader().CreateFromFile("Water_001_COLOR.jpg");
	//	data["u_DiffuseMap"] = tex1;
	//	//auto tex3 = IKIGAI::RESOURCES::TextureLoader().createResource("Water_001_SPEC.jpg");
	//	//data["u_SpecularMap"] = tex3;
	//	//auto tex4 = IKIGAI::RESOURCES::TextureLoader().createResource("Water_001_DISP.png");
	//	//data["u_HeightMap"] = tex4;
	//	data["u_Disparity"] = 30;
	//	data["u_EnableNormalMapping"] = true;
	//	data["u_Frequency"] = 20.0f;
	//	data["u_HeightScale"] = 0.0f;
	//
	//	auto tex2 = IKIGAI::RESOURCES::TextureLoader().CreateFromFile("textures\\Water_001_NORM.jpg");
	//	data["u_NormalMap"] = tex2;
	//	data["u_Shininess"] = 100;
	//	data["u_Specular"] = IKIGAI::MATHGL::Vector3{1.0f, 1.0f, 1.0f};
	//	data["u_TextureTiling"] = IKIGAI::MATHGL::Vector2f{3.0f, 3.0f};
	//
	//	data["u_UseBone"] = false;
	//	data["castShadow"] = false;
	//
	//
	//	mat->fillWithMaterial(_m);
	//}

	{
		auto obj = scene.createObject("Grass");

		auto audio = obj->addComponent<IKIGAI::ECS::AudioComponent>(IKIGAI::AUDIO::Sound3d{
			"C:\\Projects\\SimpleEngine\\CGCourse\\CGCourse\\Assets\\Engine\\Audio\\test.mp3",
			IKIGAI::MATHGL::Vector3(-300.0f, 1.0f, 10.0f)});

		auto model = obj->addComponent<IKIGAI::ECS::ModelRenderer>();
		IKIGAI::RESOURCES::ModelParserFlags flags = IKIGAI::RESOURCES::ModelParserFlags::TRIANGULATE;
		flags |= IKIGAI::RESOURCES::ModelParserFlags::GEN_SMOOTH_NORMALS;
		flags |= IKIGAI::RESOURCES::ModelParserFlags::FLIP_UVS;
		flags |= IKIGAI::RESOURCES::ModelParserFlags::GEN_UV_COORDS;
		flags |= IKIGAI::RESOURCES::ModelParserFlags::CALC_TANGENT_SPACE;
		auto m = IKIGAI::RESOURCES::ModelLoader::Create("C:\\Projects\\SimpleEngine\\CGCourse\\CGCourse\\Assets\\Engine\\Models\\Cube.fbx", flags);

		model->setModel(m);
		model->setFrustumBehaviour(IKIGAI::ECS::ModelRenderer::EFrustumBehaviour::CULL_MODEL);
		auto bs = IKIGAI::RENDER::BoundingSphere();
		bs.position = {0.0f, 0.0f, 0.0f};
		bs.radius = 1.0f;
		model->setCustomBoundingSphere(bs);
		obj->getTransform()->setLocalPosition({-300.0f, 1.0f, 10.0f});
		obj->getTransform()->setLocalRotation({0.0f, 0.0f, 0.0f, 1.0f});
		obj->getTransform()->setLocalScale({30.0f, 30.0f, 30.0f});

		auto s = IKIGAI::RESOURCES::ShaderLoader::CreateFromFile("Shaders\\grass.glsl");

		auto mat = obj->addComponent<IKIGAI::ECS::MaterialRenderer>();
		auto _m = IKIGAI::RESOURCES::MaterialLoader::Create("");

		_m->setShader(s);
		_m->setBlendable(false);
		_m->setBackfaceCulling(false);
		_m->setFrontfaceCulling(false);
		_m->setDepthTest(true);
		_m->setDepthWriting(true);
		_m->setColorWriting(true);
		_m->setGPUInstances(1);
		auto& data = _m->getUniformsData();

		auto tex1 = IKIGAI::RESOURCES::TextureLoader().CreateFromFile("Textures\\grassPack.png");
		data["gSampler"] = tex1;
		data["fAlphaTest"] = 0.25f;
		data["fAlphaMultiplier"] = 1.5f;
		data["vColor"] = IKIGAI::MATHGL::Vector4(1.0f);



		mat->fillWithMaterial(_m);
	}

	//{
	//	auto& obj = scene.createObject("Terrain");
	//
	//	auto model = obj->addComponent<IKIGAI::ECS::ModelRenderer>();
	//
	//	auto m = IKIGAI::MeshGenerator::createTerrainFromHeightmap("C:\\Projects\\SimpleEngine\\CGCourse\\CGCourse\\Assets\\Engine\\Textures\\tut017.png");
	//	model->setModel(m);
	//	model->setFrustumBehaviour(IKIGAI::ECS::ModelRenderer::EFrustumBehaviour::CULL_MODEL);
	//	auto bs = IKIGAI::RENDER::BoundingSphere();
	//	bs.position = {0.0f, 0.0f, 0.0f};
	//	bs.radius = 1.0f;
	//	model->setCustomBoundingSphere(bs);
	//	obj->transform->setLocalPosition({-100.0f, 50.0f, 10.0f});
	//	obj->transform->setLocalRotation({0.0f, 0.0f, 0.0f, 1.0f});
	//	//obj.transform->setLocalScale({100.01f, 100.01f, 100.01f});
	//	obj->transform->setLocalScale({30.0f, 30.0f, 30.0f});
	//
	//	auto s = IKIGAI::RESOURCES::ShaderLoader::CreateFromFile("Shaders\\Terrain.glsl");
	//
	//	auto mat = obj->addComponent<IKIGAI::ECS::MaterialRenderer>();
	//	auto _m = IKIGAI::RESOURCES::MaterialLoader::Create("");
	//	_m->setShader(s);
	//	_m->setBlendable(false);
	//	_m->setBackfaceCulling(true);
	//	_m->setFrontfaceCulling(false);
	//	_m->setDepthTest(true);
	//	_m->setDepthWriting(true);
	//	_m->setColorWriting(true);
	//	_m->setGPUInstances(1);
	//	auto& data = _m->getUniformsData();
	//	data["u_Diffuse"] = IKIGAI::MATHGL::Vector4{1.0f, 1.0f, 1.0f, 1.0f};
	//
	//	auto tex1 = IKIGAI::RESOURCES::TextureLoader().CreateFromFile("textures\\terrain.png");
	//	data["u_DiffuseMap"] = tex1;
	//	data["u_EnableNormalMapping"] = false;
	//	data["u_HeightScale"] = 0.0f;
	//	data["fogScaleBias"] = IKIGAI::MATHGL::Vector4(0, -0.06f, 0, 0.0008f);
	//	data["u_Shininess"] = 100;
	//	data["u_Specular"] = IKIGAI::MATHGL::Vector3{1.0f, 1.0f, 1.0f};
	//	data["u_TextureOffset"] = IKIGAI::MATHGL::Vector2f{0.0f, 0.0f};
	//	data["u_TextureTiling"] = IKIGAI::MATHGL::Vector2f{1.0f, 1.0f};
	//
	//
	//	data["u_Levels"] = IKIGAI::MATHGL::Vector4(0.2f, 0.3f, 0.55, 0.7);
	//
	//	mat->fillWithMaterial(_m);
	//}

	{
		auto obj = scene.createObject("TerrainTes");

		auto model = obj->addComponent<IKIGAI::ECS::ModelRenderer>();

		auto m = IKIGAI::MeshGenerator::createSquare(20, 350, 350);
		model->setModel(m);
		model->setFrustumBehaviour(IKIGAI::ECS::ModelRenderer::EFrustumBehaviour::CULL_MODEL);
		auto bs = IKIGAI::RENDER::BoundingSphere();
		bs.position = { 0.0f, 0.0f, 0.0f };
		bs.radius = 1.0f;
		model->setCustomBoundingSphere(bs);
		obj->getTransform()->setLocalPosition({ -100.0f, 50.0f, 10.0f });
		obj->getTransform()->setLocalRotation({ 0.0f, 0.0f, 0.0f, 1.0f });
		obj->getTransform()->setLocalScale({ 0.10f, 0.10f, 0.10f });

		auto s = IKIGAI::RESOURCES::ShaderLoader::CreateFromFile("Shaders\\tess.glsl");

		auto mat = obj->addComponent<IKIGAI::ECS::MaterialRenderer>();
		auto _m = IKIGAI::RESOURCES::MaterialLoader::Create("");
		_m->setShader(s);
		_m->setBlendable(false);
		_m->setBackfaceCulling(false);
		_m->setFrontfaceCulling(false);
		_m->setDepthTest(true);
		_m->setDepthWriting(true);
		_m->setColorWriting(true);
		_m->setGPUInstances(1);
		_m->setPatchSize(4);
		auto& data = _m->getUniformsData();

		auto tex0 = IKIGAI::RESOURCES::TextureLoader().CreateFromFile("Textures\\tut017.png");
		data["u_HeightMap"] = tex0;
		data["u_Diffuse"] = IKIGAI::MATHGL::Vector4{ 1.0f, 1.0f, 1.0f, 1.0f };
		auto tex1 = IKIGAI::RESOURCES::TextureLoader().CreateFromFile("textures\\terrain.png");
		data["u_DiffuseMap"] = tex1;
		data["u_EnableNormalMapping"] = false;
		data["u_HeightScale"] = 0.0f;
		data["fogScaleBias"] = IKIGAI::MATHGL::Vector4(0, -0.06f, 0, 0.0008f);
		data["u_Shininess"] = 100;
		data["u_Specular"] = IKIGAI::MATHGL::Vector3{ 1.0f, 1.0f, 1.0f };
		data["u_TextureOffset"] = IKIGAI::MATHGL::Vector2f{ 0.0f, 0.0f };
		data["u_TextureTiling"] = IKIGAI::MATHGL::Vector2f{ 1.0f, 1.0f };
		data["u_Levels"] = IKIGAI::MATHGL::Vector4(0.2f, 0.3f, 0.55, 0.7);
		mat->fillWithMaterial(_m);
	}

	{
		auto obj = scene.createObject("Clouds");

		auto model = obj->addComponent<IKIGAI::ECS::ModelRenderer>();
		IKIGAI::RESOURCES::ModelParserFlags flags = IKIGAI::RESOURCES::ModelParserFlags::TRIANGULATE;
		flags |= IKIGAI::RESOURCES::ModelParserFlags::GEN_SMOOTH_NORMALS;
		flags |= IKIGAI::RESOURCES::ModelParserFlags::FLIP_UVS;
		flags |= IKIGAI::RESOURCES::ModelParserFlags::GEN_UV_COORDS;
		flags |= IKIGAI::RESOURCES::ModelParserFlags::CALC_TANGENT_SPACE;
		auto m = IKIGAI::RESOURCES::ModelLoader::Create("C:\\Projects\\SimpleEngine\\CGCourse\\CGCourse\\Assets\\Engine\\Models\\Cube.fbx", flags);

		model->setModel(m);
		model->setFrustumBehaviour(IKIGAI::ECS::ModelRenderer::EFrustumBehaviour::CULL_MODEL);
		auto bs = IKIGAI::RENDER::BoundingSphere();
		bs.position = {0.0f, 0.0f, 0.0f};
		bs.radius = 1.0f;
		model->setCustomBoundingSphere(bs);
		obj->getTransform()->setLocalPosition({-300.0f, 50.0f, -300.0f});
		obj->getTransform()->setLocalRotation({0.0f, 0.0f, 0.0f, 1.0f});
		//obj.transform->setLocalScale({100.01f, 100.01f, 100.01f});
		obj->getTransform()->setLocalScale({300.0f, 300.0f, 300.0f});

		auto s = IKIGAI::RESOURCES::ShaderLoader::CreateFromFile("Shaders\\Clouds.glsl");

		auto mat = obj->addComponent<IKIGAI::ECS::MaterialRenderer>();
		auto _m = IKIGAI::RESOURCES::MaterialLoader::Create("");
		_m->setShader(s);
		_m->setBlendable(false);
		_m->setBackfaceCulling(false);
		_m->setFrontfaceCulling(false);
		_m->setDepthTest(true);
		_m->setDepthWriting(true);
		_m->setColorWriting(true);
		_m->setGPUInstances(1);
		auto& data = _m->getUniformsData();


		auto inp = obj->addComponent<IKIGAI::ECS::InputComponent>([&app, &data, s](std::chrono::duration<double> dt) {
			//auto currentScene = app.getCore().sceneManager->getCurrentScene();
			//auto mainCameraComponent = currentScene->findMainCamera();
			//data["camUp"] = mainCameraComponent->obj.transform->getLocalRotation() * IKIGAI::MATHGL::Vector3::Up;
			//data["camRight"] = mainCameraComponent->obj.transform->getLocalRotation() * IKIGAI::MATHGL::Vector3::Right;
			//s->bind();
			//auto mpos = app.getCore().inputManager->getMousePosition();
			//s->setUniformVec2("iMouse", IKIGAI::MATHGL::Vector2f(mpos.x, mpos.y));
			//s->unbind();
		});

		//data["iResolution"] = IKIGAI::MATHGL::Vector2f(800, 600);


		// Textures //

		auto tex1 = IKIGAI::RESOURCES::TextureLoader().CreateFromFile("textures\\clouds\\0.png");
		auto tex2 = IKIGAI::RESOURCES::TextureLoader().CreateFromFile("textures\\clouds\\1.png");
		auto tex3 = IKIGAI::RESOURCES::TextureLoader().CreateFromFile("textures\\clouds\\2.png");
		//auto tex4 = IKIGAI::RESOURCES::TextureLoader().createResource("textures\\clouds\\HDR_L_0.png");

		//auto tex5 = IKIGAI::RESOURCES::TextureLoader().createResource("textures\\clouds\\HDR_L_0.png");
		//auto tex6 = IKIGAI::RESOURCES::TextureLoader().createResource("textures\\clouds\\HDR_L_0.png");
		//auto tex7 = IKIGAI::RESOURCES::TextureLoader().createResource("textures\\clouds\\HDR_L_0.png");

		data["iChannel0"] = tex1;
		data["iChannel1"] = tex2;
		data["iChannel2"] = tex3;
		//data["blueNoise"] = tex4;
		//data["groundTex"] = tex5;
		//data["celestialTex"] = tex6;
		//data["cameraDepthTexture"] = tex7;


		mat->fillWithMaterial(_m);
	}

	//{
	//	auto& obj = scene.createObject("Water");
	//
	//	auto model = obj->addComponent<IKIGAI::ECS::ModelRenderer>();
	//	IKIGAI::RESOURCES::ModelParserFlags flags = IKIGAI::RESOURCES::ModelParserFlags::TRIANGULATE;
	//	flags |= IKIGAI::RESOURCES::ModelParserFlags::GEN_SMOOTH_NORMALS;
	//	flags |= IKIGAI::RESOURCES::ModelParserFlags::FLIP_UVS;
	//	flags |= IKIGAI::RESOURCES::ModelParserFlags::GEN_UV_COORDS;
	//	flags |= IKIGAI::RESOURCES::ModelParserFlags::CALC_TANGENT_SPACE;
	//	auto m = IKIGAI::RESOURCES::ModelLoader::Create("C:\\Projects\\SimpleEngine\\CGCourse\\CGCourse\\Assets\\Engine\\Models\\Cube.fbx", flags);
	//
	//	model->setModel(m);
	//	model->setFrustumBehaviour(IKIGAI::ECS::ModelRenderer::EFrustumBehaviour::CULL_MODEL);
	//	auto bs = IKIGAI::RENDER::BoundingSphere();
	//	bs.position = {0.0f, 0.0f, 0.0f};
	//	bs.radius = 1.0f;
	//	model->setCustomBoundingSphere(bs);
	//	obj->transform->setLocalPosition({300.0f, 50.0f, -300.0f});
	//	obj->transform->setLocalRotation({2 * 3.1415f, 0.0, 0.0f, 1.0f});
	//	//obj.transform->setLocalScale({100.01f, 100.01f, 100.01f});
	//	obj->transform->setLocalScale({300.0f, 300.0f, 0.01f});
	//
	//	auto s = IKIGAI::RESOURCES::ShaderLoader::CreateFromFile("Shaders\\sea.glsl");
	//
	//	auto mat = obj->addComponent<IKIGAI::ECS::MaterialRenderer>();
	//	auto _m = IKIGAI::RESOURCES::MaterialLoader::Create("");
	//	_m->setShader(s);
	//	_m->setBlendable(false);
	//	_m->setBackfaceCulling(false);
	//	_m->setFrontfaceCulling(false);
	//	_m->setDepthTest(true);
	//	_m->setDepthWriting(true);
	//	_m->setColorWriting(true);
	//	_m->setGPUInstances(1);
	//	auto& data = _m->getUniformsData();
	//
	//
	//	auto inp = obj->addComponent<IKIGAI::ECS::InputComponent>([&app, &data, s](std::chrono::duration<double> dt) {
	//		s->bind();
	//		auto mpos = app.getCore().inputManager->getMousePosition();
	//		s->setUniformVec2("iMouse", IKIGAI::MATHGL::Vector2f(mpos.x, mpos.y));
	//		s->unbind();
	//	});
	//
	//	mat->fillWithMaterial(_m);
	//}

	//{
	//
	//	auto s = IKIGAI::RESOURCES::ShaderLoader::CreateFromFile("Shaders\\BW.glsl");
	//	auto _m = IKIGAI::RESOURCES::MaterialLoader::Create("");
	//	_m->setShader(s);
	//	_m->setBlendable(false);
	//	_m->setBackfaceCulling(true);
	//	_m->setFrontfaceCulling(false);
	//	_m->setDepthTest(false);
	//	_m->setDepthWriting(false);
	//	_m->setColorWriting(true);
	//	_m->setGPUInstances(1);
	//
	//	app.getCore().renderer->addCustomPostRocessing("BlackWhite", _m);
	//}


	auto s = IKIGAI::RESOURCES::ShaderLoader::CreateFromFile("Shaders\\gui\\sprite.glsl");
	auto _m = IKIGAI::RESOURCES::MaterialLoader::Create("");
	_m->setShader(s);
	//_m->setBlendable(false);
	//_m->setBackfaceCulling(true);
	//_m->setFrontfaceCulling(false);
	//_m->setDepthTest(true);
	_m->setDepthWriting(true);
	_m->setColorWriting(true);
	_m->setGPUInstances(1);
	auto& data = _m->getUniformsData();
	auto tex1 = IKIGAI::RESOURCES::TextureLoader().CreateFromFile("textures\\gui\\btn.png");
	data["image"] = tex1;
	data["spriteColor"] = IKIGAI::MATHGL::Vector4{1, 1, 1, 1};

	auto sprite = std::make_shared<IKIGAI::GUI::GuiImage>(_m);

	scene.guiObjs.push_back(sprite);

	{
		auto s = IKIGAI::RESOURCES::ShaderLoader::CreateFromFile("Shaders\\gui\\sprite.glsl");
		auto _m = IKIGAI::RESOURCES::MaterialLoader::Create("");
		_m->setShader(s);
		//_m->setBlendable(false);
		//_m->setBackfaceCulling(true);
		//_m->setFrontfaceCulling(false);
		//_m->setDepthTest(true);
		_m->setDepthWriting(true);
		_m->setColorWriting(true);
		_m->setGPUInstances(1);
		auto& data = _m->getUniformsData();
		auto tex1 = IKIGAI::RESOURCES::TextureLoader().CreateFromFile("textures\\gui\\btn.png");
		data["image"] = tex1;
		data["spriteColor"] = IKIGAI::MATHGL::Vector4{1, 1, 1, 1};

		auto sprite = std::make_shared<IKIGAI::GUI::GuiImage>(_m);

		scene.guiObjs.push_back(sprite);
	}
	{
		auto s = IKIGAI::RESOURCES::ShaderLoader::CreateFromFile("Shaders\\gui\\sprite.glsl");
		auto _m = IKIGAI::RESOURCES::MaterialLoader::Create("");
		_m->setShader(s);
		//_m->setBlendable(false);
		//_m->setBackfaceCulling(true);
		//_m->setFrontfaceCulling(false);
		//_m->setDepthTest(true);
		_m->setDepthWriting(true);
		_m->setColorWriting(true);
		_m->setGPUInstances(1);
		auto& data = _m->getUniformsData();
		auto tex1 = IKIGAI::RESOURCES::TextureLoader().CreateFromFile("textures\\gui\\btn.png");
		data["image"] = tex1;
		data["spriteColor"] = IKIGAI::MATHGL::Vector4{1, 1, 1, 1};

		auto sprite = std::make_shared<IKIGAI::GUI::GuiImage>(_m);

		scene.guiObjs[0]->addChild(sprite);
	}

	{
		auto s = IKIGAI::RESOURCES::ShaderLoader::CreateFromFile("Shaders\\gui\\text.glsl");
		auto _m = IKIGAI::RESOURCES::MaterialLoader::Create("");
		_m->setShader(s);
		//_m->setBlendable(false);
		//_m->setBackfaceCulling(true);
		//_m->setFrontfaceCulling(false);
		//_m->setDepthTest(true);
		_m->setDepthWriting(true);
		_m->setColorWriting(true);
		_m->setGPUInstances(1);
		auto& data = _m->getUniformsData();
		data["textColor"] = IKIGAI::MATHGL::Vector4{1, 1, 1, 1};

		auto f = std::make_shared<IKIGAI::GUI::Font>("C:/Projects/SimpleEngine/CGCourse/CGCourse/Assets/Engine/Fonts/a_AlternaSw.TTF", 42);

		auto text = std::make_shared<IKIGAI::GUI::GuiLabel>("HELLOW WORLD!", f, _m);

		scene.guiObjs[0]->addChild(text);
	}
	{
		auto btn = std::make_shared<IKIGAI::GUI::GuiButton>();
		scene.guiObjs.push_back(btn);
	}
	{
		auto l = std::make_shared<IKIGAI::GUI::GuiLayout>();
		for (int i = 0; i < 5; i++) {
			auto s = IKIGAI::RESOURCES::ShaderLoader::CreateFromFile("Shaders\\gui\\sprite.glsl");
			auto _m = IKIGAI::RESOURCES::MaterialLoader::Create("");
			_m->setShader(s);
			auto& data = _m->getUniformsData();
			auto tex1 = IKIGAI::RESOURCES::TextureLoader().CreateFromFile("textures\\gui\\btn.png");
			data["image"] = tex1;
			data["spriteColor"] = IKIGAI::MATHGL::Vector4{1, 1, 1, 1};
			auto sprite = std::make_shared<IKIGAI::GUI::GuiImage>(_m);
			l->addChild(sprite);
			//sprite->setProjection(320.0f, 70.0f);
		}
		//l->calculateTransform();

		auto scroll = std::make_shared<IKIGAI::GUI::GuiScroll>(500 * 1.5f, 100.0f);
		//scroll->scroll->isScrollVertical = false;
		scroll->addChild(l);
		auto clip = std::make_shared<IKIGAI::GUI::GuiClip>(320.0f, 70.0f);
		clip->addChild(scroll);
		scene.guiObjs.push_back(clip);

	}

	//{//hard add cloth
	//	int num_part = 15;
	//	float part_dist = 0.9f;
	//	float k = -3.0f;
	//	float d = 0.20f;
	//	ClothObj cloth;
	//	cloth.Initialize(num_part, part_dist, glm::vec3(0, 50, 0));
	//	cloth.SetStructuralSprings(k, d);
	//	cloth.SetBendSprings(k, d);
	//	cloth.SetShearSprings(k, d);
	//
	//	app.getCore()->physicsManger->cloths.push_back(cloth);
	//}

	app.run();
	return 0;
}
*/
#endif