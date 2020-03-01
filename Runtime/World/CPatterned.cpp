#include "Runtime/World/CPatterned.hpp"

#include "Runtime/CStateManager.hpp"
#include "Runtime/CSimplePool.hpp"
#include "Runtime/GameGlobalObjects.hpp"
#include "Runtime/Camera/CFirstPersonCamera.hpp"
#include "Runtime/Character/CAnimData.hpp"
#include "Runtime/Character/CPASAnimParmData.hpp"
#include "Runtime/Graphics/CSkinnedModel.hpp"
#include "Runtime/MP1/World/CMetroid.hpp"
#include "Runtime/MP1/World/CSpacePirate.hpp"
#include "Runtime/Weapon/CEnergyProjectile.hpp"
#include "Runtime/Weapon/CGameProjectile.hpp"
#include "Runtime/World/CActorParameters.hpp"
#include "Runtime/World/CExplosion.hpp"
#include "Runtime/World/CPathFindSearch.hpp"
#include "Runtime/World/CPatternedInfo.hpp"
#include "Runtime/World/CPlayer.hpp"
#include "Runtime/World/CScriptActorKeyframe.hpp"
#include "Runtime/World/CScriptCoverPoint.hpp"
#include "Runtime/World/CScriptWaypoint.hpp"
#include "Runtime/World/CStateMachine.hpp"

#include "TCastTo.hpp" // Generated file, do not modify include path

namespace urde {

constexpr zeus::CColor CPatterned::skDamageColor(0.5f, 0.f, 0.f);
constexpr CMaterialList skPatternedGroundMaterialList(EMaterialTypes::Character, EMaterialTypes::Solid,
                                                      EMaterialTypes::Orbit, EMaterialTypes::GroundCollider,
                                                      EMaterialTypes::Target);
constexpr CMaterialList skPatternedFlyerMaterialList(EMaterialTypes::Character, EMaterialTypes::Solid,
                                                     EMaterialTypes::Orbit, EMaterialTypes::Target);

CPatterned::CPatterned(ECharacter character, TUniqueId uid, std::string_view name, CPatterned::EFlavorType flavor,
                       const CEntityInfo& info, const zeus::CTransform& xf, CModelData&& mData,
                       const CPatternedInfo& pInfo, CPatterned::EMovementType moveType,
                       CPatterned::EColliderType colliderType, EBodyType bodyType, const CActorParameters& actorParms,
                       EKnockBackVariant kbVariant)
: CAi(uid, pInfo.xf8_active, name, info, xf, std::move(mData),
      zeus::CAABox(pInfo.xcc_bodyOrigin - zeus::CVector3f{pInfo.xc4_halfExtent, pInfo.xc4_halfExtent, 0.f},
                   pInfo.xcc_bodyOrigin +
                       zeus::CVector3f{pInfo.xc4_halfExtent, pInfo.xc4_halfExtent, pInfo.xc8_height}),
      pInfo.x0_mass, pInfo.x54_healthInfo, pInfo.x5c_damageVulnerability,
      moveType == EMovementType::Flyer ? skPatternedFlyerMaterialList : skPatternedGroundMaterialList,
      pInfo.xfc_stateMachineId, actorParms, pInfo.xd8_stepUpHeight, 0.8f)
, x2fc_minAttackRange(pInfo.x18_minAttackRange)
, x300_maxAttackRange(pInfo.x1c_maxAttackRange)
, x304_averageAttackTime(pInfo.x20_averageAttackTime)
, x308_attackTimeVariation(pInfo.x24_attackTimeVariation)
, x328_25_verticalMovement(moveType == EMovementType::Flyer)
, x328_27_onGround(moveType != EMovementType::Flyer)
, x34c_character(character)
, x388_anim(pInfo.GetAnimationParameters().GetInitialAnimation())
, x3b4_speed(pInfo.x4_speed)
, x3b8_turnSpeed(pInfo.x8_turnSpeed)
, x3bc_detectionRange(pInfo.xc_detectionRange)
, x3c0_detectionHeightRange(pInfo.x10_detectionHeightRange)
, x3c4_detectionAngle(std::cos(zeus::degToRad(pInfo.x14_dectectionAngle)))
, x3c8_leashRadius(pInfo.x28_leashRadius)
, x3cc_playerLeashRadius(pInfo.x2c_playerLeashRadius)
, x3d0_playerLeashTime(pInfo.x30_playerLeashTime)
, x3d8_xDamageThreshold(pInfo.xdc_xDamage)
, x3dc_frozenXDamageThreshold(pInfo.xe0_frozenXDamage)
, x3e0_xDamageDelay(pInfo.xe4_xDamageDelay)
, x3fc_flavor(flavor)
, x400_31_isFlyer(moveType == CPatterned::EMovementType::Flyer)
, x402_30_updateThermalFrozenState(x402_31_thawed = actorParms.HasThermalHeat())
, x460_knockBackController(kbVariant) {
  x404_contactDamage = pInfo.x34_contactDamageInfo;
  x424_damageWaitTime = pInfo.x50_damageWaitTime;
  x454_deathSfx = pInfo.xe8_deathSfx;
  x458_iceShatterSfx = pInfo.x134_iceShatterSfx;
  x4f4_intoFreezeDur = pInfo.x100_intoFreezeDur;
  x4f8_outofFreezeDur = pInfo.x104_outofFreezeDur;
  x4fc_ = pInfo.x108_;
  x508_colliderType = colliderType;
  x50c_baseDamageMag = actorParms.GetThermalMag();
  x514_deathExplosionOffset = pInfo.x110_particle1Scale;
  x540_iceDeathExplosionOffset = pInfo.x124_particle2Scale;

  if (pInfo.x11c_particle1.IsValid())
    x520_deathExplosionParticle = {g_SimplePool->GetObj({FOURCC('PART'), pInfo.x11c_particle1})};

  if (pInfo.x120_electric.IsValid())
    x530_deathExplosionElectric = {g_SimplePool->GetObj({FOURCC('ELSC'), pInfo.x120_electric})};

  if (pInfo.x130_particle2.IsValid())
    x54c_iceDeathExplosionParticle = {g_SimplePool->GetObj({FOURCC('PART'), pInfo.x130_particle2})};

  if (x404_contactDamage.GetRadius() > 0.f)
    x404_contactDamage.SetRadius(0.f);

  xe6_29_renderParticleDBInside = false;
  if (x64_modelData) {
    x402_27_noXrayModel = !x64_modelData->HasModel(CModelData::EWhichModel::XRay);
    BuildBodyController(bodyType);
  }
}

void CPatterned::Accept(urde::IVisitor& visitor) { visitor.Visit(this); }

void CPatterned::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid, CStateManager& mgr) {
  CAi::AcceptScriptMsg(msg, uid, mgr);

  switch (msg) {
  case EScriptObjectMessage::Registered: {
    if (x508_colliderType != EColliderType::One) {
      CMaterialList include = GetMaterialFilter().GetIncludeList();
      CMaterialList exclude = GetMaterialFilter().GetExcludeList();
      include.Remove(EMaterialTypes::Character);
      exclude.Add(EMaterialTypes::Character);
      SetMaterialFilter(CMaterialFilter::MakeIncludeExclude(include, exclude));
    }

    if (HasModelData() && GetModelData()->HasAnimData() && GetModelData()->GetAnimationData()->GetIceModel()) {
      const auto& baseAABB = GetBaseBoundingBox();
      float diagExtent = (baseAABB.max - baseAABB.min).magnitude() * 0.5f;
      x510_vertexMorph = std::make_shared<CVertexMorphEffect>(zeus::skRight, zeus::CVector3f{}, diagExtent,
                                                              0.f, *mgr.GetActiveRandom());
    }

    xf8_25_angularEnabled = true;
    break;
  }
  case EScriptObjectMessage::OnFloor: {
    if (!x328_25_verticalMovement) {
      x150_momentum = {};
      AddMaterial(EMaterialTypes::GroundCollider, mgr);
    }
    x328_27_onGround = true;
    break;
  }
  case EScriptObjectMessage::Falling: {
    if (!x328_25_verticalMovement) {
      if (x450_bodyController->GetPercentageFrozen() == 0.f) {
        x150_momentum = {0.f, 0.f, -GetWeight()};
        RemoveMaterial(EMaterialTypes::GroundCollider, mgr);
      }
    }
    x328_27_onGround = false;
    break;
  }
  case EScriptObjectMessage::Activate:
    x3a0_latestLeashPosition = GetTranslation();
    break;
  case EScriptObjectMessage::Deleted:
    if (x330_stateMachineState.GetActorState() != nullptr)
      x330_stateMachineState.GetActorState()->CallFunc(mgr, *this, EStateMsg::Deactivate, 0.f);
    break;
  case EScriptObjectMessage::Damage: {
    if (TCastToConstPtr<CGameProjectile> proj = mgr.GetObjectById(uid)) {
      const CDamageInfo& info = proj->GetDamageInfo();
      if (info.GetWeaponMode().GetType() == EWeaponType::Wave) {
        if (x460_knockBackController.x81_26_enableShock && info.GetWeaponMode().IsComboed() && HealthInfo(mgr)) {
          x401_31_nextPendingShock = true;
          KnockBack(GetTransform().frontVector(), mgr, info, EKnockBackType::Radius, false, info.GetKnockBackPower());
          x460_knockBackController.DeferKnockBack(EWeaponType::Wave);
        }
      } else if (info.GetWeaponMode().GetType() == EWeaponType::Plasma) {
        if (x460_knockBackController.x81_27_enableBurn && info.GetWeaponMode().IsCharged() && HealthInfo(mgr)) {
          KnockBack(GetTransform().frontVector(), mgr, info, EKnockBackType::Radius, false, info.GetKnockBackPower());
          x460_knockBackController.DeferKnockBack(EWeaponType::Plasma);
        }
      }
      if (mgr.GetPlayer().GetUniqueId() == proj->GetOwnerId())
        x400_24_hitByPlayerProjectile = true;
    }
    break;
  }
  case EScriptObjectMessage::InvulnDamage: {
    if (TCastToConstPtr<CGameProjectile> proj = mgr.GetObjectById(uid)) {
      if (proj->GetOwnerId() == mgr.GetPlayer().GetUniqueId())
        x400_24_hitByPlayerProjectile = true;
    }
    break;
  }
  default:
    break;
  }
}

void CPatterned::MakeThermalColdAndHot() {
  x403_24_keepThermalVisorState = true;
  xe6_27_thermalVisorFlags = 3;
}

