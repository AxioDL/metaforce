#include <Runtime/GameGlobalObjects.hpp>
#include "CFluidPlane.hpp"
#include "CSimplePool.hpp"
#include "CRipple.hpp"
#include "CScriptWater.hpp"
#include "CStateManager.hpp"

namespace urde
{

CFluidPlane::CFluidPlane(CAssetId texPattern1, CAssetId texPattern2, CAssetId texColor, float alpha,
                         EFluidType fluidType, float rippleIntensity, const CFluidUVMotion& motion)
: x4_texPattern1Id(texPattern1), x8_texPattern2Id(texPattern2), xc_texColorId(texColor), x40_alpha(alpha),
  x44_fluidType(fluidType), x48_rippleIntensity(rippleIntensity), x4c_uvMotion(motion)
{
    if (g_ResFactory->GetResourceTypeById(texPattern1) == FOURCC('TXTR'))
        x10_texPattern1.emplace(g_SimplePool->GetObj(SObjectTag{FOURCC('TXTR'), texPattern1}));
    if (g_ResFactory->GetResourceTypeById(texPattern2) == FOURCC('TXTR'))
        x20_texPattern2.emplace(g_SimplePool->GetObj(SObjectTag{FOURCC('TXTR'), texPattern2}));
    if (g_ResFactory->GetResourceTypeById(texColor) == FOURCC('TXTR'))
        x30_texColor.emplace(g_SimplePool->GetObj(SObjectTag{FOURCC('TXTR'), texColor}));
}

float CFluidPlane::ProjectRippleVelocity(float baseI, float velDot) const
{
    float tmp = 0.5f * baseI * velDot * velDot;
    if (tmp != 0.f)
        tmp = std::sqrt(tmp);
    if (tmp >= 160.f)
        return 1.f;
    return tmp / 160.f;
}

float CFluidPlane::CalculateRippleIntensity(float baseI) const
{
    float mul;
    switch (x44_fluidType)
    {
    case EFluidType::NormalWater:
        mul = g_tweakGame->GetRippleIntensityNormal();
        break;
    case EFluidType::PoisonWater:
        mul = g_tweakGame->GetRippleIntensityPoison();
        break;
    case EFluidType::Lava:
        mul = g_tweakGame->GetRippleIntensityLava();
        break;
    case EFluidType::Three:
    case EFluidType::Four:
        mul = 0.8f;
        break;
    case EFluidType::Five:
        mul = 1.f;
        break;
    }

    return zeus::clamp(0.f, baseI * mul * (1.f - x48_rippleIntensity + 0.5f), 1.f);
}

void CFluidPlane::AddRipple(float mag, TUniqueId rippler, const zeus::CVector3f& center,
                            CScriptWater& water, CStateManager& mgr)
{
    if (!water.CanRippleAtPoint(center))
        return;

    mag = CalculateRippleIntensity(mag);
    mgr.GetFluidPlaneManager()->RippleManager().AddRipple(CRipple(rippler, center, mag));
}

void CFluidPlane::AddRipple(float intensity, TUniqueId rippler, const zeus::CVector3f& center,
                            const zeus::CVector3f& velocity, const CScriptWater& water, CStateManager& mgr,
                            const zeus::CVector3f& upVec)
{
    if (!water.CanRippleAtPoint(center))
        return;

    intensity = CalculateRippleIntensity(ProjectRippleVelocity(intensity, upVec.dot(velocity)));
    mgr.GetFluidPlaneManager()->RippleManager().AddRipple(CRipple(rippler, center, intensity));
}

void CFluidPlane::AddRipple(const CRipple& ripple, const CScriptWater& water, CStateManager& mgr)
{
    if (!water.CanRippleAtPoint(ripple.GetCenter()))
        return;
    mgr.GetFluidPlaneManager()->RippleManager().AddRipple(ripple);
}
}
