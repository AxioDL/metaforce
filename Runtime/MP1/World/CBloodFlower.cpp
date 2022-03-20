#include "Runtime/MP1/World/CBloodFlower.hpp"

#include <array>

#include "Runtime/CSimplePool.hpp"
#include "Runtime/CStateManager.hpp"
#include "Runtime/GameGlobalObjects.hpp"
#include "Runtime/Particle/CElementGen.hpp"
#include "Runtime/Particle/CGenDescription.hpp"
#include "Runtime/Weapon/CProjectileWeapon.hpp"
#include "Runtime/Weapon/CTargetableProjectile.hpp"
#include "Runtime/World/CPlayer.hpp"
#include "Runtime/World/CScriptTrigger.hpp"

namespace metaforce::MP1 {
CBloodFlower::CBloodFlower(TUniqueId uid, std::string_view name, const CEntityInfo& info, const zeus::CTransform& xf,
                           CModelData&& mData, const CPatternedInfo& pInfo, CAssetId partId1, CAssetId wpscId1,
                           const CActorParameters& actParms, CAssetId wpscId2, const CDamageInfo& dInfo1,
                           const CDamageInfo& dInfo2, const CDamageInfo& dInfo3, CAssetId partId2, CAssetId partId3,
                           CAssetId partId4, float f1, CAssetId partId5, u32 soundId)
: CPatterned(ECharacter::BloodFlower, uid, name, EFlavorType::Zero, info, xf, std::move(mData), pInfo,
             EMovementType::Ground, EColliderType::One, EBodyType::Restricted, actParms, EKnockBackVariant::Medium)
, x568_podEffectDesc(g_SimplePool->GetObj({FOURCC('PART'), partId1}))
, x574_podEffect(std::make_unique<CElementGen>(x568_podEffectDesc))
, x578_projectileDesc(g_SimplePool->GetObj({FOURCC('WPSC'), wpscId1}))
, x590_projectileInfo(wpscId2, dInfo1)
, x5d4_visorSfx(CSfxManager::TranslateSFXID(soundId))
, x5dc_projectileDamage(dInfo2)
, x5f8_podDamage(dInfo3)
, x614_(f1)
, x618_(partId2)
, x61c_(partId3)
, x620_(partId4) {
  x588_projectileOffset = GetModelData()->GetScale().z() * GetLocatorTransform("LCTR_FLOFLOWER"sv).origin.z();
  x574_podEffect->SetParticleEmission(false);
  x574_podEffect->SetOrientation(xf.getRotation());
  x574_podEffect->SetGlobalTranslation(xf.origin);
  x574_podEffect->SetGlobalScale(GetModelData()->GetScale());
  x590_projectileInfo.Token().Lock();
  x460_knockBackController.SetAutoResetImpulse(false);
  if (partId5.IsValid()) {
    x5c4_visorParticle = g_SimplePool->GetObj({FOURCC('PART'), partId5});
  }
}

void CBloodFlower::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid, CStateManager& mgr) {
  CPatterned::AcceptScriptMsg(msg, uid, mgr);
  if (msg == EScriptObjectMessage::Registered) {
    x450_bodyController->Activate(mgr);
    x5b8_ = GetHealthInfo(mgr)->GetHP();
  } else if (msg == EScriptObjectMessage::Damage) {
    if (x450_bodyController->IsFrozen()) {
      x450_bodyController->FrozenBreakout();
    }
    CalculateAttackTime(mgr);
    UpdateFire(mgr);
  }
}

void CBloodFlower::CalculateAttackTime(CStateManager& mgr) {
  x584_curAttackTime = x308_attackTimeVariation * -mgr.GetActiveRandom()->Float();
}

void CBloodFlower::UpdateFire(CStateManager& mgr) {
  if (x5d8_effectState == 0) {
    TurnEffectsOn(0, mgr);
  } else if (x5d8_effectState == 1) {
    TurnEffectsOff(0, mgr);
    TurnEffectsOn(1, mgr);
  } else if (x5d8_effectState == 2) {
    TurnEffectsOff(1, mgr);
    TurnEffectsOn(2, mgr);
  }

  ++x5d8_effectState;
}

constexpr std::array sFireEffects{
    "Fire1"sv,
    "Fire2"sv,
    "Fire3"sv,
};

void CBloodFlower::TurnEffectsOn(u32 effect, CStateManager& mgr) {
  GetModelData()->GetAnimationData()->SetParticleEffectState(sFireEffects[effect], true, mgr);
}