void CPatterned::UpdateThermalFrozenState(bool thawed) {
  x402_31_thawed = thawed;
  if (x403_24_keepThermalVisorState)
    return;
  xe6_27_thermalVisorFlags = u8(thawed ? 2 : 1);
}

void CPatterned::Think(float dt, CStateManager& mgr) {
  if (!GetActive())
    return;

  if (x402_30_updateThermalFrozenState)
    UpdateThermalFrozenState(x450_bodyController->GetPercentageFrozen() == 0.f);

  if (x64_modelData->GetAnimationData()->GetIceModel())
    x510_vertexMorph->Update(dt);

  if (x402_26_dieIf80PercFrozen) {
    float froz = x450_bodyController->GetPercentageFrozen();
    if (froz > 0.8f)
      x400_29_pendingMassiveFrozenDeath = true;
  }

  if (!x400_25_alive) {
    if ((x400_28_pendingMassiveDeath || x400_29_pendingMassiveFrozenDeath) && x3e0_xDamageDelay <= 0.f) {
      if (x400_29_pendingMassiveFrozenDeath) {
        SendScriptMsgs(EScriptObjectState::AboutToMassivelyDie, mgr, EScriptObjectMessage::None);
        MassiveFrozenDeath(mgr);
      } else {
        SendScriptMsgs(EScriptObjectState::AboutToMassivelyDie, mgr, EScriptObjectMessage::None);
        MassiveDeath(mgr);
      }
    } else {
      x3e0_xDamageDelay -= dt;
      if (x403_26_stateControlledMassiveDeath && x330_stateMachineState.GetName()) {
        bool isDead = x330_stateMachineState.GetName() == "Dead"sv;
        if (isDead && x330_stateMachineState.x8_time > 15.f)
          MassiveDeath(mgr);
      }
    }
  }

  UpdateAlphaDelta(dt, mgr);

  x3e4_lastHP = HealthInfo(mgr)->GetHP();
  if (!x330_stateMachineState.x4_state)
    x330_stateMachineState.SetState(mgr, *this, GetStateMachine(), "Start"sv);

  zeus::CVector3f diffVec = x4e4_latestPredictedTranslation - GetTranslation();
  if (!x328_25_verticalMovement)
    diffVec.z() = 0.f;

  if (diffVec.magSquared() > (0.1f * dt))
    x4f0_predictedLeashTime += dt;
  else
    x4f0_predictedLeashTime = 0.f;

  if (x460_knockBackController.x81_26_enableShock) {
    /* Shock on logical falling edge */
    if (!x401_31_nextPendingShock && x402_24_pendingShock)
      Shock(mgr, 0.5f + mgr.GetActiveRandom()->Range(0.f, 0.5f), 0.2f);
    x402_24_pendingShock = x401_31_nextPendingShock;
    x401_31_nextPendingShock = false;

    if (x450_bodyController->IsElectrocuting()) {
      mgr.GetActorModelParticles()->StartElectric(*this);
      if (x3f0_pendingShockDamage > 0.f && x400_25_alive) {
        const CDamageInfo dInfo(CDamageInfo{CWeaponMode{EWeaponType::Wave}, x3f0_pendingShockDamage, 0.f, 0.f}, dt);
        mgr.ApplyDamage(kInvalidUniqueId, GetUniqueId(), kInvalidUniqueId, dInfo,
                        CMaterialFilter::MakeIncludeExclude({EMaterialTypes::Solid}, {}), {});
      }
    } else {
      if (x3f0_pendingShockDamage != 0.f) {
        x450_bodyController->StopElectrocution();
        mgr.GetActorModelParticles()->StopElectric(*this);
      }
    }
  }

  if (x450_bodyController->IsOnFire()) {
    if (x400_25_alive) {
      mgr.GetActorModelParticles()->LightDudeOnFire(*this);
      const CDamageInfo dInfo(CDamageInfo{CWeaponMode{EWeaponType::Plasma}, x3ec_pendingFireDamage, 0.f, 0.f}, dt);
      mgr.ApplyDamage(kInvalidUniqueId, GetUniqueId(), kInvalidUniqueId, dInfo,
                      CMaterialFilter::MakeIncludeExclude({EMaterialTypes::Solid}, {}), {});
    }
  } else {
    if (x3ec_pendingFireDamage > 0.f)
      x3ec_pendingFireDamage = 0.f;
    if (x450_bodyController->IsFrozen())
      mgr.GetActorModelParticles()->StopThermalHotParticles(*this);
  }

  if (x401_27_phazingOut || x401_28_burning)
    x3e8_alphaDelta = -0.33333334f;

  if (x401_30_pendingDeath) {
    x401_30_pendingDeath = false;
    Death(mgr, GetTransform().frontVector(), EScriptObjectState::DeathRattle);
  }

  float thinkDt = (x400_25_alive ? dt : dt * CalcDyingThinkRate());

  x450_bodyController->Update(thinkDt, mgr);
  x450_bodyController->MultiplyPlaybackRate(x3b4_speed);
  SAdvancementDeltas deltas = UpdateAnimation(thinkDt, mgr, !x450_bodyController->IsFrozen());
  x434_posDelta = deltas.x0_posDelta;
  x440_rotDelta = deltas.xc_rotDelta;

  if (x403_25_enableStateMachine && x450_bodyController->GetPercentageFrozen() < 1.f)
    x330_stateMachineState.Update(mgr, *this, thinkDt);

  ThinkAboutMove(thinkDt);

  x460_knockBackController.Update(thinkDt, mgr, *this);
  x4e4_latestPredictedTranslation = GetTranslation() + PredictMotion(thinkDt).x0_translation;
  x328_26_solidCollision = false;
  if (x420_curDamageRemTime > 0.f)
    x420_curDamageRemTime -= dt;

  if (x401_28_burning && x3f4_burnThinkRateTimer > dt)
    x3f4_burnThinkRateTimer -= dt;

  xd0_damageMag = x50c_baseDamageMag;
  UpdateDamageColor(dt);

  if (!x450_bodyController->IsFrozen()) {
    if (x3a0_latestLeashPosition == zeus::CVector3f())
      x3a0_latestLeashPosition = GetTranslation();

    if (x3cc_playerLeashRadius != 0.f) {
      zeus::CVector3f diffVec = (GetTranslation() - mgr.GetPlayer().GetTranslation());
      if (diffVec.magSquared() > x3cc_playerLeashRadius * x3cc_playerLeashRadius)
        x3d4_curPlayerLeashTime += dt;
      else
        x3d4_curPlayerLeashTime = 0.f;
    }
  } else {
    RemoveEmitter();
  }

  if (x2f8_waypointPauseRemTime > 0.f)
    x2f8_waypointPauseRemTime -= dt;
}

void CPatterned::CollidedWith(TUniqueId other, const CCollisionInfoList& list, CStateManager& mgr) {
  if (x420_curDamageRemTime <= 0.f) {
    if (TCastToPtr<CPlayer> player = mgr.ObjectById(other)) {
      bool jumpOnHead = player->GetTimeSinceJump() < 5.f && list.GetCount() != 0 &&
                        list.Front().GetNormalLeft().z() > 0.707f;
      if (x400_25_alive || jumpOnHead) {
        CDamageInfo cDamage = GetContactDamage();
        if (!x400_25_alive || x450_bodyController->IsFrozen())
          cDamage.SetDamage(0.f);
        if (jumpOnHead) {
          mgr.ApplyDamage(GetUniqueId(), player->GetUniqueId(), GetUniqueId(), cDamage,
                          CMaterialFilter::skPassEverything, -player->GetVelocity());
          player->ResetTimeSinceJump();
        } else if (x400_25_alive && !x450_bodyController->IsFrozen()) {
          mgr.ApplyDamage(GetUniqueId(), player->GetUniqueId(), GetUniqueId(), cDamage,
                          CMaterialFilter::MakeIncludeExclude({EMaterialTypes::Solid}, {}), zeus::skZero3f);
        }
        x420_curDamageRemTime = x424_damageWaitTime;
      }
    }
  }
  static constexpr CMaterialList testList(EMaterialTypes::Solid, EMaterialTypes::Ceiling, EMaterialTypes::Wall,
                                          EMaterialTypes::Floor, EMaterialTypes::Character);
  for (const CCollisionInfo& info : list) {
    if (info.GetMaterialLeft().Intersection(testList)) {
      if (!info.GetMaterialLeft().HasMaterial(EMaterialTypes::Floor)) {
        if (!x310_moveVec.isZero() && info.GetNormalLeft().dot(x310_moveVec) >= 0.f)
          continue;
      } else if (!x400_31_isFlyer) {
        continue;
      }
      x328_26_solidCollision = true;
      return;
    }
  }
  CPhysicsActor::CollidedWith(other, list, mgr);
}

void CPatterned::Touch(CActor& act, CStateManager& mgr) {
  if (!x400_25_alive)
    return;

  if (TCastToPtr<CGameProjectile> proj = act) {
    if (mgr.GetPlayer().GetUniqueId() == proj->GetOwnerId())
      x400_24_hitByPlayerProjectile = true;
  }
}

std::optional<zeus::CAABox> CPatterned::GetTouchBounds() const { return GetBoundingBox(); }

bool CPatterned::CanRenderUnsorted(const urde::CStateManager& mgr) const {
  return x64_modelData->GetAnimationData()->GetParticleDB().AreAnySystemsDrawnWithModel()
             ? false
             : CActor::CanRenderUnsorted(mgr);
}

zeus::CVector3f CPatterned::GetAimPosition(const urde::CStateManager& mgr, float dt) const {
  zeus::CVector3f offset;
  if (dt > 0.f) {
    offset = PredictMotion(dt).x0_translation;
  }

  const CSegId segId = GetModelData()->GetAnimationData()->GetLocatorSegId("lockon_target_LCTR"sv);
  if (segId.IsValid()) {
    const zeus::CTransform xf = GetModelData()->GetAnimationData()->GetLocatorTransform(segId, nullptr);
    const zeus::CVector3f scaledOrigin = GetModelData()->GetScale() * xf.origin;

    if (const auto tb = GetTouchBounds()) {
      return offset + tb->clampToBox(x34_transform * scaledOrigin);
    }

    const zeus::CAABox aabox = GetBaseBoundingBox();
    const zeus::CAABox primBox(aabox.min + GetPrimitiveOffset(), aabox.max + GetPrimitiveOffset());

    return offset + (x34_transform * primBox.clampToBox(scaledOrigin));
  }

  return offset + GetBoundingBox().center();
}

zeus::CTransform CPatterned::GetLctrTransform(std::string_view name) const {
  return x34_transform * GetScaledLocatorTransform(name);
}

