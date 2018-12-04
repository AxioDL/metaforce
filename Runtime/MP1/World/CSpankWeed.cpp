#include "CSpankWeed.hpp"
#include "World/CPatternedInfo.hpp"
#include "World/CPlayer.hpp"
#include "Collision/CCollisionActor.hpp"
#include "CStateManager.hpp"
#include <logvisor/logvisor.hpp>

namespace urde::MP1
{
logvisor::Module SpankLog("urde::MP1::SpankWeed");
CSpankWeed::CSpankWeed(TUniqueId uid, std::string_view name, const CEntityInfo& info, const zeus::CTransform& xf,
                       CModelData&& mData, const CActorParameters& actParms, const CPatternedInfo& pInfo, float maxDetectionRange,
                       float maxHearingRange, float maxSightRange, float hideTime)
: CPatterned(ECharacter::SpankWeed, uid, name, EFlavorType::Zero, info, xf, std::move(mData), pInfo,
             EMovementType::Flyer, EColliderType::One, EBodyType::Restricted, actParms, EKnockBackVariant::Medium)
, x568_maxDetectionRange(maxDetectionRange)
, x56c_detectionHeightRange(pInfo.GetDetectionHeightRange())
, x570_maxHearingRange(maxHearingRange)
, x574_maxSightRange(maxSightRange)
, x578_hideTime(hideTime)
, x584_retreatOrigin(xf.origin)
{
    SetCallTouch(false);
    CreateShadow(false);

    zeus::CVector3f modelScale = GetModelData()->GetScale();
    if (modelScale.x != modelScale.y || modelScale.x != modelScale.z)
    {
        float scale = modelScale.magnitude() / std::sqrt(3.f);

        ModelData()->SetScale(zeus::CVector3f(scale));
        SpankLog.report(logvisor::Level::Warning, "WARNING: Non-uniform scale (%.2f, %.2f, %.2f) applied to Spank Weed"
                                                  "...changing scale to (%.2f, %.2f, %.2f)\n",
                                                  modelScale.x, modelScale.y, modelScale.z,
                                                  scale, scale, scale);
    }
    CMaterialList list = GetMaterialFilter().GetExcludeList();
    list.Add(EMaterialTypes::Character);
    list.Add(EMaterialTypes::Player);
    SetMaterialFilter(CMaterialFilter::MakeIncludeExclude(GetMaterialFilter().GetIncludeList(), list));

    CSegId segId = GetModelData()->GetAnimationData()->GetLocatorSegId("lockon_target_LCTR"sv);
    if (segId != 0xFF)
    {
        zeus::CTransform locatorXf = GetTransform() * zeus::CTransform::Scale(GetModelData()->GetScale()) *
                                     GetModelData()->GetAnimationData()->GetLocatorTransform(segId, nullptr);
        x5a8_lockonTarget = locatorXf.origin;
        x59c_lockonOffset = locatorXf.origin - GetTranslation();
    }
    x460_knockBackController.SetAutoResetImpulse(false);
}

static const SSphereJointDescription kArmCollision[]=
    {
        {"Arm_4",       1.5f},
        {"Arm_6",       1.f},
        {"Arm_7",       1.f},
        {"Arm_8",       1.f},
        {"Arm_9",       1.f},
        {"Arm_11",      1.f},
        {"Swoosh_LCTR", 1.5f}
    };

void CSpankWeed::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid, CStateManager& mgr)
{
    bool oldActive = GetActive();
    if (msg == EScriptObjectMessage::Activate)
    {
        if (x90_actorLights)
            x90_actorLights->SetDirty();
    }
    else if (msg == EScriptObjectMessage::Decrement)
    {
        if (x5b4_ != 0 && x5b4_ != 5 && x5b4_ != 6 && x5b4_ != 4)
        {
            x400_24_hitByPlayerProjectile = true;
            x428_damageCooldownTimer = x424_damageWaitTime;
        }
    }
    else if (msg == EScriptObjectMessage::Registered)
    {
        if (!x450_bodyController->GetActive())
        {
            x450_bodyController->Activate(mgr);
            zeus::CVector3f extents = GetBoundingBox().extents();

            SetBoundingBox({-extents, extents});
        }

        std::vector<CJointCollisionDescription> joints;
        joints.reserve(12);

        for (const SSphereJointDescription& joint : kArmCollision)
        {
            CSegId id = GetModelData()->GetAnimationData()->GetLocatorSegId(joint.name);
            if (id != 0xFF)
                joints.push_back(CJointCollisionDescription::SphereCollision(id, joint.radius, joint.name, 0.001f));
        }

        x594_collisionMgr.reset(new CCollisionActorManager(mgr, GetUniqueId(), GetAreaIdAlways(), joints, GetActive()));
        CMaterialList list;
        list.Add(EMaterialTypes::CameraPassthrough);
        list.Add(EMaterialTypes::Immovable);
        x594_collisionMgr->AddMaterial(mgr, list);
        if (x90_actorLights)
        {
            x90_actorLights->SetDirty();
            zeus::CVector3f swooshOrigin = GetScaledLocatorTransform("swoosh_LCTR"sv).origin;
            x90_actorLights->SetActorPositionBias(GetTransform().buildMatrix3f() * swooshOrigin);
        }
    }
    else if (msg == EScriptObjectMessage::Touched)
    {
        if (TCastToPtr<CCollisionActor> colAct = mgr.ObjectById(uid))
        {
            if (TCastToPtr<CPlayer> player = mgr.ObjectById(colAct->GetLastTouchedObject()))
            {
                if (x420_curDamageRemTime < 0.f && x5b4_ != 4 && x5b4_ != 6)
                {
                    mgr.ApplyDamage(GetUniqueId(), player->GetUniqueId(), GetUniqueId(), GetContactDamage(),
                                    CMaterialFilter::MakeIncludeExclude({EMaterialTypes::Solid}, {}), {});
                    x420_curDamageRemTime = x424_damageWaitTime;
                }
            }
        }
    }
    else if (msg == EScriptObjectMessage::Deleted)
    {
        mgr.FreeScriptObject(x590_);
        x594_collisionMgr->Destroy(mgr);
    }
    else if (msg == EScriptObjectMessage::SuspendedMove)
    {
        x594_collisionMgr->SetMovable(mgr, false);
    }

    CPatterned::AcceptScriptMsg(msg, uid, mgr);

    if (GetActive() != oldActive)
        x594_collisionMgr.reset();
}

