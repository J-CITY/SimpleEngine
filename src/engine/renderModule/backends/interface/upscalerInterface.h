#pragma once
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <memory>
#include <stdexcept>
#include <string>
#include "mathModule/math.h"
#include <serdepp/attribute/default.hpp>

namespace IKIGAI::RENDER {
class TextureInterface;
enum class UpscalerType { OFF, FSR, DLSS, AUTO };
enum class UpscalerQuality { QUALITY, BALANCED, PERFORMANCE };

struct UpscalerSettings {
    UpscalerType technology = UpscalerType::OFF;
    UpscalerQuality quality = UpscalerQuality::QUALITY;
    float sharpness = 0.0f;
    template<class Context> static constexpr auto serde(Context& context, UpscalerSettings& value) {
        using namespace serde::attribute;
        serde::serde_struct(context,value)
            .field(&UpscalerSettings::technology,"technology",default_{UpscalerType::OFF})
            .field(&UpscalerSettings::quality,"quality",default_{UpscalerQuality::QUALITY})
            .field(&UpscalerSettings::sharpness,"sharpness",default_{0.0f});
    }
    bool operator==(const UpscalerSettings&) const = default;
};

struct UpscalerCapabilities {
    bool Supported = false;
    bool Sharpening = false;
    UpscalerType Technology = UpscalerType::OFF;
    std::string Implementation, Reason;
};

struct UpscaleFrameInputs {
    std::shared_ptr<TextureInterface> Color, Depth, MotionVectors, Output;
    std::shared_ptr<TextureInterface> Exposure, Reactive, Transparency;
    MATH::Vector2i RenderSize{}, DisplaySize{};
    // Motion texture contains unjittered current UV minus previous UV, Y down.
    // Provider converts to previous minus current in render pixels.
    MATH::Vector2f Jitter{}; // Render pixels, X right and Y down.
    float Near = 0.1f, Far = 1000.0f, VerticalFov = 1.0f;
    float DeltaMilliseconds = 16.67f, PreExposure = 1.0f;
    float Sharpness = 0.0f;
    bool Reset = false;
};

class UnsupportedUpscaler : public std::runtime_error {
public: using std::runtime_error::runtime_error;
};

class UpscalerInterface {
public:
    virtual ~UpscalerInterface() = default;
    virtual UpscalerCapabilities capabilities() const = 0;
    virtual MATH::Vector2i renderSize(MATH::Vector2i display, UpscalerQuality quality) = 0;
    virtual void configure(MATH::Vector2i render, MATH::Vector2i display, UpscalerQuality quality) = 0;
    virtual MATH::Vector2f jitter(uint64_t frame, MATH::Vector2i render, MATH::Vector2i display) = 0;
    virtual void dispatch(const UpscaleFrameInputs&) = 0;
};

inline MATH::Vector2f UpscalerHaltonJitter(uint64_t frame, unsigned phases) {
    auto halton=[](unsigned index,unsigned base) {
        float f=1,r=0; while(index) { f/=base;r+=f*(index%base);index/=base; } return r;
    };
    auto index=static_cast<unsigned>(frame%std::max(1u,phases))+1;
    return {halton(index,2)-0.5f,halton(index,3)-0.5f};
}
inline void ValidateUpscaleInputs(const UpscaleFrameInputs& f) {
    if(!f.Color||!f.Depth||!f.MotionVectors||!f.Output || f.Color==f.Output)
        throw std::runtime_error("UPSCALE needs distinct color/output and depth/motion textures");
    if(f.RenderSize.x<=0||f.RenderSize.y<=0||f.DisplaySize.x<f.RenderSize.x||f.DisplaySize.y<f.RenderSize.y)
        throw std::runtime_error("Invalid UPSCALE render/display dimensions");
    if(!std::isfinite(f.Near)||!std::isfinite(f.Far)||f.Near<=0||f.Far<=f.Near ||
       !std::isfinite(f.VerticalFov)||f.VerticalFov<=0||f.VerticalFov>=3.141593f ||
       !std::isfinite(f.PreExposure)||f.PreExposure<=0||!std::isfinite(f.DeltaMilliseconds)||f.DeltaMilliseconds<0 ||
       !std::isfinite(f.Sharpness)||f.Sharpness<0||f.Sharpness>1 || !std::isfinite(f.Jitter.x)||!std::isfinite(f.Jitter.y))
        throw std::runtime_error("Invalid UPSCALE camera/exposure/frame parameters");
}
}
