#pragma once
#include "backends/gl/materialGl.h"
#include "backends/interface/frameBufferInterface.h"
#include "backends/interface/textureInterface.h"
#include "backends/vk/driverVk.h"
#include "drawable.h"
#include "gameRendererInterface.h"
#include "utilsModule/enum.h"
#include "utilsModule/ref.h"

#ifdef OPENGL_HARD_RENDER
#include "backends/gl/materialGl.h"
#include "drawable.h"
#include "frustum.h"
#include "gameRendererInterface.h"

namespace IKIGAI::SCENE_SYSTEM {
class Scene;
}

namespace IKIGAI::ECS {
class CameraComponent;
}

namespace IKIGAI::CORE {
class Core;
}

IKIGAI_ENUM_NS(IKIGAI::RENDER, RenderStates, TAA = 1, MOTION_BLUR = 2,
               BLOOM = 4, VOLUMETRIC_LIGHT = 8, GOD_RAYS = 16, HDR = 32,
               COLOR_GRADING = 64, VIGNETTE = 128, DEPTH_OF_FIELD = 256,
               OUTLINE = 512, CHROMATIC_ABBERATION = 1024, POSTERIZE = 2048,
               PIXELIZE = 4096, SHARPEN = 8192, DILATION = 16384,
               FILM_GRAIN = 32768, FOG = 65536, GUI = 131072, SSAO = 262144,
               SSR = 524288, SSGI = 1048576, SSS = 2097152)

namespace IKIGAI::RENDER {
DEFINE_ENUM_CLASS_BITWISE_OPERATORS(RenderStates)
}

namespace IKIGAI::RENDER {

class FrameBufferGl;
class TextureGl;
class ShaderGl;
class ShaderGl;
class DriverGl;
class StorageBufferGl;
class Renderer;

class GameRendererGl : public GameRendererInterface {
public:
  void loadRenderGraph(const std::string& path);

  struct SSAO {
    std::vector<MATH::Vector3f> mSSAOKernel;
    bool mUseSSAO = true;
  };
  struct SSR {
    bool mUse = true;
  };
  struct SSGI {
    bool mUse = false;
  };
  struct SSS {
    bool mUse = false;
  };
  struct DirShadowMap {
    unsigned int mDirShadowMapResolution = 4096;
    float mDirNearPlane = 0.1f;
    float mDirFarPlane = 500.0f;
    std::vector<float> mShadowCascadeLevels{
        mDirFarPlane / 50.0f, mDirFarPlane / 25.0f, mDirFarPlane / 10.0f,
        mDirFarPlane / 2.0f};
    MATH::Matrix4f dirLightSpaceMatrix;

    unsigned int mSpotShadowMapResolution = 1024;
    float mSpotNearPlane = 1.0f;
    float mSpotFarPlane = 8.0;

    unsigned int mPointShadowMapResolution = 1024;
    float mPointNearPlane = 1.0f;
    float mPointFarPlane = 25.0f;

    bool mUseShadow = true;
    bool mUseShadowBaked = true;
    bool mIsShadowBakedInit = false;
  };

  struct ChromaticAbberation {
    MATH::Vector3f params = MATH::Vector3f(0.8f, 0.08f, 0.8f);
  };

  struct HDR {
    bool isEnabled = true;
    float exposure = 1.0f;
    float gamma = 2.2f;
  };

  struct ColorGrading {
    MATH::Vector3f r = MATH::Vector3f(1.0f, 0.0f, 0.0f);
    MATH::Vector3f g = MATH::Vector3f(0.0f, 1.0f, 0.0f);
    MATH::Vector3f b = MATH::Vector3f(0.0f, 0.0f, 1.0f);
  };

  struct Vignette {
    float radius = 0.1f;
    float intensity = 100.0f;
  };

  struct IBL {
    bool useIBL = true;
  };

