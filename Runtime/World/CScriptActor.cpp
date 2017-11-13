#include "CScriptActor.hpp"
#include "CStateManager.hpp"
#include "CScriptTrigger.hpp"
#include "Camera/CGameCamera.hpp"
#include "CDamageVulnerability.hpp"
#include "CPlayerState.hpp"
#include "CScriptColorModulate.hpp"
#include "Character/IAnimReader.hpp"
#include "TCastTo.hpp"

namespace urde
{

CScriptActor::CScriptActor(TUniqueId uid, std::string_view name, const CEntityInfo& info, const zeus::CTransform& xf,
                           CModelData&& mData, const zeus::CAABox& aabb, float mass, float zMomentum,
                           const CMaterialList& matList, const CHealthInfo& hInfo, const CDamageVulnerability& dVuln,
                           const CActorParameters& actParms, bool looping, bool active, u32 w1, float f3, bool b2,
                           bool castsShadow, bool b4, bool b5)
: CPhysicsActor(uid, active, name, info, xf, std::move(mData), matList, aabb, SMoverData(mass), actParms, 0.3f, 0.1f)
, x258_initialHealth(hInfo)
, x260_currentHealth(hInfo)
, x268_damageVulnerability(dVuln)
, x2d8_(w1)
, x2dc_xrayAlpha(f3)
, x2e2_24_(b2)
, x2e2_25_dead(false)
, x2e2_26_animating(true)
, x2e2_27_(std::fabs(1.f - f3) > 0.00001)
, x2e2_28_(false)
, x2e2_29_((x2e2_24_ && x2e2_25_dead && x2d8_ != 0))
, x2e2_30_transposeRotate(b4)
, x2e2_31_(b5)
{
    if (x64_modelData && (x64_modelData->HasAnimData() || x64_modelData->HasNormalModel()) && castsShadow)
        CreateShadow(true);

    if (x64_modelData && x64_modelData->HasAnimData())
        x64_modelData->EnableLooping(looping);

    x150_momentum = zeus::CVector3f(0.f, 0.f, zMomentum);
}

void CScriptActor::Accept(IVisitor& visitor) { visitor.Visit(this); }

void CScriptActor::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid, CStateManager& mgr)
{
    if (msg == EScriptObjectMessage::Reset)
    {
        x2e2_25_dead = false;
        x260_currentHealth = x258_initialHealth;
    }
    else if (msg == EScriptObjectMessage::Increment && !GetActive())
    {
        mgr.SendScriptMsg(this, x8_uid, EScriptObjectMessage::Activate);
        CScriptColorModulate::FadeInHelper(mgr, x8_uid, x2d0_alphaMax);
    }
    else if (msg == EScriptObjectMessage::Decrement)
    {
        CScriptColorModulate::FadeOutHelper(mgr, x8_uid, x2d4_alphaMin);
    }
    else if (msg == EScriptObjectMessage::InitializedInArea)
    {
        for (const SConnection& conn : x20_conns)
        {
            if (conn.x0_state != EScriptObjectState::InheritBounds || conn.x4_msg != EScriptObjectMessage::Activate)
                continue;

            auto search = mgr.GetIdListForScript(conn.x8_objId);
            for (auto it = search.first; it != search.second; ++it)
            {
                if (TCastToConstPtr<CScriptTrigger>(mgr.GetObjectById(it->second)))
                {
                    x2e0_triggerId = it->second;
                    break;
                }
            }
        }

        if (x2e2_31_)
            CActor::AddMaterial(EMaterialTypes::Unknown54, mgr);
    }

    CActor::AcceptScriptMsg(msg, uid, mgr);
}

