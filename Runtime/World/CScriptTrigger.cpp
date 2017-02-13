#include "CScriptTrigger.hpp"
#include "Character/CModelData.hpp"
#include "CActorParameters.hpp"
#include "Collision/CMaterialList.hpp"
#include "CStateManager.hpp"
#include "TCastTo.hpp"
#include "World/CPlayer.hpp"
#include "Weapon/CGameProjectile.hpp"
#include "Weapon/CWeapon.hpp"
#include "Particle/CGenDescription.hpp"
#include "CPlayerState.hpp"

namespace urde
{

CScriptTrigger::CScriptTrigger(TUniqueId uid, const std::string& name, const CEntityInfo& info,
                               const zeus::CVector3f& pos, const zeus::CAABox& bounds, const CDamageInfo& dInfo,
                               const zeus::CVector3f& forceField, ETriggerFlags triggerFlags, bool active, bool b2,
                               bool b3)
: CActor(uid, active, name, info, zeus::CTransform::Translate(pos), CModelData::CModelDataNull(),
         CMaterialList(EMaterialTypes::Trigger), CActorParameters::None(), kInvalidUniqueId)
, x100_damageInfo(dInfo)
, x11c_forceField(forceField)
, x128_forceMagnitude(forceField.magnitude())
, x12c_flags(triggerFlags)
, x130_bounds(bounds)
{
    x148_26_deactivateOnEntered = b2;
    x148_27_deactivateOnExited = b3;
}

void CScriptTrigger::Accept(IVisitor& visitor) { visitor.Visit(this); }

void CScriptTrigger::Think(float dt, CStateManager& mgr)
{
    if (GetActive())
        UpdateInhabitants(dt, mgr);
}

void CScriptTrigger::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid, CStateManager& mgr)
{
    if (GetActive() && (msg == EScriptObjectMessage::Deactivate || msg == EScriptObjectMessage::InternalMessage12))
    {
        if (msg == EScriptObjectMessage::Deactivate)
        {
            xe8_inhabitants.clear();
            x148_25_ = false;
        }

        if (x148_28_)
        {
            x148_28_ = false;
            if (x148_29_didPhazonDamage)
            {
                mgr.Player()->DecrementPhazon();
                x148_29_didPhazonDamage = false;
            }

            if (x8_uid == mgr.GetLastTriggerId())
                mgr.SetLastTriggerId(kInvalidUniqueId);
        }
    }

    CEntity::AcceptScriptMsg(msg, uid, mgr);
    ;
}

CScriptTrigger::CObjectTracker* CScriptTrigger::FindObject(TUniqueId id)
{
    auto& inhabitants = GetInhabitants();
    const auto& iter = std::find(inhabitants.begin(), inhabitants.end(), id);

    if (iter != inhabitants.end())
        return &(*iter);
    return nullptr;
}