  struct Fog {
    MATH::Vector3f color = MATH::Vector3f(0.7f, 0.7f, 0.7f);
    float linearStart = 20.0f;
    float linearEnd = 75.0f;
    float density = 0.01f;
    int equation = 1;
    bool isEnabled = true;
  };

  struct VolumetricLight {
    float godRayMaxSteps = 150.0f;
    float godRaySampleStep = 0.15f;
    float godRayStepIncrement = 1.01f;
    float godRayAsymmetry = 0.5f;
    float maxDist = 10.0f;

    // debug
    bool tex = true;
    bool dir = true;
    bool map = false;
  };

  struct Pipeline {
    MATH::Vector4f mClearColor = MATH::Vector4f(0.0f, 0.0f, 0.50f, 1.0f);

    bool mIsPBR = false;

    DirShadowMap mDirShadowMap;

    SSAO mSSAO;
    SSR mSSR;
    SSGI mSSGI;
    SSS mSSS;

    ChromaticAbberation chromaticAbberation;
    HDR hdr;
    ColorGrading colorGrading;
    Vignette vignette;
    IBL ibl;
    Fog fog;
    VolumetricLight vl;
  };

  Pipeline mPipeline;

  std::shared_ptr<TextureGl> mRenderToScreenTexture;

  std::shared_ptr<FrameBufferGl> mDeferredFb;
  std::shared_ptr<ShaderGl> mDeferredShader;
  std::shared_ptr<TextureGl>
      mPrevDeferredTexture; // texture after deferred and forward render without
                            // post processing
  std::shared_ptr<TextureGl>
      mDeferredTexture; // texture after deferred and forward render without
                        // post processing

  std::shared_ptr<TextureGl> mEmptyTexture;
  std::shared_ptr<TextureGl> mEditorTexture;
  std::shared_ptr<TextureGl> mHDRSkyBoxTexture;

  std::unordered_map<std::string, std::shared_ptr<TextureGl>> mTextures;
  bool pingPong = false;
  std::array<std::shared_ptr<TextureGl>, 2> pingPongTex;

  void setSkyBoxTexture(const std::string &path);
  GameRendererGl(IKIGAI::CORE::Core &context);
  void createShaders();
  void sendEngineUBO();
  void sendEngineShadowUBO(std::shared_ptr<ShaderGl> shader);
  void renderScene();
  void renderEditorScene(
      IKIGAI::UTILS::Ref<IKIGAI::ECS::CameraComponent> editorCamera);
  void renderScene(
      IKIGAI::UTILS::Ref<IKIGAI::ECS::CameraComponent> mainCameraComponent);
  void renderToScreen();
  void sendIBLData();
  void prepareBrightTexture();
  bool prepareBlurTexture(std::shared_ptr<TextureGl> tex);
  void prepareGodRaysTexture();
  void applyGoodRays();
  void applyBloom();
  void applyFXAA();
  void applyHDR();
  void applyColorGrading();
  void applyVignette();
  void applyDepthOfField();
  void applyOutline();
  void applyFog();
  void applyChromaticAbberation();
  void applyPosterize();
  void applyPixelize();
  void applySharpen();
  void applyDilation();
  void applyFilmGrain();
  void applyMotionBlur();
  void applyTAA();
  void sendSSAOData();
  void applyVolumetricLight();
  void sendBounseDataToShader(std::shared_ptr<MaterialGl> material,
                              ECS::Skeletal &animator,
                              std::shared_ptr<ShaderGl> shader);
  void drawGUISubtree(UTILS::Ref<ECS::Object> obj);

  virtual void resize() override;

  void drawGUI();

  std::shared_ptr<FrameBufferGl> debug3dTextureFB;
  std::shared_ptr<TextureGl> debug3dTexture;
  int debug3dTextureLayers = 0;
  int debug3dTextureLayersCur = 0;
  bool debug3dTextureIsPersp = false;
  bool debug3dTextureIsRGB = false;
  void initDebug3dTextureFB(std::shared_ptr<TextureGl> _debug3dTexture);
  void updateDebug3dTextureFB();

