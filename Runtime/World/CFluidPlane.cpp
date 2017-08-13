#include <Runtime/GameGlobalObjects.hpp>
#include "CFluidPlane.hpp"
#include "CSimplePool.hpp"

namespace urde
{

CFluidPlane::CFluidPlane(ResId texPattern1, ResId texPattern2, ResId texColor, float alpha, EFluidType fluidType,
                         float rippleIntensity, const CFluidUVMotion& motion)
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

void CFluidPlane::Ripple(float mag, TUniqueId rippler, const zeus::CVector3f& pos,
                         CScriptWater& water, CStateManager& mgr)
{

}

void CFluidPlane::Update()
{

}
}
