#include "CEyeball.hpp"
#include "Weapon/CBeamInfo.hpp"
#include "Weapon/CGameProjectile.hpp"
#include "Weapon/CPlasmaProjectile.hpp"
#include "World/CPlayer.hpp"
#include "CStateManager.hpp"
#include "TCastTo.hpp"
namespace urde::MP1
{
CEyeball::CEyeball(TUniqueId uid, std::string_view name, CPatterned::EFlavorType flavor, const CEntityInfo& info,
                   const zeus::CTransform& xf, CModelData&& mData, const CPatternedInfo& pInfo, float f1, float f2,
                   CAssetId aId1, const CDamageInfo& dInfo, CAssetId aId2, CAssetId aId3, CAssetId aId4, CAssetId aId5,
                   u32 w1, u32 w2, u32 w3, u32 w4, u32 w5,
                   bool b1, const CActorParameters& actParms)
    : CPatterned(ECharacter::EyeBall, uid, name, flavor, info, xf, std::move(mData), pInfo, EMovementType::Flyer,
                 EColliderType::Zero, EBodyType::Restricted, actParms, EKnockBackVariant::Medium), x568_(f1), x56c_(f2),
      x570_boneTracking(*GetModelData()->GetAnimationData(), "Eye"sv, zeus::degToRad(45.f),
                        zeus::degToRad(180.f), true), x5b4_projectileInfo(aId1, dInfo), x5dc_(aId2), x5e0_(aId3),
      x5e4_(aId4),
      x5e8_(aId5), x5f4_(w1), x5f8_(w2), x5fc_(w3), x600_(w4), x604_(CSfxManager::TranslateSFXID(w5)), x60c_24_(false),
      x60c_25_(false), x60c_26_alert(false), x60c_27_(b1), x60c_28_(false)
{
    x460_knockBackController.SetAutoResetImpulse(false);
}

void CEyeball::Accept(IVisitor& visitor)
{
    visitor.Visit(this);
}

void CEyeball::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid, CStateManager& mgr)
{
    switch (msg)
    {
    case EScriptObjectMessage::ProjectileCollide:
    case EScriptObjectMessage::InvulnDamage:
    {
        if (TCastToConstPtr<CGameProjectile> proj = mgr.GetObjectById(uid))
        {
            if (proj->GetOwnerId() == mgr.GetPlayer().GetUniqueId())
                if (GetDamageVulnerability()->GetVulnerability(proj->GetDamageInfo().GetWeaponMode(), false) !=
                    EVulnerability::Deflect)
                    x400_24_hitByPlayerProjectile = true;
        }
        return;
    }
    case EScriptObjectMessage::Alert:
        x60c_26_alert = true;
        break;
    case EScriptObjectMessage::Registered:
    {
        RemoveMaterial(EMaterialTypes::Orbit, EMaterialTypes::Target, mgr);
        x450_bodyController->Activate(mgr);
        x330_stateMachineState.SetDelay(0.f);
        CreateShadow(false);
        CreateBeam(mgr);
        break;
    }
    case EScriptObjectMessage::Deleted:
    {
        if (x5ec_projectileId != kInvalidUniqueId)
        {
            mgr.FreeScriptObject(x5ec_projectileId);
            if (x608_)
            {
                CSfxManager::RemoveEmitter(x608_);
                x608_.reset();
            }
        }
        x5ec_projectileId = kInvalidUniqueId;
        break;
    }
    default:
        break;
    }
    CPatterned::AcceptScriptMsg(msg, uid, mgr);
}

void CEyeball::CreateBeam(CStateManager& mgr)
{
    if (x5ec_projectileId != kInvalidUniqueId)
        return;

    CBeamInfo beamInfo(3, x5dc_, x5e0_, x5e4_, x5e8_, 50, .5f, 1.f, 2.f, 20.f, 1.f, 1.f, 2.f,
                       zeus::CColor(1.f, 1.f, 1.f, 0.f), zeus::CColor(0.f, 1.f, 0.5f, 0.f), 150.f);
    x5ec_projectileId = mgr.AllocateUniqueId();
    mgr.AddObject(new CPlasmaProjectile(x5b4_projectileInfo.Token(), "EyeBall_Beam"sv, EWeaponType::AI, beamInfo,
                                        zeus::CTransform::Identity(), EMaterialTypes::Immovable,
                                        x5b4_projectileInfo.GetDamage(), x5ec_projectileId, GetAreaIdAlways(),
                                        GetUniqueId(), 8, false, EProjectileAttrib::KeepInCinematic));

}

void CEyeball::InActive(CStateManager&, EStateMsg msg, float)
{
    if (msg == EStateMsg::Activate)
        x450_bodyController->SetLocomotionType(pas::ELocomotionType::Relaxed);
}

void CEyeball::Cover(CStateManager&, EStateMsg msg, float)
{
    if (msg == EStateMsg::Activate)
    {
        x450_bodyController->SetLocomotionType(pas::ELocomotionType::Lurk);
        x60c_24_ = false;
        x330_stateMachineState.SetDelay(x568_);
    }
}

void CEyeball::Flinch(CStateManager& mgr, EStateMsg msg, float arg)
{
    if (msg == EStateMsg::Activate)
    {
        x32c_animState = EAnimState::One;
        x330_stateMachineState.SetDelay(x568_);
    }
    else if (msg == EStateMsg::Update)
        TryCommand(mgr, pas::EAnimationState::KnockBack, CPatternedTryFunc(&CEyeball::TryFlinch), 0);
    else if (msg == EStateMsg::Deactivate)
        x32c_animState = EAnimState::Zero;
}

void CEyeball::TryFlinch(CStateManager&, int arg)
{
    x450_bodyController->GetCommandMgr().DeliverCmd(CBCKnockBackCmd(GetTransform().basis[0], pas::ESeverity(arg)));
}

void CEyeball::Active(CStateManager& mgr, EStateMsg msg, float)
{
    if (msg == EStateMsg::Activate)
    {
        x400_24_hitByPlayerProjectile = 0;
        x450_bodyController->SetLocomotionType(pas::ELocomotionType::Combat);
        x60c_24_ = false;
    }
    else if (msg == EStateMsg::Update)
    {
        if (x330_stateMachineState.GetDelay() > x56c_)
            x60c_24_ = true;

        sub802249c8();
    }
    else if (msg == EStateMsg::Deactivate)
    {
        x330_stateMachineState.SetDelay(x568_);
        if (CPlasmaProjectile* proj = static_cast<CPlasmaProjectile*>(mgr.ObjectById(x5ec_projectileId)))
            proj->ResetBeam(mgr, true);

        x60c_24_ = false;

        CSfxManager::RemoveEmitter(x608_);
        x608_.reset();
    }
}

void CEyeball::sub802249c8()
{

}

}