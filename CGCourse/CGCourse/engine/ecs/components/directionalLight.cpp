#include "directionalLight.h"
#include "../object.h"

using namespace KUMA::ECS;

DirectionalLight::DirectionalLight(const ECS::Object& obj) : LightComponent(obj) {
	__NAME__ = "DirectionalLight";
	data.type = RENDER::Light::Type::DIRECTIONAL;

    shadowMap = std::make_shared<RESOURCES::Texture>();

    const unsigned int SHADOW_WIDTH = 1024, SHADOW_HEIGHT = 1024;
    shadowMap->LoadDepth(SHADOW_WIDTH, SHADOW_HEIGHT);
}

KUMA::MATHGL::Matrix4 DirectionalLight::GetMatrix(const KUMA::MATHGL::Vector3& center, size_t index) const {
    KUMA::MATHGL::Vector3 Center = center;
    float distance = 0.0f;
    for (size_t i = 0; i < index; i++) {
        distance += Projections[i + 1] - Projections[i];
    }
    // Center -= distance * this->CascadeDirection;

    constexpr auto floor = [](const KUMA::MATHGL::Vector3& v) -> KUMA::MATHGL::Vector3 {
        return {std::floor(v.x), std::floor(v.y), std::floor(v.z)};
    };

    MATHGL::Matrix4 LightView = MATHGL::Matrix4::CreateView(
        KUMA::MATHGL::Vector3::Normalize(obj.transform->getWorldForward()),
        KUMA::MATHGL::Vector3(0.0f, 0.0f, 0.0f),
        KUMA::MATHGL::Vector3(0.001f, 1.0f, 0.001f)
    );

    MATHGL::Matrix3 LightViewM3 = {
        LightView(0, 0), LightView(0, 1), LightView(0, 2),
        LightView(1, 0), LightView(1, 1), LightView(1, 2),
        LightView(2, 0), LightView(2, 1), LightView(2, 2)
    };
	
    Center = LightViewM3 * Center;

    auto Low = KUMA::MATHGL::Vector3(-Projections[index], -Projections[index], -Projections[index]) + Center;
    auto High = KUMA::MATHGL::Vector3(Projections[index], Projections[index], Projections[index]) + Center;

    auto shadowMapSize = float(shadowMap->height + 1);
    auto worldUnitsPerText = (High - Low) / shadowMapSize;
    auto dummy = MATHGL::Vector3(std::floor(Low.x / worldUnitsPerText.x), std::floor(Low.y / worldUnitsPerText.y), std::floor(Low.z / worldUnitsPerText.z));
	Low = MATHGL::Vector3(dummy.x * worldUnitsPerText.x, dummy.y * worldUnitsPerText.y, dummy.z * worldUnitsPerText.z);
    dummy = KUMA::MATHGL::Vector3(High.x * worldUnitsPerText.x, High.y * worldUnitsPerText.y, High.z * worldUnitsPerText.z);
    High = KUMA::MATHGL::Vector3(dummy.x * worldUnitsPerText.x, dummy.y * worldUnitsPerText.y, dummy.z * worldUnitsPerText.z);
    Center = (High + Low) * -0.5f;

    MATHGL::Matrix4 orthoProjection = MATHGL::Matrix4::CreateOrthographic(std::fabs(Low.x - High.x), std::fabs(Low.x - High.x)/ std::fabs(Low.y - High.y), Low.z, High.z);
    return orthoProjection * LightView;
}
