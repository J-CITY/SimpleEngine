#pragma once
#include <optional>
#include <serdepp/attribute/make_optional.hpp>
#include <serdepp/serializer.hpp>
#include <string>
#include <vector>

#include "mathModule/math.h"
#include "reflectionStructs.h"
#include "renderEnums.h"
#include "resourceModule/parser/parseFlags.h"
#include "textureInterface.h"
#include "utilsModule/reflection/reflection.h"
// #include "coreModule/resourceManager/parser/assimpParser.h"

namespace IKIGAI::RENDER {
enum class ResourceType {
  SHADER, TEXTURE, MODEL,
  AUDIO, MATERIAL, SKELETON_ANIMATION,
  SKELETON, SKELETON_BLENDSPACE_1D, SKELETON_BLENDSPACE_2D,
  SKELETON_STATE_GRAPH
};

struct ResourceBase {
    std::string parentPath;
};

struct TextureResource: public ResourceBase {
  ResourceType type = ResourceType::TEXTURE;
  std::string path;
  // bool needFileWatch = false;

  TextureType texType = TextureType::TEXTURE_2D;

  std::vector<std::string> pathTexture;
  std::vector<uint8_t> colorData;

  int width = 0;
  int height = 0;
  int depth = 0;
  int channels = 0;
  PixelFormat pixelType = PixelFormat::RGBA_INT;

  MinMagFilter minFilter = MinMagFilter::LINEAR;
  MinMagFilter magFilter = MinMagFilter::LINEAR;

  WrapFilter wrapS = WrapFilter::CLAMP_TO_EDGE;
  WrapFilter wrapT = WrapFilter::CLAMP_TO_EDGE;
  WrapFilter wrapR = WrapFilter::CLAMP_TO_EDGE;

  bool isFloat = false;
  bool useMipmap = true;
  int mipMapCount = 0;

  template <class Context>
  constexpr static auto serde(Context &context, TextureResource &value) {
    using Self = TextureResource;
    using namespace serde::attribute;
    serde::serde_struct(context, value)
        //.field(&Self::needFileWatch, "NeedFileWatch", default_{true})
        .field(&Self::texType, "TexType", default_{TextureType::TEXTURE_2D})
        .field(&Self::minFilter, "TexType", default_{MinMagFilter::LINEAR})
        .field(&Self::magFilter, "TexType", default_{MinMagFilter::LINEAR})
        .field(&Self::wrapS, "TexType", default_{WrapFilter::CLAMP_TO_EDGE})
        .field(&Self::wrapT, "TexType", default_{WrapFilter::CLAMP_TO_EDGE})
        .field(&Self::wrapR, "TexType", default_{WrapFilter::CLAMP_TO_EDGE})
        .field(&Self::pathTexture, "PathTexture",
               default_{std::vector<std::string>()})
        .field(&Self::colorData, "ColorData", default_{std::vector<uint8_t>()})
        .field(&Self::width, "Width", default_{0})
        .field(&Self::height, "Height", default_{0})
        .field(&Self::depth, "Depth", default_{0})
        .field(&Self::channels, "Channels", default_{0})
        .field(&Self::pixelType, "PixelType", default_{PixelFormat::RGBA_INT})
        .field(&Self::isFloat, "IsFloat", default_{false})
        .field(&Self::parentPath, "##parent", default_{std::string()})
        .field(&Self::useMipmap, "UseMipmap", default_{true});
  }
  static auto GetMembers() {
    return std::tuple{
        // IKIGAI::UTILS::MakeMemberInfo("NeedFileWatch",
        // &TextureResource::needFileWatch),
        IKIGAI::UTILS::MakeMemberInfo("TexType", &TextureResource::texType),
        IKIGAI::UTILS::MakeMemberInfo("PathTexture",
                                      &TextureResource::pathTexture),
        IKIGAI::UTILS::MakeMemberInfo("ColorData", &TextureResource::colorData),
        IKIGAI::UTILS::MakeMemberInfo("Width", &TextureResource::width),
        IKIGAI::UTILS::MakeMemberInfo("Height", &TextureResource::height),
        IKIGAI::UTILS::MakeMemberInfo("Depth", &TextureResource::depth),
        IKIGAI::UTILS::MakeMemberInfo("Channels", &TextureResource::channels),
        IKIGAI::UTILS::MakeMemberInfo("PixelType", &TextureResource::pixelType),
        IKIGAI::UTILS::MakeMemberInfo("IsFloat", &TextureResource::isFloat),
        IKIGAI::UTILS::MakeMemberInfo("UseMipmap", &TextureResource::useMipmap),
        IKIGAI::UTILS::MakeMemberInfo("WrapS", &TextureResource::wrapS),
        IKIGAI::UTILS::MakeMemberInfo("WrapT", &TextureResource::wrapT),
        IKIGAI::UTILS::MakeMemberInfo("WrapR", &TextureResource::wrapR),
        IKIGAI::UTILS::MakeMemberInfo("MinFilter", &TextureResource::minFilter),
        IKIGAI::UTILS::MakeMemberInfo("MagFilter", &TextureResource::magFilter),
    };
  }
};

struct ShaderResource : public ResourceBase {
  ResourceType type = ResourceType::SHADER;
  std::string path;