void CSpankWeed::Think(float dt, CStateManager& mgr)
{
    if (!GetActive())
        return;

    HealthInfo(mgr)->SetHP(1000000.0f);

    if (!x598_isHiding)
    {
        zeus::CVector3f eyeOrigin = GetLocatorTransform("Eye"sv).origin;
        MoveCollisionPrimitive(GetModelData()->GetScale() * eyeOrigin);
        x594_collisionMgr->Update(dt, mgr, CCollisionActorManager::EUpdateOptions::ObjectSpace);
        xe4_27_notInSortedLists = true;
    }

    CPatterned::Think(dt, mgr);
}

zeus::CVector3f CSpankWeed::GetOrbitPosition(const CStateManager& mgr) const
{
    zeus::CVector3f ret = CPatterned::GetOrbitPosition(mgr);
    float delay = std::max(1.f, x330_stateMachineState.GetDelay());
    if (x5b4_ == 3 && x5b8_ == 2)
    {
        return (ret * (1.f - delay) + ((GetTranslation() + x59c_lockonOffset) * delay));
    }
    else if (x5b4_ == 2 && x5b8_ == 3)
    {
        return (GetTranslation() + x59c_lockonOffset) * (1.f - delay) + (ret * delay);
    }
    return ret;
}

zeus::CVector3f CSpankWeed::GetAimPosition(const CStateManager&, float dt) const
{
    zeus::CVector3f pos = (dt > 0.f ? PredictMotion(dt).x0_translation : zeus::CVector3f::skZero);

    CSegId id = GetModelData()->GetAnimationData()->GetLocatorSegId("lockon_target_LCTR"sv);
    if (id != 0xFF)
    {
        zeus::CVector3f lockonOff = GetModelData()->GetAnimationData()->GetLocatorTransform(id, nullptr).origin;
        return pos + (GetTransform() * (GetModelData()->GetScale() * lockonOff));
    }

    return pos + GetBoundingBox().center();
}