zeus::CTransform CPatterned::GetLctrTransform(CSegId id) const {
  zeus::CTransform xf = x64_modelData->GetAnimationData()->GetLocatorTransform(id, nullptr);
  return x34_transform * zeus::CTransform(xf.buildMatrix3f(), x64_modelData->GetScale() * xf.origin);
}

void CPatterned::DeathDelete(CStateManager& mgr) {
  SendScriptMsgs(EScriptObjectState::Dead, mgr, EScriptObjectMessage::None);
  if (x450_bodyController->IsElectrocuting()) {
    x3f0_pendingShockDamage = 0.f;
    x450_bodyController->StopElectrocution();
    mgr.GetActorModelParticles()->StopElectric(*this);
  }
  mgr.FreeScriptObject(GetUniqueId());
}

void CPatterned::Death(CStateManager& mgr, const zeus::CVector3f& dir, EScriptObjectState state) {
  if (x400_25_alive) {
    if (!x450_bodyController->IsOnFire()) {
      x402_25_lostMassiveFrozenHP = (x3e4_lastHP - HealthInfo(mgr)->GetHP()) >= x3dc_frozenXDamageThreshold;
      if (x402_25_lostMassiveFrozenHP && x54c_iceDeathExplosionParticle &&
          x450_bodyController->GetPercentageFrozen() > 0.8f) {
        x400_29_pendingMassiveFrozenDeath = true;
      } else if ((x3e4_lastHP - HealthInfo(mgr)->GetHP()) >= x3d8_xDamageThreshold) {
        x400_28_pendingMassiveDeath = true;
      }
    }
    if (x400_28_pendingMassiveDeath || x400_29_pendingMassiveFrozenDeath) {
      if (x328_30_lookAtDeathDir && x3e0_xDamageDelay <= 0.f && dir != zeus::skZero3f) {
        SetTransform(zeus::lookAt(GetTranslation(), GetTranslation() - dir) *
                     zeus::CTransform::RotateX(zeus::degToRad(45.f)));
      }
    } else {
      x330_stateMachineState.SetState(mgr, *this, GetStateMachine(), "Dead"sv);
      RemoveMaterial(EMaterialTypes::GroundCollider, mgr);
      x328_25_verticalMovement = false;
    }
    x400_25_alive = false;
    if (x450_bodyController->HasBodyState(pas::EAnimationState::Hurled) &&
        x450_bodyController->GetBodyType() == EBodyType::Flyer) {
      x450_bodyController->GetCommandMgr().DeliverCmd(CBCHurledCmd(-dir, zeus::skZero3f));
    } else if (x450_bodyController->HasBodyState(pas::EAnimationState::Fall)) {
      x450_bodyController->GetCommandMgr().DeliverCmd(CBCKnockDownCmd(-dir, pas::ESeverity::One));
    }
    if (state != EScriptObjectState::Any) {
      SendScriptMsgs(state, mgr, EScriptObjectMessage::None);
    }
  }
}

void CPatterned::KnockBack(const zeus::CVector3f& backVec, CStateManager& mgr, const CDamageInfo& info,
                           EKnockBackType type, bool inDeferred, float magnitude) {
  CHealthInfo* hInfo = HealthInfo(mgr);
  if (!x401_27_phazingOut && !x401_28_burning && hInfo != nullptr) {
    x460_knockBackController.KnockBack(backVec, mgr, *this, info, type, magnitude);
    if (x450_bodyController->IsFrozen() && x460_knockBackController.GetActiveParms().xc_intoFreezeDur >= 0.f) {
      x450_bodyController->FrozenBreakout();
    }
    switch (x460_knockBackController.GetActiveParms().x4_animFollowup) {
    case EKnockBackAnimationFollowUp::Freeze:
      Freeze(mgr, zeus::skZero3f, zeus::CUnitVector3f(x34_transform.transposeRotate(backVec)),
             x460_knockBackController.GetActiveParms().x8_followupDuration);
      break;
    case EKnockBackAnimationFollowUp::PhazeOut:
      PhazeOut(mgr);
      break;
    case EKnockBackAnimationFollowUp::Shock:
      Shock(mgr, x460_knockBackController.GetActiveParms().x8_followupDuration, -1.f);
      break;
    case EKnockBackAnimationFollowUp::Burn:
      Burn(x460_knockBackController.GetActiveParms().x8_followupDuration, 0.25f);
      break;
    case EKnockBackAnimationFollowUp::LaggedBurnDeath:
      x401_29_laggedBurnDeath = true;
      [[fallthrough]];
    case EKnockBackAnimationFollowUp::BurnDeath:
      Burn(x460_knockBackController.GetActiveParms().x8_followupDuration, -1.f);
      Death(mgr, zeus::skZero3f, EScriptObjectState::DeathRattle);
      x400_28_pendingMassiveDeath = x400_29_pendingMassiveFrozenDeath = false;
      x400_27_fadeToDeath = x401_28_burning = true;
      x3f4_burnThinkRateTimer = 1.5f;
      x402_29_drawParticles = false;
      x450_bodyController->DouseFlames();
      mgr.GetActorModelParticles()->StopThermalHotParticles(*this);
      mgr.GetActorModelParticles()->StartBurnDeath(*this);
      if (!x401_29_laggedBurnDeath) {
        mgr.GetActorModelParticles()->EnsureFirePopLoaded(*this);
        mgr.GetActorModelParticles()->EnsureIceBreakLoaded(*this);
      }
      break;
    case EKnockBackAnimationFollowUp::Death:
      Death(mgr, zeus::skZero3f, EScriptObjectState::DeathRattle);
      break;
    case EKnockBackAnimationFollowUp::ExplodeDeath:
      Death(mgr, zeus::skZero3f, EScriptObjectState::DeathRattle);
      if (GetDeathExplosionParticle() || x530_deathExplosionElectric)
        MassiveDeath(mgr);
      else if (x450_bodyController->IsFrozen())
        x450_bodyController->FrozenBreakout();
      break;
    case EKnockBackAnimationFollowUp::IceDeath:
      Death(mgr, zeus::skZero3f, EScriptObjectState::DeathRattle);
      if (x54c_iceDeathExplosionParticle) {
        MassiveFrozenDeath(mgr);
      } else if (x450_bodyController->IsFrozen()) {
        x450_bodyController->FrozenBreakout();
      }
      break;
    default:
      break;
    }
  }
}

void CPatterned::TakeDamage(const zeus::CVector3f&, float arg) { x428_damageCooldownTimer = 0.33f; }

bool CPatterned::FixedRandom(CStateManager&, float arg) {
  return x330_stateMachineState.GetRandom() < x330_stateMachineState.x14_;
}

bool CPatterned::Random(CStateManager&, float arg) { return x330_stateMachineState.GetRandom() < arg; }

bool CPatterned::CodeTrigger(CStateManager&, float arg) { return x330_stateMachineState.x18_24_codeTrigger; }

bool CPatterned::FixedDelay(CStateManager&, float arg) {
  return x330_stateMachineState.GetTime() > x330_stateMachineState.GetDelay();
}

bool CPatterned::RandomDelay(CStateManager&, float arg) {
  return x330_stateMachineState.GetTime() > arg * x330_stateMachineState.GetRandom();
}

bool CPatterned::Delay(CStateManager&, float arg) { return x330_stateMachineState.GetTime() > arg; }

bool CPatterned::PatrolPathOver(CStateManager&, float arg) { return x2dc_destObj == kInvalidUniqueId; }

bool CPatterned::Stuck(CStateManager&, float arg) { return x4f0_predictedLeashTime > 0.2f; }

bool CPatterned::AnimOver(CStateManager&, float arg) { return x32c_animState == EAnimState::Over; }

bool CPatterned::InPosition(CStateManager&, float arg) { return x328_24_inPosition; }

bool CPatterned::HasPatrolPath(CStateManager& mgr, float arg) {
  return GetWaypointForState(mgr, EScriptObjectState::Patrol, EScriptObjectMessage::Follow) != kInvalidUniqueId;
}

bool CPatterned::Attacked(CStateManager&, float arg) { return x400_24_hitByPlayerProjectile; }

bool CPatterned::PatternShagged(CStateManager&, float arg) { return x400_30_patternShagged; }

bool CPatterned::PatternOver(CStateManager&, float arg) { return x38c_patterns.size() <= x39c_curPattern; }

bool CPatterned::HasRetreatPattern(CStateManager& mgr, float arg) {
  return GetWaypointForState(mgr, EScriptObjectState::Retreat, EScriptObjectMessage::Follow) != kInvalidUniqueId;
}

bool CPatterned::HasAttackPattern(CStateManager& mgr, float arg) {
  return GetWaypointForState(mgr, EScriptObjectState::Attack, EScriptObjectMessage::Follow) != kInvalidUniqueId;
}

bool CPatterned::NoPathNodes(CStateManager&, float arg) {
  if (CPathFindSearch* search = GetSearchPath())
    return search->OnPath(GetTranslation()) != CPathFindSearch::EResult::Success;
  return true;
}

constexpr float skActorApproachDistance = 3.f;

bool CPatterned::PathShagged(CStateManager&, float arg) {
  if (CPathFindSearch* search = GetSearchPath()) {
    if (search->IsShagged())
      return true;
    if (search->GetCurrentWaypoint() > 0 && x401_24_pathOverCount == 0) {
      zeus::CVector3f origPoint = GetTranslation() + 0.3f * zeus::skUp;
      zeus::CVector3f point = origPoint;
      search->GetSplinePoint(point, GetTranslation());
      return (point - origPoint).magSquared() > 4.f * skActorApproachDistance * skActorApproachDistance;
    }
  }
  return false;
}

bool CPatterned::PathFound(CStateManager&, float arg) {
  if (CPathFindSearch* search = GetSearchPath())
    if (!search->IsShagged())
      return true;
  return false;
}

bool CPatterned::PathOver(CStateManager&, float arg) {
  if (CPathFindSearch* search = GetSearchPath())
    if (x328_25_verticalMovement || x328_27_onGround)
      if (!search->IsShagged() && search->IsOver())
        return true;
  return false;
}

bool CPatterned::Landed(CStateManager&, float arg) {
  bool ret = x328_27_onGround && !x328_28_prevOnGround;
  x328_28_prevOnGround = x328_27_onGround;
  return ret;
}