void CBloodFlower::TurnEffectsOff(u32 effect, CStateManager& mgr) {
  GetModelData()->GetAnimationData()->SetParticleEffectState(sFireEffects[effect], false, mgr);
}

void CBloodFlower::Think(float dt, CStateManager& mgr) {
  if (!GetActive())
    return;

  CPatterned::Think(dt, mgr);
  x574_podEffect->Update(dt);
  if (x5bc_projectileDelay > 0.f)
    x5bc_projectileDelay -= dt;

  x5c0_ += dt;
}

void CBloodFlower::DoUserAnimEvent(CStateManager& mgr, const CInt32POINode& node, EUserEventType type, float dt) {
  if (type == EUserEventType::Projectile) {
    if (x58c_projectileState == 1 && x5bc_projectileDelay <= 0.f) {
      LaunchPollenProjectile(GetLctrTransform(node.GetLocatorName()), mgr, x614_, 5);
      x58c_projectileState = 0;
      x5bc_projectileDelay = 0.5f;
    }
    return;
  } else if (type == EUserEventType::Delete) {
    if (x5d8_effectState > 0)
      TurnEffectsOff((x5d8_effectState > 3 ? x5d8_effectState - 1 : 2), mgr);
  }
  CPatterned::DoUserAnimEvent(mgr, node, type, dt);
}

void CBloodFlower::LaunchPollenProjectile(const zeus::CTransform& xf, CStateManager& mgr, float var_f1,
                                          s32 maxProjectiles) {
  CProjectileInfo* proj = GetProjectileInfo();
  TLockedToken<CWeaponDescription> projToken = proj->Token();

  if (!projToken || !mgr.CanCreateProjectile(GetUniqueId(), EWeaponType::AI, maxProjectiles))
    return;

  zeus::CVector3f aimPos = mgr.GetPlayer().GetAimPosition(mgr, 0.f);

  float zDiff = xf.origin.z() - aimPos.z();
  float f2 = (zDiff > 0.f ? var_f1 : -zDiff + var_f1);
  if (zDiff > 0.f)
    var_f1 = zDiff + var_f1;
  float f7 = std::sqrt(2.f * f2 / 4.905f) + std::sqrt(2.f * var_f1 / 4.905f);
  float f4 = 1.f / f7;
  zeus::CVector3f vel{f4 * (aimPos.x() - xf.origin.x()), f4 * (aimPos.y() - xf.origin.y()),
                      2.4525f * f7 + (-zDiff / f7)};
  if (CTargetableProjectile* targProj =
          CreateArcProjectile(mgr, GetProjectileInfo()->Token(), zeus::CTransform::Translate(xf.origin),
                              GetProjectileInfo()->GetDamage(), kInvalidUniqueId)) {
    targProj->ProjectileWeapon().SetVelocity(CProjectileWeapon::GetTickPeriod() * vel);
    targProj->ProjectileWeapon().SetGravity(CProjectileWeapon::GetTickPeriod() * zeus::CVector3f(0.f, 0.f, -4.905f));
    mgr.AddObject(targProj);
  }
}

void CBloodFlower::Render(CStateManager& mgr) {
  CPatterned::Render(mgr);
  x574_podEffect->Render();
}

EWeaponCollisionResponseTypes CBloodFlower::GetCollisionResponseType(const zeus::CVector3f&, const zeus::CVector3f&,
                                                                     const CWeaponMode& weaponMode,
                                                                     EProjectileAttrib) const {
  const auto* const damageVulnerability = GetDamageVulnerability();

  if (damageVulnerability->WeaponHurts(weaponMode, false)) {
    return EWeaponCollisionResponseTypes::Unknown28;
  }

  return EWeaponCollisionResponseTypes::Unknown78;
}

bool CBloodFlower::ShouldAttack(CStateManager& mgr, float arg) {
  if (TooClose(mgr, 0.f))
    return false;

  if (x584_curAttackTime <= x304_averageAttackTime)
    return false;

  return (mgr.GetPlayer().GetTranslation().z() + mgr.GetPlayer().GetEyeHeight() <
          x588_projectileOffset + x614_ + GetTranslation().z());
}

bool CBloodFlower::ShouldTurn(CStateManager& mgr, float) {
  if (TooClose(mgr, 0.f))
    return false;

  zeus::CVector3f frontVec = GetTransform().basis[1];
  frontVec.z() = 0.f;
  frontVec.normalize();
  zeus::CVector3f posDiff = mgr.GetPlayer().GetTranslation() - GetTranslation();
  posDiff.z() = 0.f;
  posDiff.normalize();

  return posDiff.dot(frontVec) < 0.99599999f;
}

