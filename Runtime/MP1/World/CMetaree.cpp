#include "MP1/World/CMetaree.hpp"
#include "Weapon/CGameProjectile.hpp"
#include "World/CPlayer.hpp"
#include "CStateManager.hpp"
#include "CPlayerState.hpp"

#include "TCastTo.hpp"


namespace urde::MP1
{
CMetaree::CMetaree(TUniqueId uid, std::string_view name, EFlavorType flavor, const CEntityInfo& info,
                   const zeus::CTransform& xf, CModelData&& mData, const CPatternedInfo& pInfo, const CDamageInfo& dInfo,
                   float f1, const zeus::CVector3f& v1, float f2, EBodyType bodyType, float f3, float f4,
                   const CActorParameters& aParms)
: CPatterned(ECharacter::Metaree, uid, name, flavor, info, xf, std::move(mData), pInfo, EMovementType::Flyer,
             EColliderType::Zero, bodyType, aParms, EKnockBackVariant::Small)
, x568_delay(f3)
, x56c_(f4)
, x570_dropHeight(f1)
, x574_offset(v1)
, x580_attackSpeed(f2)
, x5ca_24_(true)
, x5ca_25_started(false)
, x5ca_26_deactivated(false)
{
}

void CMetaree::Accept(IVisitor& visitor)
{
    visitor.Visit(this);
}

void CMetaree::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid, CStateManager& mgr)
{
    CPatterned::AcceptScriptMsg(msg, uid, mgr);

    if (msg == EScriptObjectMessage::Start)
        x5ca_25_started = true;
    else if (msg == EScriptObjectMessage::Registered)
        x450_bodyController->Activate(mgr);
}

void CMetaree::Think(float dt, CStateManager& mgr)
{
    bool target = true;
    if (mgr.GetPlayerState()->GetCurrentVisor() != CPlayerState::EPlayerVisor::Thermal &&
        mgr.GetPlayerState()->GetCurrentVisor() != CPlayerState::EPlayerVisor::Scan)
    {
        target = x5ca_26_deactivated;
    }
    xe7_31_targetable = target;
    CPatterned::Think(dt, mgr);
}

void CMetaree::Explode(CStateManager& mgr, EStateMsg msg, float)
{
    if (msg != EStateMsg::Activate)
        return;

    mgr.ApplyDamage(GetUniqueId(), mgr.GetPlayer().GetUniqueId(), GetUniqueId(), x5ac_damgeInfo,
                    CMaterialFilter::MakeIncludeExclude({EMaterialTypes::Solid}, {}), {});
    MassiveDeath(mgr);
}

void CMetaree::Touch(CActor& act, CStateManager& mgr)
{
    if (!x400_25_alive)
        return;

    if (TCastToPtr<CGameProjectile> projectile = act)
    {
        if (projectile->GetOwnerId() != mgr.GetPlayer().GetUniqueId())
            return;

        x400_24_hitByPlayerProjectile = true;
        x590_projectileDelta = projectile->GetTranslation() - projectile->GetPreviousPos();
    }
}

void CMetaree::CollidedWith(TUniqueId& id, const CCollisionInfoList& colList, CStateManager& mgr)
{
    if (!x400_25_alive || colList.GetCount() <= 0)
        return;

    mgr.ApplyDamageToWorld(GetUniqueId(), *this, GetTranslation(), x5ac_damgeInfo,
                           CMaterialFilter::MakeInclude({EMaterialTypes::Player}));
    SendScriptMsgs(EScriptObjectState::Arrived, mgr, EScriptObjectMessage::None);
    MassiveDeath(mgr);
}