void CSpankWeed::Flinch(CStateManager& mgr, EStateMsg msg, float)
{
    if (msg == EStateMsg::Activate)
    {
        x5bc_ = 0;
        x5b4_ = 0;
        RemoveMaterial(EMaterialTypes::Orbit, EMaterialTypes::Target, mgr);
    }
    else if (msg == EStateMsg::Update)
    {
        if (x5bc_ == 0)
        {
            if (x450_bodyController->GetBodyStateInfo().GetCurrentStateId() == pas::EAnimationState::KnockBack)
                x5bc_ = 2;
            else
                x450_bodyController->GetCommandMgr().DeliverCmd(CBCKnockBackCmd({}, pas::ESeverity::Zero));
        }
        else if (x5bc_ == 2 && x450_bodyController->GetBodyStateInfo().GetCurrentStateId() != pas::EAnimationState::KnockBack)
                x5bc_ = 3;
    }
    else if (msg == EStateMsg::Deactivate)
    {
        x5b8_ = 4;
    }
}

bool CSpankWeed::Delay(CStateManager&, float)
{
    if (x400_24_hitByPlayerProjectile)
    {
        if (x330_stateMachineState.GetTime() > x578_hideTime)
        {
            x400_24_hitByPlayerProjectile = false;
            return true;
        }
        return false;
    }

    return true;
}

bool CSpankWeed::InRange(CStateManager& mgr, float)
{
    float playerDist = GetPlayerDistance(mgr);
    if (x56c_detectionHeightRange > 0.f)
    {
        return std::fabs(mgr.GetPlayer().GetTranslation().z - GetTranslation().z) < x56c_detectionHeightRange &&
               playerDist < (x574_maxSightRange * x574_maxSightRange);
    }

    return playerDist < (x574_maxSightRange * x574_maxSightRange);
}

bool CSpankWeed::HearPlayer(CStateManager& mgr, float)
{
    float playerDist = GetPlayerDistance(mgr);
    if (x56c_detectionHeightRange > 0.f)
    {
        return std::fabs(mgr.GetPlayer().GetTranslation().z - GetTranslation().z) < x56c_detectionHeightRange &&
               playerDist < (x570_maxHearingRange * x570_maxHearingRange);
    }

    return playerDist < (x570_maxHearingRange * x570_maxHearingRange);
}

bool CSpankWeed::InDetectionRange(CStateManager& mgr, float)
{
    float playerDist = GetPlayerDistance(mgr);
    if (x56c_detectionHeightRange > 0.f)
    {
        return std::fabs(mgr.GetPlayer().GetTranslation().z - GetTranslation().z) < x56c_detectionHeightRange &&
               playerDist < (x568_maxDetectionRange * x568_maxDetectionRange);
    }

    return playerDist < (x568_maxDetectionRange * x568_maxDetectionRange);
}

void CSpankWeed::Attack(CStateManager&, EStateMsg msg, float)
{
    if (msg == EStateMsg::Activate)
    {
        x450_bodyController->GetCommandMgr().DeliverCmd(CBCMeleeAttackCmd(pas::ESeverity::Zero));
    }
    else if (msg == EStateMsg::Update)
    {
        if (x450_bodyController->GetBodyStateInfo().GetCurrentStateId() == pas::EAnimationState::MeleeAttack)
            return;

        x450_bodyController->GetCommandMgr().DeliverCmd(CBCMeleeAttackCmd(pas::ESeverity::Zero));
    }
    else if (msg == EStateMsg::Deactivate)
    {
        x5b8_ = 3;
    }
}

void CSpankWeed::TargetPatrol(CStateManager& mgr, EStateMsg msg, float)
{
    if (msg == EStateMsg::Activate)
    {
        x450_bodyController->SetLocomotionType(pas::ELocomotionType::Combat);
        RemoveMaterial(EMaterialTypes::Solid, mgr);
        x5b4_ = 2;
    }
    else if (msg == EStateMsg::Deactivate)
    {
        x5b8_ = 2;
    }
}