  bool useBinary = false;

  std::map<std::string, std::string> paths;

  inline static std::map<ShaderType, std::string> toStr{
      {ShaderType::VERTEX, "VERTEX"},
      {ShaderType::FRAGMENT, "FRAGMENT"},
      {ShaderType::GEOMETRY, "GEOMETRY"},
      {ShaderType::TESSELLATION_CONTROL, "TESSELLATION_CONTROL"},
      {ShaderType::TESSELLATION_EVALUATION, "TESSELLATION_EVALUATION"},
      {ShaderType::COMPUTE, "COMPUTE"},
  };

  inline static std::map<std::string, ShaderType> toEnum{
      {"VERTEX", ShaderType::VERTEX},
      {"FRAGMENT", ShaderType::FRAGMENT},
      {"GEOMETRY", ShaderType::GEOMETRY},
      {"TESSELLATION_CONTROL", ShaderType::TESSELLATION_CONTROL},
      {"TESSELLATION_EVALUATION", ShaderType::TESSELLATION_EVALUATION},
      {"COMPUTE", ShaderType::COMPUTE},
  };
  bool hasShader(ShaderType type) const { return paths.contains(toStr[type]); }

  void setPaths(std::map<ShaderType, std::string> ps) {
    paths.clear();
    for (auto &[e, v] : ps) {
      paths[toStr[e]] = v;
    }
  }

  std::map<ShaderType, std::string> getPaths() const {
    std::map<ShaderType, std::string> res;
    for (auto &[e, v] : paths) {
      res[toEnum[e]] = v;
    }
    return res;
  }

  const std::string &getShaderPath(ShaderType type) const {
    return paths.at(toStr[type]);
  }

  mutable std::map<ShaderType, std::string> sources;
  mutable std::map<ShaderType, std::vector<uint32_t>> spirvSources;

  template <class Context>
  constexpr static auto serde(Context &context, ShaderResource &value) {
    using Self = ShaderResource;
    using namespace serde::attribute;
    serde::serde_struct(context, value)
        .field(&Self::useBinary, "UseBinary", default_{false})
  	    .field(&Self::parentPath, "##parent", default_{std::string()})
        .field(&Self::paths, "Paths");
  }

  static auto GetMembers() {
    return std::tuple{
        IKIGAI::UTILS::MakeMemberInfo("UseBinary", &ShaderResource::useBinary),
        IKIGAI::UTILS::MakeMemberInfo("Paths", &ShaderResource::paths),
    };
  }
};

struct ModelResource : public ResourceBase {
  ResourceType type = ResourceType::MODEL;
  std::string path;
  bool needFileWatch = false;

  std::string pathModel;
  std::vector<RESOURCES::ModelParserFlags> flags;