  struct EngineDirShadowUBO {
    std::vector<MATH::Matrix4f> lightSpaceMatrices;
  };

  std::map<std::string, bool> activeCustomPP;
  std::map<std::string, std::shared_ptr<MaterialGl>> customPostProcessing;

  std::shared_ptr<MaterialGl> skyBoxMaterial;

  void setSkyBoxMaterial(std::shared_ptr<MaterialInterface> m);

  void addCustomPostProcessing(const std::string &name,
                               std::shared_ptr<MaterialGl> material,
                               bool isActive = true);

  RenderStates renderStateMask = (RenderStates::BLOOM | RenderStates::GUI);

  std::vector<std::function<void()>> ppFuncs;
  std::map<RenderStates, std::function<void()>> typeToFuncPP{
      {RenderStates::TAA, [this]() { applyTAA(); }},
      {RenderStates::MOTION_BLUR, [this]() { applyMotionBlur(); }},
      {RenderStates::BLOOM, [this]() { applyBloom(); }},
      {RenderStates::VOLUMETRIC_LIGHT, [this]() { applyVolumetricLight(); }},
      {RenderStates::GOD_RAYS, [this]() { applyGoodRays(); }},
      {RenderStates::HDR, [this]() { applyHDR(); }},
      {RenderStates::COLOR_GRADING, [this]() { applyColorGrading(); }},
      {RenderStates::VIGNETTE, [this]() { applyVignette(); }},
      {RenderStates::DEPTH_OF_FIELD, [this]() { applyDepthOfField(); }},
      {RenderStates::OUTLINE, [this]() { applyOutline(); }},
      {RenderStates::CHROMATIC_ABBERATION,
       [this]() { applyChromaticAbberation(); }},
      {RenderStates::POSTERIZE, [this]() { applyPosterize(); }},
      {RenderStates::PIXELIZE, [this]() { applyPixelize(); }},
      {RenderStates::SHARPEN, [this]() { applySharpen(); }},
      {RenderStates::DILATION, [this]() { applyDilation(); }},
      {RenderStates::FILM_GRAIN, [this]() { applyFilmGrain(); }},
      {RenderStates::FOG, [this]() { applyFog(); }},
      {RenderStates::GUI, [this]() { drawGUI(); }},
  };

  void preparePipeline();

  void prepareIBL();

  std::shared_ptr<FrameBufferGl> gbufferGlobalFb;
  std::shared_ptr<TextureGl> gPositionGlobalTex;
  std::shared_ptr<TextureGl> gEyePositionGlobalTex;
  std::shared_ptr<TextureGl> gVelocityGlobalTex;

protected:
  EngineDirShadowUBO getLightSpaceMatrices(const MATH::Vector3f &lightDir,
                                           const MATH::Vector3f &lightPos);
  MATH::Matrix4f getLightSpaceMatrix(float nearPlane, float farPlane,
                                     const MATH::Vector3f &lightDir,
                                     const MATH::Vector3f &lightPos);
  bool prepareDirShadowMap(const std::string &id);
  bool prepareDirCascadeShadowMap(const std::string &id);
  void prepareSpotShadow();
  void preparePointShadow();

  void drawDeferredGBuffer();
  void drawForward();

  void drawDrawableBatching(const Drawable &p_toDraw);

  void createFrameBuffers();
  void applySSS();
  void applySSR();
  void applySSGI();
  void renderSkybox();
  void applySSAO();

  void updateLights(IKIGAI::SCENE_SYSTEM::Scene &scene);
  void updateLightsInFrustum(SCENE_SYSTEM::Scene &scene,
                             const Frustum &frustum);
  void drawDrawable(const Drawable &p_toDraw);
  void drawDrawableDebug(const Drawable &p_toDraw);
  void drawMesh(const Drawable &p_toDraw);
  void drawDrawableDeferred(const Drawable &p_toDraw);
  void drawDrawableWithShader(std::shared_ptr<ShaderGl> shader,
                              const Drawable &p_toDraw);