void CSpankWeed::Lurk(CStateManager& mgr, EStateMsg msg, float)
{
    if (msg == EStateMsg::Activate)
    {
        x460_knockBackController.SetEnableFreeze(true);
        x450_bodyController->SetLocomotionType(pas::ELocomotionType::Lurk);
        RemoveMaterial(EMaterialTypes::Solid, mgr);
        x5b4_ = 1;
    }
    else if (msg == EStateMsg::Deactivate)
    {
        x5b8_ = 1;
    }
}

void CSpankWeed::FadeOut(CStateManager&, EStateMsg msg, float)
{
    if (msg == EStateMsg::Activate)
    {
        x5bc_ = 0;
        x57c_canKnockBack = false;
        x5b4_ = 6;
    }
    else if (msg == EStateMsg::Update)
    {
        if (x5bc_ == 0)
        {
            if (x450_bodyController->GetBodyStateInfo().GetCurrentStateId() == pas::EAnimationState::Step)
                x5bc_ = 2;
            else
                x450_bodyController->GetCommandMgr().DeliverCmd(
                    CBCStepCmd(pas::EStepDirection::Backward, pas::EStepType::Normal));

        }
        else if (x5bc_ == 2)
        {
            if (x450_bodyController->GetBodyStateInfo().GetCurrentStateId() != pas::EAnimationState::Step)
                x5bc_ = 3;
        }
    }
    else if (msg == EStateMsg::Deactivate)
    {
        x5b8_ = 6;
    }
}

void CSpankWeed::FadeIn(CStateManager& mgr, EStateMsg msg, float)
{
    if (msg == EStateMsg::Activate)
    {
        x5bc_ = 0;
        x57c_canKnockBack = true;
        x5b4_ = 5;
    }
    else if (msg == EStateMsg::Update)
    {
        if (x5bc_ == 0)
        {
            if (x450_bodyController->GetBodyStateInfo().GetCurrentStateId() == pas::EAnimationState::Step)
                x5bc_ = 2;
            else
                x450_bodyController->GetCommandMgr().DeliverCmd(
                    CBCStepCmd(pas::EStepDirection::Forward, pas::EStepType::Normal));

        }
        else if (x5bc_ == 2)
        {
            if (x450_bodyController->GetBodyStateInfo().GetCurrentStateId() != pas::EAnimationState::Step)
                x5bc_ = 3;
        }
    }
    else if (msg == EStateMsg::Deactivate)
    {
        x5b8_ = 5;
    }

    xe7_28_worldLightingDirty = true;
}

void CSpankWeed::Patrol(CStateManager& mgr, EStateMsg msg, float)
{
    if (msg == EStateMsg::Activate)
    {
        x460_knockBackController.SetEnableFreeze(false);
        x450_bodyController->SetLocomotionType(pas::ELocomotionType::Relaxed);
        RemoveMaterial(EMaterialTypes::Solid, EMaterialTypes::Scannable, mgr);
        RemoveMaterial(EMaterialTypes::Orbit, EMaterialTypes::Target, mgr);
        x594_collisionMgr->SetActive(mgr, false);
        x598_isHiding = true;
        x5b4_ = 0;
    }
    else if (msg == EStateMsg::Deactivate)
    {
        AddMaterial(EMaterialTypes::Orbit, EMaterialTypes::Target, EMaterialTypes::Scannable, mgr);
        SetTranslation(x584_retreatOrigin);
        x594_collisionMgr->SetActive(mgr, true);
        x598_isHiding = false;
        x460_knockBackController.SetEnableFreeze(true);
        x5b8_ = 0;
    }
}

void CSpankWeed::KnockBack(const zeus::CVector3f& backVec, CStateManager& mgr, const CDamageInfo& info,
                                    EKnockBackType type, bool inDeferred, float magnitude)
{
    if (!x57c_canKnockBack)
        return;
    CPatterned::KnockBack(backVec, mgr, info, type, inDeferred, magnitude);
    x57c_canKnockBack = false;
}

float CSpankWeed::GetPlayerDistance(CStateManager& mgr) const
{
    return (mgr.GetPlayer().GetTranslation() - x5a8_lockonTarget).magSquared();
}
}