  template <class Context>
  constexpr static auto serde(Context &context, ModelResource &value) {
    using Self = ModelResource;
    using namespace serde::attribute;
    serde::serde_struct(context, value)
        .field(&Self::needFileWatch, "NeedFileWatch", default_{true})
        .field(&Self::pathModel, "PathModel")
        .field(&Self::parentPath, "##parent", default_{std::string()})
        .field(&Self::flags, "Flags",
               default_{std::vector<RESOURCES::ModelParserFlags>()});
  }
  static auto GetMembers() {
    return std::tuple{
        IKIGAI::UTILS::MakeMemberInfo("NeedFileWatch",
                                      &ModelResource::needFileWatch),
        IKIGAI::UTILS::MakeMemberInfo("PathModel", &ModelResource::pathModel),
        IKIGAI::UTILS::MakeMemberInfo("Flags", &ModelResource::flags),
    };
  }
};

struct SkeletonResource : public ResourceBase {
  ResourceType type = ResourceType::SKELETON;
  std::string path;

  std::string pathSkeleton;

  template <class Context>
  constexpr static auto serde(Context &context, SkeletonResource &value) {
    using Self = SkeletonResource;
    using namespace serde::attribute;
    serde::serde_struct(context, value)
        .field(&Self::pathSkeleton, "PathSkeleton")
        .field(&Self::parentPath, "##parent", default_{std::string()});
  }
  static auto GetMembers() {
    return std::tuple{
        IKIGAI::UTILS::MakeMemberInfo("PathSkeleton", &SkeletonResource::pathSkeleton),
    };
  }
};

struct AnimationResource : public ResourceBase {
  ResourceType type = ResourceType::SKELETON_ANIMATION;
  std::string path;

  std::string pathAnimation;
  bool additive = false;
  std::string additiveReference;
  std::string skeletonPath;

  template <class Context>
  constexpr static auto serde(Context &context, AnimationResource &value) {
    using Self = AnimationResource;
    using namespace serde::attribute;
    serde::serde_struct(context, value)
        .field(&Self::pathAnimation, "PathAnimation")
        .field(&Self::parentPath, "##parent", default_{std::string()})
        .field(&Self::additive, "Additive", default_{false})
        .field(&Self::additiveReference, "AdditiveReference", default_{std::string()})
		.field(&Self::skeletonPath, "SkeletonPath", default_{std::string()});
  }
  static auto GetMembers() {
    return std::tuple{
        IKIGAI::UTILS::MakeMemberInfo("PathAnimation", &AnimationResource::pathAnimation),
        IKIGAI::UTILS::MakeMemberInfo("Additive", &AnimationResource::additive),
        IKIGAI::UTILS::MakeMemberInfo("AdditiveReference", &AnimationResource::additiveReference),
    };
  }
};

//struct SkeletonBlendspace1D : public ResourceBase {
//  ResourceType type = ResourceType::SKELETON_BLENDSPACE_1D;
//  std::string path;
//
//  std::string skeletonPath;
//
//  using AnimationPath = std::string;
//  using Value = float;
//  std::vector<std::pair<Value, AnimationPath>> nodes;
//  template <class Context>
//  constexpr static auto serde(Context &context, SkeletonBlendspace1D &value) {
//    using Self = SkeletonBlendspace1D;
//    using namespace serde::attribute;
//    serde::serde_struct(context, value)
//        .field(&Self::skeletonPath, "SkeletonPath")
//        .field(&Self::parentPath, "##parent", default_{std::string()})
//        .field(&Self::nodes, "Nodes");
//  }
//  static auto GetMembers() {
//    return std::tuple{
//        IKIGAI::UTILS::MakeMemberInfo("SkeletonPath", &SkeletonBlendspace1D::skeletonPath),
//        IKIGAI::UTILS::MakeMemberInfo("Nodes", &SkeletonBlendspace1D::nodes),
//    };
//  }
//};

struct SkeletonBlendspace : public ResourceBase {
  ResourceType type = ResourceType::SKELETON_BLENDSPACE_2D;
  std::string path;

  std::string skeletonPath;

  using AnimationPath = std::string;
  using Value = float;
  struct Row {
    Value value;
    std::vector<std::pair<Value, AnimationPath>> nodes;

    template <class Context>
    constexpr static auto serde(Context &context, Row &v) {
      using namespace serde::attribute;
      serde::serde_struct(context, v)
          .field(&Row::value, "Value")
          .field(&Row::nodes, "Nodes");
    }
  };
  std::vector<Row> nodes;