void CBloodFlower::Active(CStateManager& mgr, EStateMsg msg, float arg) {
  if (msg == EStateMsg::Activate) {
    x32c_animState = EAnimState::Ready;
    CalculateAttackTime(mgr);
  } else if (msg == EStateMsg::Update) {
    TryCommand(mgr, pas::EAnimationState::LoopReaction, &CPatterned::TryLoopReaction, 0);
    x450_bodyController->GetCommandMgr().DeliverCmd(CBCAdditiveAimCmd());
    x584_curAttackTime += arg;
    x450_bodyController->GetCommandMgr().DeliverAdditiveTargetVector(
        GetTransform().transposeRotate(mgr.GetPlayer().GetTranslation() - GetTranslation()));
  } else if (msg == EStateMsg::Deactivate) {
    x450_bodyController->GetCommandMgr().DeliverCmd(CBodyStateCmd(EBodyStateCmd::ExitState));
    x450_bodyController->GetCommandMgr().DeliverCmd(CBodyStateCmd(EBodyStateCmd::AdditiveIdle));
  }
}

void CBloodFlower::InActive(CStateManager& mgr, EStateMsg msg, float) {
  if (msg == EStateMsg::Activate) {
    x450_bodyController->SetLocomotionType(pas::ELocomotionType::Relaxed);
    x400_24_hitByPlayerProjectile = false;
  } else if (msg == EStateMsg::Deactivate) {
    x5c0_ = 0.f;
  }
}

void CBloodFlower::BulbAttack(CStateManager& mgr, EStateMsg msg, float) {
  if (msg == EStateMsg::Activate) {
    x450_bodyController->GetCommandMgr().DeliverCmd(
        CBCProjectileAttackCmd(pas::ESeverity::Zero, mgr.GetPlayer().GetTranslation(), true));
    x58c_projectileState = 1;
  }
}

void CBloodFlower::PodAttack(CStateManager& mgr, EStateMsg msg, float arg) {
  if (msg == EStateMsg::Activate) {
    x450_bodyController->GetCommandMgr().DeliverCmd(CBCMeleeAttackCmd(pas::ESeverity::Zero));
    x574_podEffect->SetParticleEmission(true);
    ActivateTriggers(mgr, true);
  } else if (msg == EStateMsg::Update) {
    if (!TooClose(mgr, 0.f)) {
      return;
    }

    mgr.ApplyDamage(GetUniqueId(), mgr.GetPlayer().GetUniqueId(), GetUniqueId(), x5f8_podDamage,
                    CMaterialFilter::MakeIncludeExclude({EMaterialTypes::Solid}, {}), {});
  } else if (msg == EStateMsg::Deactivate) {
    x574_podEffect->SetParticleEmission(false);
    ActivateTriggers(mgr, false);
    x450_bodyController->GetCommandMgr().DeliverCmd(CBCKnockBackCmd({}, pas::ESeverity::One));
  }
}

void CBloodFlower::ActivateTriggers(CStateManager& mgr, bool activate) {
  for (const SConnection& conn : GetConnectionList()) {
    auto search = mgr.GetIdListForScript(conn.x8_objId);
    for (auto it = search.first; it != search.second; ++it) {
      if (TCastToPtr<CScriptTrigger> trigger = mgr.ObjectById(it->second)) {
        mgr.SendScriptMsg(trigger, GetUniqueId(),
                          (activate ? EScriptObjectMessage::Activate : EScriptObjectMessage::Deactivate));
      }
    }
  }
}

CTargetableProjectile* CBloodFlower::CreateArcProjectile(CStateManager& mgr, const TToken<CWeaponDescription>& desc,
                                                         const zeus::CTransform& xf, const CDamageInfo& damage,
                                                         TUniqueId uid) {

  if (!x578_projectileDesc) {
    return nullptr;
  }

  TUniqueId projId = mgr.AllocateUniqueId();
  auto* targProj = new CTargetableProjectile(
      desc, EWeaponType::AI, xf, EMaterialTypes::Character, damage, x5dc_projectileDamage, projId, GetAreaIdAlways(),
      GetUniqueId(), x578_projectileDesc, uid, EProjectileAttrib::None, {x5c4_visorParticle}, x5d4_visorSfx, false);
  if (mgr.GetPlayer().GetOrbitTargetId() == GetUniqueId()) {
    mgr.GetPlayer().ResetAimTargetPrediction(projId);
    mgr.GetPlayer().SetOrbitTargetId(projId, mgr);
  }

  return targProj;
}

} // namespace metaforce::MP1
