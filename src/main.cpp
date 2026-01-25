// #include "core/engine.hpp"
// #include "core/sdl-wrapper.hpp"
// #include "coreModule/core/app.h"
//
// #include <iostream>
// #include <map>
//
// #include <unordered_map>
// #include <variant>
//
// #include "coreModule/platform.hpp"
// #include "mathModule/math.h"
// #include "renderModule/backends/gl/materialGl.h"
// #include "renderModule/backends/gl/modelGl.h"
// #include "resourceModule/parser/assimpParser.h"
// #include "sceneModule/sceneManager.h"
// #include "utilsModule/reflection/reflection.h"
// using namespace std;
// #include <serdepp/adaptor/nlohmann_json.hpp>
// #include <serdepp/attribute/default.hpp>
// #include <serdepp/serializer.hpp>
//
// struct Component {
//  int c = 0;
//  template <class Context>
//  constexpr static auto serde(Context &context, Component &value) {
//    using Self = Component;
//    using namespace serde::attribute;
//    serde::serde_struct(context, value).field(&Self::c, "c", default_{0});
//  }
//};
// struct ComponentA : public Component {
//  int a = 0;
//};
// struct ComponentB : public Component {
//  int a = 0;
//  template <class Context>
//  constexpr static auto serde(Context &context, ComponentB &value) {
//    using Self = ComponentB;
//    using namespace serde::attribute;
//    serde::serde_struct(context, value).field(&Self::a, "a", default_{0});
//  }
//};
// struct ComponentC : public Component {
//  int a = 0;
//};
//
// struct Comtainer {
//  unordered_map<int, std::vector<std::shared_ptr<Component>>> components;
//};
//
// struct Text {
//  std::string text;
//
//  template <class Context>
//  constexpr static auto serde(Context &context, Text &value) {
//    using Self = Text;
//    using namespace serde::attribute;
//    serde::serde_struct(context, value)
//        .field(&Self::text, "text", default_{""});
//  }
//};
// struct Obj {
//  int id = 0;
//  void setId(int i) { id = i; }
//  int getId() const { return id; }
//
//  std::variant<int, std::string, float, ComponentB, std::shared_ptr<Text>>
//      variantData;
//  std::string strData;
//  std::vector<int> vecData;
//  std::unordered_map<int, std::string> mapData;
//
//  static auto GetMembers() {
//    return std::tuple{
//        IKIGAI::UTILS::MakeMemberInfo("id", &Obj::getId, &Obj::setId),
//        IKIGAI::UTILS::MakeMemberInfo("mapData", &Obj::mapData)};
//  }
//
//  template <class Context>
//  constexpr static auto serde(Context &context, Obj &value) {
//    using Self = Obj;
//    using namespace serde::attribute;
//    serde::serde_struct(context, value)
//        .field(&Self::id, "id", default_{0})
//        .field(&Self::vecData, "vecData")
//        .field(&Self::variantData, "variantData");
//  }
//};
//
// Comtainer comtainer;
//
// template <class T, class U>
// std::string value_type(std::unordered_map<T, U> const &m) {
//  return typeid(U).name();
//}
// #include <serdepp/adaptor/reflection.hpp>

// #ifdef DX12_BACKEND
// #include <Windows.h>
// #include <iostream>
// #include <dxgi1_4.h>
// #include <d3d12.h>
// #include <D3Dcompiler.h>
// #include <DirectXMath.h>
// #include <wrl/client.h>
// #include <DirectXPackedVector.h>
// using Microsoft::WRL::ComPtr;
// using namespace DirectX;
// using namespace DirectX::PackedVector;
//
// #pragma comment(lib, "d3dcompiler.lib")
// #pragma comment(lib, "D3D12.lib")
//
// int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE prevInstance, PSTR cmdLine,
// int showCmd) {
//
// #if defined(DEBUG) | defined(_DEBUG)
//	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
// #endif
//	IKIGAI::CORE::App::hInstance = hInstance;
//	IKIGAI::CORE::App app;
//	app.getCore()->sceneManager->loadFromFile("scenes/sceneDx12.json");
//
//	app.run();
//	return 0;
// }
// #endif

#ifndef OCULUS
#include <iostream>

#include "ecsModule/world.h"

#include "utilsModule/reflection/reflection/ReflMngr.hpp"
#include "utilsModule/reflection/reflection/ranges/FieldRange.hpp"
#include "utilsModule/reflection/reflection/ranges/MethodRange.hpp"

#include <nameof.hpp>

struct Vec {
  float x = 0;
  float y = 0;
  float norm() const { return std::sqrt(x * x + y * y); }

  void setX(float v) { x = v; }
  float getX() const { return x; }
};

namespace AA {
struct A {
  int this_is_the_name;
};
} // namespace AA

#include "EcsDemo.h"
#include "ReflectionDemo.h"

int main(int, char *[]) {
  // IKIGAI::CORE::App app;
  ////
  /// app.getCore()->sceneManager->loadFromFile("C:/Users/DaniilGlushchenko/Desktop/IkigaiEngine/assets/engine/scenes/scene1.json");
  // std::cout << "START" << std::endl;
  //  app.run();


  Example::EcsDemo::run();
  std::cout << "______________" << std::endl;
  ExampleReflection::ReflectionDemo::run();

  // Obtains name of member.
  //using namespace AA;
  //std::cout << NAMEOF_TYPE(A) << "\n";

  //Vec v;
  //
  //Ubpa::UDRefl::Mngr.RegisterType<Vec>();
  //Ubpa::UDRefl::Mngr.AddField<&Vec::x, &Vec::getX, &Vec::setX>("x");
  //Ubpa::UDRefl::Mngr.AddField<&Vec::y>("y");
  //Ubpa::UDRefl::Mngr.AddMethod<&Vec::norm>("norm");
  //
  //for (auto &&[name, info] : Ubpa::UDRefl::FieldRange_of<Vec>) {
  //  std::cout << name.GetView() << std::endl;
  //
  //  if (info.setter) {
  //    info.setter->Invoke(&v, nullptr, Ubpa::UDRefl::TempArgsView{2.0f});
  //  }
  //  // info.fieldptr.Var(&v) = 2;
  //}
  //
  //std::cout << v.x << " " << v.y << "\n";
  //
  //for (auto &&[name, info] : Ubpa::UDRefl::MethodRange_of<Vec>) {
  //  std::cout << name.GetView() << std::endl;
  //  // info.methodptr.Invoke(&v);
  //}
  //
  //IKIGAI::ECS2::World world;
  //
  //auto e0 = world.createEntity();
  //std::cout << (int)e0.getUniqueId() << std::endl;


  return 0;
}
#endif