  template <class Context>
  constexpr static auto serde(Context &context, SkeletonBlendspace &value) {
    using Self = SkeletonBlendspace;
    using namespace serde::attribute;
    serde::serde_struct(context, value)
        .field(&Self::skeletonPath, "SkeletonPath")
        .field(&Self::parentPath, "##parent", default_{std::string()})
        .field(&Self::nodes, "Nodes");
  }
  static auto GetMembers() {
    return std::tuple{
        IKIGAI::UTILS::MakeMemberInfo("SkeletonPath", &SkeletonBlendspace::skeletonPath),
        IKIGAI::UTILS::MakeMemberInfo("Nodes", &SkeletonBlendspace::nodes),
    };
  }
};

enum class ConditionOp { EQUAL, NOT_EQUAL, LESS, GREATER, LESS_EQUAL, GREATER_EQUAL };

enum class ConditionType { COMPARE, LOGICAL_AND, LOGICAL_OR, LOGICAL_NOT, SCRIPT };

struct ConditionVar {
    ConditionType type = ConditionType::COMPARE;

    // For COMPARE
    std::string varLeft;
    ConditionOp op = ConditionOp::EQUAL;
    bool isRightVar = false;
    std::string varRight;
    float constRight = 0.0f;

    // For LOGICAL (AND, OR, NOT)
    std::vector<ConditionVar> children;

    // For SCRIPT
    std::string scriptFunc;

    template <class Context>
    constexpr static auto serde(Context &context, ConditionVar &v) {
        using namespace serde::attribute;
        serde::serde_struct(context, v)
            .field(&ConditionVar::type, "Type", default_{ConditionType::COMPARE})
            .field(&ConditionVar::varLeft, "VarLeft", default_{std::string()})
            .field(&ConditionVar::op, "Op", default_{ConditionOp::EQUAL})
            .field(&ConditionVar::isRightVar, "IsRightVar", default_{false})
            .field(&ConditionVar::varRight, "VarRight", default_{std::string()})
            .field(&ConditionVar::constRight, "ConstRight", default_{0.0f})
            .field(&ConditionVar::children, "Children", default_{std::vector<ConditionVar>()})
            .field(&ConditionVar::scriptFunc, "ScriptFunc", default_{std::string()});
    }
};

struct AnimTransitionResource {
    std::string targetState;
    std::vector<ConditionVar> conditions;
    float blendTime;

    template <class Context>
    constexpr static auto serde(Context &context, AnimTransitionResource &v) {
        using namespace serde::attribute;
        serde::serde_struct(context, v)
            .field(&AnimTransitionResource::targetState, "TargetState")
            .field(&AnimTransitionResource::conditions, "Conditions", default_{std::vector<ConditionVar>()})
            .field(&AnimTransitionResource::blendTime, "BlendTime", default_{0.0f});
    }
};

enum class AnimStateType { ANIMATION, BLENDSPACE_1D, BLENDSPACE_2D };

struct AnimStateResource {
    std::string name;
    AnimStateType type;
    std::string resourcePath;
    
    std::string blendXVariable;
    std::string blendYVariable;

    std::vector<AnimTransitionResource> transitions;

    template <class Context>
    constexpr static auto serde(Context &context, AnimStateResource &v) {
        using namespace serde::attribute;
        serde::serde_struct(context, v)
            .field(&AnimStateResource::name, "Name")
            .field(&AnimStateResource::type, "Type")
            .field(&AnimStateResource::resourcePath, "ResourcePath")
            .field(&AnimStateResource::blendXVariable, "BlendXVariable", default_{std::string()})
            .field(&AnimStateResource::blendYVariable, "BlendYVariable", default_{std::string()})
            .field(&AnimStateResource::transitions, "Transitions", default_{std::vector<AnimTransitionResource>()});
    }
};

struct SkeletonStateGraphResource : public ResourceBase {
    ResourceType type = ResourceType::SKELETON_STATE_GRAPH;
    std::string path;

    std::string skeletonPath;
    std::string initialState;
    std::vector<AnimStateResource> states;

    template <class Context>
    constexpr static auto serde(Context &context, SkeletonStateGraphResource &value) {
        using Self = SkeletonStateGraphResource;
        using namespace serde::attribute;
        serde::serde_struct(context, value)
            .field(&Self::skeletonPath, "SkeletonPath")
            .field(&Self::initialState, "InitialState")
            .field(&Self::states, "States")
            .field(&Self::parentPath, "##parent", default_{std::string()});
    }