bool CPatterned::PlayerSpot(CStateManager& mgr, float arg) {
  if (mgr.GetPlayer().GetMorphballTransitionState() == CPlayer::EPlayerMorphBallState::Unmorphed) {
    zeus::CVector3f aimPosition = mgr.GetPlayer().GetAimPosition(mgr, 0.f);
    zeus::CVector3f center = GetBoundingBox().center();
    zeus::CVector3f aimToCenter = center - aimPosition;
    float aimToCenterMag = aimToCenter.magnitude();
    zeus::CVector3f aimToCenterNorm = aimToCenter * (1.f / aimToCenterMag);
    zeus::CVector3f screenSpace = mgr.GetCameraManager()->GetFirstPersonCamera()->ConvertToScreenSpace(center);
    if (screenSpace.z() > 0.f && screenSpace.x() * screenSpace.x() < 1.f && screenSpace.y() * screenSpace.y() < 1.f) {
      CRayCastResult res = mgr.RayStaticIntersection(aimPosition, aimToCenterNorm, aimToCenterMag,
                                                     CMaterialFilter::MakeInclude(EMaterialTypes::Solid));
      return res.IsInvalid();
    }
  }
  return false;
}

bool CPatterned::SpotPlayer(CStateManager& mgr, float arg) {
  zeus::CVector3f gunToPlayer = mgr.GetPlayer().GetAimPosition(mgr, 0.f) - GetGunEyePos();
  float lookDot = gunToPlayer.dot(x34_transform.basis[1]);
  if (lookDot > 0.f)
    return lookDot * lookDot > gunToPlayer.magSquared() * x3c4_detectionAngle;
  return false;
}

bool CPatterned::Leash(CStateManager&, float arg) {
  bool ret = x3d4_curPlayerLeashTime > x3d0_playerLeashTime;
  if (ret) {
    float posToLeashMagSq = (x3a0_latestLeashPosition - GetTranslation()).magSquared();
    return posToLeashMagSq > x3c8_leashRadius * x3c8_leashRadius;
  }
  return ret;
}

bool CPatterned::InDetectionRange(CStateManager& mgr, float arg) {
  zeus::CVector3f delta = GetTranslation() - mgr.GetPlayer().GetTranslation();
  const float maxRange = x3bc_detectionRange * x3bc_detectionRange;
  const float dist = delta.magSquared();
  if (dist < maxRange) {
    if (x3c0_detectionHeightRange > 0.f)
      return delta.z() * delta.z() < x3c0_detectionHeightRange * x3c0_detectionHeightRange;
    return true;
  }
  return false;
}

bool CPatterned::InMaxRange(CStateManager& mgr, float arg) {
  return (mgr.GetPlayer().GetTranslation() - GetTranslation()).magSquared() < x300_maxAttackRange * x300_maxAttackRange;
}

bool CPatterned::TooClose(CStateManager& mgr, float arg) {
  return (mgr.GetPlayer().GetTranslation() - GetTranslation()).magSquared() < x2fc_minAttackRange * x2fc_minAttackRange;
}

bool CPatterned::InRange(CStateManager& mgr, float arg) {
  float range = 0.5f * (x2fc_minAttackRange + x300_maxAttackRange);
  return (mgr.GetPlayer().GetTranslation() - GetTranslation()).magSquared() < range * range;
}

bool CPatterned::OffLine(CStateManager&, float arg) {
  zeus::CVector3f curLine = GetTranslation() - x2ec_reflectedDestPos;
  zeus::CVector3f pathLine = x2e0_destPos - x2ec_reflectedDestPos;
  float distSq;
  if (curLine.dot(pathLine) <= 0.f) {
    distSq = curLine.magSquared();
  } else {
    pathLine.normalize();
    distSq = (curLine - pathLine.dot(curLine) * pathLine).magSquared();
    zeus::CVector3f delta = GetTranslation() - x2e0_destPos;
    if (pathLine.dot(delta) > 0.f)
      distSq = delta.magSquared();
  }
  return distSq > arg * arg;
}

void CPatterned::PathFind(CStateManager& mgr, EStateMsg msg, float dt) {
  if (CPathFindSearch* search = GetSearchPath()) {
    switch (msg) {
    case EStateMsg::Activate: {
      if (search->Search(GetTranslation(), x2e0_destPos) == CPathFindSearch::EResult::Success) {
        x2ec_reflectedDestPos = GetTranslation();
        zeus::CVector3f destPos;
        if (search->GetCurrentWaypoint() + 1 < search->GetWaypoints().size())
          destPos = search->GetWaypoints()[search->GetCurrentWaypoint() + 1];
        else
          destPos = search->GetWaypoints()[search->GetCurrentWaypoint()];
        SetDestPos(destPos);
        x328_24_inPosition = false;
        ApproachDest(mgr);
      }
      break;
    }
    case EStateMsg::Update: {
      if (search->GetCurrentWaypoint() < search->GetWaypoints().size() - 1) {
        if (x328_24_inPosition || x328_27_onGround)
          x401_24_pathOverCount += 1;
        zeus::CVector3f biasedPos = GetTranslation() + 0.3f * zeus::skUp;
        x2ec_reflectedDestPos = biasedPos - (x2e0_destPos - biasedPos);
        ApproachDest(mgr);
        zeus::CVector3f biasedForward = x34_transform.basis[1] * x64_modelData->GetScale().y() + biasedPos;
        search->GetSplinePointWithLookahead(biasedForward, biasedPos, 3.f * x64_modelData->GetScale().y());
        SetDestPos(biasedForward);
        if (search->SegmentOver(biasedPos))
          search->SetCurrentWaypoint(search->GetCurrentWaypoint() + 1);
      }
      break;
    }
    default:
      break;
    }
  }
}

void CPatterned::Dead(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case EStateMsg::Activate:
    x31c_faceVec = zeus::skZero3f;
    break;
  case EStateMsg::Update:
    x450_bodyController->GetCommandMgr().DeliverCmd(CBodyStateCmd(EBodyStateCmd::Die));
    if (!x400_27_fadeToDeath) {
      if (x450_bodyController->GetBodyStateInfo().GetCurrentState()->IsDead()) {
        x400_27_fadeToDeath = true;
        x3e8_alphaDelta = -0.333333f;
        RemoveMaterial(EMaterialTypes::Character, EMaterialTypes::Solid, EMaterialTypes::Target, EMaterialTypes::Orbit,
                       mgr);
        AddMaterial(EMaterialTypes::ProjectilePassthrough, mgr);
      }
    }
    break;
  default:
    break;
  }
}

void CPatterned::TargetPlayer(CStateManager& mgr, EStateMsg msg, float dt) {
  if (msg == EStateMsg::Activate) {
    x2dc_destObj = mgr.GetPlayer().GetUniqueId();
    SetDestPos(mgr.GetPlayer().GetTranslation());
    x2ec_reflectedDestPos = GetTranslation();
    x328_24_inPosition = false;
  }
}

void CPatterned::TargetPatrol(CStateManager& mgr, EStateMsg msg, float dt) {
  if (msg == EStateMsg::Activate) {
    x2dc_destObj = GetWaypointForState(mgr, EScriptObjectState::Patrol, EScriptObjectMessage::Follow);
    if (TCastToConstPtr<CActor> act = mgr.GetObjectById(x2dc_destObj))
      SetDestPos(act->GetTranslation());
    x2ec_reflectedDestPos = GetTranslation();
    x328_24_inPosition = false;
  }
}

void CPatterned::FollowPattern(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case EStateMsg::Activate:
    SetupPattern(mgr);
    if (x328_29_noPatternShagging || !IsPatternObstructed(mgr, GetTranslation(), x2e0_destPos)) {
      ApproachDest(mgr);
    } else {
      x39c_curPattern = x38c_patterns.size();
      x400_30_patternShagged = true;
    }
    break;
  case EStateMsg::Update:
    if (x328_24_inPosition) {
      x39c_curPattern += 1;
      UpdatePatternDestPos(mgr);
      if (!x328_29_noPatternShagging && IsPatternObstructed(mgr, GetTranslation(), x2e0_destPos)) {
        x39c_curPattern = x38c_patterns.size();
        x400_30_patternShagged = true;
      } else if (x39c_curPattern < x38c_patterns.size()) {
        x2ec_reflectedDestPos = GetTranslation();
        x328_24_inPosition = false;
        x3b0_moveSpeed = x38c_patterns[x39c_curPattern].GetSpeed();
        x380_behaviour = EBehaviour(x38c_patterns[x39c_curPattern].GetBehaviour());
        x30c_behaviourOrient = EBehaviourOrient(x38c_patterns[x39c_curPattern].GetBehaviourOrient());
        x384_behaviourModifiers = EBehaviourModifiers(x38c_patterns[x39c_curPattern].GetBehaviourModifiers());
      }
    } else {
      UpdatePatternDestPos(mgr);
    }
    ApproachDest(mgr);
    break;
  case EStateMsg::Deactivate:
    x38c_patterns.clear();
    x400_30_patternShagged = false;
  }
}

void CPatterned::Patrol(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case EStateMsg::Activate:
    if (x3ac_lastPatrolDest == kInvalidUniqueId) {
      x2dc_destObj = GetWaypointForState(mgr, EScriptObjectState::Patrol, EScriptObjectMessage::Follow);
      x30c_behaviourOrient = EBehaviourOrient::MoveDir;
      x3b0_moveSpeed = 1.f;
      if (x2dc_destObj != kInvalidUniqueId) {
        if (TCastToConstPtr<CScriptWaypoint> wp = mgr.GetObjectById(x2dc_destObj)) {
          x30c_behaviourOrient = EBehaviourOrient(wp->GetBehaviourOrient());
          x3b0_moveSpeed = wp->GetSpeed();
        }
      }
    } else {
      x2dc_destObj = x3ac_lastPatrolDest;
    }
    x2ec_reflectedDestPos = GetTranslation();
    x328_24_inPosition = false;
    x2d8_patrolState = EPatrolState::Patrol;
    x2f8_waypointPauseRemTime = 0.f;
    break;
  case EStateMsg::Update:
    switch (x2d8_patrolState) {
    case EPatrolState::Patrol:
      if (x328_24_inPosition && x2dc_destObj != kInvalidUniqueId) {
        if (TCastToConstPtr<CScriptWaypoint> wp = mgr.GetObjectById(x2dc_destObj)) {
          if (wp->GetPause() > 0.f) {
            x2f8_waypointPauseRemTime = wp->GetPause();
            x2d8_patrolState = EPatrolState::Pause;
          }
        }
      }
      if (x2dc_destObj == kInvalidUniqueId)
        x2d8_patrolState = EPatrolState::Done;
      UpdateDest(mgr);
      ApproachDest(mgr);
      break;
    case EPatrolState::Pause:
      if (x2f8_waypointPauseRemTime <= 0.f)
        x2d8_patrolState = EPatrolState::Patrol;
      break;
    case EPatrolState::Done:
      if (x2dc_destObj != kInvalidUniqueId)
        x2d8_patrolState = EPatrolState::Patrol;
      break;
    default:
      break;
    }
    break;
  case EStateMsg::Deactivate:
    x3ac_lastPatrolDest = x2dc_destObj;
    x2d8_patrolState = EPatrolState::Invalid;
    break;
  default:
    break;
  }
}