void CScriptTrigger::UpdateInhabitants(float dt, CStateManager& mgr)
{
#if 0
    bool inhabitantExited = false;
    bool player = false;
    for (auto it = xe8_inhabitants.begin(); it != xe8_inhabitants.end();)
    {
        TCastToPtr<CActor> act(mgr.ObjectById((*it).GetObjectId()));
        if (act->GetUniqueId() == mgr.Player()->GetUniqueId())
        {
            TCastToPtr<CPlayer> pl(act);
            if (bool(x12c_flags & ETriggerFlags::DetectPlayer))
            {
                player = true;
                using EPlayerMorphBallState = CPlayer::EPlayerMorphBallState;
                EPlayerMorphBallState mState = pl->GetMorphballTransitionState();
                if ((mState == EPlayerMorphBallState::Morphed &&
                     bool(x12c_flags & ETriggerFlags::DetectMorphedPlayer)) ||
                    (mState == EPlayerMorphBallState::Unmorphed &&
                     bool(x12c_flags & ETriggerFlags::DetectUnmorphedPlayer)))
                {
                    it = xe8_inhabitants.erase(it);
                    if (x148_28_)
                    {
                        x148_28_ = false;
                        if (x148_29_didPhazonDamage)
                        {
                            mgr.Player()->DecrementPhazon();
                            x148_29_didPhazonDamage = false;
                        }

                        if (mgr.GetLastTriggerId() == GetUniqueId())
                            mgr.SetLastTriggerId(kInvalidUniqueId);
                    }

                    InhabitantExited(*act, mgr);
                    continue;
                }
            }

            const auto& touchBounds = GetTouchBounds();
            const auto& actTouchBounds = act->GetTouchBounds();
            if (touchBounds && actTouchBounds)
            {
                if (actTouchBounds->intersects(*touchBounds))
                {
                    inhabitantExited = true;
                    InhabitantIdle(*act, mgr);
                    if (act->HealthInfo() && x100_damageInfo.GetDamage() > 0.f)
                        mgr.ApplyDamage(GetUniqueId(), act->GetUniqueId(), GetUniqueId(), x100_damageInfo,
                                        CMaterialFilter::MakeIncludeExclude({EMaterialTypes::Solid}, {0ull}));

                    TCastToPtr<CPhysicsActor> physAct{act};
                    if (physAct)
                    {
                        float forceMult = 1.f;
                        if (bool(x12c_flags & ETriggerFlags::UseBooleanIntersection))
                            forceMult =
                                touchBounds->booleanIntersection(*actTouchBounds).volume() / actTouchBounds->volume();

                        zeus::CVector3f force = forceMult * x11c_forceField;
                        if (bool(x12c_flags & ETriggerFlags::UseCollisionImpulses))
                        {
                            physAct->ApplyImpulseWR(force, zeus::CAxisAngle::sIdentity);
                            physAct->UseCollisionImpulses();
                        }
                        else
                            physAct->ApplyForceWR(force, zeus::CAxisAngle::sIdentity);
                    }
                }
            }
            else
            {
                it = xe8_inhabitants.erase(it);
                if (mgr.Player()->GetUniqueId() == (*it).GetObjectId())
                {
                    if (x148_28_)
                    {
                        x148_28_ = false;
                        if (x148_29_didPhazonDamage)
                        {
                            mgr.Player()->DecrementPhazon();
                            x148_29_didPhazonDamage = false;
                        }
                    }
                }

                if (mgr.GetLastTriggerId() == GetUniqueId())
                    mgr.SetLastTriggerId(kInvalidUniqueId);

                InhabitantExited(*act, mgr);
                continue;
            }
        }
        else
        {
            it = xe8_inhabitants.erase(it);
            if (mgr.Player()->GetUniqueId() == (*it).GetObjectId())
            {
                if (x148_28_)
                {
                    x148_28_ = false;
                    if (x148_29_didPhazonDamage)
                    {
                        mgr.Player()->DecrementPhazon();
                        x148_29_didPhazonDamage = false;
                    }
                }
            }

            if (mgr.GetLastTriggerId() == GetUniqueId())
                mgr.SetLastTriggerId(kInvalidUniqueId);
        }
    }

    if (bool(x12c_flags & ETriggerFlags::DetectPlayerIfInside) && x148_24_playerInside && !inhabitantExited)
    {
        SendScriptMsgs(EScriptObjectState::Inside, mgr, EScriptObjectMessage::None);
        return;
    }

    if (!player)
    {
        SendScriptMsgs(EScriptObjectState::Exited, mgr, EScriptObjectMessage::None);
        if (x148_27_deactivateOnExited)
        {
            mgr.SendScriptMsg(GetUniqueId(), mgr.GetEditorIdForUniqueId(GetUniqueId()),
                              EScriptObjectMessage::Deactivate, EScriptObjectState::Exited);
        }
    }
#endif
}

std::list<CScriptTrigger::CObjectTracker>& CScriptTrigger::GetInhabitants() { return xe8_inhabitants; }

rstl::optional_object<zeus::CAABox> CScriptTrigger::GetTouchBounds() const
{
    if (x30_24_active)
        return {GetTriggerBoundsWR()};
    return {};
}
static const CWeaponMode sktonOHurtWeaponMode = CWeaponMode(EWeaponType::Power, false, false, true);

