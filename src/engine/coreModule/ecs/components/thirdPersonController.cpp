#include "thirdPersonController.h"
#if USE_PHYSICS_3D
#include "coreModule/ecs/object.h"
#include "skeletalAnimationComponent.h"
#include "skeletalComponent.h"
#include "physicsModule/characterMotor.h"
#include "resourceModule/fileSystem/fileSystem.h"
#include "resourceModule/serviceManager.h"
#include "resourceModule/skeletonAnimationManager.h"
#include "sceneModule/sceneManager.h"
#include "skeletalModule/animationBlend.h"
#include "skeletalModule/animationInstance.h"
#include "skeletalModule/characterAnimation.h"
#include "windowModule/inputManager/inputManager.h"
#include "windowModule/window/window.h"
#include <map>
#include <stdexcept>
#include <iostream>
#include <thread>
#ifdef USE_EDITOR
#include <imgui.h>
#endif

namespace IKIGAI::ECS {
namespace {
using namespace PHYSICS::CHARACTER;
using PHYSICS::Vec3;
constexpr float radians=.01745329252f;
namespace Motion=SKELETON::CHARACTER;
Vec3 rotate(Vec3 v,float heading) {return MATH::QuaternionF(Vec3{0,heading,0})*v;}
class CharacterPlayable final : public SKELETON::IAnimationPlayable {
public:
    struct Clip {std::shared_ptr<SKELETON::AnimSample> sample; bool loop=true;float phaseOffset=0,plant=0,release=0;bool contact=false,singleHand=false;};
    std::map<std::string,Clip> clips;
    std::string state="Idle";
    std::string secondary;
    float secondaryWeight=0;
    std::string turn;
    float turnWeight=0,externalTime=-1;
    Vec3 poseOffset{};
    int hips=-1;
    SKELETON::Pose output{}, previous{};
    SKELETON::AnimBlend blend;
    std::shared_ptr<SKELETON::Skeleton> skeleton;
    float fade=1, phase=0, rate=1;
    explicit CharacterPlayable(std::shared_ptr<SKELETON::Skeleton> s):blend(s),skeleton(std::move(s)) {hips=skeleton->findJointIndex("Hips");}
    void select(const std::string& value, float speed=1) {
        if(!clips.contains(value)) return;
        rate=speed;
        secondary.clear();secondaryWeight=0;
        turn.clear();turnWeight=0;
        if(state==value) return;
        externalTime=-1;poseOffset={};
        previous=output;fade=0;state=value;
        auto& c=clips.at(state);c.sample->seek(0);
    }
    float duration(const std::string& value) const {return clips.at(value).sample->duration();}
    void update(float dt) override {
        auto& c=clips.at(state);
        if(externalTime>=0) c.sample->seek(externalTime);
        else if(c.loop) {
            // One common stride clock, including inactive directional clips.
            phase+=dt*rate/ std::max(.1f,c.sample->duration());
            const float p=phase+c.phaseOffset;
            c.sample->seek((p-std::floor(p))*c.sample->duration());
        } else {c.sample->setPlaybackRate(rate);c.sample->update(dt);}
        fade=std::min(1.f,fade+dt/.15f);
        output=*c.sample->getPose();
        if(c.loop && !secondary.empty() && clips.contains(secondary)) {
            auto& other=clips.at(secondary);
            const float p=phase+other.phaseOffset;
            other.sample->seek((p-std::floor(p))*other.sample->duration());
            output=*blend.blend(&output,other.sample->getPose(),secondaryWeight);
        }
        if(c.loop && !turn.empty() && clips.contains(turn)) {
            auto& other=clips.at(turn);const float p=phase+other.phaseOffset;
            other.sample->seek((p-std::floor(p))*other.sample->duration());
            output=*blend.blend(&output,other.sample->getPose(),turnWeight);
        }
        if(hips>=0) output.keyframes[hips].translation+=poseOffset*100.f;
        if(fade<1 && previous.numKeyframes==output.numKeyframes)
            output=*blend.blend(&previous,&output,fade);
    }
    SKELETON::Pose* getPose() override {return &output;}
    std::unique_ptr<IAnimationPlayable> clone() const override {
        auto result=std::make_unique<CharacterPlayable>(skeleton);
        for(const auto& [name,c] : clips) {
            std::shared_ptr<IAnimationPlayable> sample=std::move(c.sample->clone());
            auto copy=c;copy.sample=std::dynamic_pointer_cast<SKELETON::AnimSample>(sample);
            result->clips.emplace(name,std::move(copy));
        }
        result->state=state;result->output=output;result->previous=previous;
        result->fade=fade;result->phase=phase;result->rate=rate;
        result->secondary=secondary;result->secondaryWeight=secondaryWeight;
        result->turn=turn;result->turnWeight=turnWeight;
        result->externalTime=externalTime;result->poseOffset=poseOffset;
        return result;
    }
};
bool pathClear(const PHYSICS::World& world, Capsule capsule, Vec3 end) {
    const auto hit=sweep(world,capsule,end-capsule.feet);
    return !hit;
}
}
struct ThirdPersonController::Runtime {
    Motor motor;
    std::weak_ptr<Object> visual,camera;
    std::shared_ptr<CharacterPlayable> animation;
    Vec3 spawn{}, previous{}, rendered{}, wish{}, facing{0,0,1};
    float yaw=180,pitch=15,heading=180,turnRate=0,arm=4.5f,remainder=0,fixedDt=1.f/60;
    bool captured=false,jump=false,sprint=false,debug=false;
    bool traversing=false,vault=false;
    Vec3 traversalStart{},traversalRaised{},traversalEnd{};
    Vec3 ledge{},originStart{},originEnd{},visualFeet{};
    PHYSICS::Id traversalObstacle=0;
    float progress=0,actionDuration=1,landing=0;
    float plantPhase=0,releasePhase=0;
    float airTime=0,stairs=0;
    std::string action="Idle";
    std::string smokeScenario;
    std::vector<std::string> smokeSequence;
    size_t smokeIndex=0;
    float smokeTime=0,smokePeak=0,smokeCaptureTime=-1;
    float smokeCapture=-1,smokeLandTime=0,smokeContactError=0;
    bool smokeStairs=false,smokeTurn=false,smokeLand=false;
    bool smokeFired=false,smokeTraversed=false,smokePassed=false;
    void beginSmoke(const std::string& scenario) {
        smokeScenario=scenario;smokeTime=smokePeak=smokeLandTime=smokeContactError=0;
        smokeFired=smokeTraversed=smokePassed=smokeStairs=smokeTurn=smokeLand=false;
        wish={};jump=sprint=traversing=false;landing=airTime=stairs=turnRate=progress=0;
        yaw=heading=180;animation->select("Idle");
        if(scenario=="vault") motor.reset({0,.03f,-1.9f});
        else if(scenario=="vault-low") motor.reset({-4,.03f,-.9f});
        else if(scenario=="mantle") motor.reset({4,.03f,-1.7f});
        else if(scenario=="mantle-high" || scenario=="mantle-sprint") motor.reset({4,.03f,-11.7f});
        else if(scenario.starts_with("stairs")) motor.reset({-9,.03f,-1.8f});
        else if(scenario.starts_with("diagonal") || scenario.starts_with("turn")) motor.reset({0,.03f,8});
        else motor.reset(spawn);
        previous=rendered=motor.capsule.feet;
    }
    ~Runtime() {
        if(captured && RESOURCES::ServiceManager::Check<WINDOW::Window>())
            RESOURCES::ServiceManager::Get<WINDOW::Window>().setCursorVisible(true,false);
    }
    bool tryTraverse(const PHYSICS::World& world, Vec3 direction) {
        if(!motor.grounded || length(direction)<.1f) return false;
        const auto plan=planTraversal(world,motor.capsule,direction);
        if(!plan) return false;
        traversalStart=plan.start;traversalRaised=plan.raised;traversalEnd=plan.end;
        ledge=plan.ledge;
        traversalObstacle=plan.obstacle;
        const float height=ledge.y-plan.start.y;
        action=plan.vault ? (height>.75f?"Vault":"VaultLow") :
            (height>1.85f?(sprint?"MantleSprint":"MantleHigh"):"Mantle");
        if(!animation->clips.contains(action)) action=plan.vault?"Vault":"Mantle";
        actionDuration=std::max(.4f,animation->duration(action));
        progress=0;traversing=true;vault=plan.vault;motor.velocity={};
        heading=std::atan2(direction.x,direction.z)/radians;
        animation->select(action);
        auto& clip=animation->clips.at(action);
        clip.sample->seek(0);
        originStart=traversalStart-rotate(Motion::feet(*animation->skeleton,*clip.sample->getPose()),heading);
        clip.sample->seek(clip.sample->duration());
        originEnd=traversalEnd-rotate(Motion::feet(*animation->skeleton,*clip.sample->getPose()),heading);
        plantPhase=clip.plant;releasePhase=clip.release;
        if(clip.contact && !vault) {
            // A lower ledge needs a later, tucked pose before both the hands
            // can be planted and the feet can stay above the ground. Determine
            // that reach from the actual skeleton instead of lifting the body.
            for(float phase=clip.plant;phase<clip.release-.06f;phase+=1.f/120) {
                clip.sample->seek(phase*clip.sample->duration());
                const auto& pose=*clip.sample->getPose();
                const float lowest=Motion::feet(*animation->skeleton,pose).y;
                const float hands=Motion::hand(*animation->skeleton,pose,clip.singleHand).y;
                plantPhase=phase;
                if(ledge.y+lowest-hands>=traversalStart.y+.025f) break;
            }
        }
        clip.sample->seek(0);animation->externalTime=0;
        if(!smokeScenario.empty()) std::cout<<"CHARACTER_TRAVERSAL "<<action<<" plant="<<plantPhase<<" release="<<releasePhase<<std::endl;
        visualFeet=traversalStart;
        return true;
    }
};
ThirdPersonController::ThirdPersonController(UTILS::Ref<Object> object):ComponentBase(object) {
    __NAME__="ThirdPersonController";settings.Type=GetType<ThirdPersonController>();
}
ThirdPersonController::ThirdPersonController(UTILS::Ref<Object> object,const ComponentBase::Descriptor& descriptor)
    :ThirdPersonController(object) {settings=static_cast<const Descriptor&>(descriptor);settings.Type=GetType<ThirdPersonController>();}
void ThirdPersonController::onDisable() {
    if(runtime) {runtime->wish={};runtime->jump=false;
        if(runtime->captured && RESOURCES::ServiceManager::Check<WINDOW::Window>())
            RESOURCES::ServiceManager::Get<WINDOW::Window>().setCursorVisible(true,false);
        runtime->captured=false;
    }
}
void ThirdPersonController::onDestroy() {onDisable();runtime.reset();}
void ThirdPersonController::onUpdate(std::chrono::duration<double> delta) {
    auto& scene=RESOURCES::ServiceManager::Get<SCENE_SYSTEM::SceneManager>().getCurrentScene();
    if(!runtime) {
        auto visual=scene.findObjectByName(settings.Visual),camera=scene.findObjectByName(settings.Camera);
        if(!visual || !camera) throw std::runtime_error("Third person visual/camera reference is missing");
        auto skeleton=visual->getComponent<SkeletalComponent>();
        auto animator=visual->getComponent<SkeletalAnimationComponent>();
        if(!skeleton || !skeleton->getSkeleton() || !animator) throw std::runtime_error("Third person visual needs skeleton/animator");
        if(settings.Radius<=0 || settings.Height<settings.Radius*2 || settings.CameraDistance<.5f)
            throw std::runtime_error("Invalid third person capsule/camera settings");
        runtime=std::make_shared<Runtime>();auto& r=*runtime;
        r.visual=visual;r.camera=camera;r.spawn=obj->mTransform->getWorldPosition();
        r.motor.capsule.radius=settings.Radius;r.motor.capsule.height=settings.Height;r.motor.reset(r.spawn);
        r.previous=r.rendered=r.spawn;r.arm=settings.CameraDistance;
        r.animation=std::make_shared<CharacterPlayable>(skeleton->getSkeleton());
        auto bytes=RESOURCES::ServiceManager::Get<RESOURCES::FileSystem>().getFile(settings.Animations)->read();
        const auto manifest=nlohmann::json::parse(bytes.begin(),bytes.end());
        r.smokeScenario=manifest.value("SmokeScenario",std::string{});
        r.smokeCapture=manifest.value("CapturePhase",-1.f);
        r.smokeCaptureTime=manifest.value("CaptureTime",-1.f);
        r.smokeSequence=manifest.value("SmokeSequence",std::vector<std::string>{});
        const auto skel=skeleton->getSkeleton();const int hips=skel->findJointIndex("Hips");
        float hipHeight=90;
        for(const auto& item : manifest.at("Clips")) {
            const auto name=item.at("State").get<std::string>();
            auto source=RESOURCES::ServiceManager::Get<RESOURCES::SkeletonAnimationLoader>().loadResource(item.at("Path").get<std::string>());
            if(!source) throw std::runtime_error("Cannot load character clip: "+name);
            auto clip=std::make_shared<SKELETON::Animation>(*source);
            const bool loop=item.value("Loop",true);
            const auto rootMode=item.value("RootMode",std::string{});
            if(hips>=0 && hips<int(clip->channels.size())) {
                auto& keys=clip->channels[hips].translationKeyframes;
                if(name=="Idle" && !keys.empty()) hipHeight=keys.front().translation.y;
                if(!keys.empty() && rootMode!="traversal") {
                    const auto first=keys.front(),last=keys.back();
                    for(auto& k:keys) {
                        k.translation.x=0;k.translation.z=0;
                        if(rootMode=="stairs") {
                            const float t=last.time>first.time ? float((k.time-first.time)/(last.time-first.time)) : 0;
                            k.translation.y-=(last.translation.y-first.translation.y)*t;
                        } else if(name=="Jump") k.translation.y=hipHeight;
                    }
                }
                if(item.value("RemoveYaw",false)) {
                    for(auto& key:clip->channels[hips].rotationKeyframes) {
                        const Vec3 forward=key.rotation*Vec3{0,0,1};
                        const float yaw=std::atan2(forward.x,forward.z)/radians;
                        key.rotation=MATH::QuaternionF(Vec3{0,-yaw,0})*key.rotation;
                    }
                }
            }
            auto sample=std::make_shared<SKELETON::AnimSample>(skel,clip);
            sample->setLooping(loop);sample->seek(0);
            r.animation->clips.emplace(name,CharacterPlayable::Clip{sample,loop,item.value("PhaseOffset",0.f),
                item.value("Plant",0.f),item.value("Release",0.f),item.value("Contact",false),item.value("SingleHand",false)});
            std::cout << "CHARACTER_CLIP " << name << " joints=" << skel->getNumJolts()
                << " duration=" << sample->duration() << std::endl;
        }
        for(const auto& required:{"Idle","Run","Back","Left","Right","Jump","Fall","Land","Vault","Mantle"})
            if(!r.animation->clips.contains(required)) throw std::runtime_error(std::string("Missing character state: ")+required);
        // Match actual support feet, rather than assuming mirrored FBX clips
        // share a frame-zero contact. This also aligns the turning gait.
        const int left=skel->findJointIndex("LeftFoot"),right=skel->findJointIndex("RightFoot");
        if(left>=0 && right>=0) {
            auto stride=[&](const std::string& name) {
                std::array<float,32> values{};auto& sample=r.animation->clips.at(name).sample;
                for(int i=0;i<32;++i) {
                    sample->seek(sample->duration()*float(i)/32);
                    const auto p=Motion::positions(*skel,*sample->getPose());
                    values[i]=p[left].y-p[right].y;
                }
                sample->seek(0);return values;
            };
            const auto reference=stride("Run");
            for(const auto name:{"Back","Left","Right","Sprint","TurnLeft","TurnRight","BackTurnLeft","BackTurnRight","Stairs","StairsRun"})
                if(r.animation->clips.contains(name)) {
                    const float offset=Motion::strideOffset(reference,stride(name));
                    r.animation->clips.at(name).phaseOffset=offset;
                    std::cout<<"CHARACTER_PHASE "<<name<<' '<<offset<<std::endl;
                }
        }
        if(!r.smokeSequence.empty()) r.beginSmoke(r.smokeSequence.front());
        else if(!r.smokeScenario.empty()) r.beginSmoke(r.smokeScenario);
        r.animation->update(0);animator->setPlayable(r.animation);
    }
    auto& r=*runtime;const float dt=std::clamp(float(delta.count()),0.f,.1f);
    // Diagnostic manifests run hidden; guarantee enough wall time for fixed
    // physics ticks even when an empty/loading window renders very quickly.
    if(!r.smokeScenario.empty()) std::this_thread::sleep_for(std::chrono::milliseconds(8));
    auto& input=RESOURCES::ServiceManager::Get<INPUT_SYSTEM::InputManager>();
    auto& window=RESOURCES::ServiceManager::Get<WINDOW::Window>();
    using K=INPUT_SYSTEM::EKey;
    bool justCaptured=false;
    if(r.smokeScenario.empty()) {
    if(!window.hasFocus() || input.wasKeyPressedThisFrame(K::KEY_ESCAPE)) onDisable();
    else if(input.wasMouseButtonPressedThisFrame(INPUT_SYSTEM::EMouseButton::MOUSE_BUTTON_LEFT)) {
#ifdef USE_EDITOR
        if(!ImGui::GetIO().WantCaptureMouse)
#endif
        {r.captured=true;justCaptured=true;window.setCursorVisible(false,true);}
    }
    if(r.captured) {
        const auto mouse=justCaptured ? MATH::Vector2f{0,0} : input.getMouseDelta();
        r.yaw-=mouse.x*settings.Sensitivity;r.pitch=std::clamp(r.pitch+mouse.y*settings.Sensitivity,-35.f,70.f);
        const float x=float(input.isKeyPressed(K::KEY_D))-float(input.isKeyPressed(K::KEY_A));
        const float z=float(input.isKeyPressed(K::KEY_W))-float(input.isKeyPressed(K::KEY_S));
        Vec3 local{x,0,z};if(length(local)>1) local=unit(local);
        r.facing={std::sin(r.yaw*radians),0,std::cos(r.yaw*radians)};
        const Vec3 right{-r.facing.z,0,r.facing.x};
        r.sprint=input.isKeyPressed(K::KEY_LEFT_SHIFT);
        r.wish=(right*local.x+r.facing*local.z)*(r.sprint?settings.SprintSpeed:settings.RunSpeed);
        r.jump|=input.wasKeyPressedThisFrame(K::KEY_SPACE);
        if(input.wasKeyPressedThisFrame(K::KEY_R)) {r.motor.reset(r.spawn);r.previous=r.rendered=r.spawn;r.traversing=false;}
        if(input.wasKeyPressedThisFrame(K::KEY_F3)) r.debug=!r.debug;
    } else r.wish={};
    }
    r.remainder+=dt;
    const float alpha=std::clamp(r.remainder/r.fixedDt,0.f,1.f);
    r.rendered=r.previous*(1-alpha)+r.motor.capsule.feet*alpha;
    // The root is the physics position; only the child visual is interpolated.
    auto visual=r.visual.lock(),camera=r.camera.lock();if(!visual || !camera) return;
    visual->mTransform->setLocalPosition(r.rendered-r.motor.capsule.feet);
    visual->mTransform->setLocalRotation(MATH::QuaternionF(Vec3{0,r.heading,0}));
    const Vec3 pivot=(r.traversing?r.visualFeet:r.rendered)+Vec3{0,1.45f,0};
    const Vec3 forward{std::sin(r.yaw*radians)*std::cos(r.pitch*radians),-std::sin(r.pitch*radians),std::cos(r.yaw*radians)*std::cos(r.pitch*radians)};
    const float desired=settings.CameraDistance;
    const Capsule probe{pivot-Vec3{0,.18f,0},.18f,.36f};
    const auto hit=sweep(scene.physics3d(),probe,forward*(-desired));
    const float safe=hit ? std::max(.15f,desired*hit.fraction-.06f) : desired;
    r.arm=safe<r.arm ? safe : r.arm+(safe-r.arm)*(1-std::exp(-8*dt));
    camera->mTransform->setLocalPosition(pivot-forward*r.arm);
    camera->mTransform->setLocalRotation(MATH::QuaternionF(Vec3{r.pitch,r.yaw,0}));
#ifdef USE_EDITOR
    ImGui::SetNextWindowPos(ImVec2(20,75),ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowBgAlpha(.85f);
    if(ImGui::Begin("Third Person Playground",nullptr,ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::TextUnformatted("Click viewport to play | Esc: cursor / menus");
        ImGui::TextUnformatted("WASD: run | Shift: sprint | Space: jump / vault / climb");
        ImGui::TextUnformatted("Mouse: orbit | R: respawn | F3: diagnostics");
        ImGui::TextUnformatted("Blue barriers: vault | Orange blocks: climb");
        ImGui::Text("State: %s | speed %.2f m/s",r.action.c_str(),length(Vec3{r.motor.velocity.x,0,r.motor.velocity.z}));
        if(r.debug) {ImGui::Text("Feet %.2f %.2f %.2f | grounded %s",r.motor.capsule.feet.x,r.motor.capsule.feet.y,r.motor.capsule.feet.z,r.motor.grounded?"yes":"no");
            ImGui::Text("Traversal %.2f | camera %.2f m",r.progress,r.arm);}
        if(ImGui::Button(r.captured?"Release cursor":"Play")) {r.captured=!r.captured;window.setCursorVisible(!r.captured,r.captured);}
    }
    ImGui::End();
#endif
}
void ThirdPersonController::onFixedUpdate(std::chrono::duration<double> delta) {
    if(!runtime) return;
    auto& r=*runtime;const float dt=std::clamp(float(delta.count()),0.f,.1f);
    r.fixedDt=dt;r.remainder=std::max(0.f,r.remainder-dt);r.previous=r.motor.capsule.feet;
    auto& world=RESOURCES::ServiceManager::Get<SCENE_SYSTEM::SceneManager>().getCurrentScene().physics3d();
    if((r.smokeCapture>=0 && r.traversing && r.progress>=r.smokeCapture) ||
        (r.smokeCaptureTime>=0 && r.smokeTime>=r.smokeCaptureTime)) {
        if(!r.smokePassed) {
            r.smokePassed=true;
            if(!r.traversing) r.animation->externalTime=r.animation->clips.at(r.action).sample->time();
            std::cout<<"CHARACTER_SMOKE_OK "<<r.smokeScenario<<" capture="<<r.progress<<std::endl;
        }
        return;
    }
    if(!r.smokeScenario.empty()) {
        r.smokeTime+=dt;
        if(!r.smokeFired && r.motor.grounded && r.smokeTime>.1f) {
            r.smokeFired=true;r.jump=true;
            if(r.smokeScenario!="jump") r.wish={0,0,-3.5f};
            if(r.smokeScenario.starts_with("stairs") || r.smokeScenario.starts_with("diagonal") || r.smokeScenario.starts_with("turn")) r.jump=false;
        }
        if(r.smokeScenario=="mantle-sprint") r.sprint=true;
        if(r.smokeScenario.starts_with("stairs")) {
            r.sprint=r.smokeScenario=="stairs-run";
            r.wish=r.motor.capsule.feet.z>-6.2f ? Vec3{0,0,r.sprint?-6.5f:-3.5f} : Vec3{};
        } else if(r.smokeScenario.starts_with("diagonal"))
            r.wish=r.smokeTime<1.3f ? Vec3{r.smokeScenario=="diagonal-right"?2.475f:-2.475f,0,-2.475f}:Vec3{};
        else if(r.smokeScenario.starts_with("turn")) {
            r.yaw+=(r.smokeScenario=="turn-left"?70:-70)*dt;
            r.wish=r.smokeTime<1.5f?Vec3{std::sin(r.yaw*radians),0,std::cos(r.yaw*radians)}*3.5f:Vec3{};
        }
    }
    if(r.motor.capsule.feet.y < -10) {r.motor.reset(r.spawn);r.traversing=false;r.previous=r.spawn;}
    if(r.jump && !r.traversing && r.tryTraverse(world,unit(r.wish,{0,0,0}))) r.jump=false;
    if(!r.smokeScenario.empty()) {
        r.smokeTraversed|=r.traversing;
        if(!r.smokeScenario.starts_with("stairs") && !r.smokeScenario.starts_with("diagonal") && !r.smokeScenario.starts_with("turn")) r.wish={};
    }
    if(r.traversing) {
        r.progress=std::min(1.f,r.progress+dt/r.actionDuration);
        const float t=r.progress;
        const Vec3 raisedEnd{r.traversalEnd.x,r.traversalRaised.y,r.traversalEnd.z};
        Vec3 target;
        if(t<.3f) target=r.traversalStart+(r.traversalRaised-r.traversalStart)*(t/.3f);
        else if(t<.8f) target=r.traversalRaised+(raisedEnd-r.traversalRaised)*((t-.3f)/.5f);
        else target=raisedEnd+(r.traversalEnd-raisedEnd)*((t-.8f)/.2f);
        if(pathClear(world,r.motor.capsule,target)) r.motor.capsule.feet=target;
        else {r.traversing=false;r.motor.grounded=false;}
        auto& clip=r.animation->clips.at(r.action);
        r.animation->externalTime=t*clip.sample->duration();
        clip.sample->seek(r.animation->externalTime);
        const auto& pose=*clip.sample->getPose();
        Vec3 origin;
        if(clip.contact) origin=Motion::anchorOrigin(r.originStart,r.originEnd,r.ledge,
            rotate(Motion::hand(*r.animation->skeleton,pose,clip.singleHand),r.heading),t,r.plantPhase,r.releasePhase);
        else origin=MATH::Vector3f::Lerp(r.originStart,r.originEnd,Motion::smooth(0,1,t));
        // Root motion remains in the pose. Warp its origin onto the ledge and
        // landing, rather than lifting a frozen pelvis along the capsule path.
        // Check the support feet against actual surfaces, including the far edge.
        const auto points=Motion::positions(*r.animation->skeleton,pose);
        float lift=0;
        Filter footFilter;
        // A braced foot can be below the ledge and against its vertical face.
        // Treating that foot as standing on top lifted the whole body away
        // from its planted hands. Require top support only when stepping out.
        if(!r.vault && t<.8f) footFilter.ignore=r.traversalObstacle;
        for(const auto name:{"LeftFoot","RightFoot","LeftToeBase","RightToeBase"}) {
            const int index=r.animation->skeleton->findJointIndex(name);if(index<0) continue;
            const auto p=origin+rotate(points[index]*.01f,r.heading);
            const auto support=raycast(world,{p.x,r.traversalRaised.y+1.f,p.z},{0,-4.5f,0},footFilter);
            if(support && support.normal.y>.7f) lift=std::max(lift,support.point.y+.025f-p.y);
        }
        origin.y+=lift;
        if(!r.smokeScenario.empty() && clip.contact && t>r.plantPhase+.03f && t<r.releasePhase-.03f) {
            const auto hand=origin+rotate(Motion::hand(*r.animation->skeleton,pose,clip.singleHand),r.heading);
            r.smokeContactError=std::max(r.smokeContactError,length(hand-r.ledge));
            if(r.smokeContactError>.16f) throw std::runtime_error("Traversal hand lifted off ledge: phase="+std::to_string(t)+" error="+std::to_string(r.smokeContactError));
        }
        r.animation->poseOffset=rotate(origin-r.motor.capsule.feet,-r.heading);
        const int hips=r.animation->hips;
        const auto hip=hips>=0?points[hips]*.01f:Vec3{0,.9f,0};
        r.visualFeet=origin+rotate(hip,r.heading)-Vec3{0,.9f,0};
        if(t>=1) {r.traversing=false;r.motor.grounded=true;r.landing=0;r.airTime=0;}
        r.jump=false;
    } else {
        const bool wasGrounded=r.motor.grounded;
        const float impact=r.motor.velocity.y;
        r.motor.move(world,r.wish,r.jump,dt);r.jump=false;
        r.airTime=r.motor.grounded ? r.airTime : r.airTime+dt;
        if(!wasGrounded && r.motor.grounded) {
            // Tiny stair gaps are support changes, not landings. Start the short
            // recovery clip on contact, without a fixed post-landing hold.
            if(r.airTime>.12f && impact < -1.5f)
                r.landing=std::min(.32f,r.animation->duration("Land")/1.2f);
            r.airTime=0;
        }
        if(r.motor.steppedUp) r.stairs=.4f;
        else r.stairs=std::max(0.f,r.stairs-dt);
        r.landing=std::max(0.f,r.landing-dt);
        if(!r.motor.grounded && (r.motor.velocity.y>1 || r.airTime>.12f)) r.action=r.motor.velocity.y>1 ? "Jump" : "Fall";
        else if(r.landing>0 && length(r.wish)<.1f) r.action="Land";
        else if(length(Vec3{r.motor.velocity.x,0,r.motor.velocity.z})<.15f) r.action="Idle";
        else {
            const float change=Motion::angleDelta(r.heading,r.yaw)*(1-std::exp(-14*dt));
            r.heading+=change;
            r.turnRate+=(change/std::max(.001f,dt)-r.turnRate)*(1-std::exp(-10*dt));
            const Vec3 forward{std::sin(r.heading*radians),0,std::cos(r.heading*radians)};
            const Vec3 right{-forward.z,0,forward.x};
            const float f=r.motor.velocity.dot(forward),s=r.motor.velocity.dot(right);
            // Keep one primary gait per forward/backward quadrant. Switching
            // primary clips at 45 degrees used to restart fades on diagonals.
            r.action=f>=0 ? "Run":"Back";
            if(std::abs(f)<.1f && std::abs(s)>.1f) r.action=s>0?"Right":"Left";
            if(r.stairs>0 && f>.5f && std::abs(s)<std::abs(f)*.7f)
                r.action=length(Vec3{r.motor.velocity.x,0,r.motor.velocity.z})>2.2f?"StairsRun":"Stairs";
            else if(r.sprint && r.action=="Run" && std::abs(s)<std::abs(f)*.3f) r.action="Sprint";
        }
        float speed=1;
        if(r.action=="Jump") speed=1.5f;
        if(r.action=="Land") speed=1.2f;
        if(r.action=="Run" || r.action=="Back") speed=std::clamp(length(Vec3{r.motor.velocity.x,0,r.motor.velocity.z})/3.5f,.5f,1.8f);
        if(r.action=="Left" || r.action=="Right") speed=r.sprint?2.4f:1.7f;
        if(r.action=="Stairs" || r.action=="StairsRun")
            speed=std::clamp(length(Vec3{r.motor.velocity.x,0,r.motor.velocity.z})*r.animation->duration(r.action)/1.6f,.7f,3.f);
        r.animation->select(r.action,speed);
        if(r.action=="Run" || r.action=="Back" || r.action=="Left" || r.action=="Right") {
            const Vec3 forward{std::sin(r.heading*radians),0,std::cos(r.heading*radians)};
            const Vec3 right{-forward.z,0,forward.x};
            const float f=r.motor.velocity.dot(forward),s=r.motor.velocity.dot(right);
            r.animation->secondary=s>=0?"Right":"Left";
            r.animation->secondaryWeight=std::abs(s)/std::max(.001f,std::abs(s)+std::abs(f));
        }
        if(r.action=="Run" || r.action=="Back" || r.action=="Sprint") {
            const bool back=r.action=="Back";
            r.animation->turn=std::string(back?"BackTurn":"Turn")+(r.turnRate>0?"Left":"Right");
            r.animation->turnWeight=std::clamp((std::abs(r.turnRate)-10)/120.f,0.f,.75f);
        } else r.turnRate*=std::exp(-10*dt);
    }
    obj->mTransform->setLocalPosition(r.motor.capsule.feet);
    if(!r.smokeScenario.empty()) {
        r.smokePeak=std::max(r.smokePeak,r.motor.capsule.feet.y);
        r.smokeStairs|=r.action=="Stairs" || r.action=="StairsRun";
        r.smokeTurn|=r.animation->turnWeight>.2f;
        r.smokeLand|=r.action=="Land";
        if(r.action=="Land") r.smokeLandTime+=dt;
        if(r.smokeScenario.starts_with("stairs") && r.smokeTime>.15f && (r.action=="Fall" || r.action=="Land"))
            throw std::runtime_error("Stairs triggered a fall/landing animation");
        const bool passed=r.smokeScenario=="jump" ? (r.smokePeak>.9f && r.motor.grounded && r.smokeTime>1.2f && r.smokeLand && r.smokeLandTime<=.34f && r.action=="Idle")
            : r.smokeScenario.starts_with("stairs") ? (r.smokeStairs && r.motor.grounded && r.motor.capsule.feet.y>1.1f && r.smokeTime>2)
            : r.smokeScenario.starts_with("diagonal") ? (r.smokeTime>1.4f && r.motor.grounded)
            : r.smokeScenario.starts_with("turn") ? (r.smokeTime>1.6f && r.smokeTurn && r.motor.grounded)
            : (r.smokeTraversed && !r.traversing && r.motor.grounded &&
                (r.smokeScenario.starts_with("vault") ? r.motor.capsule.feet.z < -2.5f : r.motor.capsule.feet.y>1.1f));
        if(passed && !r.smokePassed) {
            r.smokePassed=true;
            std::cout<<"CHARACTER_SMOKE_OK "<<r.smokeScenario<<" feet="<<r.motor.capsule.feet.x<<','<<r.motor.capsule.feet.y<<','<<r.motor.capsule.feet.z
                <<" contactError="<<r.smokeContactError<<std::endl;
        }
        if(!r.smokePassed && r.smokeTime>5) throw std::runtime_error("Character smoke scenario failed: "+r.smokeScenario);
        if(r.smokePassed && r.smokeIndex+1<r.smokeSequence.size()) {
            const auto next=r.smokeSequence[++r.smokeIndex];r.beginSmoke(next);
            obj->mTransform->setLocalPosition(r.motor.capsule.feet);
        }
    }
}
}
#endif