void CPatterned::TryCommand(CStateManager& mgr, pas::EAnimationState state, CPatternedTryFunc func, int arg) {
  if (state == x450_bodyController->GetCurrentStateId())
    x32c_animState = EAnimState::Repeat;
  else if (x32c_animState == EAnimState::Ready)
    (this->*func)(mgr, arg);
  else
    x32c_animState = EAnimState::Over;
}

void CPatterned::TryLoopReaction(CStateManager& mgr, int arg) {
  x450_bodyController->GetCommandMgr().DeliverCmd(CBCLoopReactionCmd(pas::EReactionType(arg)));
}

void CPatterned::TryProjectileAttack(CStateManager&, int arg) {
  x450_bodyController->GetCommandMgr().DeliverCmd(CBCProjectileAttackCmd(pas::ESeverity(arg), x2e0_destPos, false));
}
void CPatterned::TryMeleeAttack(CStateManager& mgr, int arg) {
  x450_bodyController->GetCommandMgr().DeliverCmd(CBCMeleeAttackCmd(pas::ESeverity(arg)));
}

void CPatterned::TryGenerate(CStateManager& mgr, int arg) {
  x450_bodyController->GetCommandMgr().DeliverCmd(CBCGenerateCmd(pas::EGenerateType(arg), x2e0_destPos, false));
}

void CPatterned::TryGenerateNoXf(CStateManager& mgr, int arg) {
  x450_bodyController->GetCommandMgr().DeliverCmd(CBCGenerateCmd(pas::EGenerateType::Zero, x2e0_destPos, true));
}

void CPatterned::TryJump(CStateManager& mgr, int arg) {
  x450_bodyController->GetCommandMgr().DeliverCmd(CBCJumpCmd(x2e0_destPos, pas::EJumpType(arg)));
}

void CPatterned::TryTurn(CStateManager& mgr, int arg) {
  x450_bodyController->GetCommandMgr().DeliverCmd(
    CBCLocomotionCmd(zeus::skZero3f, (x2e0_destPos - GetTranslation()).normalized(), 1.f));
}

void CPatterned::TryGetUp(CStateManager& mgr, int arg) {
  x450_bodyController->GetCommandMgr().DeliverCmd(CBCGetupCmd(pas::EGetupType(arg)));
}

void CPatterned::TryTaunt(CStateManager& mgr, int arg) {
  x450_bodyController->GetCommandMgr().DeliverCmd(CBCTauntCmd(pas::ETauntType(arg)));
}

void CPatterned::TryJumpInLoop(CStateManager& mgr, int arg) {
  x450_bodyController->GetCommandMgr().DeliverCmd(CBCJumpCmd(x2e0_destPos, pas::EJumpType(arg), true));
}

void CPatterned::TryDodge(CStateManager& mgr, int arg) {
  x450_bodyController->GetCommandMgr().DeliverCmd(CBCStepCmd(pas::EStepDirection(arg), pas::EStepType::Dodge));
}

void CPatterned::TryRollingDodge(CStateManager& mgr, int arg) {
  x450_bodyController->GetCommandMgr().DeliverCmd(CBCStepCmd(pas::EStepDirection(arg), pas::EStepType::RollDodge));
}

void CPatterned::TryBreakDodge(CStateManager& mgr, int arg) {
  x450_bodyController->GetCommandMgr().DeliverCmd(CBCStepCmd(pas::EStepDirection(arg), pas::EStepType::BreakDodge));
}

void CPatterned::TryCover(CStateManager& mgr, int arg) {
  if (CScriptCoverPoint* cp = GetCoverPoint(mgr, x2dc_destObj)) {
    x450_bodyController->GetCommandMgr().DeliverCmd(CBCCoverCmd(pas::ECoverDirection(arg), cp->GetTranslation(),
                                                                -cp->GetTransform().basis[1]));
  }
}

void CPatterned::TryWallHang(CStateManager& mgr, int arg) {
  x450_bodyController->GetCommandMgr().DeliverCmd(CBCWallHangCmd(x2dc_destObj));
}

void CPatterned::TryKnockBack(CStateManager& mgr, int arg) {
  x450_bodyController->GetCommandMgr().DeliverCmd(CBCKnockBackCmd(GetTranslation(), pas::ESeverity(arg)));
}

void CPatterned::TryGenerateDeactivate(urde::CStateManager& mgr, int arg) {
  x450_bodyController->GetCommandMgr().DeliverCmd(CBCGenerateCmd(pas::EGenerateType(arg), zeus::skZero3f));
}

void CPatterned::TryStep(CStateManager& mgr, int arg) {
  x450_bodyController->GetCommandMgr().DeliverCmd(CBCStepCmd(pas::EStepDirection(arg), pas::EStepType::Normal));
}

void CPatterned::BuildBodyController(EBodyType bodyType) {
  if (x450_bodyController)
    return;

  x450_bodyController = std::make_unique<CBodyController>(*this, x3b8_turnSpeed, bodyType);
  auto anim =
      x450_bodyController->GetPASDatabase().FindBestAnimation(CPASAnimParmData(24, CPASAnimParm::FromEnum(0)), -1);
  x460_knockBackController.x81_26_enableShock = anim.first > 0.f;
}

void CPatterned::GenerateDeathExplosion(CStateManager& mgr) {
  if (auto particle = GetDeathExplosionParticle()) {
    zeus::CTransform xf(GetTransform());
    xf.origin = GetTransform() * (x64_modelData->GetScale() * x514_deathExplosionOffset);
    CExplosion* explo = new CExplosion(*particle, mgr.AllocateUniqueId(), true,
                                       CEntityInfo(GetAreaIdAlways(), CEntity::NullConnectionList), "", xf, 1,
                                       zeus::skOne3f, zeus::skWhite);
    mgr.AddObject(explo);
  }
  if (x530_deathExplosionElectric) {
    zeus::CTransform xf(GetTransform());
    xf.origin = GetTransform() * (x64_modelData->GetScale() * x514_deathExplosionOffset);
    CExplosion* explo = new CExplosion(*x530_deathExplosionElectric, mgr.AllocateUniqueId(), true,
                                       CEntityInfo(GetAreaIdAlways(), CEntity::NullConnectionList), "", xf, 1,
                                       zeus::skOne3f, zeus::skWhite);
    mgr.AddObject(explo);
  }
}

void CPatterned::MassiveDeath(CStateManager& mgr) {
  CSfxManager::AddEmitter(x454_deathSfx, GetTranslation(), zeus::skZero3f, true, false, 0x7f, kInvalidAreaId);
  if (!x401_28_burning) {
    SendScriptMsgs(EScriptObjectState::MassiveDeath, mgr, EScriptObjectMessage::None);
    GenerateDeathExplosion(mgr);
  }
  DeathDelete(mgr);
  x400_28_pendingMassiveDeath = x400_29_pendingMassiveFrozenDeath = false;
}

void CPatterned::GenerateIceDeathExplosion(CStateManager& mgr) {
  if (x54c_iceDeathExplosionParticle) {
    zeus::CTransform xf(GetTransform());
    xf.origin = GetTransform() * (x64_modelData->GetScale() * x540_iceDeathExplosionOffset);
    CExplosion* explo = new CExplosion(*x54c_iceDeathExplosionParticle, mgr.AllocateUniqueId(), true,
                                       CEntityInfo(GetAreaIdAlways(), CEntity::NullConnectionList), "", xf, 1,
                                       zeus::skOne3f, zeus::skWhite);
    mgr.AddObject(explo);
  }
}

void CPatterned::MassiveFrozenDeath(CStateManager& mgr) {
  if (x458_iceShatterSfx == 0xffff)
    x458_iceShatterSfx = x454_deathSfx;
  CSfxManager::AddEmitter(x458_iceShatterSfx, GetTranslation(), zeus::skZero3f, true, false, 0x7f,
                          kInvalidAreaId);
  SendScriptMsgs(EScriptObjectState::MassiveFrozenDeath, mgr, EScriptObjectMessage::None);
  GenerateIceDeathExplosion(mgr);
  float toPlayerDist = (mgr.GetPlayer().GetTranslation() - GetTranslation()).magnitude();
  if (toPlayerDist < 40.f)
    mgr.GetCameraManager()->AddCameraShaker(
        CCameraShakeData::BuildPatternedExplodeShakeData(GetTranslation(), 0.25f, 0.3f, 40.f), true);
  DeathDelete(mgr);
  x400_28_pendingMassiveDeath = x400_29_pendingMassiveFrozenDeath = false;
}

void CPatterned::Burn(float duration, float damage) {
  switch (GetDamageVulnerability()->GetVulnerability(CWeaponMode(EWeaponType::Plasma), false)) {
  case EVulnerability::Weak:
    x450_bodyController->SetOnFire(1.5f * duration);
    x3ec_pendingFireDamage = 1.5f * damage;
    break;
  case EVulnerability::Normal:
    x450_bodyController->SetOnFire(duration);
    x3ec_pendingFireDamage = damage;
    break;
  default:
    break;
  }
}

void CPatterned::Shock(CStateManager& mgr, float duration, float damage) {
  switch (GetDamageVulnerability()->GetVulnerability(CWeaponMode(EWeaponType::Wave), false)) {
  case EVulnerability::Weak:
    x450_bodyController->SetElectrocuting(1.5f * duration);
    x3f0_pendingShockDamage = 1.5f * damage;
    break;
  case EVulnerability::Normal:
    x450_bodyController->SetElectrocuting(duration);
    x3f0_pendingShockDamage = damage;
    break;
  default:
    break;
  }
}

