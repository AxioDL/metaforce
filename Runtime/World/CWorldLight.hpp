#ifndef CWORLDLIGHT_HPP
#define CWORLDLIGHT_HPP

#include "Graphics/CLight.hpp"

namespace urde
{
class CWorldLight
{
    ELightType x0_type = ELightType::Custom;
    zeus::CVector3f x4_color;
    zeus::CVector3f x10_position;
    zeus::CVector3f x1c_direction;
    float x28_q = 0.f;
    float x2c_cutoffAngle = 0.f;
    float x30_ = 0.f;
    bool  x34_castShadows = false;
    float x38_ = 0.f;
    EFalloffType x3c_falloff = EFalloffType::Linear;
    float x40_ = 0.f;
public:
    CWorldLight(const CWorldLight&) = default;
    CWorldLight(CInputStream& in);
    ELightType GetLightType() const;
    const zeus::CVector3f& GetDirection() const;
    const zeus::CVector3f& GetPosition() const;
    bool DoesCastShadows() const;

    CLight GetAsCGraphicsLight() const;
};

}

#endif // CWORLDLIGHT_HPP
