#include "CBodyController.hpp"
#include "World/CActor.hpp"
#include "TCastTo.hpp"
#include "World/CPhysicsActor.hpp"
#include "World/CActorModelParticles.hpp"
#include "Character/CPASAnimParm.hpp"
#include "Character/CPASAnimParmData.hpp"
#include "CStateManager.hpp"

namespace urde
{

CBodyController::CBodyController(CActor& actor, float turnSpeed, EBodyType bodyType)
: x0_actor(actor), x2a4_bodyStateInfo(actor, bodyType),
  x2f4_bodyType(bodyType), x2fc_turnSpeed(turnSpeed)
{
    x300_28_playDeathAnims = true;
    x2a4_bodyStateInfo.x18_bodyController = this;
}

void CBodyController::EnableAnimation(bool e)
{
    x0_actor.ModelData()->AnimationData()->EnableAnimation(e);
}

void CBodyController::Activate(CStateManager& mgr)
{
    x300_25_active = true;
    x2a4_bodyStateInfo.SetState(pas::EAnimationState(GetPASDatabase().GetDefaultState()));
    x2a4_bodyStateInfo.GetCurrentState()->Start(*this, mgr);
    x2a4_bodyStateInfo.GetCurrentAdditiveState()->Start(*this, mgr);
}

void CBodyController::UpdateBody(float dt, CStateManager& mgr)
{
    UpdateFrozenInfo(dt, mgr);
    if (x320_fireDur > 0.f)
    {
        if (x328_timeOnFire > x320_fireDur)
        {
            x328_timeOnFire = 0.f;
            x320_fireDur = 0.f;
        }
        else
        {
            x328_timeOnFire += dt;
        }
    }
    else if (x324_electrocutionDur > 0.f)
    {
        if (x32c_timeElectrocuting > x324_electrocutionDur)
        {
            x32c_timeElectrocuting = 0.f;
            x324_electrocutionDur = 0.f;
        }
        else
        {
            x32c_timeElectrocuting += dt;
        }
    }

    if (GetPercentageFrozen() < 1.f && x300_28_playDeathAnims)
    {
        pas::EAnimationState nextState = x2a4_bodyStateInfo.GetCurrentState()->UpdateBody(dt, *this, mgr);
        if (nextState != pas::EAnimationState::Invalid)
        {
            x2a4_bodyStateInfo.GetCurrentState()->Shutdown(*this);
            x2a4_bodyStateInfo.SetState(nextState);
            x2a4_bodyStateInfo.GetCurrentState()->Start(*this, mgr);
        }

        nextState = x2a4_bodyStateInfo.GetCurrentAdditiveState()->UpdateBody(dt, *this, mgr);
        if (nextState != pas::EAnimationState::Invalid)
        {
            x2a4_bodyStateInfo.GetCurrentAdditiveState()->Shutdown(*this);
            x2a4_bodyStateInfo.SetAdditiveState(nextState);
            x2a4_bodyStateInfo.GetCurrentAdditiveState()->Start(*this, mgr);
        }
    }
}

void CBodyController::SetTurnSpeed(float s)
{
    x2fc_turnSpeed = std::max(0.f, s);
}

void CBodyController::Update(float dt, CStateManager& mgr)
{
    SetPlaybackRate(1.f);
    if (x300_25_active)
    {
        x300_24_animationOver =
            !x0_actor.GetModelData()->GetAnimationData()->IsAnimTimeRemaining(dt, "Whole Body"sv);
        x4_cmdMgr.BlendSteeringCmds();
        x2dc_rot = zeus::CQuaternion::skNoRotation;
        UpdateBody(dt, mgr);
        if (TCastToPtr<CPhysicsActor> act = x0_actor)
            act->RotateInOneFrameOR(x2dc_rot, dt);
        x4_cmdMgr.Reset();
    }
}

bool CBodyController::HasBodyState(pas::EAnimationState s) const
{
    return GetPASDatabase().HasState(s32(s));
}

void CBodyController::SetCurrentAnimation(const CAnimPlaybackParms& parms, bool loop, bool noTrans)
{
    x0_actor.ModelData()->AnimationData()->SetAnimation(parms, noTrans);
    x0_actor.ModelData()->EnableLooping(loop);
    x2f8_curAnim = parms.GetAnimationId();
}

float CBodyController::GetAnimTimeRemaining() const
{
    return x0_actor.GetModelData()->GetAnimationData()->GetAnimTimeRemaining("Whole Body");
}

void CBodyController::SetPlaybackRate(float r)
{
    x0_actor.ModelData()->AnimationData()->SetPlaybackRate(r);
}

const CPASDatabase& CBodyController::GetPASDatabase() const
{
    return x0_actor.GetModelData()->GetAnimationData()->GetCharacterInfo().GetPASDatabase();
}

void CBodyController::MultiplyPlaybackRate(float r)
{
    x0_actor.ModelData()->AnimationData()->MultiplyPlaybackRate(r);
}

void CBodyController::FaceDirection(const zeus::CVector3f& v0, float dt)
{
    if (x300_26_frozen)
        return;
    zeus::CVector3f noZ = v0;
    noZ.z = 0.f;
    if (noZ.canBeNormalized())
    {
        if (TCastToPtr<CPhysicsActor> act = x0_actor)
        {
            zeus::CQuaternion rot =
                zeus::CQuaternion::lookAt(act->GetTransform().basis[1],
                noZ.normalized(), zeus::degToRad(dt * x2fc_turnSpeed));
            rot.setImaginary(act->GetTransform().transposeRotate(rot.getImaginary()));
            act->RotateInOneFrameOR(rot, dt);
        }
    }
}

void CBodyController::FaceDirection3D(const zeus::CVector3f& v0, const zeus::CVector3f& v1, float dt)
{
    if (x300_26_frozen)
        return;
    if (v0.canBeNormalized() && v1.canBeNormalized())
    {
        if (TCastToPtr<CPhysicsActor> act = x0_actor)
        {
            zeus::CUnitVector3f uv0 = v0;
            zeus::CUnitVector3f uv1 = v1;
            float dot = uv0.dot(uv1);
            if (std::fabs(dot - 1.f) > 0.00001f)
            {
                if (dot < -0.9999f)
                {
                    zeus::CQuaternion rot =
                        zeus::CQuaternion::fromAxisAngle(act->GetTransform().basis[2],
                        zeus::degToRad(dt * x2fc_turnSpeed));
                    rot.setImaginary(act->GetTransform().transposeRotate(rot.getImaginary()));
                    act->RotateInOneFrameOR(rot, dt);
                }
                else
                {
                    zeus::CQuaternion rot =
                        zeus::CQuaternion::clampedRotateTo(uv1, uv0, zeus::degToRad(dt * x2fc_turnSpeed));
                    rot.setImaginary(x0_actor.GetTransform().transposeRotate(rot.getImaginary()));
                    act->RotateInOneFrameOR(rot, dt);
                }
            }
        }
    }
}

bool CBodyController::HasBodyInfo(CActor& act)
{
    return act.GetModelData()->GetAnimationData()->GetCharacterInfo().GetPASDatabase().GetNumAnimStates() != 0;
}

void CBodyController::PlayBestAnimation(const CPASAnimParmData& parms, CRandom16& r)
{
    std::pair<float, s32> best = GetPASDatabase().FindBestAnimation(parms, r, -1);
    CAnimPlaybackParms playParms(best.second, -1, 1.f, true);
    SetCurrentAnimation(playParms, false, false);
}

void CBodyController::LoopBestAnimation(const CPASAnimParmData& parms, CRandom16& r)
{
    std::pair<float, s32> best = GetPASDatabase().FindBestAnimation(parms, r, -1);
    CAnimPlaybackParms playParms(best.second, -1, 1.f, true);
    SetCurrentAnimation(playParms, true, false);
}

void CBodyController::Freeze(float intoFreezeDur, float frozenDur, float breakoutDur)
{
    x304_intoFreezeDur = intoFreezeDur;
    x308_frozenDur = frozenDur;
    x30c_breakoutDur = breakoutDur;
    x300_26_frozen = true;
    x300_27_hasBeenFrozen = true;

    if (TCastToPtr<CPhysicsActor> act = x0_actor)
    {
        x314_backedUpForce = act->GetConstantForce();
        act->SetConstantForce(zeus::CVector3f::skZero);
        act->SetMomentumWR(zeus::CVector3f::skZero);
    }

    x320_fireDur = 0.f;
    x328_timeOnFire = 0.f;
    x310_timeFrozen = 0.f;
}

void CBodyController::UnFreeze()
{
    SetPlaybackRate(1.f);
    x300_26_frozen = false;
    x304_intoFreezeDur = 0.f;
    x308_frozenDur = 0.f;
    x30c_breakoutDur = 0.f;
    x310_timeFrozen = 0.f;
    x0_actor.SetVolume(1.f);

    if (TCastToPtr<CPhysicsActor> act = x0_actor)
    {
        act->SetConstantForce(x314_backedUpForce);
        act->SetVelocityWR(x314_backedUpForce * (1.f / act->GetMass()));
    }
}

float CBodyController::GetPercentageFrozen() const
{
    float sum = x304_intoFreezeDur + x308_frozenDur + x30c_breakoutDur;
    if (x310_timeFrozen == 0.f || sum == 0.f)
        return 0.f;

    if (x310_timeFrozen <= x304_intoFreezeDur && x304_intoFreezeDur > 0.f)
        return x310_timeFrozen / x304_intoFreezeDur;

    if (x310_timeFrozen < sum - x30c_breakoutDur)
        return 1.f;
    if (x30c_breakoutDur <= 0.f)
        return 1.f;

    return 1.f - (x310_timeFrozen - (x308_frozenDur + x304_intoFreezeDur)) / x30c_breakoutDur;
}

void CBodyController::SetOnFire(float dur)
{
    x320_fireDur = dur;
    x328_timeOnFire = 0.f;
    if (IsFrozen())
        UnFreeze();
}

void CBodyController::DouseFlames()
{
    if (x320_fireDur <= 0.f)
        return;
    x320_fireDur = 0.f;
    x328_timeOnFire = 0.f;
}

void CBodyController::SetElectrocuting(float dur)
{
    if (!IsElectrocuting())
    {
        CBCAdditiveReactionCmd reaction(pas::EAdditiveReactionType::Electrocution, 1.f, true);
        x4_cmdMgr.DeliverCmd(reaction);
    }
    x324_electrocutionDur = dur;
    x32c_timeElectrocuting = 0.f;
    if (IsFrozen())
        UnFreeze();
    else if (IsOnFire())
        DouseFlames();
}

void CBodyController::DouseElectrocuting()
{
    x324_electrocutionDur = 0.f;
    x32c_timeElectrocuting = 0.f;
    CBodyStateCmd cmd(EBodyStateCmd::StopReaction);
    x4_cmdMgr.DeliverCmd(cmd);
}

void CBodyController::UpdateFrozenInfo(float dt, CStateManager& mgr)
{
    if (x300_26_frozen)
    {
        float totalTime = x304_intoFreezeDur + x308_frozenDur + x30c_breakoutDur;
        if (x310_timeFrozen > totalTime &&
            x2a4_bodyStateInfo.GetCurrentAdditiveStateId() != pas::EAnimationState::AdditiveReaction)
        {
            UnFreeze();
            x0_actor.SendScriptMsgs(EScriptObjectState::UnFrozen, mgr, EScriptObjectMessage::None);
            mgr.GetActorModelParticles()->StartIce(x0_actor);
            return;
        }
        if (x310_timeFrozen <= totalTime)
        {
            float percUnfrozen = 1.f;
            if (x310_timeFrozen < totalTime - x30c_breakoutDur)
                percUnfrozen = 1.f - GetPercentageFrozen();
            MultiplyPlaybackRate(percUnfrozen);
            x310_timeFrozen += dt;
            x0_actor.SetVolume(percUnfrozen);
            if (x310_timeFrozen > totalTime && HasIceBreakoutState())
            {
                CBCAdditiveReactionCmd cmd(pas::EAdditiveReactionType::IceBreakout, 1.f, false);
                x4_cmdMgr.DeliverCmd(cmd);
            }
        }
    }
}

bool CBodyController::HasIceBreakoutState() const
{
    CPASAnimParmData parms(24, CPASAnimParm::FromEnum(3));
    std::pair<float, s32> best = GetPASDatabase().FindBestAnimation(parms, -1);
    return best.first > 0.f;
}

void CBodyController::StopElectrocution()
{
    x324_electrocutionDur = 0.f;
    x32c_timeElectrocuting = 0.f;
    x4_cmdMgr.DeliverCmd(CBodyStateCmd(EBodyStateCmd::StopReaction));
}

void CBodyController::FrozenBreakout()
{
    if (x300_26_frozen)
    {
        float timeToBreakout = x304_intoFreezeDur + x308_frozenDur;
        if (x310_timeFrozen < timeToBreakout)
            x310_timeFrozen = timeToBreakout;
    }
}

}
