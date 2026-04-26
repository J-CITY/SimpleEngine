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
enum class ResourceType { SHADER, TEXTURE, MODEL, AUDIO, MATERIAL };

struct TextureResource {
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

struct ShaderResource {
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
        .field(&Self::paths, "Paths");
  }

  static auto GetMembers() {
    return std::tuple{
        IKIGAI::UTILS::MakeMemberInfo("UseBinary", &ShaderResource::useBinary),
        IKIGAI::UTILS::MakeMemberInfo("Paths", &ShaderResource::paths),
    };
  }
};

struct ModelResource {
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

struct MaterialResource {
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