void CScriptActor::Think(float dt, CStateManager& mgr)
{
    if (!GetActive())
        return;

    if (HasModelData() && x64_modelData->HasAnimData())
    {
        bool timeRemaining = x64_modelData->GetAnimationData()->IsAnimTimeRemaining(dt - FLT_EPSILON, "Whole Body");
        bool loop = x64_modelData->GetIsLoop();

        SAdvancementDeltas deltas = CActor::UpdateAnimation(dt, mgr, true);

        if (timeRemaining || loop)
        {
            x2e2_26_animating = true;

            if (x2e2_30_transposeRotate)
                MoveToOR(
                    x34_transform.rotate(x64_modelData->GetScale() * x34_transform.transposeRotate(deltas.x0_posDelta)),
                    dt);
            else
                MoveToOR(deltas.x0_posDelta, dt);

            RotateToOR(deltas.xc_rotDelta, dt);
        }

        if (!timeRemaining && x2e2_26_animating && !loop)
        {
            SendScriptMsgs(EScriptObjectState::MaxReached, mgr, EScriptObjectMessage::None);
            x2e2_26_animating = false;
        }
    }

    if (!x2e2_25_dead && HealthInfo(mgr)->GetHP() <= 0.f)
    {
        x2e2_25_dead = true;
        SendScriptMsgs(EScriptObjectState::Dead, mgr, EScriptObjectMessage::None);
    }
}

void CScriptActor::PreRender(CStateManager& mgr, const zeus::CFrustum& frustum)
{
    CActor::PreRender(mgr, frustum);

    if (xe4_30_outOfFrustum && TCastToPtr<CCinematicCamera>(mgr.GetCameraManager()->GetCurrentCamera(mgr)))
        xe4_30_outOfFrustum = false;

    if (xe4_30_outOfFrustum && !x2e2_29_ && !x2e2_27_)
    {
        zeus::CColor col(1.f, 1.f, x2dc_xrayAlpha);
        if (mgr.GetPlayerState()->GetActiveVisor(mgr) == CPlayerState::EPlayerVisor::XRay)
        {
            xb4_drawFlags.x0_blendMode = 5;
            xb4_drawFlags.x1_matSetIdx = 0;
            xb4_drawFlags.x2_flags = 3;
            xb4_drawFlags.x4_color = col;
            x2e2_28_ = true;
        }
        else if (x2e2_28_)
        {
            x2e2_28_ = false;
            if (xb4_drawFlags.x0_blendMode != 5 && xb4_drawFlags.x1_matSetIdx != 0 &&
                xb4_drawFlags.x2_flags != 3 && xb4_drawFlags.x4_color != col)
            {
                xb4_drawFlags.x0_blendMode = 5;
                xb4_drawFlags.x1_matSetIdx = 0;
                xb4_drawFlags.x2_flags = 3;
                xb4_drawFlags.x4_color = col;
            }
        }

        if (!x2e2_24_ && xe6_27_renderVisorFlags == 2 &&
            mgr.GetPlayerState()->GetActiveVisor(mgr) == CPlayerState::EPlayerVisor::XRay)
        {
            xb4_drawFlags.x2_flags &= ~3;
        }
        else
        {
            xb4_drawFlags.x2_flags |= 3;
        }

        if (x2d8_ != 0)
            xb4_drawFlags.x1_matSetIdx = 0;
    }

    if (mgr.GetObjectById(x2e0_triggerId) == nullptr)
        x2e0_triggerId = kInvalidUniqueId;
}

zeus::CAABox CScriptActor::GetSortingBounds(const CStateManager& mgr) const
{
    if (x2e0_triggerId != kInvalidUniqueId)
    {
        TCastToConstPtr<CScriptTrigger> trigger(mgr.GetObjectById(x2e0_triggerId));
        if (trigger)
            return trigger->GetTriggerBoundsWR();
    }

    return CActor::GetSortingBounds(mgr);
}

EWeaponCollisionResponseTypes
CScriptActor::GetCollisionResponseType(const zeus::CVector3f& v1, const zeus::CVector3f& v2, const
                                        CWeaponMode& wMode, int w) const
{
    const CDamageVulnerability* dVuln = GetDamageVulnerability();
    if (dVuln->GetVulnerability(wMode, false) == EVulnerability::Reflect)
    {
        EVulnerability phazonVuln = dVuln->GetPhazonVulnerability(wMode);
        if (phazonVuln < EVulnerability::PassThrough && phazonVuln >= EVulnerability::Normal)
            return EWeaponCollisionResponseTypes::Unknown15;
    }
    return CActor::GetCollisionResponseType(v1, v2, wMode, w);
}

rstl::optional_object<zeus::CAABox> CScriptActor::GetTouchBounds() const
{
    if (GetActive() && x68_material.HasMaterial(EMaterialTypes::Solid))
        return {CPhysicsActor::GetBoundingBox()};
    return {};
}

void CScriptActor::Touch(CActor&, CStateManager&) {}
}
