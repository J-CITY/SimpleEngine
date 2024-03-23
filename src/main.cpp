//#include "core/engine.hpp"
//#include "core/sdl-wrapper.hpp"
#include "coreModule/core/app.h"

#include <iostream>
#include <map>

#include <unordered_map>
#include <variant>

#include "coreModule/platform.hpp"
#include "mathModule/math.h"
#include "renderModule/backends/gl/materialGl.h"
#include "renderModule/backends/gl/modelGl.h"
#include "resourceModule/parser/assimpParser.h"
#include "sceneModule/sceneManager.h"
#include "utilsModule/reflection/reflection.h"
using namespace std;
#include <serdepp/serializer.hpp>
#include <serdepp/adaptor/nlohmann_json.hpp>
#include <serdepp/attribute/default.hpp>

struct Component
{
	int c = 0;
	template<class Context>
	constexpr static auto serde(Context& context, Component& value) {
		using Self = Component;
		using namespace serde::attribute;
		serde::serde_struct(context, value)
			.field(&Self::c, "c", default_{ 0 });
	}
};
struct ComponentA:public Component
{
	int a = 0;
};
struct ComponentB :public Component
{
	int a = 0;
	template<class Context>
	constexpr static auto serde(Context& context, ComponentB& value) {
		using Self = ComponentB;
		using namespace serde::attribute;
		serde::serde_struct(context, value)
			.field(&Self::a, "a", default_{ 0 });
	}
};
struct ComponentC :public Component
{
	int a = 0;
};

struct Comtainer
{
	unordered_map<int, std::vector<std::shared_ptr<Component>>> components;
};

struct Text
{
	std::string text;

	template<class Context>
	constexpr static auto serde(Context& context, Text& value) {
		using Self = Text;
		using namespace serde::attribute;
		serde::serde_struct(context, value)
			.field(&Self::text, "text", default_{ "" });
	}
};
struct Obj
{
	int id = 0;
	void setId(int i) { id = i;}
	int getId() { return id; }
	
	std::variant<int, std::string, float, ComponentB, std::shared_ptr<Text>> variantData;
	std::string strData;
	std::vector<int> vecData;
	std::unordered_map<int, std::string> mapData;


	static auto GetMembers() {
		return std::tuple{
			IKIGAI::UTILS::MakeMemberInfo("id", &Obj::getId, &Obj::setId),
			IKIGAI::UTILS::MakeMemberInfo("mapData", &Obj::mapData)
		};
	}

	template<class Context>
	constexpr static auto serde(Context& context, Obj& value) {
		using Self = Obj;
		using namespace serde::attribute;
		serde::serde_struct(context, value)
			.field(&Self::id, "id", default_{ 0 })
			.field(&Self::vecData, "vecData")
			.field(&Self::variantData, "variantData");
	}

};

Comtainer comtainer;


template <class T, class U>
std::string value_type(std::unordered_map<T, U> const& m) {
	return typeid(U).name();
}
#include <serdepp/adaptor/reflection.hpp>



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
	IKIGAI::CORE::App::hInstance = hInstance;
	IKIGAI::CORE::App app;
	app.getCore()->sceneManager->loadFromFile("scenes/sceneDx12.json");

	app.run();
	return 0;
}
#endif