void CScriptTrigger::Touch(CActor& act, CStateManager& mgr)
{
    if (!act.GetActive() || act.GetMaterialList().HasMaterial(EMaterialTypes::Trigger))
        return;

    if (FindObject(act.GetUniqueId()) == nullptr)
    {
        ETriggerFlags testFlags = ETriggerFlags::None;
        TCastToPtr<CPlayer> pl(act);
        if (pl)
        {

            if (x128_forceMagnitude > 0.f && (x12c_flags & ETriggerFlags::DetectPlayer) != ETriggerFlags::None &&
                mgr.GetLastTriggerId() == kInvalidUniqueId)
                mgr.SetLastTriggerId(x8_uid);

            testFlags |= ETriggerFlags::DetectPlayer;
            if (pl->GetMorphballTransitionState() == CPlayer::EPlayerMorphBallState::Unmorphed)
                testFlags |= ETriggerFlags::DetectUnmorphedPlayer;
            else if (pl->GetMorphballTransitionState() == CPlayer::EPlayerMorphBallState::Morphed)
                testFlags |= ETriggerFlags::DetectMorphedPlayer;
        }
        else if (TCastToPtr<CAi>(act))
        {
            testFlags |= ETriggerFlags::DetectAI;
        }
        else if (TCastToPtr<CGameProjectile>(act))
        {
            testFlags |= ETriggerFlags::DetectProjectiles1 | ETriggerFlags::DetectProjectiles2 |
                         ETriggerFlags::DetectProjectiles3 | ETriggerFlags::DetectProjectiles4 |
                         ETriggerFlags::DetectProjectiles5 | ETriggerFlags::DetectProjectiles6 |
                         ETriggerFlags::DetectProjectiles7;
        }
        else if (CWeapon* weap = TCastToPtr<CWeapon>(act))
        {
            if ((weap->GetAttribField() & CWeapon::EProjectileAttrib::Bombs) != CWeapon::EProjectileAttrib::None)
                testFlags |= ETriggerFlags::DetectBombs;
            else if ((weap->GetAttribField() & CWeapon::EProjectileAttrib::PowerBombs) !=
                     CWeapon::EProjectileAttrib::None)
                testFlags |= ETriggerFlags::DetectPowerBombs;
        }

        if ((testFlags & x12c_flags) != ETriggerFlags::None)
        {
            xe8_inhabitants.push_back(act.GetUniqueId());
            InhabitantAdded(act, mgr);

            if (pl)
            {
                if (x148_28_ == false)
                {
                    x148_28_ = true;
                    if (x148_29_didPhazonDamage)
                    {
                        mgr.Player()->DecrementPhazon();
                        x148_29_didPhazonDamage = false;
                    }
                    else if (x100_damageInfo.GetDamage() > 0.f)
                    {
                        const CDamageVulnerability* dVuln = mgr.Player()->GetDamageVulnerability();
                        if (dVuln->WeaponHurts(x100_damageInfo.GetWeaponMode(), 0) &&
                            x100_damageInfo.GetWeaponMode().GetType() == EWeaponType::Phazon &&
                            !mgr.GetPlayerState()->HasPowerUp(CPlayerState::EItemType::PhazonSuit))
                        {
                            pl->IncrementPhazon();
                            x148_29_didPhazonDamage = true;
                        }
                    }
                }
            }

            SendScriptMsgs(EScriptObjectState::Entered, mgr, EScriptObjectMessage::None);

            if (x148_26_deactivateOnEntered)
            {
                mgr.SendScriptMsg(x8_uid, mgr.GetEditorIdForUniqueId(x8_uid), EScriptObjectMessage::Deactivate,
                                  EScriptObjectState::Entered);
                if (act.HealthInfo() && x100_damageInfo.GetDamage() > 0.f)
                {
                    mgr.ApplyDamage(x8_uid, act.GetUniqueId(), x8_uid, x100_damageInfo,
                                    CMaterialFilter::MakeIncludeExclude({EMaterialTypes::Solid}, {0ull}),
                                    zeus::CVector3f::skZero);
                }
            }

            if ((x12c_flags & ETriggerFlags::KillOnEnter) != ETriggerFlags::None && act.HealthInfo())
            {
                CHealthInfo* hInfo = act.HealthInfo();
                mgr.ApplyDamage(
                    x8_uid, act.GetUniqueId(), x8_uid, {sktonOHurtWeaponMode, 10.f * hInfo->GetHP(), 0.f, 0.f},
                    CMaterialFilter::MakeIncludeExclude({EMaterialTypes::Solid}, {0ull}), zeus::CVector3f::skZero);
            }
        }
        else
            InhabitantRejected(act, mgr);
    }
}

zeus::CAABox CScriptTrigger::GetTriggerBoundsWR() const
{
    return {x130_bounds.min + x34_transform.origin, x130_bounds.max + x34_transform.origin};
}
}
