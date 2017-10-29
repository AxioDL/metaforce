#include "CCompoundTargetReticle.hpp"
#include "GameGlobalObjects.hpp"
#include "Camera/CGameCamera.hpp"
#include "CSimplePool.hpp"
#include "CStateManager.hpp"
#include "TCastTo.hpp"
#include "World/CScriptGrapplePoint.hpp"
#include "World/CPlayer.hpp"
#include "World/CWorld.hpp"

namespace urde
{
const CTargetReticleRenderState CTargetReticleRenderState::skZeroRenderState(kInvalidUniqueId, 1.f,
                                                                             zeus::CVector3f::skZero, 0.f, 1.f, true);

static float offshoot_func(float f1, float f2, float f3) { return (f1 * 0.5f) + zeus::fastSinF((f3 - 0.5f) * f2); }

static float calculate_premultiplied_overshoot_offset(float f1) { return 2.f * (M_PIF - std::asin(1.f / f1)); }

CCompoundTargetReticle::SOuterItemInfo::SOuterItemInfo(const char* res) : x0_model(g_SimplePool->GetObj(res)) {}

CCompoundTargetReticle::CCompoundTargetReticle(const CStateManager& mgr)
: x0_leadingOrientation(mgr.GetCameraManager()->GetCurrentCamera(mgr)->GetTransform().buildMatrix3f())
, x10_laggingOrientation(mgr.GetCameraManager()->GetCurrentCamera(mgr)->GetTransform().buildMatrix3f())
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
, xf4_targetPos(CalculateOrbitZoneReticlePosition(mgr, false))
, x100_laggingTargetPos(CalculateOrbitZoneReticlePosition(mgr, true))
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

CCompoundTargetReticle::SScanReticuleRenderer::SScanReticuleRenderer()
{
    m_token = CGraphics::CommitResources([this](boo::IGraphicsDataFactory::Context& ctx)
    {
        for (int i=0 ; i<2 ; ++i)
        {
            m_lineRenderers[i].emplace(ctx, CLineRenderer::EPrimitiveMode::Lines, 8, nullptr, true);
            for (int j=0 ; j<4 ; ++j)
                m_stripRenderers[i][j].emplace(ctx, CLineRenderer::EPrimitiveMode::LineStrip, 4, nullptr, true);
        }
        return true;
    });
}

CCompoundTargetReticle::EReticleState CCompoundTargetReticle::GetDesiredReticleState(const CStateManager& mgr) const
{
    switch (mgr.GetPlayerState()->GetCurrentVisor())
    {
    case CPlayerState::EPlayerVisor::Scan:
        return EReticleState::Scan;
    case CPlayerState::EPlayerVisor::XRay:
        return EReticleState::XRay;
    case CPlayerState::EPlayerVisor::Combat:
    default:
        return EReticleState::Combat;
    case CPlayerState::EPlayerVisor::Thermal:
        return EReticleState::Thermal;
    }
}

void CCompoundTargetReticle::Update(float dt, const CStateManager& mgr)
{
    float angle = x10_laggingOrientation.angleFrom(x0_leadingOrientation).asDegrees();
    float t;
    if (angle < 0.1f || angle > 45.f)
        t = 1.f;
    else
        t = std::min(1.f, g_tweakTargeting->x224_ * dt / angle);
    x10_laggingOrientation = t == 1.f ? x0_leadingOrientation :
                             zeus::CQuaternion::slerp(x10_laggingOrientation, x0_leadingOrientation, t);
    xf4_targetPos = CalculateOrbitZoneReticlePosition(mgr, false);
    x100_laggingTargetPos = CalculateOrbitZoneReticlePosition(mgr, true);
    UpdateCurrLockOnGroup(dt, mgr);
    UpdateNextLockOnGroup(dt, mgr);
    UpdateOrbitZoneGroup(dt, mgr);
    EReticleState desiredState = GetDesiredReticleState(mgr);
    if (desiredState != x20_prevState && x20_prevState == x24_nextState)
    {
        x24_nextState = desiredState;
        x28_noDrawTicks = 2;
    }
    if (x20_prevState != x24_nextState && x28_noDrawTicks <= 0)
    {
        x20_prevState = x24_nextState;
        float combat = false;
        float scan = false;
        float xray = false;
        float thermal = false;
        switch (x24_nextState)
        {
        case EReticleState::Combat:
            combat = true;
            break;
        case EReticleState::Scan:
            scan = true;
            break;
        case EReticleState::XRay:
            xray = true;
            break;
        case EReticleState::Thermal:
            thermal = true;
            break;
        default:
            break;
        }
        if (combat)
            x40_seeker.Lock();
        else
            x40_seeker.Unlock();
        if (combat)
            x4c_lockConfirm.Lock();
        else
            x4c_lockConfirm.Unlock();
        if (combat)
            x58_targetFlower.Lock();
        else
            x58_targetFlower.Unlock();
        if (combat)
            x64_missileBracket.Lock();
        else
            x64_missileBracket.Unlock();
        if (combat)
            x70_innerBeamIcon.Lock();
        else
            x70_innerBeamIcon.Unlock();
        if (combat)
            x7c_lockFire.Lock();
        else
            x7c_lockFire.Unlock();
        if (combat)
            x88_lockDagger.Lock();
        else
            x88_lockDagger.Unlock();
        if (combat)
            xa0_chargeTickFirst.Lock();
        else
            xa0_chargeTickFirst.Unlock();
        if (xray)
            xac_xrayRetRing.Lock();
        else
            xac_xrayRetRing.Unlock();
        if (thermal)
            xb8_thermalReticle.Lock();
        else
            xb8_thermalReticle.Unlock();
        if (combat)
            xc4_chargeGauge.x0_model.Lock();
        else
            xc4_chargeGauge.x0_model.Unlock();
        if (scan)
            x94_grapple.Lock();
        else
            x94_grapple.Unlock();
        for (SOuterItemInfo& info : xe0_outerBeamIconSquares)
        {
            if (combat)
                info.x0_model.Lock();
            else
                info.x0_model.Unlock();
        }
    }
    CPlayerGun* gun = mgr.GetPlayer().GetPlayerGun();
    bool fullyCharged = (gun->IsCharging() ? gun->GetChargeBeamFactor() : 0.f) >= 1.f;
    if (fullyCharged != x21a_fullyCharged)
        x21a_fullyCharged = fullyCharged;
    if (x21a_fullyCharged)
        x214_ = std::min(dt / g_tweakTargeting->x1b8_ + x214_, g_tweakTargeting->x0_);
    else
        x214_ = std::max(x214_ - dt / g_tweakTargeting->x1b8_, 0.f);
    bool missileActive = gun->GetMissleMode() == CPlayerGun::EMissleMode::Active;
    if (missileActive != x1f4_missileActive)
    {
        if (x1f8_ != 0.f)
            x1f8_ = FLT_EPSILON - x1f8_;
        else
            x1f8_ = FLT_EPSILON;
        x1f4_missileActive = missileActive;
    }
    CPlayerState::EBeamId beam = gun->GetCurrentBeam();
    if (beam != x200_beam)
    {
        x204_ = g_tweakTargeting->xec_;
        for (int i=0 ; i<9 ; ++i)
        {
            zeus::CRelAngle f1 = g_tweakTargeting->xf8_[int(beam)].floats[i];
            SOuterItemInfo& icon = xe0_outerBeamIconSquares[i];
            zeus::CRelAngle f7 = f1.asRadians() - icon.x10_;
            if ((i & 0x1) == 1)
                f7 = (f1 > 0.f) ? zeus::CRelAngle(-2.f * M_PIF - f1) : zeus::CRelAngle(2.f * M_PIF + f1);
            icon.xc_ = icon.x10_;
            icon.x18_ = f7;
            icon.x14_ = f1;
        }
        zeus::CRelAngle f30 = g_tweakTargeting->x108_[int(beam)];
        float f2 = f30.asRadians() - xc4_chargeGauge.x10_;
        if ((rand() & 0x1) == 1)
            f2 = (f2 > 0.f) ? -2.f * M_PIF - f2 : 2.f * M_PIF + f2;
        xc4_chargeGauge.xc_ = xc4_chargeGauge.x10_;
        xc4_chargeGauge.x18_ = f2;
        xc4_chargeGauge.x14_ = f30;
        x200_beam = beam;
        x208_ = 0.f;
    }
    if (gun->GetLastFireButtonStates() & 0x1)
    {
        if (!x218_beamShot)
            x210_ = g_tweakTargeting->x12c_;
        x218_beamShot = true;
    }
    else
    {
        x218_beamShot = false;
    }
    if (gun->GetLastFireButtonStates() & 0x2)
    {
        if (!x219_missileShot)
            x1fc_ = g_tweakTargeting->xc8_;
        x219_missileShot = true;
    }
    else
    {
        x219_missileShot = false;
    }
    if (TCastToConstPtr<CScriptGrapplePoint> point = mgr.GetObjectById(xf2_))
    {
        if (point->GetUniqueId() != x1dc_grapplePoint0)
        {
            float tmp;
            if (point->GetUniqueId() == x1de_grapplePoint1)
                tmp = std::max(FLT_EPSILON, x1e4_grapplePoint1T);
            else
                tmp = FLT_EPSILON;
            x1de_grapplePoint1 = x1dc_grapplePoint0;
            x1e4_grapplePoint1T = x1e0_grapplePoint0T;
            x1e0_grapplePoint0T = tmp;
            x1dc_grapplePoint0 = point->GetUniqueId();
        }
    }
    else if (x1dc_grapplePoint0 != kInvalidUniqueId)
    {
        x1de_grapplePoint1 = x1dc_grapplePoint0;
        x1e4_grapplePoint1T = x1e0_grapplePoint0T;
        x1e0_grapplePoint0T = 0.f;
        x1dc_grapplePoint0 = kInvalidUniqueId;
    }
    if (x1e0_grapplePoint0T > 0.f)
        x1e0_grapplePoint0T = std::min(dt / 0.5f + x1e0_grapplePoint0T, 1.f);
    if (x1e4_grapplePoint1T > 0.f)
    {
        x1e4_grapplePoint1T = std::max(0.f, x1e4_grapplePoint1T - dt / 0.5f);
        if (x1e4_grapplePoint1T == 0.f)
            x1de_grapplePoint1 = kInvalidUniqueId;
    }
    x1f0_xrayRetAngle = zeus::CRelAngle(zeus::degToRad(g_tweakTargeting->GetXRayRetAngleSpeed() * dt) + x1f0_xrayRetAngle);
    x1ec_seekerAngle = zeus::CRelAngle(zeus::degToRad(g_tweakTargeting->GetSeekerAngleSpeed() * dt) + x1ec_seekerAngle);
}

void CTargetReticleRenderState::InterpolateWithClamp(const CTargetReticleRenderState& a,
                                                     CTargetReticleRenderState& out,
                                                     const CTargetReticleRenderState& b, float t)
{
    t = zeus::clamp(0.f, t, 1.f);
    float omt = 1.f - t;
    out.x4_radiusWorld = omt * a.x4_radiusWorld + t * b.x4_radiusWorld;
    out.x14_factor = omt * a.x14_factor + t * b.x14_factor;
    out.x18_minVpClampScale = omt * a.x18_minVpClampScale + t * b.x18_minVpClampScale;
    out.x8_positionWorld = zeus::CVector3f::lerp(a.x8_positionWorld, b.x8_positionWorld, t);

    if (t == 1.f)
        out.x0_target = b.x0_target;
    else if (t == 0.f)
        out.x0_target = a.x0_target;
    else
        out.x0_target = kInvalidUniqueId;
}

void CCompoundTargetReticle::UpdateCurrLockOnGroup(float dt, const CStateManager& mgr)
{
    // TODO: Finish
}

void CCompoundTargetReticle::UpdateNextLockOnGroup(float dt, const CStateManager& mgr)
{
    // TODO: Finish
}

void CCompoundTargetReticle::UpdateOrbitZoneGroup(float dt, const CStateManager& mgr)
{
    // TODO: Finish
}

void CCompoundTargetReticle::Draw(const CStateManager& mgr, bool hideLockon) const
{
    if (mgr.GetPlayer().GetMorphballTransitionState() == CPlayer::EPlayerMorphBallState::Unmorphed &&
        !mgr.GetCameraManager()->IsInCinematicCamera())
    {
        zeus::CTransform camXf = mgr.GetCameraManager()->GetCurrentCameraTransform(mgr);
        CGraphics::SetViewPointMatrix(camXf);
        if (!hideLockon)
        {
            DrawCurrLockOnGroup(camXf.basis, mgr);
            DrawNextLockOnGroup(camXf.basis, mgr);
            DrawOrbitZoneGroup(camXf.basis, mgr);
        }
        DrawGrappleGroup(camXf.basis, mgr, hideLockon);
    }
    if (x28_noDrawTicks > 0)
        --x28_noDrawTicks;
}

void CCompoundTargetReticle::DrawGrapplePoint(const CScriptGrapplePoint& point, float t, const CStateManager& mgr,
                                              const zeus::CMatrix3f& rot, bool zEqual) const
{
    zeus::CVector3f orbitPos = point.GetOrbitPosition(mgr);
    zeus::CColor color;
    if (point.GetGrappleParameters().GetLockSwingTurn())
        color = g_tweakTargeting->GetLockedGrapplePointSelectColor();
    else
        color = g_tweakTargeting->GetGrapplePointSelectColor();
    color = zeus::CColor::lerp(color, g_tweakTargeting->GetGrapplePointColor(), t);
    zeus::CMatrix3f scale(
        CalculateClampedScale(orbitPos, 1.f, g_tweakTargeting->GetGrappleClampMin(),
                              g_tweakTargeting->GetGrappleClampMax(), mgr) *
            (1.f - t) * g_tweakTargeting->GetGrappleScale() + t * g_tweakTargeting->GetGrappleSelectScale());
    zeus::CTransform modelXf(scale, orbitPos);
    CGraphics::SetModelMatrix(modelXf);
    CModelFlags flags(7, 0, 0, color);
    x94_grapple->Draw(flags);
}

void CCompoundTargetReticle::DrawGrappleGroup(const zeus::CMatrix3f& rot,
                                              const CStateManager& mgr, bool hideLockon) const
{
    if (x28_noDrawTicks > 0)
        return;

    if (mgr.GetPlayerState()->HasPowerUp(CPlayerState::EItemType::GrappleBeam) &&
        x94_grapple.IsLoaded() && x20_prevState != EReticleState::Scan)
    {
        if (hideLockon)
        {
            for (const CEntity* ent : mgr.GetAllObjectList())
            {
                if (TCastToConstPtr<CScriptGrapplePoint> point = ent)
                {
                    if (point->GetActive())
                    {
                        if (point->GetAreaIdAlways() != kInvalidAreaId)
                        {
                            const CGameArea* area = mgr.GetWorld()->GetAreaAlways(point->GetAreaIdAlways());
                            CGameArea::EOcclusionState occState = area->IsPostConstructed() ?
                                                                  area->GetOcclusionState() :
                                                                  CGameArea::EOcclusionState::Occluded;
                            if (occState != CGameArea::EOcclusionState::Visible)
                                continue;
                        }
                        float t = 0.f;
                        if (point->GetUniqueId() == x1dc_grapplePoint0)
                            t = x1e0_grapplePoint0T;
                        else if (point->GetUniqueId() == x1de_grapplePoint1)
                            t = x1e4_grapplePoint1T;
                        if (std::fabs(t) < 0.00001f)
                            DrawGrapplePoint(*point, t, mgr, rot, true);
                    }
                }
            }
        }
        else
        {
            TCastToConstPtr<CScriptGrapplePoint> point0 = mgr.GetObjectById(x1dc_grapplePoint0);
            TCastToConstPtr<CScriptGrapplePoint> point1 = mgr.GetObjectById(x1de_grapplePoint1);
            for (int i=0 ; i<2 ; ++i)
            {
                const CScriptGrapplePoint* point = i == 0 ? point0.GetPtr() : point1.GetPtr();
                float t = i == 0 ? x1e0_grapplePoint0T : x1e4_grapplePoint1T;
                if (point)
                    DrawGrapplePoint(*point, t, mgr, rot, false);
            }
        }
    }
}

void CCompoundTargetReticle::DrawCurrLockOnGroup(const zeus::CMatrix3f& rot, const CStateManager& mgr) const
{
    if (x28_noDrawTicks > 0)
        return;

    // TODO: Finish
}

void CCompoundTargetReticle::DrawNextLockOnGroup(const zeus::CMatrix3f& rot, const CStateManager& mgr) const
{
    if (x28_noDrawTicks > 0)
        return;

    zeus::CVector3f x408 = x174_.GetTargetPositionWorld();
    float visorFactor = mgr.GetPlayerState()->GetVisorTransitionFactor();

    bool scanRet = false;
    bool xrayRet = false;
    bool thermalRet = false;
    switch (x20_prevState)
    {
    case EReticleState::Scan:
        scanRet = true;
        break;
    case EReticleState::XRay:
        xrayRet = true;
        break;
    case EReticleState::Thermal:
        thermalRet = true;
        break;
    default:
        break;
    }

    if (!xrayRet && x174_.GetFactor() > 0.f && x40_seeker.IsLoaded())
    {
        zeus::CMatrix3f scale(
            CalculateClampedScale(x408, x174_.GetRadiusWorld(),
                                  x174_.GetMinViewportClampScale() * g_tweakTargeting->GetSeekerClampMin(),
                                  g_tweakTargeting->GetSeekerClampMax(), mgr) * g_tweakTargeting->GetSeekerScale());
        zeus::CTransform modelXf(rot * zeus::CMatrix3f::RotateY(x1ec_seekerAngle) * scale,
                                 x174_.GetTargetPositionWorld());
        CGraphics::SetModelMatrix(modelXf);
        zeus::CColor color = g_tweakTargeting->GetSeekerColor();
        color.a *= x174_.GetFactor();
        CModelFlags flags(7, 0, 0, color);
        x40_seeker->Draw(flags);
    }

    if (xrayRet && xac_xrayRetRing.IsLoaded())
    {
        zeus::CMatrix3f scale(
            CalculateClampedScale(x408, x174_.GetRadiusWorld(),
                                  x174_.GetMinViewportClampScale() * g_tweakTargeting->GetReticuleClampMin(),
                                  g_tweakTargeting->GetReticuleClampMax(), mgr) *
                g_tweakTargeting->GetReticuleScale());
        zeus::CTransform modelXf(rot * scale * zeus::CMatrix3f::RotateY(x1f0_xrayRetAngle),
                                 x174_.GetTargetPositionWorld());
        CGraphics::SetModelMatrix(modelXf);
        zeus::CColor color = g_tweakTargeting->GetXRayRetRingColor();
        color.a *= visorFactor;
        CModelFlags flags(7, 0, 0, color);
        xac_xrayRetRing->Draw(flags);
    }

    if (thermalRet && xb8_thermalReticle.IsLoaded())
    {
        zeus::CMatrix3f scale(
            CalculateClampedScale(x408, x174_.GetRadiusWorld(),
                                  x174_.GetMinViewportClampScale() * g_tweakTargeting->GetReticuleClampMin(),
                                  g_tweakTargeting->GetReticuleClampMax(), mgr) *
            g_tweakTargeting->GetReticuleScale());
        zeus::CTransform modelXf(rot * scale, x174_.GetTargetPositionWorld());
        CGraphics::SetModelMatrix(modelXf);
        zeus::CColor color = g_tweakTargeting->GetThermalReticuleColor();
        color.a *= visorFactor;
        CModelFlags flags(7, 0, 0, color);
        xb8_thermalReticle->Draw(flags);
    }

    if (scanRet && visorFactor > 0.f)
    {
        float factor = visorFactor * x174_.GetFactor();
        zeus::CMatrix3f scale(
            CalculateClampedScale(x408, x174_.GetRadiusWorld(),
                                  x174_.GetMinViewportClampScale() * g_tweakTargeting->GetScanTargetClampMin(),
                                  g_tweakTargeting->GetScanTargetClampMax(), mgr) *
                (1.f / factor));
        zeus::CTransform modelXf(rot * scale, x174_.GetTargetPositionWorld());
        CGraphics::SetModelMatrix(modelXf);
        // compare, GX_LESS, no update
        float alpha = 0.5f * factor;
        zeus::CColor color = g_tweakGuiColors->GetScanReticuleColor();
        color.a *= alpha;
        for (int i=0 ; i<2 ; ++i)
        {
            float lineWidth = i ? 2.5f : 1.f;
            auto& rend = *m_scanRetRenderer.m_lineRenderers[i];
            rend.Reset();
            rend.AddVertex({-0.5f, 0.f, 0.f}, color, lineWidth);
            rend.AddVertex({-20.5f, 0.f, 0.f}, color, lineWidth);
            rend.AddVertex({0.5f, 0.f, 0.f}, color, lineWidth);
            rend.AddVertex({20.5f, 0.f, 0.f}, color, lineWidth);
            rend.AddVertex({0.f, 0.f, -0.5f}, color, lineWidth);
            rend.AddVertex({0.f, 0.f, -20.5f}, color, lineWidth);
            rend.AddVertex({0.f, 0.f, 0.5f}, color, lineWidth);
            rend.AddVertex({0.f, 0.f, 20.5f}, color, lineWidth);
            rend.Render();

            for (int j=0 ; j<4 ; ++j)
            {
                float xSign = j < 2 ? -1.f : 1.f;
                float zSign = (j & 0x1) ? -1.f : 1.f;
                // begin line strip
                auto& rend = *m_scanRetRenderer.m_stripRenderers[i][j];
                rend.Reset();
                rend.AddVertex({0.5f * xSign, 0.f, 0.1f * zSign}, color, lineWidth);
                rend.AddVertex({0.5f * xSign, 0.f, 0.35f * zSign}, color, lineWidth);
                rend.AddVertex({0.35f * xSign, 0.f, 0.5f * zSign}, color, lineWidth);
                rend.AddVertex({0.1f * xSign, 0.f, 0.5f * zSign}, color, lineWidth);
                rend.Render();
            }
        }
    }
}

void CCompoundTargetReticle::DrawOrbitZoneGroup(const zeus::CMatrix3f& rot, const CStateManager& mgr) const
{
    if (x28_noDrawTicks > 0)
        return;

    if (x1e8_crosshairsScale > 0.f && x34_crosshairs.IsLoaded())
    {
        CGraphics::SetModelMatrix(zeus::CTransform(rot, xf4_targetPos) * zeus::CTransform::Scale(x1e8_crosshairsScale));
        zeus::CColor color = g_tweakTargeting->GetCrosshairsColor();
        color.a *= x1e8_crosshairsScale;
        CModelFlags flags(7, 0, 0, color);
        x34_crosshairs->Draw(flags);
    }
}

void CCompoundTargetReticle::UpdateTargetParameters(CTargetReticleRenderState& state, const CStateManager& mgr)
{
    if (auto act = TCastToConstPtr<CActor>(mgr.GetAllObjectList().GetObjectById(state.GetTargetId())))
    {
        state.SetRadiusWorld(CalculateRadiusWorld(*act, mgr));
        state.SetTargetPositionWorld(CalculatePositionWorld(*act, mgr));
    }
    else if (state.GetIsOrbitZoneIdlePosition())
    {
        state.SetRadiusWorld(1.f);
        state.SetTargetPositionWorld((x20_prevState == EReticleState::XRay || x20_prevState == EReticleState::Thermal) ?
                                     x100_laggingTargetPos : xf4_targetPos);
    }
}

float CCompoundTargetReticle::CalculateRadiusWorld(const CActor& act, const CStateManager& mgr) const
{
    auto tb = act.GetTouchBounds();
    zeus::CAABox aabb = tb ? *tb : zeus::CAABox(act.GetAimPosition(mgr, 0.f), act.GetAimPosition(mgr, 0.f));

    float radius;
    zeus::CVector3f delta = aabb.max - aabb.min;
    switch (g_tweakTargeting->GetTargetRadiusMode())
    {
    case 0:
    {
        radius = std::min(delta.x, std::min(delta.y, delta.z)) * 0.5f;
        break;
    }
    case 1:
    {
        radius = std::max(delta.x, std::max(delta.y, delta.z)) * 0.5f;
        break;
    }
    default:
    {
        radius = (delta.x + delta.y + delta.z) / 6.f;
        break;
    }
    }

    return radius > 0.f ? radius : 1.f;
}

zeus::CVector3f CCompoundTargetReticle::CalculatePositionWorld(const CActor& act, const CStateManager& mgr) const
{
    if (x20_prevState == EReticleState::Scan)
        return act.GetOrbitPosition(mgr);
    return act.GetAimPosition(mgr, 0.f);
}

zeus::CVector3f
CCompoundTargetReticle::CalculateOrbitZoneReticlePosition(const CStateManager& mgr, bool lag) const
{
    const CGameCamera* curCam = mgr.GetCameraManager()->GetCurrentCamera(mgr);
    float distMul = 224.f / float(g_tweakPlayer->GetOrbitScreenBoxHalfExtentY(0)) /
        std::tan(zeus::degToRad(0.5f * curCam->GetFov()));
    zeus::CTransform camXf = mgr.GetCameraManager()->GetCurrentCameraTransform(mgr);
    zeus::CVector3f lookDir = camXf.basis[1];
    if (lag)
        lookDir = x10_laggingOrientation.transform(lookDir);
    return lookDir * distMul + camXf.origin;
}

bool CCompoundTargetReticle::IsGrappleTarget(TUniqueId uid, const CStateManager& mgr) const
{
    return TCastToConstPtr<CScriptGrapplePoint>(mgr.GetAllObjectList().GetObjectById(uid)).operator bool();
}

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

void CCompoundTargetReticle::Touch()
{
    if (x34_crosshairs.IsLoaded())
        x34_crosshairs->Touch(0);
    if (x40_seeker.IsLoaded())
        x40_seeker->Touch(0);
    if (x4c_lockConfirm.IsLoaded())
        x4c_lockConfirm->Touch(0);
    if (x58_targetFlower.IsLoaded())
        x58_targetFlower->Touch(0);
    if (x64_missileBracket.IsLoaded())
        x64_missileBracket->Touch(0);
    if (x70_innerBeamIcon.IsLoaded())
        x70_innerBeamIcon->Touch(0);
    if (x7c_lockFire.IsLoaded())
        x7c_lockFire->Touch(0);
    if (x88_lockDagger.IsLoaded())
        x88_lockDagger->Touch(0);
    if (x94_grapple.IsLoaded())
        x94_grapple->Touch(0);
    if (xa0_chargeTickFirst.IsLoaded())
        xa0_chargeTickFirst->Touch(0);
    if (xac_xrayRetRing.IsLoaded())
        xac_xrayRetRing->Touch(0);
    if (xb8_thermalReticle.IsLoaded())
        xb8_thermalReticle->Touch(0);
    if (xc4_chargeGauge.x0_model.IsLoaded())
        xc4_chargeGauge.x0_model->Touch(0);
    for (SOuterItemInfo& info : xe0_outerBeamIconSquares)
        if (info.x0_model.IsLoaded())
            info.x0_model->Touch(0);
}

}