#ifndef OCULUS
#if defined OPENGL_BACKEND || defined VULKAN_BACKEND
int main(int, char* []) {
	//std::cout << "FIRST" << std::endl;
	//Obj obj;
	//obj.id = 42;
	//obj.mapData = {
	//	{1, "1"},
	//	{2, "2"},
	//	{3, "3"},
	//};
	//obj.vecData = { 1,2,3 };
	//obj.strData = "qwerty";
	//obj.variantData = std::make_shared<Text>();
	//
	//auto meta = obj.GetMembers();
	//
	//auto a = &Obj::getId;

	//cout << std::get<0>(meta).getName() << std::get<0>(meta).get(obj) << endl;
	//cout << typeid(std::get<1>(meta).get(obj)).name() << endl;


	//auto myTuple = tuple{
	//	IKIGAI::UTILS::MetaPair {"a", 3.14},
	//	IKIGAI::UTILS::MetaPair {"b", "!"}
	//};

	//auto e = std::get<0>(myTuple);
//
	////cout << e.key << " " << e.value << endl;
//
	////cout << serde::serialize<nlohmann::json>(obj) << endl;
//
	//IKIGAI::MATH::Vector4<float> vec(1,2,3,4);
	//IKIGAI::MATH::Vector4<float> v1 = vec.xxxx;
	////std::cout << v1.x << v1.y << v1.z << v1.w << std::endl;
	//IKIGAI::MATH::Vector4<float> v2 = vec.abgr;
	//float v3 = vec.r;
	//IKIGAI::MATH::Vector2<float> v4 = vec.rg;
	//IKIGAI::MATH::Vector3<float> v5 = vec.grb;

	IKIGAI::CORE::App app;
	//app.getCore()->sceneManager->loadFromFile("assets/scenes/scene.json");
	app.getCore()->sceneManager->loadFromFile("assets/scenes/scene.json");
	//app.getCore()->sceneManager->loadFromFile("assets/scenes/sceneDx12.json");
	/*
	auto& scene = app.getCore()->sceneManager->getCurrentScene();
	
	{
		auto obj = scene.createObject("Camera");
		auto cam = obj->addComponent<IKIGAI::ECS::CameraComponent>();
		cam->setFov(60.0f);
		cam->setSize(5.0f);
		cam->setNear(0.1f);
		cam->setFar(1000.0f);
		cam->setFrustumGeometryCulling(false);
		cam->setFrustumLightCulling(false);
		cam->setProjectionMode(IKIGAI::RENDER::Camera::ProjectionMode::PERSPECTIVE);
		obj->getTransform()->setLocalPosition({ 0.f, 0.0f, -2.0f });
		obj->getTransform()->setLocalRotation({ 0.0f, 0.0f, 0.0f, 1.0f });
		obj->getTransform()->setLocalScale({ 1.0f, 1.0f, 1.0f });
		//obj->addComponent<IKIGAI::ECS::ScriptComponent>("Controller");
	}
	{
		auto obj = scene.createObject("Box");
	
		auto modelComponent = obj->addComponent<IKIGAI::ECS::ModelRenderer>();
		std::string objFilename = IKIGAI::UTILS::GetRealPath("assets/models/crate.obj");
		auto model = std::make_shared<IKIGAI::RENDER::ModelGl>(objFilename);
		IKIGAI::RESOURCES::AssimpParser parser;
		IKIGAI::RESOURCES::ModelParserFlags flags = IKIGAI::RESOURCES::ModelParserFlags::TRIANGULATE;
		flags |= IKIGAI::RESOURCES::ModelParserFlags::GEN_SMOOTH_NORMALS;
		flags |= IKIGAI::RESOURCES::ModelParserFlags::FLIP_UVS;
		flags |= IKIGAI::RESOURCES::ModelParserFlags::GEN_UV_COORDS;
		flags |= IKIGAI::RESOURCES::ModelParserFlags::CALC_TANGENT_SPACE;
		flags |= IKIGAI::RESOURCES::ModelParserFlags::JOIN_IDENTICAL_VERTICES;
		flags |= IKIGAI::RESOURCES::ModelParserFlags::IMPROVE_CACHE_LOCALITY;
		flags |= IKIGAI::RESOURCES::ModelParserFlags::FIND_INVALID_DATA;
		flags |= IKIGAI::RESOURCES::ModelParserFlags::FIND_INSTANCES;
		flags |= IKIGAI::RESOURCES::ModelParserFlags::OPTIMIZE_MESHES;
		flags |= IKIGAI::RESOURCES::ModelParserFlags::OPTIMIZE_GRAPH;
		flags |= IKIGAI::RESOURCES::ModelParserFlags::DEBONE;
		parser.LoadModel(objFilename, model, flags);
		modelComponent->setModel(model);

		auto bs = IKIGAI::RENDER::BoundingSphere();
		bs.position = { 0.0f, 0.0f, 0.0f };
		bs.radius = 1.0f;
		modelComponent->setCustomBoundingSphere(bs);

		obj->getTransform()->setLocalPosition({ 0.0f, 0.0f, 0.0f });
		obj->getTransform()->setLocalRotation(IKIGAI::MATH::QuaternionF({ 0.0f, 30.0f, 0.0f }));
		obj->getTransform()->setLocalScale({ 1.f, 1.f, 1.f });

		std::string vshaderFilename = IKIGAI::UTILS::GetRealPath("assets/shaders/opengl/default.vert");
		std::string fshaderFilename = IKIGAI::UTILS::GetRealPath("assets/shaders/opengl/default.frag");
		auto s = std::make_shared<IKIGAI::RENDER::ShaderGl>(vshaderFilename, fshaderFilename);

		auto mat = obj->addComponent<IKIGAI::ECS::MaterialRenderer>();
		auto _m = std::make_shared<IKIGAI::RENDER::MaterialGl>();
		_m->setShader(s);
		_m->mBlendable = (false);
		_m->mBackfaceCulling = (false);
		_m->mFrontfaceCulling = (false);
		_m->mDepthTest = (true);
		_m->mDepthWriting = (true);
		_m->mColorWriting = (true);
		_m->mGpuInstances = (1);

		std::string texFilename = IKIGAI::UTILS::GetRealPath("assets/textures/crate.png");

		auto tex = IKIGAI::RENDER::TextureGl::Create(texFilename, true);
		_m->setShader(s);
		_m->mUniformData["u_sampler"] = tex;
		mat->fillWithMaterial(_m);
	}
	
	nlohmann::json v = R"(
	{
	"Objects": [
		{
			"Name": "Camera",
			"Id": 1,
			"Components": [
				{
					"TransformComponentType": "class IKIGAI::ECS::TransformComponent",
					"LocalPosition": {
						"X": 0,
						"Y": 0,
						"Z": -2
					},
					"LocalScale": {
						"X": 1,
						"Y": 1,
						"Z": 1
					},
					"LocalRotation": {
						"X": 0,
						"Y": 0,
						"Z": 0
					}
				},
				{
					"CameraComponentType": "class IKIGAI::ECS::CameraComponent",
					"Far": 1000.0,
					"Fov": 60.0,
					"Near": 0.1,
					"Mode": "PERSPECTIVE",
					"Size": 5.0,
					"FrustumCulling": false,
					"FrustumLight": false,
					"FrustumCullingBVH": false
				},
				{
					"AudioListenerComponentType": "class IKIGAI::ECS::AudioListenerComponent"
				}//,
				//{
				//	"ScriptComponentType": "class IKIGAI::ECS::ScriptComponent",
				//	"Path": "a/b/c.lua"
				//}
			]
		},
		{
			"Name": "lightDir",
			"Id": 100,
			"Components": [
				{
					"TransformComponentType": "class IKIGAI::ECS::TransformComponent",
					"LocalPosition": {
						"X": 0,
						"Y": 10,
						"Z": -10
					},
					"LocalScale": {
						"X": 1,
						"Y": 1,
						"Z": 1
					},
					"LocalRotation": {
						"X": 0,
						"Y": 0,
						"Z": 0
					}
				},
				{
					"DirectionalLightType": "class IKIGAI::ECS::DirectionalLight",
					"Color": {
						"X": 1.0,
						"Y": 1.0,
						"Z": 1.0
					},
					"Distance": 1000.0,
					"Intensity": 1.0
				}
			]
		},
		{
			"Name": "lightAmbient",
			"Id": 101,
			"Components": [
				{
					"TransformComponentType": "class IKIGAI::ECS::TransformComponent",
					"LocalPosition": {
						"X": 0,
						"Y": 0,
						"Z": 0
					},
					"LocalScale": {
						"X": 1,
						"Y": 1,
						"Z": 1
					},
					"LocalRotation": {
						"X": 0,
						"Y": 0,
						"Z": 0
					}
				},
				{
					"AmbientLightType": "class IKIGAI::ECS::AmbientLight",
					"Color": {
						"X": 1.0,
						"Y": 1.0,
						"Z": 1.0
					},
					"Size": {
						"X": 1.0,
						"Y": 1.0,
						"Z": 1.0
					},
					"Intensity": 0.3
				}
			]
		},
		{
			"Name": "box1",
			"Id": 2,
			"Components": [
				{
					"TransformComponentType": "class IKIGAI::ECS::TransformComponent",
					"LocalPosition": {
						"X": 0,
						"Y": 0,
						"Z": 0
					},
					"LocalScale": {
						"X": 1,
						"Y": 1,
						"Z": 1
					},
					"LocalRotation": {
						"X": 0,
						"Y": 30,
						"Z": 0
					}
				},
				{
					"ModelRendererType": "class IKIGAI::ECS::ModelRenderer",
					"Path": "assets/models/crate.obj"
				},
				{
					"MaterialRendererType": "class IKIGAI::ECS::MaterialRenderer",
					"MaterialNames": ["default"],
					"Materials": ["assets/materials/box.json"]
				},
				{
					"AudioComponentType": "class IKIGAI::ECS::AudioComponent",
					"AudioPath": "audios/test.sound"
				}
			]
		}
	]
	})"_json;

	auto t = serde::deserialize<IKIGAI::SCENE_SYSTEM::Scene::Descriptor>(v);

	auto v_to_json = serde::serialize<nlohmann::json>(t);

	auto str = v_to_json.dump(4);
	*/

	std::cout << "START" << std::endl;
	app.run();
	return 0;
}
#endif
#endif