  void sendEngineUBO(ShaderGl &shader, const MATH::Matrix4f &world);
  void sendEngineShadowData(ShaderGl &shader);

  IKIGAI::CORE::Core &mContext;

  OpaqueDrawables mOpaqueMeshesForward;
  TransparentDrawables mTransparentMeshesForward;

  OpaqueDrawables mOpaqueMeshesDeferred;
  TransparentDrawables mTransparentMeshesDeferred;

  std::shared_ptr<StorageBufferGl> mLightSSBO;

  DriverGl *mDriver;

  int frameCount = 0;
};
} // namespace IKIGAI::RENDER
#endif
namespace IKIGAI {
namespace ECS {
class CameraComponent;
}
} // namespace IKIGAI

namespace IKIGAI {
namespace CORE {
class Core;
}
} // namespace IKIGAI
#ifdef OCULUS
#include "util_egl.h"
#include "util_oxr.h"
#endif
namespace IKIGAI::RENDER {
class DriverDx12;
class TextureGl;
class FrameBufferGl;
class DriverGl;

enum class DrawContent { FORWARD, DEFERRED, GUI, QUAD };

struct PipelineStage {
  using UniformType =
      std::variant<float, int, bool, MATH::Vector2f, MATH::Vector3f,
                   MATH::Vector4f, std::shared_ptr<TextureInterface>>;

  struct Descriptor {
    using UniformTypeDescr =
        std::variant<float, int, bool, MATH::Vector2f, MATH::Vector3f,
                     MATH::Vector4f, std::string>;
    std::string Name;
    DrawContent Draw;
    std::string Material;
    std::string FrameBuffer;
    std::map<std::string, UniformTypeDescr> Uniforms;
    std::map<std::string, std::string> BufferLinks; // ShaderBufferName -> Pipeline/GlobalBufferName
    std::map<std::string, std::map<std::string, UniformTypeDescr>> BufferOverrides; // BufferName -> MemberName -> Value

    template <class Context>
    constexpr static auto serde(Context &context, Descriptor &value) {
      using Self = Descriptor;
      using namespace serde::attribute;
      serde::serde_struct(context, value)
          .field(&Self::Name, "Name")
          .field(&Self::Draw, "Draw")
          .field(&Self::Material, "Material")
          .field(&Self::FrameBuffer, "FrameBuffer", default_{""})
          .field(&Self::Uniforms, "Uniforms")
          .field(&Self::BufferLinks, "BufferLinks")
          .field(&Self::BufferOverrides, "BufferOverrides");
    }
  };
  PipelineStage() = default;
  // PipelineStage(const Descriptor& descriptor) {
  //
  // }

  std::string mName;
  std::shared_ptr<FrameBufferInterface> mFrameBuffer;
  std::shared_ptr<MaterialInterface> mMaterial;
  DrawContent mDrawContent;
  std::map<std::string, UniformType> mUniforms;
  std::map<std::string, std::string> mBufferLinks; 
  std::map<std::string, std::map<std::string, UniformType>> mBufferOverrides;
};

struct RenderGraphPipeline {
  struct TextDescr {
    std::string Name;
    std::string TexturePath;
    int Width = 0;
    int Height = 0;

    template <class Context>
    constexpr static auto serde(Context &context, TextDescr &value) {
      using Self = TextDescr;
      using namespace serde::attribute;
      serde::serde_struct(context, value)
          .field(&Self::Name, "Name")
          .field(&Self::TexturePath, "TexturePath", default_{""})
          .field(&Self::Height, "Height", default_{0})
          .field(&Self::Width, "Width", default_{0});
    }
  };

  struct FrameDesc {
    std::string Name;
    std::vector<std::string> Textures;
    std::string Depth;

    template <class Context>
    constexpr static auto serde(Context &context, FrameDesc &value) {
      using Self = FrameDesc;
      using namespace serde::attribute;
      serde::serde_struct(context, value)
          .field(&Self::Name, "Name")
          .field(&Self::Textures, "Textures")
          .field(&Self::Depth, "Depth", default_{""});
    }
  };