void CMetaree::Flee(CStateManager& mgr, EStateMsg msg, float)
{
    if (msg == EStateMsg::Activate)
    {
        ApplyImpulseWR(5.f * (GetMass() * (x590_projectileDelta * zeus::CVector3f{1.f, 1.f, 0.f})),
                       zeus::CAxisAngle::sIdentity);

        SetMomentumWR({0.f, 0.f, -GetGravityConstant() * GetMass()});
        SetTranslation(GetTranslation());
        x5a8_ = 0;
    }
    else if (msg == EStateMsg::Update)
    {
        if (x5a8_ != 0)
            return;

        if (x450_bodyController->GetBodyStateInfo().GetCurrentStateId() == pas::EAnimationState::LieOnGround)
        {
            x5a8_ = 1;
            return;
        }

        x450_bodyController->GetCommandMgr().DeliverCmd(CBCKnockDownCmd({0.f, 1.f, 0.f}, pas::ESeverity::Zero));
    }
}

void CMetaree::Dead(CStateManager& mgr, EStateMsg msg, float)
{
    if (msg != EStateMsg::Activate)
        return;

    mgr.ApplyDamageToWorld(GetUniqueId(), *this, GetTranslation(), x5ac_damgeInfo,
                           CMaterialFilter::MakeIncludeExclude({EMaterialTypes::Player}, {}));
    DeathDelete(mgr);
}

void CMetaree::Attack(CStateManager&, EStateMsg msg, float)
{
    if (msg == EStateMsg::Activate)
    {
        x5a8_ = 0;
        zeus::CVector3f dir = (x584_lookPos - GetTranslation()).normalized();
        SetVelocityWR(x580_attackSpeed * dir);
        CSfxManager::AddEmitter(x5c8_attackSfx, GetTranslation(), {}, true, false, 127, kInvalidAreaId);
        x450_bodyController->SetLocomotionType(pas::ELocomotionType::Combat);
        x59c_velocity = x580_attackSpeed * dir;
    }
    else if (msg == EStateMsg::Update)
    {
        if (x450_bodyController->GetPercentageFrozen() == 0.f)
            SetVelocityWR(x59c_velocity);
        else
        {
            Stop();
            SetVelocityWR({});
        }
    }
}

void CMetaree::Halt(CStateManager& mgr, EStateMsg msg, float)
{
    if (msg != EStateMsg::Activate)
        return;

    Stop();
    SetVelocityWR({});
    SetMomentumWR({});
    x450_bodyController->SetLocomotionType(pas::ELocomotionType::Lurk);
    x584_lookPos = x574_offset + mgr.GetPlayer().GetTranslation();
    SetTransform(zeus::lookAt(GetTranslation(), x584_lookPos));

}

void CMetaree::Active(CStateManager& mgr, EStateMsg msg, float)
{
    if (msg == EStateMsg::Activate)
    {
        x400_24_hitByPlayerProjectile = false;
        x584_lookPos = GetTranslation() - zeus::CVector3f{0.f, 0.f, x570_dropHeight};
        x450_bodyController->GetCommandMgr().DeliverCmd(CBCGenerateCmd(pas::EGenerateType::Zero, x584_lookPos));
        SetMomentumWR({0.f, 0.f, -GetGravityConstant() * GetMass()});
    }
    else if (msg == EStateMsg::Update)
    {
        x450_bodyController->GetCommandMgr().SetTargetVector(
            (mgr.GetPlayer().GetTranslation() - GetTranslation()).normalized());
    }
    else if (msg == EStateMsg::Deactivate)
    {
        SetMomentumWR({});
    }
}

void CMetaree::InActive(CStateManager&, EStateMsg msg, float)
{
    if (msg == EStateMsg::Activate)
    {
        if (!x5ca_26_deactivated)
            x450_bodyController->SetLocomotionType(pas::ELocomotionType::Relaxed);
    }
    else if (msg == EStateMsg::Deactivate)
    {
        x5ca_26_deactivated = true;
    }
}

bool CMetaree::InRange(CStateManager& mgr, float arg)
{
    if (x5ca_25_started)
        return true;

    return CPatterned::InRange(mgr, arg);
}

bool CMetaree::ShouldAttack(CStateManager&, float)
{
    return GetTranslation().z() < x584_lookPos.z();
}
}