void CPatterned::Freeze(CStateManager& mgr, const zeus::CVector3f& pos, const zeus::CUnitVector3f& dir,
                        float frozenDur) {
  if (x402_25_lostMassiveFrozenHP)
    x402_26_dieIf80PercFrozen = true;
  bool playSfx = false;
  if (x450_bodyController->IsFrozen()) {
    x450_bodyController->Freeze(x460_knockBackController.GetActiveParms().xc_intoFreezeDur, frozenDur,
                                x4f8_outofFreezeDur);
    mgr.GetActorModelParticles()->EnsureElectricLoaded(*this);
    playSfx = true;
  } else if (!x450_bodyController->IsElectrocuting() && !x450_bodyController->IsOnFire()) {
    x450_bodyController->Freeze(x4f4_intoFreezeDur, frozenDur, x4f8_outofFreezeDur);
    if (x510_vertexMorph)
      x510_vertexMorph->Reset(dir, pos, x4f4_intoFreezeDur);
    playSfx = true;
  }

  if (playSfx) {
    u16 sfx;
    if (x460_knockBackController.GetVariant() != EKnockBackVariant::Small &&
        CPatterned::CastTo<MP1::CMetroid>(mgr.GetObjectById(GetUniqueId())))
      sfx = SFXsfx0701;
    else
      sfx = SFXsfx0708;
    CSfxManager::AddEmitter(sfx, GetTranslation(), zeus::skZero3f, true, false, 0x7f, kInvalidAreaId);
  }
}

zeus::CVector3f CPatterned::GetGunEyePos() const {
  zeus::CVector3f origin = GetTranslation();
  zeus::CAABox baseBox = GetBaseBoundingBox();
  origin.z() = 0.6f * (baseBox.max.z() - baseBox.min.z()) + origin.z();
  return origin;
}

void CPatterned::SetupPlayerCollision(bool v) {
  CMaterialList include = GetMaterialFilter().GetIncludeList();
  CMaterialList exclude = GetMaterialFilter().GetExcludeList();
  CMaterialList* modList = (v ? &exclude : &include);
  modList->Add(EMaterialTypes::Player);
  SetMaterialFilter(CMaterialFilter::MakeIncludeExclude(include, exclude));
}

CGameProjectile* CPatterned::LaunchProjectile(const zeus::CTransform& gunXf, CStateManager& mgr, int maxAllowed,
                                  EProjectileAttrib attrib, bool playerHoming,
                                  const std::optional<TLockedToken<CGenDescription>>& visorParticle,
                                  u16 visorSfx, bool sendCollideMsg, const zeus::CVector3f& scale) {
  CProjectileInfo* pInfo = GetProjectileInfo();
  if (pInfo->Token().IsLoaded()) {
    if (mgr.CanCreateProjectile(GetUniqueId(), EWeaponType::AI, maxAllowed)) {
      TUniqueId homingId = playerHoming ? mgr.GetPlayer().GetUniqueId() : kInvalidUniqueId;
      CEnergyProjectile* newProjectile =
          new CEnergyProjectile(true, pInfo->Token(), EWeaponType::AI, gunXf, EMaterialTypes::Character,
                                pInfo->GetDamage(), mgr.AllocateUniqueId(), GetAreaIdAlways(), GetUniqueId(), homingId,
                                attrib, false, scale, visorParticle, visorSfx, sendCollideMsg);
      mgr.AddObject(newProjectile);
      return newProjectile;
    }
  }
  return nullptr;
}

void CPatterned::DoUserAnimEvent(CStateManager& mgr, const CInt32POINode& node, EUserEventType type, float dt) {
  switch (type) {
  case EUserEventType::Projectile: {
    zeus::CTransform lctrXf = GetLctrTransform(node.GetLocatorName());
    zeus::CVector3f aimPos = mgr.GetPlayer().GetAimPosition(mgr, 0.f);
    if ((aimPos - lctrXf.origin).normalized().dot(lctrXf.basis[1]) > 0.f) {
      zeus::CTransform gunXf = zeus::lookAt(lctrXf.origin, aimPos);
      LaunchProjectile(gunXf, mgr, 1, EProjectileAttrib::None, false, {}, 0xffff, false, zeus::skOne3f);
    } else {
      LaunchProjectile(lctrXf, mgr, 1, EProjectileAttrib::None, false, {}, 0xffff, false, zeus::skOne3f);
    }
    break;
  }
  case EUserEventType::DamageOn: {
    zeus::CTransform lctrXf = GetLctrTransform(node.GetLocatorName());
    zeus::CVector3f xfOrigin = x34_transform * (x64_modelData->GetScale() * lctrXf.origin);
    zeus::CVector3f margin = zeus::CVector3f(1.f, 1.f, 0.5f) * x64_modelData->GetScale();
    if (zeus::CAABox(xfOrigin - margin, xfOrigin + margin).intersects(mgr.GetPlayer().GetBoundingBox())) {
      mgr.ApplyDamage(GetUniqueId(), mgr.GetPlayer().GetUniqueId(), GetUniqueId(), GetContactDamage(),
                      CMaterialFilter::MakeIncludeExclude({EMaterialTypes::Solid}, {}), zeus::skZero3f);
    }
    break;
  }
  case EUserEventType::Delete: {
    if (!x400_25_alive) {
      if (!x400_27_fadeToDeath) {
        x3e8_alphaDelta = -0.333333f;
        x400_27_fadeToDeath = true;
      }
      RemoveMaterial(EMaterialTypes::Character, EMaterialTypes::Solid, EMaterialTypes::Target, EMaterialTypes::Orbit,
                     mgr);
      AddMaterial(EMaterialTypes::ProjectilePassthrough, mgr);
    } else {
      DeathDelete(mgr);
    }
    break;
  }
  case EUserEventType::BreakLockOn: {
    RemoveMaterial(EMaterialTypes::Character, EMaterialTypes::Target, EMaterialTypes::Orbit, mgr);
    break;
  }
  case EUserEventType::BecomeShootThrough: {
    AddMaterial(EMaterialTypes::ProjectilePassthrough, mgr);
    break;
  }
  case EUserEventType::RemoveCollision: {
    RemoveMaterial(EMaterialTypes::Solid, mgr);
    break;
  }
  default:
    break;
  }
  CActor::DoUserAnimEvent(mgr, node, type, dt);
}

void CPatterned::UpdateAlphaDelta(float dt, CStateManager& mgr) {
  if (x3e8_alphaDelta == 0.f)
    return;

  float alpha = dt * x3e8_alphaDelta + x42c_color.a();
  if (alpha > 1.f) {
    alpha = 1.f;
    x3e8_alphaDelta = 0.f;
  } else if (alpha < 0.f) {
    alpha = 0.f;
    x3e8_alphaDelta = 0.f;
    if (x400_27_fadeToDeath)
      DeathDelete(mgr);
  }
  x94_simpleShadow->SetUserAlpha(alpha);
  SetModelAlpha(alpha);
  x64_modelData->GetAnimationData()->GetParticleDB().SetModulationColorAllActiveEffects(zeus::CColor(1.f, alpha));
}

float CPatterned::CalcDyingThinkRate() const {
  float f0 = (x401_28_burning ? (x3f4_burnThinkRateTimer / 1.5f) : 1.f);
  return zeus::max(0.1f, f0);
}

void CPatterned::UpdateDamageColor(float dt) {
  if (x428_damageCooldownTimer > 0.f) {
    x428_damageCooldownTimer = std::max(0.f, x428_damageCooldownTimer - dt);
    float alpha = x42c_color.a();
    x42c_color =
        zeus::CColor::lerp(zeus::skBlack, x430_damageColor, std::min(x428_damageCooldownTimer / 0.33f, 1.f));
    x42c_color.a() = alpha;
    if (!x450_bodyController->IsFrozen())
      xd0_damageMag = x50c_baseDamageMag + x428_damageCooldownTimer;
  }
}

CScriptCoverPoint* CPatterned::GetCoverPoint(CStateManager& mgr, TUniqueId id) const {
  if (id != kInvalidUniqueId) {
    if (TCastToPtr<CScriptCoverPoint> cp = mgr.ObjectById(id))
      return cp.GetPtr();
  }
  return nullptr;
}

void CPatterned::SetCoverPoint(CScriptCoverPoint* cp, TUniqueId& id) {
  cp->SetInUse(true);
  id = cp->GetUniqueId();
}

void CPatterned::ReleaseCoverPoint(CStateManager& mgr, TUniqueId& id) {
  if (CScriptCoverPoint* cp = GetCoverPoint(mgr, id)) {
    cp->SetInUse(false);
    id = kInvalidUniqueId;
  }
}

TUniqueId CPatterned::GetWaypointForState(CStateManager& mgr, EScriptObjectState state,
                                          EScriptObjectMessage msg) const {
  rstl::reserved_vector<TUniqueId, 12> ids;
  for (const auto& conn : GetConnectionList()) {
    if (conn.x0_state == state && conn.x4_msg == msg) {
      TUniqueId id = mgr.GetIdForScript(conn.x8_objId);
      if (const CEntity* ent = mgr.GetObjectById(id))
        if (ent->GetActive())
          ids.push_back(id);
    }
  }

  if (!ids.empty())
    return ids[mgr.GetActiveRandom()->Next() % ids.size()];

  return kInvalidUniqueId;
}

void CPatterned::UpdateActorKeyframe(CStateManager& mgr) const {
  if (TCastToConstPtr<CScriptWaypoint> wp = mgr.GetObjectById(x2dc_destObj))
    for (const auto& conn : wp->GetConnectionList())
      if (conn.x0_state == EScriptObjectState::Arrived && conn.x4_msg == EScriptObjectMessage::Action)
        if (TCastToPtr<CScriptActorKeyframe> kf = mgr.ObjectById(mgr.GetIdForScript(conn.x8_objId)))
          if (kf->GetActive() && kf->IsPassive())
            kf->UpdateEntity(GetUniqueId(), mgr);
}

pas::EStepDirection CPatterned::GetStepDirection(const zeus::CVector3f& moveVec) const {
  zeus::CVector3f localMove = x34_transform.transposeRotate(moveVec);
  float angle = zeus::CVector3f::getAngleDiff(localMove, zeus::skForward);
  if (angle < zeus::degToRad(45.f))
    return pas::EStepDirection::Forward;
  else if (angle > zeus::degToRad(135.f))
    return pas::EStepDirection::Backward;
  else if (localMove.dot(zeus::skRight) > 0.f)
    return pas::EStepDirection::Right;
  else
    return pas::EStepDirection::Left;
}

bool CPatterned::IsPatternObstructed(CStateManager& mgr, const zeus::CVector3f& p0, const zeus::CVector3f& p1) const {
  CMaterialFilter filter = CMaterialFilter::MakeInclude(EMaterialTypes::Character);
  zeus::CVector3f delta = p1 - p0;
  rstl::reserved_vector<TUniqueId, 1024> nearList;
  bool ret = false;
  if (delta.canBeNormalized()) {
    zeus::CVector3f deltaNorm = delta.normalized();
    float deltaMag = delta.magnitude();
    mgr.BuildNearList(nearList, p0, deltaNorm, deltaMag, filter, this);
    TUniqueId bestId = kInvalidUniqueId;
    CRayCastResult res = mgr.RayWorldIntersection(bestId, p0, deltaNorm, deltaMag,
                                                  CMaterialFilter::MakeInclude(EMaterialTypes::Solid), nearList);
    ret = res.IsValid();
  }
  return ret;
}