  enum class BufferType { UNIFORM, STORAGE };

  struct BufferDescriptor {
    std::string Name;
    size_t Size = 0;
    size_t Stride = 0; // Needed for SSBO
    BufferType Type; // UNIFORM or STORAGE
    // Optional: initial data?

    template <class Context>
    constexpr static auto serde(Context& context, BufferDescriptor& value) {
      using Self = BufferDescriptor;
      using namespace serde::attribute;
      serde::serde_struct(context, value)
        .field(&Self::Name, "Name")
        .field(&Self::Size, "Size", default_{0})
        .field(&Self::Stride, "Stride", default_{0})
        .field(&Self::Type, "Type", default_{BufferType::UNIFORM});
    }
  };

  struct Descriptor {
    std::string Name;
    std::vector<TextDescr> Textures;
    std::vector<FrameDesc> FrameBuffers;
    std::vector<BufferDescriptor> Buffers;
    std::vector<PipelineStage::Descriptor> StartStages;
    std::vector<PipelineStage::Descriptor> Stages;

    template <class Context>
    constexpr static auto serde(Context& context, Descriptor& value) {
      using Self = Descriptor;
      using namespace serde::attribute;
      serde::serde_struct(context, value)
        .field(&Self::Name, "Name")
        .field(&Self::Textures, "Textures")
        .field(&Self::FrameBuffers, "FrameBuffers")
        .field(&Self::Buffers, "Buffers")
        .field(&Self::Stages, "Stages")
        .field(&Self::StartStages, "StartStages");
    }
  };

  RenderGraphPipeline() = default;
  RenderGraphPipeline(const Descriptor& descriptor, Renderer& renderer);

  std::map<std::string, std::shared_ptr<FrameBufferInterface>> mFrameBuffers;
  std::map<std::string, std::shared_ptr<TextureInterface>> mTextures;
  std::map<std::string, std::shared_ptr<StorageBufferInterface>> mStorageBuffers;
  std::map<std::string, std::shared_ptr<UniformBufferInterface>> mUniformBuffers;

  std::list<std::unique_ptr<PipelineStage>> mStartStages;
  std::list<std::unique_ptr<PipelineStage>> mStages;
  
  bool mIsInitialized = false;

  void run(GameRendererGl& renderer);
};

class GameRendererGl : public GameRendererInterface {
  friend struct RenderGraphPipeline;
  std::unique_ptr<RenderGraphPipeline> mRenderPipeline;

  IKIGAI::CORE::Core &mContext;
  // DriverVk* mDriver;
  // DriverDx12* mDriver;
  DriverGl *mDriver;
  unsigned long long frameCount = 0ull;

  // std::shared_ptr<FrameBufferGl> mainFb;
  // std::shared_ptr<TextureGl> sceneTexture;

  void createShaders();
  void createFrameBuffers();

  void renderScene(IKIGAI::ECS::CameraComponent *mainCameraComponent);
  // void sendBounseDataToShader(std::shared_ptr<MaterialGl> material,
  // ECS::Skeletal& animator,
  //                             std::shared_ptr<ShaderGl> shader);
  void drawDrawable(const Drawable &p_toDraw,
                    IKIGAI::ECS::CameraComponent *mainCameraComponent);
  // void renderPipeline(UTILS::Ref<IKIGAI::ECS::CameraComponent>
  // mainCameraComponent);

public:
  GameRendererGl(CORE::Core &context);

  void renderScene() override;

#ifdef OCULUS
  void renderSceneOculus(XrCompositionLayerProjectionView &layerView,
                         render_target_t &rtarget, XrPosef &stagePose,
                         uint32_t viewID);
#endif
  // void renderToScreen();
  // void drawGUISubtree(UTILS::Ref<ECS::Object> obj);
  // void drawGUI();
};
} // namespace IKIGAI::RENDER