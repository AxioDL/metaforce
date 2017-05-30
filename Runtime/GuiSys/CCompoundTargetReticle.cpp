#include "CCompoundTargetReticle.hpp"
#include "GameGlobalObjects.hpp"
#include "Camera/CGameCamera.hpp"
#include "CSimplePool.hpp"
#include "Graphics/CModel.hpp"
#include "CStateManager.hpp"
#include <string.h>

namespace urde
{
const CTargetReticleRenderState CTargetReticleRenderState::skZeroRenderState(kInvalidUniqueId, 1.f,
                                                                             zeus::CVector3f::skZero, 0.f, 1.f, true);

static float offshoot_func(float f1, float f2, float f3) { return (f1 * 0.5f) + zeus::fastSinF((f3 - 0.5f) * f2); }

static float calculate_premultiplied_overshoot_offset(float f1) { return 2.f * (M_PIF - std::asin(1.f / f1)); }

CCompoundTargetReticle::SOuterItemInfo::SOuterItemInfo(const char* res) : x0_(g_SimplePool->GetObj(res)) {}

CTargetReticleRenderState::CTargetReticleRenderState(TUniqueId target, float f1, const zeus::CVector3f& vec, float f2,
                                                     float f3, bool b1)
: x0_target(target), x4_(f1), x8_(vec), x14_(f2), x18_(f3), x1c_(b1)
{
}

CCompoundTargetReticle::CCompoundTargetReticle(const CStateManager& mgr)
: x0_(mgr.GetCameraManager()->GetCurrentCamera(mgr)->GetTransform().buildMatrix3f())
, x10_(mgr.GetCameraManager()->GetCurrentCamera(mgr)->GetTransform().buildMatrix3f())
, x2c_overshootOffsetHalf(0.5f * g_tweakTargeting->GetOvershootOffset())
, x30_premultOvershootOffset(calculate_premultiplied_overshoot_offset(g_tweakTargeting->GetOvershootOffset()))
, x34_crosshairs(g_SimplePool->GetObj(skCrosshairsReticleAssetName))
, x40_seeker(g_SimplePool->GetObj(skSeekerAssetName))
, x4c_lockConfirm(g_SimplePool->GetObj(skLockConfirmAssetName))
, x58_targetFlower(g_SimplePool->GetObj(skTargetFlowerAssetName))
, x64_missileBracket(g_SimplePool->GetObj(skMissileBracketAssetName))
, x70_innerBeamIcon(g_SimplePool->GetObj(skInnerBeamIconName))
, x7c_lockFire(g_SimplePool->GetObj(skLockFireAssetName))
, x88_lockDagger(g_SimplePool->GetObj(skLockDaggerAssetName))
, x94_grapple(g_SimplePool->GetObj(skGrappleReticleAssetName))
, xa0_chargeTickFirst(g_SimplePool->GetObj(skChargeBeamTickAssetName))
, xac_xrayRetRing(g_SimplePool->GetObj(skXRayRingModelName))
, xb8_thermalReticle(g_SimplePool->GetObj(skThermalReticleAssetName))
, xc4_chargeGauge(skChargeGaugeAssetName)
, xf4_(CalculateOrbitZoneReticlePosition(mgr))
, x100_(CalculateOrbitZoneReticlePosition(mgr))
, x208_(g_tweakTargeting->GetXD0())
{
    xe0_outerBeamIconSquares.reserve(9);
    for (u32 i = 0; i < 9; ++i)
    {
        char name[1024];
        sprintf(name, "%s%d", skOuterBeamIconSquareNameBase, i);
        xe0_outerBeamIconSquares.emplace_back(name);
    }
    x34_crosshairs.Lock();
}

zeus::CVector3f CCompoundTargetReticle::CalculateOrbitZoneReticlePosition(const CStateManager&) const { return {}; }

float CCompoundTargetReticle::CalculateClampedScale(const zeus::CVector3f& pos, float scale,
                                                    float clampMin, float clampMax,
                                                    const CStateManager& mgr)
{
    const CGameCamera* cam = mgr.GetCameraManager()->GetCurrentCamera(mgr);
    mgr.GetCameraManager()->GetCurrentCameraTransform(mgr);
    zeus::CVector3f viewPos = cam->GetTransform().transposeRotate(pos - cam->GetTransform().origin);
    float unclampedX = (cam->GetPerspectiveMatrix().multiplyOneOverW(viewPos + zeus::CVector3f(scale, 0.f, 0.f)).x -
                        cam->GetPerspectiveMatrix().multiplyOneOverW(viewPos).x) * 640.f;
    return zeus::clamp(clampMin, unclampedX, clampMax) / unclampedX * scale;
}

}
