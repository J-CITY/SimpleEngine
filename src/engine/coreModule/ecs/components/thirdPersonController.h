#pragma once
#if USE_PHYSICS_3D
#include <memory>
#include "component.h"

namespace IKIGAI::ECS {
// Camera/visual names are local scene references. The physical root has unit scale.
class ThirdPersonController : public ComponentBase {
public:
    struct Descriptor : ComponentBase::Descriptor {
        std::string Type;
        std::string Visual = "Player Visual", Camera = "Player Camera";
        std::string Animations = "/games/third-person/animations.json";
        float RunSpeed=3.5f, SprintSpeed=6.5f, Radius=.3f, Height=1.8f;
        float CameraDistance=4.5f, Sensitivity=.12f;
        template<class C> static constexpr auto serde(C& c, Descriptor& v) {
            using namespace serde::attribute;
            serde::serde_struct(c,v).field(&Descriptor::Type,"ThirdPersonControllerType")
                .field(&Descriptor::Visual,"Visual",default_{std::string("Player Visual")})
                .field(&Descriptor::Camera,"Camera",default_{std::string("Player Camera")})
                .field(&Descriptor::Animations,"Animations",default_{std::string("/games/third-person/animations.json")})
                .field(&Descriptor::RunSpeed,"RunSpeed",default_{3.5f})
                .field(&Descriptor::SprintSpeed,"SprintSpeed",default_{6.5f})
                .field(&Descriptor::Radius,"Radius",default_{.3f})
                .field(&Descriptor::Height,"Height",default_{1.8f})
                .field(&Descriptor::CameraDistance,"CameraDistance",default_{4.5f})
                .field(&Descriptor::Sensitivity,"Sensitivity",default_{.12f});
        }
    };
    explicit ThirdPersonController(UTILS::Ref<Object> object);
    ThirdPersonController(UTILS::Ref<Object> object, const ComponentBase::Descriptor& descriptor);
    void onUpdate(std::chrono::duration<double>) override;
    void onFixedUpdate(std::chrono::duration<double>) override;
    void onDisable() override;
    void onDestroy() override;
    Descriptor getDescriptor() const {return settings;}
    static auto GetMembers() {return std::tuple{};}
    Descriptor settings;
private:
    struct Runtime;
    std::shared_ptr<Runtime> runtime;
};
template<> inline std::string GetType<ThirdPersonController>() {return "class IKIGAI::ECS::ThirdPersonController";}
template<> inline std::string GetComponentName<ThirdPersonController>() {return "ThirdPersonController";}
}
#endif
