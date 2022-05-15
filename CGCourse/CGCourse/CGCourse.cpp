
#include "CameraRT.h"
//#include "DiffuseLightRT.h"
#include "ObjectRT.h"
#include "RenderRT.h"
#include "engine/ecs/components/ambientSphereLight.h"
#include "engine/ecs/components/directionalLight.h"
#include "engine/ecs/components/directionalLight.h"
#include "engine/ecs/components/cameraComponent.h"
#include "engine/core/app.h"
#include "engine/ecs/components/inputComponent.h"
#include "engine/ecs/components/physicsComponent.h"
#include "engine/ecs/components/pointLight.h"
#include "engine/physics/body.h"
#include "engine/physics/narrowPhase.h"
#include "engine/physics/PhysicWorld.h"
#include "engine/resourceManager/resource/bone.h"
#include "engine/utils/meshGenerator.h"
#include "game/World.h"
using namespace std;
extern "C" {
	_declspec(dllexport) DWORD NvOptimusEnablement = 0x00000001;
}
Game::World* world = nullptr;

std::shared_ptr<KUMA::RESOURCES::Animation> danceAnimation;
std::shared_ptr<KUMA::RESOURCES::Animator> animator;

float liveTime = 20.0f;
float liveTime2 = 20.0f;

namespace Game {
	Game::ChunkDataTypePtr _GetChunkDataForMeshing(int cx, int cz) {
		if (world && world->ChunkExistsInMap(cx, cz)) {
			Game::Chunk* chunk = world->RetrieveChunkFromMap(cx, cz);
			return &chunk->p_ChunkContents;
		}

		return nullptr;
	}

	Game::ChunkLightDataTypePtr _GetChunkLightDataForMeshing(int cx, int cz) {
		if (world && world->ChunkExistsInMap(cx, cz)) {
			Game::Chunk* chunk = world->RetrieveChunkFromMap(cx, cz);
			return &chunk->p_ChunkLightInformation;
		}

		return nullptr;
	}