void CPatterned::UpdateDest(CStateManager& mgr) {
  if (x328_24_inPosition && x2dc_destObj != kInvalidUniqueId) {
    if (TCastToPtr<CScriptWaypoint> wp = mgr.ObjectById(x2dc_destObj)) {
      UpdateActorKeyframe(mgr);
      x2dc_destObj = wp->NextWaypoint(mgr);
      if (x2dc_destObj != kInvalidUniqueId) {
        x2ec_reflectedDestPos = GetTranslation();
        x328_24_inPosition = false;
        if (TCastToConstPtr<CScriptWaypoint> wp2 = mgr.GetObjectById(x2dc_destObj)) {
          x3b0_moveSpeed = wp->GetSpeed();
          x30c_behaviourOrient = EBehaviourOrient(wp->GetBehaviourOrient());
          if (wp->GetBehaviourModifiers() & 0x2) {
            x450_bodyController->GetCommandMgr().DeliverCmd(CBCJumpCmd(wp2->GetTranslation(), pas::EJumpType::Normal));
          } else if (wp->GetBehaviourModifiers() & 0x4) {
            TUniqueId wp3Id = wp2->NextWaypoint(mgr);
            if (wp3Id != kInvalidUniqueId) {
              if (TCastToConstPtr<CScriptWaypoint> wp3 = mgr.GetObjectById(wp3Id)) {
                x450_bodyController->GetCommandMgr().DeliverCmd(
                    CBCJumpCmd(wp2->GetTranslation(), wp3->GetTranslation(), pas::EJumpType::Normal));
              }
            }
          }
        }
      }
      mgr.SendScriptMsg(wp.GetPtr(), GetUniqueId(), EScriptObjectMessage::Arrived);
    }
  }

  if (x2dc_destObj != kInvalidUniqueId)
    if (TCastToConstPtr<CActor> act = mgr.GetObjectById(x2dc_destObj))
      SetDestPos(act->GetTranslation());
}

void CPatterned::ApproachDest(CStateManager& mgr) {
  zeus::CVector3f faceVec = mgr.GetPlayer().GetTranslation() - GetTranslation();
  zeus::CVector3f moveVec = x2e0_destPos - GetTranslation();
  if (!x328_25_verticalMovement)
    moveVec.z() = 0.f;
  zeus::CVector3f pathLine = x2e0_destPos - x2ec_reflectedDestPos;
  if (pathLine.dot(moveVec) <= 0.f)
    x328_24_inPosition = true;
  else if (moveVec.magSquared() < 3.f * 3.f)
    moveVec = pathLine;
  if (!x328_24_inPosition) {
    if (moveVec.canBeNormalized())
      moveVec.normalize();
    switch (x30c_behaviourOrient) {
    case EBehaviourOrient::MoveDir:
      faceVec = moveVec;
      break;
    case EBehaviourOrient::Destination:
      if (x39c_curPattern && x39c_curPattern < x38c_patterns.size()) {
        faceVec = x38c_patterns[x39c_curPattern].GetForward();
      } else if (x2dc_destObj != kInvalidUniqueId) {
        if (TCastToConstPtr<CScriptWaypoint> wp = mgr.GetObjectById(x2dc_destObj))
          faceVec = wp->GetTransform().basis[1];
      }
      break;
    default:
      break;
    }
    x31c_faceVec = faceVec;
    x310_moveVec = x3b0_moveSpeed * moveVec;
    pas::EStepDirection stepDir;
    if (!KnockbackWhenFrozen()) {
      x450_bodyController->GetCommandMgr().DeliverCmd(CBCLocomotionCmd(x310_moveVec, x31c_faceVec, 1.f));
    } else if (x30c_behaviourOrient == EBehaviourOrient::MoveDir ||
               !x450_bodyController->HasBodyState(pas::EAnimationState::Step)) {
      x450_bodyController->GetCommandMgr().DeliverCmd(CBCLocomotionCmd(x310_moveVec, zeus::skZero3f, 1.f));
    } else if ((stepDir = GetStepDirection(x310_moveVec)) != pas::EStepDirection::Forward) {
      x450_bodyController->GetCommandMgr().DeliverCmd(CBCStepCmd(stepDir, pas::EStepType::Normal));
    } else {
      x450_bodyController->GetCommandMgr().DeliverCmd(CBCLocomotionCmd(x310_moveVec, zeus::skZero3f, 1.f));
    }
    x450_bodyController->GetCommandMgr().DeliverTargetVector(x31c_faceVec);
  } else if (x450_bodyController->GetBodyStateInfo().GetMaxSpeed() > FLT_EPSILON) {
    x450_bodyController->GetCommandMgr().DeliverCmd(CBCLocomotionCmd(
        (x138_velocity.magnitude() / x450_bodyController->GetBodyStateInfo().GetMaxSpeed()) * x34_transform.basis[1],
        zeus::skZero3f, 1.f));
  }
}

std::pair<CScriptWaypoint*, CScriptWaypoint*> CPatterned::GetDestWaypoints(CStateManager& mgr) const {
  std::pair<CScriptWaypoint*, CScriptWaypoint*> ret{};
  if (TCastToPtr<CScriptWaypoint> wp = mgr.ObjectById(x2dc_destObj)) {
    ret.first = wp.GetPtr();
    ret.second = TCastToPtr<CScriptWaypoint>(mgr.ObjectById(wp->FollowWaypoint(mgr))).GetPtr();
  }
  return ret;
}

zeus::CQuaternion CPatterned::FindPatternRotation(const zeus::CVector3f& dir) const {
  zeus::CVector3f wpDeltaFlat = x368_destWPDelta;
  wpDeltaFlat.z() = 0.f;
  wpDeltaFlat.normalize();
  zeus::CVector3f dirFlat = dir;
  dirFlat.z() = 0.f;
  dirFlat.normalize();

  zeus::CQuaternion q;
  if ((wpDeltaFlat - dirFlat).magSquared() > 3.99f)
    q.rotateZ(zeus::degToRad(180.f));
  else
    q = zeus::CQuaternion::shortestRotationArc(wpDeltaFlat, dirFlat);

  if (x328_25_verticalMovement) {
    q = zeus::CQuaternion::shortestRotationArc(
            (q * zeus::CQuaternion(0.f, x368_destWPDelta) * q.inverse()).getImaginary().normalized(),
            dir.normalized()) *
        q;
  }

  return q;
}

zeus::CVector3f CPatterned::FindPatternDir(CStateManager& mgr) const {
  zeus::CVector3f ret;
  switch (x378_patternOrient) {
  case EPatternOrient::StartToPlayerStart:
    ret = x35c_patternStartPlayerPos - x350_patternStartPos;
    break;
  case EPatternOrient::StartToPlayer:
    ret = mgr.GetPlayer().GetTranslation() - x350_patternStartPos;
    break;
  case EPatternOrient::ReversePlayerForward:
    ret = -mgr.GetPlayer().GetTransform().basis[1];
    break;
  case EPatternOrient::Forward:
    ret = GetTransform().basis[1];
    break;
  default:
    break;
  }
  return ret;
}

void CPatterned::UpdatePatternDestPos(CStateManager& mgr) {
  if (x39c_curPattern < x38c_patterns.size()) {
    if (x368_destWPDelta != zeus::skZero3f) {
      zeus::CVector3f patternDir = FindPatternDir(mgr);
      SetDestPos(FindPatternRotation(patternDir).transform(x38c_patterns[x39c_curPattern].GetPos()));
      if (x37c_patternFit == EPatternFit::Zero) {
        float magSq;
        if (x328_25_verticalMovement)
          magSq = patternDir.magSquared() / x368_destWPDelta.magSquared();
        else
          magSq = patternDir.toVec2f().magSquared() / x368_destWPDelta.toVec2f().magSquared();
        SetDestPos(std::sqrt(magSq) * x2e0_destPos);
      }
    } else {
      SetDestPos(x38c_patterns[x39c_curPattern].GetPos());
    }
  }

  switch (x374_patternTranslate) {
  case EPatternTranslate::RelativeStart:
    SetDestPos(x2e0_destPos + x350_patternStartPos);
    break;
  case EPatternTranslate::RelativePlayerStart:
    SetDestPos(x2e0_destPos + x35c_patternStartPlayerPos);
    break;
  case EPatternTranslate::RelativePlayer:
    SetDestPos(x2e0_destPos + mgr.GetPlayer().GetTranslation());
    break;
  default:
    break;
  }
}