    static auto GetMembers() {
        return std::tuple{
            IKIGAI::UTILS::MakeMemberInfo("SkeletonPath", &SkeletonStateGraphResource::skeletonPath),
            IKIGAI::UTILS::MakeMemberInfo("InitialState", &SkeletonStateGraphResource::initialState),
            IKIGAI::UTILS::MakeMemberInfo("States", &SkeletonStateGraphResource::states),
        };
    }
};

struct MaterialResource : public ResourceBase {
  ResourceType Type = ResourceType::MATERIAL;
  std::string path;
  bool NeedFileWatch = false;

  std::string ShaderPath;
  bool Blendable = false;
  bool BackfaceCulling = true;
  bool FrontfaceCulling = false;
  bool DepthTest = true;
  bool DepthWriting = true;
  bool ColorWriting = true;
  unsigned GpuInstances = 1;
  bool IsDeferred = false;
  DepthFunction DepthFunc = DepthFunction::LESS;

  using UniformType =
      std::variant<float, int, bool, std::string, MATH::Vector4f,
                   MATH::Vector3f, MATH::Vector2f, MATH::Matrix4f,
                   MATH::Matrix3f>;

  std::map<std::string, UniformType> Uniforms;

  template <class Context>
  constexpr static auto serde(Context &context, MaterialResource &value) {
    using Self = MaterialResource;
    using namespace serde::attribute;
    serde::serde_struct(context, value)
        .field(&Self::NeedFileWatch, "NeedFileWatch", default_{true})
        .field(&Self::ShaderPath, "ShaderPath", default_{""})
        .field(&Self::Blendable, "Blendable", default_{false})
        .field(&Self::BackfaceCulling, "BackfaceCulling", default_{true})
        .field(&Self::FrontfaceCulling, "FrontfaceCulling", default_{false})
        .field(&Self::DepthTest, "DepthTest", default_{true})
        .field(&Self::DepthWriting, "DepthWriting", default_{true})
        .field(&Self::ColorWriting, "ColorWriting", default_{true})
        .field(&Self::GpuInstances, "GpuInstances", default_{1})
        .field(&Self::IsDeferred, "IsDeferred", default_{false})
        .field(&Self::DepthFunc, "DepthFunc", default_{DepthFunction::LESS})
        .field(&Self::parentPath, "##parent", default_{std::string()})
        .field(&Self::Uniforms, "Uniforms");
  }
  static auto GetMembers() {
    return std::tuple{
        IKIGAI::UTILS::MakeMemberInfo("NeedFileWatch",
                                      &MaterialResource::NeedFileWatch),
        IKIGAI::UTILS::MakeMemberInfo("ShaderPath",
                                      &MaterialResource::ShaderPath),
        IKIGAI::UTILS::MakeMemberInfo("Blendable",
                                      &MaterialResource::Blendable),
        IKIGAI::UTILS::MakeMemberInfo("BackfaceCulling",
                                      &MaterialResource::BackfaceCulling),
        IKIGAI::UTILS::MakeMemberInfo("FrontfaceCulling",
                                      &MaterialResource::FrontfaceCulling),
        IKIGAI::UTILS::MakeMemberInfo("DepthTest",
                                      &MaterialResource::DepthTest),
        IKIGAI::UTILS::MakeMemberInfo("DepthWriting",
                                      &MaterialResource::DepthWriting),
        IKIGAI::UTILS::MakeMemberInfo("ColorWriting",
                                      &MaterialResource::ColorWriting),
        IKIGAI::UTILS::MakeMemberInfo("GpuInstances",
                                      &MaterialResource::GpuInstances),
        IKIGAI::UTILS::MakeMemberInfo("IsDeferred",
                                      &MaterialResource::IsDeferred),
        IKIGAI::UTILS::MakeMemberInfo("DepthFunc",
                                      &MaterialResource::DepthFunc),
        IKIGAI::UTILS::MakeMemberInfo("Uniforms", &MaterialResource::Uniforms),
    };
  }
};

} // namespace IKIGAI::RENDER