	Game::Block* GetWorldBlock(const KUMA::MATHGL::Vector3& block_pos) {
		std::pair<Game::Block*, Game::Chunk*> block = world->GetBlockFromPosition(block_pos);
		return block.first;
	}
}
int main() {
	KUMA::CORE_SYSTEM::App app;

	auto scene = app.getCore().sceneManager->getCurrentScene();

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
			auto obj = scene->createObject("PointLight");
			auto light = obj->addComponent<KUMA::ECS::PointLight>();
			light->setIntensity(0.90f);
			light->setQuadratic(0.90f);
			light->setColor({255.0f, 200.0f, 255.0f});
			light->Radius = 25.0f;
			//obj.transform->setLocalPosition({rpos(), rpos(), rpos()});
			obj->transform->setLocalPosition({0, 5, 0});
			obj->transform->setLocalRotation({0.0f, 0.0f, 0.0f, 1.0f});
			obj->transform->setLocalScale({1.0f, 1.0f, 1.0f});
		}
	}

	{


		for (auto i = 0; i < 1; i++) {
			auto& obj = scene->createObject("DirLight");
			auto light = obj->addComponent<KUMA::ECS::DirectionalLight>();
			//light->shadowRes = 2048;
			light->orthoBoxSize = 100;
			//light->zNear = 1.0f;
			//light->zFar = 700.0f;
			//light->strength = 1.0f;
			light->distance = 100;

			light->setIntensity(1.0f);

			light->setColor({1.0f, 1.0f, 1.0f});
			obj->transform->setLocalPosition({-20.0f, 40.0f, 10.0f});
			obj->transform->setLocalRotation({0.81379771, -0.17101006, 0.29619816, 0.46984628});
			obj->transform->setLocalScale({1.5f, 1.5f, 1.5f});
			obj->addScript("ControllerDirLight");
		}
	}

	{
		auto& obj = scene->createObject("Point2Light");
		auto light = obj->addComponent<KUMA::ECS::PointLight>();
		//light->shadowRes = 2048;
		//light->zNear = 1.0f;
		//light->zFar = 2000.0f;
		//light->strength = 1.0f;
		//light->aspect = 1.0f;

		light->setIntensity(1.0f);
		light->setQuadratic(1.0f);
		light->setLinear(1.0f);
		light->setColor({1.0f, 1.0f, 1.0f});
		obj->transform->setLocalPosition({0.0f, 14.0f, -45.0f});
		obj->transform->setLocalRotation({0.81379771f, -0.17101006f, 0.29619816f, 0.46984628f});
		obj->transform->setLocalScale({1.0f, 1.0f, 1.0f});
	}
	{
		auto& obj = scene->createObject("Point3Light");
		auto light = obj->addComponent<KUMA::ECS::PointLight>();
		//light->shadowRes = 2048;
		//light->zNear = 1.0f;
		//light->zFar = 2000.0f;
		//light->strength = 1.0f;
		//light->aspect = 1.0f;

		light->setIntensity(1.0f);
		light->setQuadratic(1.0f);
		light->setLinear(1.0f);
		light->setColor({255.0f, 255.0f, 255.0f});
		obj->transform->setLocalPosition({-20.0f, 40.0f, 10.0f});
		obj->transform->setLocalRotation({0.81379771f, -0.17101006f, 0.29619816f, 0.46984628f});
		obj->transform->setLocalScale({1.0f, 1.0f, 1.0f});
	}
	{
		auto& obj = scene->createObject("Point3Light");
		auto light = obj->addComponent<KUMA::ECS::PointLight>();
		//light->shadowRes = 2048;
		//light->zNear = 1.0f;
		//light->zFar = 2000.0f;
		//light->strength = 1.0f;
		//light->aspect = 1.0f;

		light->setIntensity(1.0f);
		light->setQuadratic(1.0f);
		light->setLinear(1.0f);
		light->setColor({255.0f, 255.0f, 255.0f});
		obj->transform->setLocalPosition({-20.0f, 35.0f, 15.0f});
		obj->transform->setLocalRotation({0.81379771f, -0.17101006f, 0.29619816f, 0.46984628f});
		obj->transform->setLocalScale({1.0f, 1.0f, 1.0f});
	}
	{
		auto& obj = scene->createObject("Point3Light");
		auto light = obj->addComponent<KUMA::ECS::PointLight>();
		//light->shadowRes = 2048;
		//light->zNear = 1.0f;
		//light->zFar = 2000.0f;
		//light->strength = 1.0f;
		//light->aspect = 1.0f;

		light->setIntensity(1.0f);
		light->setQuadratic(1.0f);
		light->setLinear(1.0f);
		light->setColor({255.0f, 255.0f, 255.0f});
		obj->transform->setLocalPosition({-20.0f, 35.0f, 5.0f});
		obj->transform->setLocalRotation({0.81379771f, -0.17101006f, 0.29619816f, 0.46984628f});
		obj->transform->setLocalScale({1.0f, 1.0f, 1.0f});
	}
	{
		auto& obj = scene->createObject("AmbLight");

		nlohmann::json j;
		obj->transform->onSerialize(j);
		obj->transform->onDeserialize(j);
		auto s = j.dump(2);

		auto light = obj->addComponent<KUMA::ECS::AmbientSphereLight>();
		light->setIntensity(1.0f);
		light->setColor({1.0f, 0.7f, 0.8f});
		light->setRadius(1.0f);
		obj->transform->setLocalPosition({0.0f, 0.0f, 0.0f});
		obj->transform->setLocalRotation({0.0f, 0.0f, 0.0f, 1.0f});
		obj->transform->setLocalScale({1.0f, 1.0f, 1.0f});
	}
	{
		auto& obj = scene->createObject("Camera");
		auto cam = obj->addComponent<KUMA::ECS::CameraComponent>();
		cam->setFov(45.0f);
		cam->setSize(5.0f);
		cam->setNear(0.1f);
		cam->setFar(1000.0f);
		cam->setFrustumGeometryCulling(false);
		cam->setFrustumLightCulling(false);
		cam->setProjectionMode(KUMA::RENDER::Camera::ProjectionMode::PERSPECTIVE);
		obj->transform->setLocalPosition({0.f, 140.0f, 0.0f});
		obj->transform->setLocalRotation({0.0f, 0.98480773f, -0.17364819f, 0.0f});
		obj->transform->setLocalScale({1.0f, 1.0f, 1.0f});
		obj->addScript("Controller");

		/*auto inp = obj.addComponent<KUMA::ECS::InputComponent>([&app](float dt) {
			auto& core = app.getCore();
			if (core.inputManager->isKeyPressed(KUMA::INPUT_SYSTEM::EKey::KEY_S)) {
				auto pos = core.sceneManager.getCurrentScene()->findObjectByName("Camera")->transform->getLocalPosition();
				pos.z += 1.0f;
				core.sceneManager.getCurrentScene()->findObjectByName("Camera")->transform->setLocalPosition(pos);
			}
			if (core.inputManager->isKeyPressed(KUMA::INPUT_SYSTEM::EKey::KEY_W)) {
				auto pos = core.sceneManager.getCurrentScene()->findObjectByName("Camera")->transform->getLocalPosition();
				pos.z -= 1.0f;
				core.sceneManager.getCurrentScene()->findObjectByName("Camera")->transform->setLocalPosition(pos);
			}

			if (core.inputManager->isKeyPressed(KUMA::INPUT_SYSTEM::EKey::KEY_A)) {
				auto pos = core.sceneManager.getCurrentScene()->findObjectByName("Camera")->transform->getLocalPosition();
				pos.x -= 1.0f;
				core.sceneManager.getCurrentScene()->findObjectByName("Camera")->transform->setLocalPosition(pos);
			}
			if (core.inputManager->isKeyPressed(KUMA::INPUT_SYSTEM::EKey::KEY_D)) {
				auto pos = core.sceneManager.getCurrentScene()->findObjectByName("Camera")->transform->getLocalPosition();
				pos.x += 1.0f;
				core.sceneManager.getCurrentScene()->findObjectByName("Camera")->transform->setLocalPosition(pos);
			}

			if (core.inputManager->isKeyPressed(KUMA::INPUT_SYSTEM::EKey::KEY_Q)) {
				auto pos = core.sceneManager.getCurrentScene()->findObjectByName("Camera")->transform->getLocalPosition();
				pos.y -= 1.0f;
				core.sceneManager.getCurrentScene()->findObjectByName("Camera")->transform->setLocalPosition(pos);
			}
			if (core.inputManager->isKeyPressed(KUMA::INPUT_SYSTEM::EKey::KEY_E)) {
				auto pos = core.sceneManager.getCurrentScene()->findObjectByName("Camera")->transform->getLocalPosition();
				pos.y += 1.0f;
				core.sceneManager.getCurrentScene()->findObjectByName("Camera")->transform->setLocalPosition(pos);
			}

				static bool isFirst = true;
			static std::pair<int, int> prev;

			auto mpos = core.inputManager->getMousePosition();
			if (isFirst) {
				prev = mpos;
				isFirst = false;
			}
			static KUMA::MATHGL::Vector2f offset(0.0f, 0.0f);
			static KUMA::MATHGL::Vector2f look(0.0f, 0.0f);

			offset.x = mpos.first - prev.first;
			offset.y = mpos.second - prev.second;

			prev = mpos;

			offset = offset * 0.5f;

			look = look + offset;

			if (look.y > 89.0f) look.y = 89;
			if (look.y < -89.0f) look.y = -89;

			if (core.inputManager->isKeyPressed(KUMA::INPUT_SYSTEM::EKey::KEY_Z)) {
				core.sceneManager.getCurrentScene()->findObjectByName("Camera")->transform->setLocalRotation(
					KUMA::MATHGL::Quaternion(KUMA::MATHGL::Vector3(look.y, -look.x, 0)));
			}
			auto pos = core.sceneManager.getCurrentScene()->findObjectByName("Camera")->transform->getLocalPosition();

		});*/

		//world = new Game::World(42, KUMA::MATHGL::Vector2f(800, 600), "test", Game::Generation_Normal, &obj);
		//app->renderer.world = world;
	}
	/*{
		auto& obj = scene->createObject("A");

		auto model = obj.addComponent<KUMA::ECS::ModelRenderer>();
		KUMA::RESOURCES::ModelParserFlags flags = KUMA::RESOURCES::ModelParserFlags::TRIANGULATE;
		flags |= KUMA::RESOURCES::ModelParserFlags::GEN_SMOOTH_NORMALS;
		flags |= KUMA::RESOURCES::ModelParserFlags::FLIP_UVS;

		//flags |= KUMA::RESOURCES::ModelParserFlags::OPTIMIZE_MESHES;
		//flags |= KUMA::RESOURCES::ModelParserFlags::OPTIMIZE_GRAPH;
		//flags |= KUMA::RESOURCES::ModelParserFlags::FIND_INSTANCES;
		//flags |= KUMA::RESOURCES::ModelParserFlags::CALC_TANGENT_SPACE;
		//flags |= KUMA::RESOURCES::ModelParserFlags::JOIN_IDENTICAL_VERTICES;
		//flags |= KUMA::RESOURCES::ModelParserFlags::DEBONE;
		//flags |= KUMA::RESOURCES::ModelParserFlags::FIND_INVALID_DATA;
		//flags |= KUMA::RESOURCES::ModelParserFlags::IMPROVE_CACHE_LOCALITY;
		flags |= KUMA::RESOURCES::ModelParserFlags::GEN_UV_COORDS;
		//flags |= KUMA::RESOURCES::ModelParserFlags::PRE_TRANSFORM_VERTICES;

		//auto m = KUMA::RESOURCES::ModelLoader::Create("C:\\Projects\\SimpleEngine\\CGCourse\\CGCourse\\Assets\\guard\\boblampclean.md5mesh", flags);
		auto m = KUMA::RESOURCES::ModelLoader::Create("C:\\Projects\\SimpleEngine\\CGCourse\\CGCourse\\Assets\\Game\\textures\\dancing_vampire.dae", flags);
		//auto m = KUMA::RESOURCES::ModelLoader::Create("C:\\Projects\\SimpleEngine\\CGCourse\\CGCourse\\Assets\\User\\cottage_fbx.fbx", flags);
		//auto m = KUMA::RESOURCES::ModelLoader::Create("C:\\Projects\\SimpleEngine\\CGCourse\\CGCourse\\Assets\\User\\stone\\Stonefbx.fbx", flags);
		//auto m = KUMA::RESOURCES::ModelLoader::Create("C:\\Projects\\SimpleEngine\\CGCourse\\CGCourse\\Assets\\astroBoy_walk_Maya.dae", flags);
		//auto m = KUMA::RESOURCES::ModelLoader::Create("C:\\Projects\\SimpleEngine\\CGCourse\\CGCourse\\Assets\\Drone.fbx", flags);

		danceAnimation = std::make_shared<KUMA::RESOURCES::Animation>("C:\\Projects\\SimpleEngine\\CGCourse\\CGCourse\\Assets\\Game\\textures\\dancing_vampire.dae", m.get());
		animator = std::make_shared<KUMA::RESOURCES::Animator>(danceAnimation.get());
		KUMA::RENDER::BaseRender::animator = animator;
		obj.addComponent<KUMA::ECS::InputComponent>([](float dt) {
			if (animator)
			animator->UpdateAnimation(dt);
		});
		model->setModel(m);
		model->setFrustumBehaviour(KUMA::ECS::ModelRenderer::EFrustumBehaviour::CULL_MODEL);
		auto bs = KUMA::RENDER::BoundingSphere();
		bs.position = {0.0f, 0.0f, 0.0f};
		bs.radius = 1.0f;
		model->setCustomBoundingSphere(bs);
		obj.transform->setLocalPosition({0.0f, 0.0f, 0.0f});
		obj.transform->setLocalRotation({0.0f, 0.0f, 0.0f, 1.0f});
		//obj.transform->setLocalScale({100.01f, 100.01f, 100.01f});
		obj.transform->setLocalScale({1.0f, 1.0f, 1.0f});

		auto s = KUMA::RESOURCES::ShaderLoader::Create("C:\\Projects\\SimpleEngine\\CGCourse\\CGCourse\\Assets\\Engine\\Shaders\\Standard.glsl");

		auto mat = obj.addComponent<KUMA::ECS::MaterialRenderer>();
		auto _m = KUMA::RESOURCES::MaterialLoader::Create("");

		_m->setShader(s);
		_m->setBlendable(false);
		_m->setBackfaceCulling(true);
		_m->setFrontfaceCulling(false);
		_m->setDepthTest(true);
		_m->setDepthWriting(true);
		_m->setColorWriting(true);
		_m->setGPUInstances(1);
		auto& data = _m->getdata["sData();
		data["u_Diffuse"] = KUMA::MATHGL::Vector4{1.0f, 1.0f, 1.0f, 1.0f};

		auto tex1 = KUMA::RESOURCES::TextureLoader().createResource("textures\\Vampire_diffuse.png");
		data["u_DiffuseMap"] = tex1;

		data["u_EnableNormalMapping"] = 1;
		data["u_HeightScale"] = 0;

		auto tex2 = KUMA::RESOURCES::TextureLoader().createResource("textures\\Vampire_normal.png");
		data["u_NormalMap"] = tex2;
		data["u_Shininess"] = 100;
		data["u_Specular"] = KUMA::MATHGL::Vector3{1.0f, 1.0f, 1.0f};
		data["u_TextureOffset"] = KUMA::MATHGL::Vector2f{0.0f, 0.0f};
		data["u_TextureTiling"] = KUMA::MATHGL::Vector2f{1.0f, 1.0f};
		mat->fillWithMaterial(_m);


	}*/

	/*{
		for (int i = 0; i < 0; i++) {
			auto& obj = scene->createObject("AA"+std::to_string(i));

			auto model = obj.addComponent<KUMA::ECS::ModelRenderer>();
			KUMA::RESOURCES::ModelParserFlags flags = KUMA::RESOURCES::ModelParserFlags::TRIANGULATE;
			flags |= KUMA::RESOURCES::ModelParserFlags::GEN_SMOOTH_NORMALS;
			flags |= KUMA::RESOURCES::ModelParserFlags::FLIP_UVS;

			//flags |= KUMA::RESOURCES::ModelParserFlags::OPTIMIZE_MESHES;
			//flags |= KUMA::RESOURCES::ModelParserFlags::OPTIMIZE_GRAPH;
			//flags |= KUMA::RESOURCES::ModelParserFlags::FIND_INSTANCES;
			//flags |= KUMA::RESOURCES::ModelParserFlags::CALC_TANGENT_SPACE;
			//flags |= KUMA::RESOURCES::ModelParserFlags::JOIN_IDENTICAL_VERTICES;
			//flags |= KUMA::RESOURCES::ModelParserFlags::DEBONE;
			//flags |= KUMA::RESOURCES::ModelParserFlags::FIND_INVALID_DATA;
			//flags |= KUMA::RESOURCES::ModelParserFlags::IMPROVE_CACHE_LOCALITY;
			flags |= KUMA::RESOURCES::ModelParserFlags::GEN_UV_COORDS;
			//flags |= KUMA::RESOURCES::ModelParserFlags::PRE_TRANSFORM_VERTICES;

			//auto m = KUMA::RESOURCES::ModelLoader::Create("C:\\Projects\\SimpleEngine\\CGCourse\\CGCourse\\Assets\\guard\\boblampclean.md5mesh", flags);
			//auto m = KUMA::RESOURCES::ModelLoader::Create("C:\\Projects\\SimpleEngine\\CGCourse\\CGCourse\\Assets\\Game\\textures\\dancing_vampire.dae", flags);
			//auto m = KUMA::RESOURCES::ModelLoader::Create("C:\\Projects\\SimpleEngine\\CGCourse\\CGCourse\\Assets\\Engine\Models\\Cube.fbx", flags);
			auto m = KUMA::RESOURCES::ModelLoader::Create("C:\\Projects\\SimpleEngine\\CGCourse\\CGCourse\\Assets\\User\\cottage_fbx.fbx", flags);
			//auto m = KUMA::RESOURCES::ModelLoader::Create("C:\\Projects\\SimpleEngine\\CGCourse\\CGCourse\\Assets\\User\\stone\\Stonefbx.fbx", flags);
			//auto m = KUMA::RESOURCES::ModelLoader::Create("C:\\Projects\\SimpleEngine\\CGCourse\\CGCourse\\Assets\\astroBoy_walk_Maya.dae", flags);
			//auto m = KUMA::RESOURCES::ModelLoader::Create("C:\\Projects\\SimpleEngine\\CGCourse\\CGCourse\\Assets\\Drone.fbx", flags);

			//danceAnimation = std::make_shared<KUMA::RESOURCES::Animation>("C:\\Projects\\SimpleEngine\\CGCourse\\CGCourse\\Assets\\Game\\textures\\dancing_vampire.dae", m.get());
			//animator = std::make_shared<KUMA::RESOURCES::Animator>(danceAnimation.get());
			//KUMA::RENDER::BaseRender::animator = animator;
			//obj.addComponent<KUMA::ECS::InputComponent>([](float dt) {
			//	if (animator)
			//		animator->UpdateAnimation(dt);
			//});
			model->setModel(m);
			model->setFrustumBehaviour(KUMA::ECS::ModelRenderer::EFrustumBehaviour::CULL_MODEL);
			auto bs = KUMA::RENDER::BoundingSphere();
			bs.position = {0.0f, 0.0f, 0.0f};
			bs.radius = 1.0f;
			model->setCustomBoundingSphere(bs);

			auto rpos = []() {
				auto LO = -100.0f;
				auto HI = 100.0f;
				return  LO + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (HI - LO)));
			};

			obj.transform->setLocalPosition({rpos(), rpos(), rpos()});
			obj.transform->setLocalRotation({0.0f, 0.0f, 0.0f, 1.0f});
			//obj.transform->setLocalScale({100.01f, 100.01f, 100.01f});
			obj.transform->setLocalScale({0.020f, 0.020f, 0.020f});

			//auto s = KUMA::RESOURCES::ShaderLoader::Create("C:\\Projects\\SimpleEngine\\CGCourse\\CGCourse\\Assets\\Engine\\Shaders\\Standard.glsl");

			auto mat = obj.addComponent<KUMA::ECS::MaterialRenderer>();
			auto _m = KUMA::RESOURCES::MaterialLoader::Create("");

			//_m->setShader(s);
			_m->setBlendable(false);
			_m->setBackfaceCulling(true);
			_m->setFrontfaceCulling(false);
			_m->setDepthTest(true);
			_m->setDepthWriting(true);
			_m->setColorWriting(true);
			_m->setGPUInstances(1);
			auto& data = _m->getdata["sData();
			//data["u_Diffuse"] = KUMA::MATHGL::Vector4{1.0f, 1.0f, 1.0f, 1.0f};
			//auto tex1 = KUMA::RESOURCES::TextureLoader().createResource("textures\\cottage_diffuse.png");
			//data["u_DiffuseMap"] = tex1;
			//data["u_EnableNormalMapping"] = 1;
			//data["u_HeightScale"] = 0;
			//auto tex2 = KUMA::RESOURCES::TextureLoader().createResource("textures\\cottage_normal.png");
			//data["u_NormalMap"] = tex2;
			//data["u_Shininess"] = 100;
			//data["u_Specular"] = KUMA::MATHGL::Vector3{1.0f, 1.0f, 1.0f};
			//data["u_TextureOffset"] = KUMA::MATHGL::Vector2f{0.0f, 0.0f};
			//data["u_TextureTiling"] = KUMA::MATHGL::Vector2f{1.0f, 1.0f};
			//mat->fillWithMaterial(_m);

			auto tex1 =  KUMA::RESOURCES::TextureLoader().createResource("textures\\brick_albedo.jpg");
			//uint8_t buffer [] = {0, 200, 0};
			//tex1->Load(buffer, 1, 1, 3, false, KUMA::RESOURCES::TextureFormat::RGB);
			data["AlbedoMap"] = tex1;
			//_m->albedoMap = tex1;

			auto texM = KUMA::RESOURCES::TextureLoader().createResource("textures\\brick_metalic.png");
			data["MetallicMap"] = texM;

			auto texE = KUMA::RESOURCES::TextureLoader().createResource("textures\\brick_metalic.png");
			data["EmissiveMap"] = texE;
			auto tex2 = KUMA::RESOURCES::TextureLoader().createResource("textures\\brick_ao.jpg");
			data["AmbientOcclusionMap"] = tex2;
			data["BaseColor"] = KUMA::MATHGL::Vector3(1.0f, 1.0f, 1.0f);
			data["Displacement"] = 0.025f;
			data["Emission"] = 0.0f;
			auto tex3 = KUMA::RESOURCES::TextureLoader().createResource("textures\\brick_height.jpg");
			data["HeightMap"] = tex3;
			//data["Height_map"] = 20;
			data["id"] = 1;
			data["MetallicFactor"] = 0.0f;
			data["UVMultipliers"] = KUMA::MATHGL::Vector2f(1.0f, 1.0f);
			//data["metallic_map"] : 18446744073709551615,
			data["Name"] = "DefaultMaterial";
			auto texN = KUMA::RESOURCES::TextureLoader().createResource("textures\\brick_normal.jpg");
			data["NormalMap"] = texN;
			data["RoughnessRactor"] = 0.75f;
			auto tex4 = KUMA::RESOURCES::TextureLoader().createResource("textures\\brick_roughness.jpg");
			data["RoughnessMap"] = tex4;
			data["Transparency"] = 1.0f;
			data["RoughnessFactor"] = 0.75f;
			mat->fillWithMaterial(_m);
		}

		for (int i = 0; i < 2; i++) {
			auto& obj = scene->createObject("AA" + std::to_string(i));

			auto model = obj.addComponent<KUMA::ECS::ModelRenderer>();
			KUMA::RESOURCES::ModelParserFlags flags = KUMA::RESOURCES::ModelParserFlags::TRIANGULATE;
			//flags |= KUMA::RESOURCES::ModelParserFlags::GEN_SMOOTH_NORMALS;
			//flags |= KUMA::RESOURCES::ModelParserFlags::FLIP_UVS;
			//flags |= KUMA::RESOURCES::ModelParserFlags::GEN_UV_COORDS;
			auto m = KUMA::RESOURCES::ModelLoader::Create("C:\\Projects\\SimpleEngine\\CGCourse\\CGCourse\\Assets\\Engine\\Models\\Cube.fbx", flags);

			model->setModel(m);
			model->setFrustumBehaviour(KUMA::ECS::ModelRenderer::EFrustumBehaviour::CULL_MODEL);
			auto bs = KUMA::RENDER::BoundingSphere();
			bs.position = {0.0f, 0.0f, 0.0f};
			bs.radius = 1.0f;
			model->setCustomBoundingSphere(bs);

			auto rpos = []() {
				auto LO = -100.0f;
				auto HI = 100.0f;
				return  LO + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (HI - LO)));
			};

			if (i == 0) {
				obj.transform->setLocalPosition({0.0f, 0.0f, 0.0f});
				obj.transform->setLocalRotation({0.0f, 0.0f, 0.0f, 1.0f});
				obj.transform->setLocalScale({100.70f, 0.01f, 100.70f});
			}
			else
			{
				obj.transform->setLocalPosition({0.0f, 5.0f, 0.0f});
				obj.transform->setLocalRotation({0.0f, 0.0f, 0.0f, 1.0f});
				obj.transform->setLocalScale({10.0f, 10.0f, 10.0f});
			}
			auto mat = obj.addComponent<KUMA::ECS::MaterialRenderer>();
			auto _m = KUMA::RESOURCES::MaterialLoader::Create("");
			_m->setBlendable(false);
			_m->setBackfaceCulling(true);
			_m->setFrontfaceCulling(false);
			_m->setDepthTest(true);
			_m->setDepthWriting(true);
			_m->setColorWriting(true);
			_m->setGPUInstances(1);
			auto& data = _m->getdata["sData();

			auto tex1 = KUMA::RESOURCES::TextureLoader().createResource("textures\\brick_albedo.jpg");
			data["AlbedoMap"] = tex1;

			auto texM = KUMA::RESOURCES::TextureLoader().createResource("textures\\brick_metalic.png");
			data["MetallicMap"] = texM;

			auto texE = KUMA::RESOURCES::TextureLoader().createResource("textures\\brick_metalic.png");
			data["EmissiveMap"] = texE;
			auto tex2 = KUMA::RESOURCES::TextureLoader().createResource("textures\\brick_ao.jpg");
			data["AmbientOcclusionMap"] = tex2;
			data["BaseColor"] = KUMA::MATHGL::Vector3(1.0f, 1.0f, 1.0f);
			data["Displacement"] = 0.025f;
			data["Emission"] = 0.0f;
			auto tex3 = KUMA::RESOURCES::TextureLoader().createResource("textures\\brick_height.jpg");
			data["HeightMap"] = tex3;
			//data["Height_map"] = 20;
			data["id"] = 1;
			data["MetallicFactor"] = 0.0f;
			data["UVMultipliers"] = KUMA::MATHGL::Vector2f(1.0f, 1.0f);
			//data["metallic_map"] : 18446744073709551615,
			data["Name"] = "DefaultMaterial";
			auto texN = KUMA::RESOURCES::TextureLoader().createResource("textures\\brick_normal.jpg");
			data["NormalMap"] = texN;
			data["RoughnessRactor"] = 0.75f;
			auto tex4 = KUMA::RESOURCES::TextureLoader().createResource("textures\\brick_roughness.jpg");
			data["RoughnessMap"] = tex4;
			data["Transparency"] = 1.0f;
			data["RoughnessFactor"] = 0.75f;
			mat->fillWithMaterial(_m);
		}
	}*/
	{
		auto& obj = scene->createObject("AAA");

		auto model = obj->addComponent<KUMA::ECS::ModelRenderer>();
		KUMA::RESOURCES::ModelParserFlags flags = KUMA::RESOURCES::ModelParserFlags::TRIANGULATE;
		flags |= KUMA::RESOURCES::ModelParserFlags::GEN_SMOOTH_NORMALS;
		//flags |= KUMA::RESOURCES::ModelParserFlags::FLIP_UVS;

		//flags |= KUMA::RESOURCES::ModelParserFlags::OPTIMIZE_MESHES;
		//flags |= KUMA::RESOURCES::ModelParserFlags::OPTIMIZE_GRAPH;
		//flags |= KUMA::RESOURCES::ModelParserFlags::FIND_INSTANCES;
		flags |= KUMA::RESOURCES::ModelParserFlags::CALC_TANGENT_SPACE;
		//flags |= KUMA::RESOURCES::ModelParserFlags::JOIN_IDENTICAL_VERTICES;
		//flags |= KUMA::RESOURCES::ModelParserFlags::DEBONE;
		//flags |= KUMA::RESOURCES::ModelParserFlags::FIND_INVALID_DATA;
		//flags |= KUMA::RESOURCES::ModelParserFlags::IMPROVE_CACHE_LOCALITY;
		//flags |= KUMA::RESOURCES::ModelParserFlags::GEN_UV_COORDS;
		//flags |= KUMA::RESOURCES::ModelParserFlags::PRE_TRANSFORM_VERTICES;

		//auto m = KUMA::RESOURCES::ModelLoader::Create("C:\\Projects\\SimpleEngine\\CGCourse\\CGCourse\\Assets\\guard\\boblampclean.md5mesh", flags);
		auto m = KUMA::RESOURCES::ModelLoader::Create("C:\\Projects\\SimpleEngine\\CGCourse\\CGCourse\\Assets\\Game\\textures\\dancing_vampire.dae", flags);
		//auto m = KUMA::RESOURCES::ModelLoader::Create("C:\\Projects\\SimpleEngine\\CGCourse\\CGCourse\\Assets\\User\\cottage_fbx.fbx", flags);
		//auto m = KUMA::RESOURCES::ModelLoader::Create("C:\\Projects\\SimpleEngine\\CGCourse\\CGCourse\\Assets\\User\\stone\\Stonefbx.fbx", flags);
		//auto m = KUMA::RESOURCES::ModelLoader::Create("C:\\Projects\\SimpleEngine\\CGCourse\\CGCourse\\Assets\\astroBoy_walk_Maya.dae", flags);
		//auto m = KUMA::RESOURCES::ModelLoader::Create("C:\\Projects\\SimpleEngine\\CGCourse\\CGCourse\\Assets\\Drone.fbx", flags);

		danceAnimation = std::make_shared<KUMA::RESOURCES::Animation>("C:\\Projects\\SimpleEngine\\CGCourse\\CGCourse\\Assets\\Game\\textures\\dancing_vampire.dae", m.get());
		animator = std::make_shared<KUMA::RESOURCES::Animator>(danceAnimation.get());
		KUMA::RENDER::BaseRender::animator = animator;
		obj->addComponent<KUMA::ECS::InputComponent>([](float dt) {
			if (animator)
				animator->UpdateAnimation(dt);
		});
		model->setModel(m);
		model->setFrustumBehaviour(KUMA::ECS::ModelRenderer::EFrustumBehaviour::CULL_MODEL);
		auto bs = KUMA::RENDER::BoundingSphere();
		bs.position = {0.0f, 0.0f, 0.0f};
		bs.radius = 1.0f;
		model->setCustomBoundingSphere(bs);
		obj->transform->setLocalPosition({0.0f, 0.0f, -40.0f});
		obj->transform->setLocalRotation({0.0f, 0.0f, 0.0f, 1.0f});
		//obj.transform->setLocalScale({100.01f, 100.01f, 100.01f});
		obj->transform->setLocalScale({10.20f, 10.20f, 10.20f});

		auto s = KUMA::RESOURCES::ShaderLoader::CreateFromFile("Shaders\\Standard.glsl");

		auto mat = obj->addComponent<KUMA::ECS::MaterialRenderer>();

		auto mm = KUMA::RESOURCES::MaterialLoader::CreateFromFile("Materials/vampire.mat");
		mat->fillWithMaterial(mm);
		//auto _m = KUMA::RESOURCES::MaterialLoader::Create("Materials/vampire.mat");
		//_m->setShader(s);
		//_m->setBlendable(false);
		//_m->setBackfaceCulling(true);
		//_m->setFrontfaceCulling(false);
		//_m->setDepthTest(true);
		//_m->setDepthWriting(true);
		//_m->setColorWriting(true);
		//_m->setGPUInstances(1);
		//auto& data = _m->getUniformsData();
		//data["u_Diffuse"] = KUMA::MATHGL::Vector4{1.0f, 1.0f, 1.0f, 1.0f};
		//
		//auto tex1 = KUMA::RESOURCES::TextureLoader().CreateFromFile("textures\\Vampire_diffuse.png");
		//data["u_DiffuseMap"] = tex1;
		//
		//data["u_EnableNormalMapping"] = true;
		//data["u_HeightScale"] = 0.0f;
		//
		//auto tex2 = KUMA::RESOURCES::TextureLoader().CreateFromFile("textures\\Vampire_normal.png");
		//data["u_NormalMap"] = tex2;
		//auto tex3 = KUMA::RESOURCES::TextureLoader().CreateFromFile("textures\\noiseTexture.png");
		//data["u_Noise"] = tex3;
		//data["fogScaleBias"] = KUMA::MATHGL::Vector4(0, -0.06f, 0, 0.0008f);
		//data["u_Shininess"] = 100;
		//data["u_Specular"] = KUMA::MATHGL::Vector3{1.0f, 1.0f, 1.0f};
		//data["u_TextureOffset"] = KUMA::MATHGL::Vector2f{0.0f, 0.0f};
		//data["u_TextureTiling"] = KUMA::MATHGL::Vector2f{1.0f, 1.0f};
		//data["u_UseBone"] = true;
		//mat->fillWithMaterial(_m);
	}


	{
		auto& obj = scene->createObject("Plane");

		auto model = obj->addComponent<KUMA::ECS::ModelRenderer>();
		KUMA::RESOURCES::ModelParserFlags flags = KUMA::RESOURCES::ModelParserFlags::TRIANGULATE;
		flags |= KUMA::RESOURCES::ModelParserFlags::GEN_SMOOTH_NORMALS;
		flags |= KUMA::RESOURCES::ModelParserFlags::FLIP_UVS;
		flags |= KUMA::RESOURCES::ModelParserFlags::GEN_UV_COORDS;
		flags |= KUMA::RESOURCES::ModelParserFlags::CALC_TANGENT_SPACE;

		//auto m = KUMA::RESOURCES::ModelLoader::Create("C:\\Projects\\SimpleEngine\\CGCourse\\CGCourse\\Assets\\User\\cottage_fbx.fbx", flags);
		auto m = KUMA::RESOURCES::ModelLoader::Create("C:\\Projects\\SimpleEngine\\CGCourse\\CGCourse\\Assets\\Engine\\Models\\Cube.fbx", flags);
		model->setModel(m);
		model->setFrustumBehaviour(KUMA::ECS::ModelRenderer::EFrustumBehaviour::CULL_MODEL);
		auto bs = KUMA::RENDER::BoundingSphere();
		bs.position = {0.0f, 0.0f, 0.0f};
		bs.radius = 100.0f;
		model->setCustomBoundingSphere(bs);
		obj->transform->setLocalPosition({0.0f, -5.0f, 10.0f});
		obj->transform->setLocalRotation({0.0f, 0.0f, 0.0f, 1.0f});
		//obj.transform->setLocalScale({100.01f, 100.01f, 100.01f});
		obj->transform->setLocalScale({300.0f, 1.0f, 300.0f});

		auto s = KUMA::RESOURCES::ShaderLoader::CreateFromFile("Shaders\\Standard.glsl");

		auto mat = obj->addComponent<KUMA::ECS::MaterialRenderer>();
		auto _m = KUMA::RESOURCES::MaterialLoader::Create("");

		//s = app.renderer.context.renderer->shadersMap["deferredGBuffer"];
		_m->setShader(s);
		_m->setBlendable(false);
		_m->setBackfaceCulling(true);
		_m->setFrontfaceCulling(false);
		_m->setDepthTest(true);
		_m->setDepthWriting(true);
		_m->setColorWriting(true);
		_m->setGPUInstances(1);
		auto& data = _m->getUniformsData();
		data["u_Diffuse"] = KUMA::MATHGL::Vector4{1.0f, 1.0f, 1.0f, 1.0f};

		auto tex1 = KUMA::RESOURCES::TextureLoader().CreateFromFile("textures\\brick_albedo.jpg");
		data["u_DiffuseMap"] = tex1;

		data["u_EnableNormalMapping"] = true;
		data["u_HeightScale"] = 0.0f;

		auto tex2 = KUMA::RESOURCES::TextureLoader().CreateFromFile("textures\\brick_normal.jpg");
		data["u_NormalMap"] = tex2;
		auto tex3 = KUMA::RESOURCES::TextureLoader().CreateFromFile("textures\\noiseTexture.png");
		data["u_Noise"] = tex3;
		data["fogScaleBias"] = KUMA::MATHGL::Vector4(0, -0.06f, 0, 0.0008f);
		data["u_Shininess"] = 100;
		data["u_Specular"] = KUMA::MATHGL::Vector3{1.0f, 1.0f, 1.0f};
		data["u_TextureOffset"] = KUMA::MATHGL::Vector2f{0.0f, 0.0f};
		data["u_TextureTiling"] = KUMA::MATHGL::Vector2f{1.0f, 1.0f};
		data["u_UseBone"] = false;
		mat->fillWithMaterial(_m);


		//auto pc = obj->addComponent<KUMA::ECS::PhysicsComponent>();
		auto body = new cyclone::RigidBody();
		body->setPosition(obj->transform->getLocalPosition().x, obj->transform->getLocalPosition().y, obj->transform->getLocalPosition().z);
		body->setOrientation(1, 0, 0, 0);
		body->setVelocity(0, 0, 0);
		body->setRotation(0, 0, 0);

		body->setInverseMass(0);

		cyclone::Matrix3 tensor;
		tensor.setBlockInertiaTensor(cyclone::Vector3{30, 1.f, 30} *0.5f, 0);
		body->setInertiaTensor(tensor);

		body->setLinearDamping(0.95f);
		body->setAngularDamping(0.8f);
		body->clearAccumulators();
		body->setAcceleration(0, 0, 0);

		body->setCanSleep(false);
		body->setAwake();

		body->calculateDerivedData();
		auto colider = new cyclone::CollisionBox();
		colider->halfSize = {30.0f, 1.0f, 30.0f} *0.5f;
		//colider->transform = obj->transform->getLocalMatrix();
		colider->body = body;
		colider->calculateInternals();

		//pc->body = b;
		//pc->collider = std::move(colider);
		//pc->boundingSphere = bs;
		//pc->collisionType = KUMA::ECS::CollisionType::OBB;

		app.getCore().physicsManger->bodies.push_back(body);
		app.getCore().physicsManger->bodiesCollide.push_back(colider);
		//app.getCore().physicsManger->objectsList.push_back(pc);

		auto inp = obj->addComponent<KUMA::ECS::InputComponent>([body, obj](float dt) {
			//if (pc->body)
			obj->transform->setLocalPosition(KUMA::MATHGL::Vector3(body->getPosition().x, body->getPosition().y, body->getPosition().z));
			obj->transform->setLocalRotation(KUMA::MATHGL::Quaternion(
				body->getOrientation().i, body->getOrientation().j, body->getOrientation().k, body->getOrientation().r));

			//pc->collider->transform = pc->obj.transform->getLocalMatrix();
			//pc->boundingSphere.position = pc->obj.transform->getLocalPosition();
			//if (pc->obj.transform->getLocalPosition().y < -500.0f) {
			//	pc->obj.transform->setLocalPosition(KUMA::MATHGL::Vector3(obj->transform->getLocalPosition().x, 500.0f, obj->transform->getLocalPosition().z));
			//}
		});
	}

	{
		auto& obj = scene->createObject("Box1");

		auto model = obj->addComponent<KUMA::ECS::ModelRenderer>();
		KUMA::RESOURCES::ModelParserFlags flags = KUMA::RESOURCES::ModelParserFlags::TRIANGULATE;
		//flags |= KUMA::RESOURCES::ModelParserFlags::GEN_SMOOTH_NORMALS;
		flags |= KUMA::RESOURCES::ModelParserFlags::FLIP_UVS;
		flags |= KUMA::RESOURCES::ModelParserFlags::GEN_UV_COORDS;
		flags |= KUMA::RESOURCES::ModelParserFlags::CALC_TANGENT_SPACE;

		//auto m = KUMA::RESOURCES::ModelLoader::Create("C:\\Projects\\SimpleEngine\\CGCourse\\CGCourse\\Assets\\User\\cottage_fbx.fbx", flags);
		auto m = KUMA::RESOURCES::ModelLoader::Create("C:\\Projects\\SimpleEngine\\CGCourse\\CGCourse\\Assets\\Engine\\Models\\Cube.fbx", flags);
		model->setModel(m);
		model->setFrustumBehaviour(KUMA::ECS::ModelRenderer::EFrustumBehaviour::CULL_MODEL);
		auto bs = KUMA::RENDER::BoundingSphere();
		bs.position = {0.0f, 0.0f, 0.0f};
		bs.radius = 10.50f;
		model->setCustomBoundingSphere(bs);
		obj->transform->setLocalPosition({0.0f, 55.0f, 0.0f});
		obj->transform->setLocalRotation({0.0f, 0.0f, 0.0f, 1.0f});
		//obj.transform->setLocalScale({100.01f, 100.01f, 100.01f});
		obj->transform->setLocalScale({10.5f, 10.5f, 10.5f});

		auto s = KUMA::RESOURCES::ShaderLoader::CreateFromFile("Shaders\\Standard.glsl");

		auto mat = obj->addComponent<KUMA::ECS::MaterialRenderer>();
		auto _m = KUMA::RESOURCES::MaterialLoader::Create("");

		_m->setShader(s);
		_m->setBlendable(false);
		_m->setBackfaceCulling(true);
		_m->setFrontfaceCulling(false);
		_m->setDepthTest(true);
		_m->setDepthWriting(true);
		_m->setColorWriting(true);
		_m->setGPUInstances(1);
		auto& data = _m->getUniformsData();
		data["u_Diffuse"] = KUMA::MATHGL::Vector4{1.0f, 1.0f, 1.0f, 1.0f};

		auto tex1 = KUMA::RESOURCES::TextureLoader().CreateFromFile("textures\\brick_albedo.jpg");
		data["u_DiffuseMap"] = tex1;

		data["u_EnableNormalMapping"] = true;
		data["u_HeightScale"] = 0.0f;

		auto tex2 = KUMA::RESOURCES::TextureLoader().CreateFromFile("textures\\brick_normal.jpg");
		data["u_NormalMap"] = tex2;
		auto tex3 = KUMA::RESOURCES::TextureLoader().CreateFromFile("textures\\noiseTexture.png");
		data["u_Noise"] = tex3;
		data["fogScaleBias"] = KUMA::MATHGL::Vector4(0, -0.06f, 0, 0.0008f);
		data["u_Shininess"] = 100;
		data["u_Specular"] = KUMA::MATHGL::Vector3{1.0f, 1.0f, 1.0f};
		data["u_TextureOffset"] = KUMA::MATHGL::Vector2f{0.0f, 0.0f};
		data["u_TextureTiling"] = KUMA::MATHGL::Vector2f{1.0f, 1.0f};
		data["u_UseBone"] = false;
		mat->fillWithMaterial(_m);

		//auto pc = obj->addComponent<KUMA::ECS::PhysicsComponent>();
		auto body = new cyclone::RigidBody();
		body->setPosition(obj->transform->getLocalPosition().x, obj->transform->getLocalPosition().y, obj->transform->getLocalPosition().z);
		body->setOrientation(1, 0, 0, 0);
		body->setVelocity(0, 0, 0);
		body->setRotation(0, 0, 0);
		//halfSize = cyclone::Vector3(1, 1, 1);
		float mass = 5555.0f;
		body->setMass(mass);

		cyclone::Matrix3 tensor;
		tensor.setBlockInertiaTensor(cyclone::Vector3{10.5f, 10.5f, 10.5f} *0.5f, mass);
		body->setInertiaTensor(tensor);

		body->setLinearDamping(0.95f);
		body->setAngularDamping(0.8f);
		body->clearAccumulators();
		body->setAcceleration(0, -10.0f, 0);

		body->setCanSleep(false);
		body->setAwake();

		body->calculateDerivedData();

		auto colider = new cyclone::CollisionBox();
		colider->halfSize = cyclone::Vector3{10.5f, 10.5f, 10.5f} *0.5f;
		//colider->transform = obj->transform->getLocalMatrix();
		colider->body = body;
		colider->calculateInternals();
		//colider->transform.data[3]  += 10.5f * 0.5;
		//colider->transform.data[7]  += 10.5f * 0.5;
		//colider->transform.data[11] += 10.5f * 0.5;
		//colider->body = ;
		//pc->body = body;
		//pc->collider = std::move(colider);
		//pc->boundingSphere = bs;
		//pc->collisionType = KUMA::ECS::CollisionType::OBB;

		app.getCore().physicsManger->bodies.push_back(body);
		app.getCore().physicsManger->bodiesCollide.push_back(colider);

		auto inp = obj->addComponent<KUMA::ECS::InputComponent>([body, obj](float dt) {
			//if (pc->body)
			obj->transform->setLocalPosition(KUMA::MATHGL::Vector3(body->getPosition().x, body->getPosition().y, body->getPosition().z));
			//obj->transform->setLocalRotation(KUMA::MATHGL::Quaternion(
			//	KUMA::MATHGL::Vector3(TO_DEGREES(body->getRotation().x), TO_DEGREES(body->getRotation().y), TO_DEGREES(body->getRotation().z))));
			obj->transform->setLocalRotation(KUMA::MATHGL::Quaternion(
				body->getOrientation().i, body->getOrientation().j, body->getOrientation().k, body->getOrientation().r));


			//pc->collider->transform = pc->obj.transform->getLocalMatrix();
			//pc->boundingSphere.position = pc->obj.transform->getLocalPosition();
			//if (pc->obj.transform->getLocalPosition().y < -500.0f) {
			//	pc->obj.transform->setLocalPosition(KUMA::MATHGL::Vector3(obj->transform->getLocalPosition().x, 500.0f, obj->transform->getLocalPosition().z));
			//}
			liveTime -= dt;
			if (liveTime < 0) {
				liveTime = 20.0f;
				body->setPosition(0.0f, 55.0f, 0.0f);
				body->setOrientation(1, 0, 0, 0);
				body->setVelocity(0, 0, 0);
				body->setRotation(cyclone::Vector3(0, 0, 0));
			}
		});
	}


	{
		auto& obj = scene->createObject("BoxN");

		auto model = obj->addComponent<KUMA::ECS::ModelRenderer>();
		KUMA::RESOURCES::ModelParserFlags flags = KUMA::RESOURCES::ModelParserFlags::TRIANGULATE;
		//flags |= KUMA::RESOURCES::ModelParserFlags::GEN_SMOOTH_NORMALS;
		flags |= KUMA::RESOURCES::ModelParserFlags::FLIP_UVS;
		flags |= KUMA::RESOURCES::ModelParserFlags::GEN_UV_COORDS;
		flags |= KUMA::RESOURCES::ModelParserFlags::CALC_TANGENT_SPACE;

		//auto m = KUMA::RESOURCES::ModelLoader::Create("C:\\Projects\\SimpleEngine\\CGCourse\\CGCourse\\Assets\\User\\cottage_fbx.fbx", flags);
		auto m = KUMA::RESOURCES::ModelLoader::Create("C:\\Projects\\SimpleEngine\\CGCourse\\CGCourse\\Assets\\Engine\\Models\\Cube.fbx", flags);
		model->setModel(m);
		model->setFrustumBehaviour(KUMA::ECS::ModelRenderer::EFrustumBehaviour::CULL_MODEL);
		auto bs = KUMA::RENDER::BoundingSphere();
		bs.position = {0.0f, 0.0f, 0.0f};
		bs.radius = 10.50;
		model->setCustomBoundingSphere(bs);
		obj->transform->setLocalPosition({7.0f, 155.0f, 0.0f});
		obj->transform->setLocalRotation({0.0f, 0.0f, 0.0f, 1.0f});
		//obj.transform->setLocalScale({100.01f, 100.01f, 100.01f});
		obj->transform->setLocalScale({10.5f, 10.5f, 10.5f});

		auto s = KUMA::RESOURCES::ShaderLoader::CreateFromFile("Shaders\\Standard.glsl");

		auto mat = obj->addComponent<KUMA::ECS::MaterialRenderer>();
		auto _m = KUMA::RESOURCES::MaterialLoader::Create("");

		_m->setShader(s);
		_m->setBlendable(false);
		_m->setBackfaceCulling(true);
		_m->setFrontfaceCulling(false);
		_m->setDepthTest(true);
		_m->setDepthWriting(true);
		_m->setColorWriting(true);
		_m->setGPUInstances(1);
		auto& data = _m->getUniformsData();
		data["u_Diffuse"] = KUMA::MATHGL::Vector4{1.0f, 1.0f, 1.0f, 1.0f};

		auto tex1 = KUMA::RESOURCES::TextureLoader().CreateFromFile("textures\\brick_albedo.jpg");
		data["u_DiffuseMap"] = tex1;

		data["u_EnableNormalMapping"] = true;
		data["u_HeightScale"] = 0.0f;

		auto tex2 = KUMA::RESOURCES::TextureLoader().CreateFromFile("textures\\brick_normal.jpg");
		data["u_NormalMap"] = tex2;
		auto tex3 = KUMA::RESOURCES::TextureLoader().CreateFromFile("textures\\noiseTexture.png");
		data["u_Noise"] = tex3;
		data["fogScaleBias"] = KUMA::MATHGL::Vector4(0, -0.06f, 0, 0.0008f);
		data["u_Shininess"] = 100;
		data["u_Specular"] = KUMA::MATHGL::Vector3{1.0f, 1.0f, 1.0f};
		data["u_TextureOffset"] = KUMA::MATHGL::Vector2f{0.0f, 0.0f};
		data["u_TextureTiling"] = KUMA::MATHGL::Vector2f{1.0f, 1.0f};
		data["u_UseBone"] = false;
		mat->fillWithMaterial(_m);

		//auto pc = obj->addComponent<KUMA::ECS::PhysicsComponent>();
		auto body = new cyclone::RigidBody();
		body->setPosition(obj->transform->getLocalPosition().x, obj->transform->getLocalPosition().y, obj->transform->getLocalPosition().z);
		body->setOrientation(1, 0, 0, 0);
		body->setVelocity(0, 0, 0);
		body->setRotation(0, 0, 0);
		//halfSize = cyclone::Vector3(1, 1, 1);
		float mass = 5555.0f;
		body->setMass(mass);

		cyclone::Matrix3 tensor;
		tensor.setBlockInertiaTensor(cyclone::Vector3{10.5f, 10.5f, 10.5f} *0.5f, mass);
		body->setInertiaTensor(tensor);

		body->setLinearDamping(0.95f);
		body->setAngularDamping(0.8f);
		body->clearAccumulators();
		body->setAcceleration(0, -10.0f, 0);

		body->setCanSleep(false);
		body->setAwake();

		body->calculateDerivedData();

		auto colider = new cyclone::CollisionBox();
		colider->halfSize = cyclone::Vector3{10.5f, 10.5f, 10.5f} *0.5f;
		//colider->transform = obj->transform->getLocalMatrix();
		colider->body = body;

		colider->calculateInternals();
		//colider->transform.data[3]	+= 10.5f * 0.5;
	//colider->transform.data[7]	+= 10.5f * 0.5;
	//colider->transform.data[11] += 10.5f * 0.5;
	//colider->body = b;
	//pc->body = body;
	//pc->collider = std::move(colider);
	//pc->boundingSphere = bs;
	//pc->collisionType = KUMA::ECS::CollisionType::OBB;


		app.getCore().physicsManger->bodies.push_back(body);
		app.getCore().physicsManger->bodiesCollide.push_back(colider);

		auto inp = obj->addComponent<KUMA::ECS::InputComponent>([body, obj](float dt) {
			//if (pc->body)
			obj->transform->setLocalPosition(KUMA::MATHGL::Vector3(body->getPosition().x, body->getPosition().y, body->getPosition().z));
			//obj->transform->setLocalRotation(KUMA::MATHGL::Quaternion(
			//	KUMA::MATHGL::Vector3(TO_DEGREES(body->getRotation().x), TO_DEGREES(body->getRotation().y), TO_DEGREES(body->getRotation().z))));
			obj->transform->setLocalRotation(KUMA::MATHGL::Quaternion(
				body->getOrientation().i, body->getOrientation().j, body->getOrientation().k, body->getOrientation().r));

			//pc->collider->transform = pc->obj.transform->getLocalMatrix();
			//pc->boundingSphere.position = pc->obj.transform->getLocalPosition();
			//if (pc->obj.transform->getLocalPosition().y < -500.0f) {
			//	pc->obj.transform->setLocalPosition(KUMA::MATHGL::Vector3(obj->transform->getLocalPosition().x, 500.0f, obj->transform->getLocalPosition().z));
			//}
			liveTime2 -= dt;
			if (liveTime2 < 0) {
				liveTime2 = 20.0f;
				body->setPosition(7.0f, 155.0f, 0.0f);
				body->setOrientation(1, 0, 0, 0);
				body->setVelocity(0, 0, 0);
				body->setRotation(cyclone::Vector3(0, 0, 0));
			}
		});
	}

	{
		auto& obj = scene->createObject("Box2");

		auto model = obj->addComponent<KUMA::ECS::ModelRenderer>();
		KUMA::RESOURCES::ModelParserFlags flags = KUMA::RESOURCES::ModelParserFlags::TRIANGULATE;
		flags |= KUMA::RESOURCES::ModelParserFlags::GEN_SMOOTH_NORMALS;
		flags |= KUMA::RESOURCES::ModelParserFlags::FLIP_UVS;
		flags |= KUMA::RESOURCES::ModelParserFlags::GEN_UV_COORDS;
		flags |= KUMA::RESOURCES::ModelParserFlags::CALC_TANGENT_SPACE;

		//auto m = KUMA::RESOURCES::ModelLoader::Create("C:\\Projects\\SimpleEngine\\CGCourse\\CGCourse\\Assets\\User\\cottage_fbx.fbx", flags);
		auto m = KUMA::RESOURCES::ModelLoader::Create("C:\\Projects\\SimpleEngine\\CGCourse\\CGCourse\\Assets\\Engine\\Models\\Cube.fbx", flags);
		model->setModel(m);
		model->setFrustumBehaviour(KUMA::ECS::ModelRenderer::EFrustumBehaviour::CULL_MODEL);
		auto bs = KUMA::RENDER::BoundingSphere();
		bs.position = {0.0f, 0.0f, 0.0f};
		bs.radius = 1.0f;
		model->setCustomBoundingSphere(bs);
		obj->transform->setLocalPosition({20.0f, 0.0f, 10.0f});
		obj->transform->setLocalRotation({0.0f, 0.0f, 0.0f, 1.0f});
		//obj.transform->setLocalScale({100.01f, 100.01f, 100.01f});
		obj->transform->setLocalScale({10.5f, 10.5f, 10.5f});

		auto s = KUMA::RESOURCES::ShaderLoader::CreateFromFile("Shaders\\Standard.glsl");

		auto mat = obj->addComponent<KUMA::ECS::MaterialRenderer>();
		auto _m = KUMA::RESOURCES::MaterialLoader::Create("");

		_m->setShader(s);
		_m->setBlendable(false);
		_m->setBackfaceCulling(true);
		_m->setFrontfaceCulling(false);
		_m->setDepthTest(true);
		_m->setDepthWriting(true);
		_m->setColorWriting(true);
		_m->setGPUInstances(1);
		auto& data = _m->getUniformsData();
		data["u_Diffuse"] = KUMA::MATHGL::Vector4{1.0f, 1.0f, 1.0f, 1.0f};

		auto tex1 = KUMA::RESOURCES::TextureLoader().CreateFromFile("textures\\brick_albedo.jpg");
		auto tex2 = KUMA::RESOURCES::TextureLoader().CreateFromFile("textures\\brick_normal.jpg");

		data["u_DiffuseMap"] = tex1;

		data["u_EnableNormalMapping"] = true;
		data["u_HeightScale"] = 0.0f;

		data["u_NormalMap"] = tex2;
		data["u_Shininess"] = 100;
		data["u_Specular"] = KUMA::MATHGL::Vector3{1.0f, 1.0f, 1.0f};
		data["u_TextureOffset"] = KUMA::MATHGL::Vector2f{0.0f, 0.0f};
		data["u_TextureTiling"] = KUMA::MATHGL::Vector2f{1.0f, 1.0f};
		data["u_UseBone"] = false;
		mat->fillWithMaterial(_m);
	}
	{
		auto& obj = scene->createObject("Box3");

		auto model = obj->addComponent<KUMA::ECS::ModelRenderer>();
		KUMA::RESOURCES::ModelParserFlags flags = KUMA::RESOURCES::ModelParserFlags::TRIANGULATE;
		flags |= KUMA::RESOURCES::ModelParserFlags::GEN_SMOOTH_NORMALS;
		flags |= KUMA::RESOURCES::ModelParserFlags::FLIP_UVS;
		flags |= KUMA::RESOURCES::ModelParserFlags::GEN_UV_COORDS;
		flags |= KUMA::RESOURCES::ModelParserFlags::CALC_TANGENT_SPACE;

		//auto m = KUMA::RESOURCES::ModelLoader::Create("C:\\Projects\\SimpleEngine\\CGCourse\\CGCourse\\Assets\\User\\cottage_fbx.fbx", flags);
		auto m = KUMA::RESOURCES::ModelLoader::Create("C:\\Projects\\SimpleEngine\\CGCourse\\CGCourse\\Assets\\Engine\\Models\\Cube.fbx", flags);
		model->setModel(m);
		model->setFrustumBehaviour(KUMA::ECS::ModelRenderer::EFrustumBehaviour::CULL_MODEL);
		auto bs = KUMA::RENDER::BoundingSphere();
		bs.position = {0.0f, 0.0f, 0.0f};
		bs.radius = 1.0f;
		model->setCustomBoundingSphere(bs);
		obj->transform->setLocalPosition({-10.0f, 0.0f, 20.0f});
		obj->transform->setLocalRotation({0.0f, 0.0f, 0.0f, 1.0f});
		//obj.transform->setLocalScale({100.01f, 100.01f, 100.01f});
		obj->transform->setLocalScale({5.25f, 5.25f, 5.25f});

		auto s = KUMA::RESOURCES::ShaderLoader::CreateFromFile("Shaders\\StandardPBR.glsl");

		auto mat = obj->addComponent<KUMA::ECS::MaterialRenderer>();
		auto _m = KUMA::RESOURCES::MaterialLoader::Create("");
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
		data["u_Diffuse"] = KUMA::MATHGL::Vector4{1.0f, 1.0f, 1.0f, 1.0f};

		auto tex1 = KUMA::RESOURCES::TextureLoader().CreateFromFile("textures\\brick_albedo.jpg");
		data["u_DiffuseMap"] = tex1;

		data["u_EnableNormalMapping"] = true;
		data["u_HeightScale"] = 0.0f;

		auto tex2 = KUMA::RESOURCES::TextureLoader().CreateFromFile("textures\\brick_normal.jpg");
		data["u_NormalMap"] = tex2;
		auto tex3 = KUMA::RESOURCES::TextureLoader().CreateFromFile("textures\\noiseTexture.png");
		data["u_Noise"] = tex3;
		data["fogScaleBias"] = KUMA::MATHGL::Vector4(0, -0.06f, 0, 0.0008f);
		data["u_Shininess"] = 100;
		data["u_Specular"] = KUMA::MATHGL::Vector3{1.0f, 1.0f, 1.0f};
		data["u_TextureOffset"] = KUMA::MATHGL::Vector2f{0.0f, 0.0f};
		data["u_TextureTiling"] = KUMA::MATHGL::Vector2f{1.0f, 1.0f};
		data["u_UseBone"] = false;
		mat->fillWithMaterial(_m);
	}


	{
		auto& obj = scene->createObject("BoxDirLight");

		auto model = obj->addComponent<KUMA::ECS::ModelRenderer>();
		KUMA::RESOURCES::ModelParserFlags flags = KUMA::RESOURCES::ModelParserFlags::TRIANGULATE;
		flags |= KUMA::RESOURCES::ModelParserFlags::GEN_SMOOTH_NORMALS;
		flags |= KUMA::RESOURCES::ModelParserFlags::FLIP_UVS;
		flags |= KUMA::RESOURCES::ModelParserFlags::GEN_UV_COORDS;
		flags |= KUMA::RESOURCES::ModelParserFlags::CALC_TANGENT_SPACE;

		//auto m = KUMA::RESOURCES::ModelLoader::Create("C:\\Projects\\SimpleEngine\\CGCourse\\CGCourse\\Assets\\User\\cottage_fbx.fbx", flags);
		auto m = KUMA::RESOURCES::ModelLoader::Create("C:\\Projects\\SimpleEngine\\CGCourse\\CGCourse\\Assets\\Engine\\Models\\Sphere.fbx", flags);
		model->setModel(m);
		model->setFrustumBehaviour(KUMA::ECS::ModelRenderer::EFrustumBehaviour::CULL_MODEL);
		auto bs = KUMA::RENDER::BoundingSphere();
		bs.position = {0.0f, 0.0f, 0.0f};
		bs.radius = 1.0f;
		model->setCustomBoundingSphere(bs);
		obj->transform->setLocalPosition({-20.0f, 35.0f, 10.0f});
		obj->transform->setLocalRotation({0.0f, 0.0f, 0.0f, 1.0f});
		//obj.transform->setLocalScale({100.01f, 100.01f, 100.01f});
		obj->transform->setLocalScale({1.25f, 1.25f, 1.25f});

		auto s = KUMA::RESOURCES::ShaderLoader::CreateFromFile("Shaders\\Standard.glsl");

		auto mat = obj->addComponent<KUMA::ECS::MaterialRenderer>();
		auto _m = KUMA::RESOURCES::MaterialLoader::Create("");

		_m->setShader(s);
		_m->setBlendable(false);
		_m->setBackfaceCulling(true);
		_m->setFrontfaceCulling(false);
		_m->setDepthTest(true);
		_m->setDepthWriting(true);
		_m->setColorWriting(true);
		_m->setGPUInstances(1);
		auto& data = _m->getUniformsData();
		data["u_Diffuse"] = KUMA::MATHGL::Vector4{1.0f, 1.0f, 1.0f, 1.0f};

		auto tex1 = KUMA::RESOURCES::TextureLoader().CreateFromFile("textures\\brick_metalic.jpg");
		data["u_DiffuseMap"] = tex1;

		data["u_EnableNormalMapping"] = true;
		data["u_HeightScale"] = 0.0f;

		auto tex2 = KUMA::RESOURCES::TextureLoader().CreateFromFile("textures\\brick_metalic.jpg");
		data["u_NormalMap"] = tex2;
		data["u_Shininess"] = 100;
		data["u_Specular"] = KUMA::MATHGL::Vector3{1.0f, 1.0f, 1.0f};
		data["u_TextureOffset"] = KUMA::MATHGL::Vector2f{0.0f, 0.0f};
		data["u_TextureTiling"] = KUMA::MATHGL::Vector2f{1.0f, 1.0f};
		data["u_UseBone"] = false;
		mat->fillWithMaterial(_m);



	}

	{
		auto& obj = scene->createObject("Center");

		auto model = obj->addComponent<KUMA::ECS::ModelRenderer>();
		KUMA::RESOURCES::ModelParserFlags flags = KUMA::RESOURCES::ModelParserFlags::TRIANGULATE;
		flags |= KUMA::RESOURCES::ModelParserFlags::GEN_SMOOTH_NORMALS;
		flags |= KUMA::RESOURCES::ModelParserFlags::FLIP_UVS;
		flags |= KUMA::RESOURCES::ModelParserFlags::GEN_UV_COORDS;
		flags |= KUMA::RESOURCES::ModelParserFlags::CALC_TANGENT_SPACE;

		//auto m = KUMA::RESOURCES::ModelLoader::Create("C:\\Projects\\SimpleEngine\\CGCourse\\CGCourse\\Assets\\User\\cottage_fbx.fbx", flags);
		auto m = KUMA::RESOURCES::ModelLoader::Create("C:\\Projects\\SimpleEngine\\CGCourse\\CGCourse\\Assets\\Engine\\Models\\Cube.fbx", flags);
		model->setModel(m);
		model->setFrustumBehaviour(KUMA::ECS::ModelRenderer::EFrustumBehaviour::CULL_MODEL);
		auto bs = KUMA::RENDER::BoundingSphere();
		bs.position = {0.0f, 0.0f, 0.0f};
		bs.radius = 1.0f;
		model->setCustomBoundingSphere(bs);
		obj->transform->setLocalPosition({0.0f, 0.0f, 0.0f});
		obj->transform->setLocalRotation({0.0f, 0.0f, 0.0f, 1.0f});
		//obj.transform->setLocalScale({100.01f, 100.01f, 100.01f});
		obj->transform->setLocalScale({1.25f, 1.25f, 1.25f});

		auto s = KUMA::RESOURCES::ShaderLoader::CreateFromFile("Shaders\\Standard.glsl");

		auto mat = obj->addComponent<KUMA::ECS::MaterialRenderer>();
		auto _m = KUMA::RESOURCES::MaterialLoader::Create("");

		_m->setShader(s);
		_m->setBlendable(false);
		_m->setBackfaceCulling(true);
		_m->setFrontfaceCulling(false);
		_m->setDepthTest(true);
		_m->setDepthWriting(true);
		_m->setColorWriting(true);
		_m->setGPUInstances(1);
		auto& data = _m->getUniformsData();
		data["u_Diffuse"] = KUMA::MATHGL::Vector4{1.0f, 1.0f, 1.0f, 1.0f};

		auto tex1 = KUMA::RESOURCES::TextureLoader().CreateFromFile("textures\\brick_albedo.jpg");
		data["u_DiffuseMap"] = tex1;

		data["u_EnableNormalMapping"] = 1;
		data["u_HeightScale"] = 0;

		auto tex2 = KUMA::RESOURCES::TextureLoader().CreateFromFile("textures\\brick_normal.jpg");
		data["u_NormalMap"] = tex2;
		auto tex3 = KUMA::RESOURCES::TextureLoader().CreateFromFile("textures\\noiseTexture.png");
		data["u_Noise"] = tex3;
		data["fogScaleBias"] = KUMA::MATHGL::Vector4(0, -0.06f, 0, 0.0008f);
		data["u_Shininess"] = 100;
		data["u_Specular"] = KUMA::MATHGL::Vector3{1.0f, 1.0f, 1.0f};
		data["u_TextureOffset"] = KUMA::MATHGL::Vector2f{0.0f, 0.0f};
		data["u_TextureTiling"] = KUMA::MATHGL::Vector2f{1.0f, 1.0f};
		data["u_UseBone"] = false;
		mat->fillWithMaterial(_m);
	}
	/*{
		auto& obj = scene->createObject("Start");

		auto model = obj.addComponent<KUMA::ECS::ModelRenderer>();
		KUMA::RESOURCES::ModelParserFlags flags = KUMA::RESOURCES::ModelParserFlags::TRIANGULATE;
		flags |= KUMA::RESOURCES::ModelParserFlags::GEN_SMOOTH_NORMALS;
		flags |= KUMA::RESOURCES::ModelParserFlags::FLIP_UVS;
		flags |= KUMA::RESOURCES::ModelParserFlags::GEN_UV_COORDS;

		//auto m = KUMA::RESOURCES::ModelLoader::Create("C:\\Projects\\SimpleEngine\\CGCourse\\CGCourse\\Assets\\User\\cottage_fbx.fbx", flags);
		auto m = KUMA::RESOURCES::ModelLoader::Create("C:\\Projects\\SimpleEngine\\CGCourse\\CGCourse\\Assets\\Engine\\Models\\Cube.fbx", flags);
		model->setModel(m);
		model->setFrustumBehaviour(KUMA::ECS::ModelRenderer::EFrustumBehaviour::CULL_MODEL);
		auto bs = KUMA::RENDER::BoundingSphere();
		bs.position = {0.0f, 0.0f, 0.0f};
		bs.radius = 1.0f;
		model->setCustomBoundingSphere(bs);
		obj.transform->setLocalPosition({-20.0f, 40.0f, 10.0f});
		obj.transform->setLocalRotation({0.0f, 0.0f, 0.0f, 1.0f});
		//obj.transform->setLocalScale({100.01f, 100.01f, 100.01f});
		obj.transform->setLocalScale({1.25f, 1.25f, 1.25f});

		auto s = KUMA::RESOURCES::ShaderLoader::Create("C:\\Projects\\SimpleEngine\\CGCourse\\CGCourse\\Assets\\Engine\\Shaders\\Standard.glsl");

		auto mat = obj.addComponent<KUMA::ECS::MaterialRenderer>();
		auto _m = KUMA::RESOURCES::MaterialLoader::Create("");

		_m->setShader(s);
		_m->setBlendable(false);
		_m->setBackfaceCulling(true);
		_m->setFrontfaceCulling(false);
		_m->setDepthTest(true);
		_m->setDepthWriting(true);
		_m->setColorWriting(true);
		_m->setGPUInstances(1);
		auto& data = _m->getdata["sData();
		data["u_Diffuse"] = KUMA::MATHGL::Vector4{1.0f, 1.0f, 1.0f, 1.0f};

		auto tex1 = KUMA::RESOURCES::TextureLoader().createResource("textures\\brick_albedo.jpg");
		data["u_DiffuseMap"] = tex1;

		data["u_EnableNormalMapping"] = 1;
		data["u_HeightScale"] = 0;

		auto tex2 = KUMA::RESOURCES::TextureLoader().createResource("textures\\brick_normal.jpg");
		data["u_NormalMap"] = tex2;
		data["u_Shininess"] = 100;
		data["u_Specular"] = KUMA::MATHGL::Vector3{1.0f, 1.0f, 1.0f};
		data["u_TextureOffset"] = KUMA::MATHGL::Vector2f{0.0f, 0.0f};
		data["u_TextureTiling"] = KUMA::MATHGL::Vector2f{1.0f, 1.0f};
		data["u_UseBone"] = false;
		mat->fillWithMaterial(_m);
	}*/

	{
		auto& obj = scene->createObject("Smoke");

		auto model = obj->addComponent<KUMA::ECS::ModelRenderer>();
		KUMA::RESOURCES::ModelParserFlags flags = KUMA::RESOURCES::ModelParserFlags::TRIANGULATE;
		flags |= KUMA::RESOURCES::ModelParserFlags::GEN_SMOOTH_NORMALS;
		//flags |= KUMA::RESOURCES::ModelParserFlags::OPTIMIZE_MESHES;
		//flags |= KUMA::RESOURCES::ModelParserFlags::OPTIMIZE_GRAPH;
		//flags |= KUMA::RESOURCES::ModelParserFlags::FIND_INSTANCES;
		//flags |= KUMA::RESOURCES::ModelParserFlags::CALC_TANGENT_SPACE;
		flags |= KUMA::RESOURCES::ModelParserFlags::FLIP_UVS;
		flags |= KUMA::RESOURCES::ModelParserFlags::LIMIT_BONE_WEIGHTS;
		//flags |= KUMA::RESOURCES::ModelParserFlags::JOIN_IDENTICAL_VERTICES;
		//flags |= KUMA::RESOURCES::ModelParserFlags::DEBONE;
		//flags |= KUMA::RESOURCES::ModelParserFlags::FIND_INVALID_DATA;
		//flags |= KUMA::RESOURCES::ModelParserFlags::IMPROVE_CACHE_LOCALITY;
		flags |= KUMA::RESOURCES::ModelParserFlags::GEN_UV_COORDS;
		//flags |= KUMA::RESOURCES::ModelParserFlags::PRE_TRANSFORM_VERTICES;
		auto m = KUMA::RESOURCES::ModelLoader::Create("C:\\Projects\\SimpleEngine\\CGCourse\\CGCourse\\Assets\\Engine\\Models\\Sphere.fbx", flags);

		model->setModel(m);
		model->setFrustumBehaviour(KUMA::ECS::ModelRenderer::EFrustumBehaviour::CULL_MODEL);
		auto bs = KUMA::RENDER::BoundingSphere();
		bs.position = {0.0f, 0.0f, 0.0f};
		bs.radius = 1.0f;
		model->setCustomBoundingSphere(bs);
		obj->transform->setLocalPosition({-128.0f, 30.0f, 0.0f});
		obj->transform->setLocalRotation({0.0f, 0.0f, 0.0f, 1.0f});
		obj->transform->setLocalScale({1.01f, 1.01f, 1.01f});

		auto s = KUMA::RESOURCES::ShaderLoader::CreateFromFile("Shaders\\Smoke.glsl");

		auto mat = obj->addComponent<KUMA::ECS::MaterialRenderer>();
		auto _m = KUMA::RESOURCES::MaterialLoader::Create("");

		_m->setShader(s);
		_m->setBlendable(true);
		_m->setBackfaceCulling(true);
		_m->setFrontfaceCulling(false);
		_m->setDepthTest(true);
		_m->setDepthWriting(false);
		_m->setColorWriting(true);
		_m->setGPUInstances(100);
		auto& data = _m->getUniformsData();
		data["u_Diffuse"] = KUMA::MATHGL::Vector4{0.7f, 0.7f, 0.7f, 0.5f};

		auto tex1 = KUMA::RESOURCES::TextureLoader().CreateFromFile("textures\\smoke-17.png");
		data["u_DiffuseMap"] = tex1;

		data["u_CosinAmplitude"] = KUMA::MATHGL::Vector3{0.5f, 1.0f, 0.9f};
		data["u_CosinFrequency"] = KUMA::MATHGL::Vector3{3.5f, 1.0f, 0.5f};

		data["u_MaxHeight"] = 10;
		data["u_Spacing"] = KUMA::MATHGL::Vector3{0.3f, 0.0f, 0.5f};
		data["u_TextureOffset"] = KUMA::MATHGL::Vector2f{1.0f, 0.0f};
		data["u_TextureTiling"] = KUMA::MATHGL::Vector2f{1.0f, 1.0f};
		data["u_Velocity"] = KUMA::MATHGL::Vector3{0.0f, 0.5f, 0.0f};
		data["u_UseBone"] = false;
		data["castShadow"] = false;


		mat->fillWithMaterial(_m);
	}

	{
		auto& obj = scene->createObject("Water");

		auto model = obj->addComponent<KUMA::ECS::ModelRenderer>();
		KUMA::RESOURCES::ModelParserFlags flags = KUMA::RESOURCES::ModelParserFlags::TRIANGULATE;
		flags |= KUMA::RESOURCES::ModelParserFlags::GEN_SMOOTH_NORMALS;
		flags |= KUMA::RESOURCES::ModelParserFlags::FLIP_UVS;
		flags |= KUMA::RESOURCES::ModelParserFlags::GEN_UV_COORDS;
		flags |= KUMA::RESOURCES::ModelParserFlags::CALC_TANGENT_SPACE;
		auto m = KUMA::RESOURCES::ModelLoader::Create("C:\\Projects\\SimpleEngine\\CGCourse\\CGCourse\\Assets\\Engine\\Models\\Sphere.fbx", flags);

		model->setModel(m);
		model->setFrustumBehaviour(KUMA::ECS::ModelRenderer::EFrustumBehaviour::CULL_MODEL);
		auto bs = KUMA::RENDER::BoundingSphere();
		bs.position = {0.0f, 0.0f, 0.0f};
		bs.radius = 1.0f;
		model->setCustomBoundingSphere(bs);
		obj->transform->setLocalPosition({-100.0f, 1.0f, 10.0f});
		obj->transform->setLocalRotation({0.0f, 0.0f, 0.0f, 1.0f});
		obj->transform->setLocalScale({30.0f, 30.0f, 30.0f});

		auto s = KUMA::RESOURCES::ShaderLoader::CreateFromFile("Shaders\\Fluid.glsl");

		auto mat = obj->addComponent<KUMA::ECS::MaterialRenderer>();
		auto _m = KUMA::RESOURCES::MaterialLoader::Create("");

		_m->setShader(s);
		_m->setBlendable(true);
		_m->setBackfaceCulling(true);
		_m->setFrontfaceCulling(false);
		_m->setDepthTest(true);
		_m->setDepthWriting(true);
		_m->setColorWriting(true);
		_m->setGPUInstances(1);
		auto& data = _m->getUniformsData();
		data["u_Amplitude"] = 0.1f;
		data["u_Diffuse"] = KUMA::MATHGL::Vector4{0.1f, 0.6f, 1.0f, 0.9f};

		auto tex1 = KUMA::RESOURCES::TextureLoader().CreateFromFile("Water_001_COLOR.jpg");
		data["u_DiffuseMap"] = tex1;
		//auto tex3 = KUMA::RESOURCES::TextureLoader().createResource("Water_001_SPEC.jpg");
		//data["u_SpecularMap"] = tex3;
		//auto tex4 = KUMA::RESOURCES::TextureLoader().createResource("Water_001_DISP.png");
		//data["u_HeightMap"] = tex4;
		data["u_Disparity"] = 30;
		data["u_EnableNormalMapping"] = true;
		data["u_Frequency"] = 20.0f;
		data["u_HeightScale"] = 0.0f;

		auto tex2 = KUMA::RESOURCES::TextureLoader().CreateFromFile("textures\\Water_001_NORM.jpg");
		data["u_NormalMap"] = tex2;
		data["u_Shininess"] = 100;
		data["u_Specular"] = KUMA::MATHGL::Vector3{1.0f, 1.0f, 1.0f};
		data["u_TextureTiling"] = KUMA::MATHGL::Vector2f{3.0f, 3.0f};

		data["u_UseBone"] = false;
		data["castShadow"] = false;


		mat->fillWithMaterial(_m);
	}

	{
		auto& obj = scene->createObject("Grass");

		auto audio = obj->addComponent<KUMA::ECS::AudioComponent>(KUMA::AUDIO::Sound3d{
			"C:\\Projects\\SimpleEngine\\CGCourse\\CGCourse\\Assets\\Engine\\Audio\\test.mp3",
			KUMA::MATHGL::Vector3(-300.0f, 1.0f, 10.0f)});

		auto model = obj->addComponent<KUMA::ECS::ModelRenderer>();
		KUMA::RESOURCES::ModelParserFlags flags = KUMA::RESOURCES::ModelParserFlags::TRIANGULATE;
		flags |= KUMA::RESOURCES::ModelParserFlags::GEN_SMOOTH_NORMALS;
		flags |= KUMA::RESOURCES::ModelParserFlags::FLIP_UVS;
		flags |= KUMA::RESOURCES::ModelParserFlags::GEN_UV_COORDS;
		flags |= KUMA::RESOURCES::ModelParserFlags::CALC_TANGENT_SPACE;
		auto m = KUMA::RESOURCES::ModelLoader::Create("C:\\Projects\\SimpleEngine\\CGCourse\\CGCourse\\Assets\\Engine\\Models\\Cube.fbx", flags);

		model->setModel(m);
		model->setFrustumBehaviour(KUMA::ECS::ModelRenderer::EFrustumBehaviour::CULL_MODEL);
		auto bs = KUMA::RENDER::BoundingSphere();
		bs.position = {0.0f, 0.0f, 0.0f};
		bs.radius = 1.0f;
		model->setCustomBoundingSphere(bs);
		obj->transform->setLocalPosition({-300.0f, 1.0f, 10.0f});
		obj->transform->setLocalRotation({0.0f, 0.0f, 0.0f, 1.0f});
		obj->transform->setLocalScale({30.0f, 30.0f, 30.0f});

		auto s = KUMA::RESOURCES::ShaderLoader::CreateFromFile("Shaders\\grass.glsl");

		auto mat = obj->addComponent<KUMA::ECS::MaterialRenderer>();
		auto _m = KUMA::RESOURCES::MaterialLoader::Create("");

		_m->setShader(s);
		_m->setBlendable(false);
		_m->setBackfaceCulling(false);
		_m->setFrontfaceCulling(false);
		_m->setDepthTest(true);
		_m->setDepthWriting(true);
		_m->setColorWriting(true);
		_m->setGPUInstances(1);
		auto& data = _m->getUniformsData();

		auto tex1 = KUMA::RESOURCES::TextureLoader().CreateFromFile("Textures\\grassPack.png");
		data["gSampler"] = tex1;
		data["fAlphaTest"] = 0.25f;
		data["fAlphaMultiplier"] = 1.5f;
		data["vColor"] = KUMA::MATHGL::Vector4(1.0f);



		mat->fillWithMaterial(_m);
	}

	{
		auto& obj = scene->createObject("Terrain");

		auto model = obj->addComponent<KUMA::ECS::ModelRenderer>();

		auto m = KUMA::MeshGenerator::createTerrainFromHeightmap("C:\\Projects\\SimpleEngine\\CGCourse\\CGCourse\\Assets\\Engine\\Textures\\tut017.png");
		model->setModel(m);
		model->setFrustumBehaviour(KUMA::ECS::ModelRenderer::EFrustumBehaviour::CULL_MODEL);
		auto bs = KUMA::RENDER::BoundingSphere();
		bs.position = {0.0f, 0.0f, 0.0f};
		bs.radius = 1.0f;
		model->setCustomBoundingSphere(bs);
		obj->transform->setLocalPosition({-100.0f, 50.0f, 10.0f});
		obj->transform->setLocalRotation({0.0f, 0.0f, 0.0f, 1.0f});
		//obj.transform->setLocalScale({100.01f, 100.01f, 100.01f});
		obj->transform->setLocalScale({30.0f, 30.0f, 30.0f});

		auto s = KUMA::RESOURCES::ShaderLoader::CreateFromFile("Shaders\\Terrain.glsl");

		auto mat = obj->addComponent<KUMA::ECS::MaterialRenderer>();
		auto _m = KUMA::RESOURCES::MaterialLoader::Create("");
		_m->setShader(s);
		_m->setBlendable(false);
		_m->setBackfaceCulling(true);
		_m->setFrontfaceCulling(false);
		_m->setDepthTest(true);
		_m->setDepthWriting(true);
		_m->setColorWriting(true);
		_m->setGPUInstances(1);
		auto& data = _m->getUniformsData();
		data["u_Diffuse"] = KUMA::MATHGL::Vector4{1.0f, 1.0f, 1.0f, 1.0f};

		auto tex1 = KUMA::RESOURCES::TextureLoader().CreateFromFile("textures\\terrain.png");
		data["u_DiffuseMap"] = tex1;
		data["u_EnableNormalMapping"] = false;
		data["u_HeightScale"] = 0.0f;
		data["fogScaleBias"] = KUMA::MATHGL::Vector4(0, -0.06f, 0, 0.0008f);
		data["u_Shininess"] = 100;
		data["u_Specular"] = KUMA::MATHGL::Vector3{1.0f, 1.0f, 1.0f};
		data["u_TextureOffset"] = KUMA::MATHGL::Vector2f{0.0f, 0.0f};
		data["u_TextureTiling"] = KUMA::MATHGL::Vector2f{1.0f, 1.0f};


		data["u_Levels"] = KUMA::MATHGL::Vector4(0.2f, 0.3f, 0.55, 0.7);

		mat->fillWithMaterial(_m);
	}

	{
		auto& obj = scene->createObject("Clouds");

		auto model = obj->addComponent<KUMA::ECS::ModelRenderer>();
		KUMA::RESOURCES::ModelParserFlags flags = KUMA::RESOURCES::ModelParserFlags::TRIANGULATE;
		flags |= KUMA::RESOURCES::ModelParserFlags::GEN_SMOOTH_NORMALS;
		flags |= KUMA::RESOURCES::ModelParserFlags::FLIP_UVS;
		flags |= KUMA::RESOURCES::ModelParserFlags::GEN_UV_COORDS;
		flags |= KUMA::RESOURCES::ModelParserFlags::CALC_TANGENT_SPACE;
		auto m = KUMA::RESOURCES::ModelLoader::Create("C:\\Projects\\SimpleEngine\\CGCourse\\CGCourse\\Assets\\Engine\\Models\\Cube.fbx", flags);

		model->setModel(m);
		model->setFrustumBehaviour(KUMA::ECS::ModelRenderer::EFrustumBehaviour::CULL_MODEL);
		auto bs = KUMA::RENDER::BoundingSphere();
		bs.position = {0.0f, 0.0f, 0.0f};
		bs.radius = 1.0f;
		model->setCustomBoundingSphere(bs);
		obj->transform->setLocalPosition({-300.0f, 50.0f, -300.0f});
		obj->transform->setLocalRotation({0.0f, 0.0f, 0.0f, 1.0f});
		//obj.transform->setLocalScale({100.01f, 100.01f, 100.01f});
		obj->transform->setLocalScale({300.0f, 300.0f, 300.0f});

		auto s = KUMA::RESOURCES::ShaderLoader::CreateFromFile("Shaders\\Clouds.glsl");

		auto mat = obj->addComponent<KUMA::ECS::MaterialRenderer>();
		auto _m = KUMA::RESOURCES::MaterialLoader::Create("");
		_m->setShader(s);
		_m->setBlendable(false);
		_m->setBackfaceCulling(false);
		_m->setFrontfaceCulling(false);
		_m->setDepthTest(true);
		_m->setDepthWriting(true);
		_m->setColorWriting(true);
		_m->setGPUInstances(1);
		auto& data = _m->getUniformsData();


		auto inp = obj->addComponent<KUMA::ECS::InputComponent>([&app, &data, s](float dt) {
			//auto currentScene = app.getCore().sceneManager->getCurrentScene();
			//auto mainCameraComponent = currentScene->findMainCamera();
			//data["camUp"] = mainCameraComponent->obj.transform->getLocalRotation() * KUMA::MATHGL::Vector3::Up;
			//data["camRight"] = mainCameraComponent->obj.transform->getLocalRotation() * KUMA::MATHGL::Vector3::Right;
			s->bind();
			//auto gp = app.getCore().renderer->gPosition;
			//glActiveTexture(GL_TEXTURE6);
			//glBindTexture(GL_TEXTURE_2D, gp);
			auto mpos = app.getCore().inputManager->getMousePosition();
			s->setUniformVec2("iMouse", KUMA::MATHGL::Vector2f(mpos.x, mpos.y));
			s->unbind();
		});

		//data["iResolution"] = KUMA::MATHGL::Vector2f(800, 600);


		// Textures //

		auto tex1 = KUMA::RESOURCES::TextureLoader().CreateFromFile("textures\\clouds\\0.png");
		auto tex2 = KUMA::RESOURCES::TextureLoader().CreateFromFile("textures\\clouds\\1.png");
		auto tex3 = KUMA::RESOURCES::TextureLoader().CreateFromFile("textures\\clouds\\2.png");
		//auto tex4 = KUMA::RESOURCES::TextureLoader().createResource("textures\\clouds\\HDR_L_0.png");

		//auto tex5 = KUMA::RESOURCES::TextureLoader().createResource("textures\\clouds\\HDR_L_0.png");
		//auto tex6 = KUMA::RESOURCES::TextureLoader().createResource("textures\\clouds\\HDR_L_0.png");
		//auto tex7 = KUMA::RESOURCES::TextureLoader().createResource("textures\\clouds\\HDR_L_0.png");

		data["iChannel0"] = tex1;
		data["iChannel1"] = tex2;
		data["iChannel2"] = tex3;
		//data["blueNoise"] = tex4;
		//data["groundTex"] = tex5;
		//data["celestialTex"] = tex6;
		//data["cameraDepthTexture"] = tex7;


		mat->fillWithMaterial(_m);
	}

	app.run();
}