void CPatterned::SetupPattern(CStateManager& mgr) {
  EScriptObjectState state = GetDesiredAttackState(mgr);
  x2dc_destObj = GetWaypointForState(mgr, state, EScriptObjectMessage::Follow);
  if (x2dc_destObj == kInvalidUniqueId && state != EScriptObjectState::Attack)
    x2dc_destObj = GetWaypointForState(mgr, EScriptObjectState::Attack, EScriptObjectMessage::Follow);
  x38c_patterns.clear();
  if (x2dc_destObj != kInvalidUniqueId) {
    x350_patternStartPos = GetTranslation();
    x35c_patternStartPlayerPos = mgr.GetPlayer().GetTranslation();
    auto destWPs = GetDestWaypoints(mgr);
    if (destWPs.first) {
      x374_patternTranslate = EPatternTranslate(destWPs.first->GetPatternTranslate());
      x378_patternOrient = EPatternOrient(destWPs.first->GetPatternOrient());
      x37c_patternFit = EPatternFit(destWPs.first->GetPatternFit());
      if (destWPs.second)
        x368_destWPDelta = destWPs.second->GetTranslation() - destWPs.first->GetTranslation();
      else
        x368_destWPDelta = zeus::skZero3f;

      int numPatterns = 0;
      CScriptWaypoint* curWp = destWPs.first;
      do {
        ++numPatterns;
        curWp = TCastToPtr<CScriptWaypoint>(mgr.ObjectById(curWp->NextWaypoint(mgr))).GetPtr();
        if (!curWp)
          break;
      } while (curWp->GetUniqueId() != destWPs.first->GetUniqueId());
      x38c_patterns.reserve(numPatterns);

      zeus::CVector3f basePos;
      switch (x374_patternTranslate) {
      case EPatternTranslate::RelativePlayerStart:
        if (destWPs.second)
          basePos = destWPs.second->GetTranslation();
        break;
      case EPatternTranslate::Absolute:
        break;
      default:
        basePos = destWPs.first->GetTranslation();
        break;
      }

      curWp = destWPs.first;
      do {
        zeus::CVector3f wpForward = curWp->GetTransform().basis[1];
        if (x368_destWPDelta != zeus::skZero3f)
          wpForward = FindPatternRotation(FindPatternDir(mgr)).transform(wpForward);
        x38c_patterns.emplace_back(curWp->GetTranslation() - basePos, wpForward, curWp->GetSpeed(),
                                   curWp->GetBehaviour(), curWp->GetBehaviourOrient(), curWp->GetBehaviourModifiers(),
                                   curWp->GetAnimation());
        curWp = TCastToPtr<CScriptWaypoint>(mgr.ObjectById(curWp->NextWaypoint(mgr))).GetPtr();
        if (!curWp)
          break;
      } while (curWp->GetUniqueId() != destWPs.first->GetUniqueId());
    }
  }

  x400_30_patternShagged = false;
  x39c_curPattern = 0;
  x328_24_inPosition = false;
  x2ec_reflectedDestPos = GetTranslation();
  if (!x38c_patterns.empty()) {
    x3b0_moveSpeed = x38c_patterns.front().GetSpeed();
    x380_behaviour = EBehaviour(x38c_patterns.front().GetBehaviour());
    x30c_behaviourOrient = EBehaviourOrient(x38c_patterns.front().GetBehaviourOrient());
    x384_behaviourModifiers = EBehaviourModifiers(x38c_patterns.front().GetBehaviourModifiers());
  }
}

EScriptObjectState CPatterned::GetDesiredAttackState(CStateManager& mgr) const {
  float deltaMagSq = (GetTranslation() - mgr.GetPlayer().GetTranslation()).magSquared();
  if (deltaMagSq < x2fc_minAttackRange * x2fc_minAttackRange)
    return EScriptObjectState::Retreat;
  else if (deltaMagSq > x300_maxAttackRange * x300_maxAttackRange)
    return EScriptObjectState::CloseIn;
  else
    return EScriptObjectState::Attack;
}

float CPatterned::GetAnimationDistance(const CPASAnimParmData& data) const {
  auto bestAnim = x64_modelData->GetAnimationData()->GetCharacterInfo().GetPASDatabase().FindBestAnimation(data, -1);
  float dist = 1.f;
  if (bestAnim.first > FLT_EPSILON) {
    dist = x64_modelData->GetAnimationData()->GetAnimationDuration(bestAnim.second) *
           x64_modelData->GetAnimationData()->GetAverageVelocity(bestAnim.second);
  }
  return dist;
}

void CPatterned::PreRender(CStateManager& mgr, const zeus::CFrustum& frustum) {
  if (mgr.GetPlayerState()->GetActiveVisor(mgr) == CPlayerState::EPlayerVisor::Thermal) {
    SetCalculateLighting(false);
    x90_actorLights->BuildConstantAmbientLighting(zeus::skWhite);
  } else
    SetCalculateLighting(true);

  zeus::CColor col = x42c_color;
  u8 alpha = GetModelAlphau8(mgr);
  if (x402_27_noXrayModel && mgr.GetPlayerState()->GetActiveVisor(mgr) == CPlayerState::EPlayerVisor::XRay)
    alpha = 76;

  if (alpha < 255) {
    if (col.r() == 0.f && col.g() == 0.f && col.b() == 0.f)
      col = zeus::skWhite; /* Not being damaged */

    if (x401_29_laggedBurnDeath) {
      int stripedAlpha = 255;
      if (alpha > 127)
        stripedAlpha = (alpha - 128) * 2;
      xb4_drawFlags = CModelFlags(3, 0, 3, zeus::CColor(0.f, (stripedAlpha * stripedAlpha) / 65025.f));
    } else if (x401_28_burning) {
      xb4_drawFlags = CModelFlags(5, 0, 3, zeus::CColor(0.f, 1.f));
    } else {
      zeus::CColor col2 = col;
      col2.a() = alpha / 255.f;
      xb4_drawFlags = CModelFlags(5, 0, 3, col2);
    }
  } else {
    if (col.r() != 0.f || col.g() != 0.f || col.b() != 0.f) {
      /* Being damaged */
      zeus::CColor col2 = col;
      col2.a() = alpha / 255.f;
      xb4_drawFlags = CModelFlags(2, 0, 3, zeus::skWhite);
      /* Make color additive */
      xb4_drawFlags.addColor = col2;
    } else {
      xb4_drawFlags = CModelFlags(0, 0, 3, zeus::skWhite);
    }
  }

  CActor::PreRender(mgr, frustum);
}

void CPatterned::AddToRenderer(const zeus::CFrustum& frustum, CStateManager& mgr) {
  if (x402_29_drawParticles) {
    if (x64_modelData && !x64_modelData->IsNull()) {
      int mask, target;
      mgr.GetCharacterRenderMaskAndTarget(x402_31_thawed, mask, target);
      if (CAnimData* aData = x64_modelData->GetAnimationData()) {
        aData->GetParticleDB().AddToRendererClippedMasked(frustum, mask, target);
      }
    }
  }
  CActor::AddToRenderer(frustum, mgr);
}

void CPatterned::RenderIceModelWithFlags(const CModelFlags& flags) const {
  CModelFlags useFlags = flags;
  useFlags.x1_matSetIdx = 0;
  CAnimData* animData = x64_modelData->GetAnimationData();
  if (CMorphableSkinnedModel* iceModel = animData->GetIceModel().GetObj())
    animData->Render(*iceModel, useFlags, {*x510_vertexMorph}, iceModel->GetMorphMagnitudes());
}

void CPatterned::Render(CStateManager& mgr) {
  int mask = 0;
  int target = 0;
  if (x402_29_drawParticles) {
    mgr.GetCharacterRenderMaskAndTarget(x402_31_thawed, mask, target);
    x64_modelData->GetAnimationData()->GetParticleDB().RenderSystemsToBeDrawnFirstMasked(mask, target);
  }
  if ((mgr.GetThermalDrawFlag() == EThermalDrawFlag::Cold && !x402_31_thawed) ||
      (mgr.GetThermalDrawFlag() == EThermalDrawFlag::Hot && x402_31_thawed) ||
      mgr.GetThermalDrawFlag() == EThermalDrawFlag::Bypass) {
    if (x401_28_burning) {
      const CTexture* ashy = mgr.GetActorModelParticles()->GetAshyTexture(*this);
      u8 alpha = GetModelAlphau8(mgr);
      if (ashy && ((!x401_29_laggedBurnDeath && alpha <= 255) || alpha <= 127)) {
        if (xe5_31_pointGeneratorParticles)
          mgr.SetupParticleHook(*this);
        zeus::CColor addColor;
        if (x401_29_laggedBurnDeath) {
          addColor = zeus::skClear;
        } else {
          addColor = mgr.GetThermalDrawFlag() == EThermalDrawFlag::Hot ? zeus::skWhite : zeus::skBlack;
        }
        x64_modelData->DisintegrateDraw(mgr, GetTransform(), *ashy, addColor,
                                        alpha * (x401_29_laggedBurnDeath ? 0.00787402f : 0.00392157f));
        if (xe5_31_pointGeneratorParticles) {
          CSkinnedModel::ClearPointGeneratorFunc();
          mgr.GetActorModelParticles()->Render(mgr, *this);
        }
      } else {
        CPhysicsActor::Render(mgr);
      }
    } else {
      CPhysicsActor::Render(mgr);
    }

    if (x450_bodyController->IsFrozen() && !x401_28_burning) {
      RenderIceModelWithFlags(CModelFlags(0, 0, 3, zeus::skWhite));
    }
  }

  if (x402_29_drawParticles) {
    x64_modelData->GetAnimationData()->GetParticleDB().RenderSystemsToBeDrawnLastMasked(mask, target);
  }
}

void CPatterned::ThinkAboutMove(float dt) {
  bool doMove = true;
  if (!x328_25_verticalMovement && !x328_27_onGround) {
    x310_moveVec.zeroOut();
    doMove = false;
  }

  if (doMove && x39c_curPattern < x38c_patterns.size()) {
    zeus::CVector3f frontVec = GetTransform().frontVector();
    zeus::CVector3f x31cCpy = x31c_faceVec;
    if (x31c_faceVec.magSquared() > 0.1f)
      x31cCpy.normalize();
    float mag = frontVec.dot(x31cCpy);

    switch (x3f8_moveState) {
    case EMoveState::Zero:
      if (!x328_26_solidCollision)
        break;
      [[fallthrough]];
    case EMoveState::One:
      doMove = false;
      if (mag > 0.85f) {
        doMove = true;
        x3f8_moveState = EMoveState::Two;
        break;
      }
      x3f8_moveState = EMoveState::One;
      break;
    case EMoveState::Two:
      x3f8_moveState = EMoveState::Three;
      [[fallthrough]];
    case EMoveState::Three:
      doMove = true;
      if (!x328_26_solidCollision) {
        x3f8_moveState = EMoveState::Zero;
        break;
      }
      if (mag > 0.9f)
        x3f8_moveState = EMoveState::Four;
      break;
    case EMoveState::Four:
      x328_24_inPosition = true;
      doMove = false;
      x3f8_moveState = EMoveState::Zero;
      break;
    default:
      break;
    }
  }

  if (!x401_26_disableMove && doMove) {
    const CBodyState* state = x450_bodyController->GetBodyStateInfo().GetCurrentState();
    if (state->ApplyAnimationDeltas() && !zeus::close_enough(x2e0_destPos - GetTranslation(), {}))
      MoveToOR((x64_modelData->GetScale() * x434_posDelta) * x55c_moveScale, dt);
  }

  RotateToOR(x440_rotDelta, dt);
}

void CPatterned::PhazeOut(CStateManager& mgr) {
  if (!x400_27_fadeToDeath)
    SendScriptMsgs(EScriptObjectState::DeathRattle, mgr, EScriptObjectMessage::None);
  x401_27_phazingOut = true;
  x450_bodyController->SetPlaybackRate(0.f);
  x64_modelData->GetAnimationData()->GetParticleDB().SetUpdatesEnabled(false);
}

bool CPatterned::ApplyBoneTracking() const {
  if (x400_25_alive)
    return x460_knockBackController.GetFlinchRemTime() <= 0.f;

  return false;
}


} // namespace urde
