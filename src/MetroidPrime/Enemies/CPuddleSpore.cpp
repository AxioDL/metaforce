#include "MetroidPrime/Enemies/CPuddleSpore.hpp"

#include "Kyoto/Audio/CSfxManager.hpp"
#include "Kyoto/Math/CUnitVector3f.hpp"

#include "MetroidPrime/CFluidPlaneCPU.hpp"
#include "MetroidPrime/CGameCollision.hpp"
#include "MetroidPrime/CPhysicsState.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/Player/CMorphBall.hpp"
#include "MetroidPrime/Player/CPlayer.hpp"
#include "MetroidPrime/ScriptObjects/CScriptWater.hpp"
#include "MetroidPrime/TCastTo.hpp"
#include "MetroidPrime/Weapons/CEnergyProjectile.hpp"

int CPuddleSpore::kEyeCount = 16;
const char* CPuddleSpore::kEyeLocators[] = {
    "Glow_1_LCTR",  "Glow_2_LCTR",  "Glow_3_LCTR",  "Glow_4_LCTR",  "Glow_5_LCTR",  "Glow_6_LCTR",
    "Glow_7_LCTR",  "Glow_8_LCTR",  "Glow_9_LCTR",  "Glow_10_LCTR", "Glow_11_LCTR", "Glow_12_LCTR",
    "Glow_13_LCTR", "Glow_14_LCTR", "Glow_15_LCTR", "Glow_16_LCTR",
};

CPuddleSpore::CPuddleSpore(const TUniqueId uid, const rstl::string& name, const EFlavorType flavor,
                           const CEntityInfo& info, const CTransform4f& xf, const CModelData& mData,
                           const CPatternedInfo& pInfo, const EColliderType colType,
                           const CAssetId glowFx, const float f1, const float f2, const float f3,
                           const float f4, const float f5, const CActorParameters& actParms,
                           const CAssetId weapon, const CDamageInfo& dInfo)
: CPatterned(kC_PuddleSpore, uid, name, flavor, info, xf, mData, pInfo, kMT_Flyer, colType,
             kBT_Restricted, actParms, kCS_Medium)
, x568_stateTime(0.f)
, x56c_glowTime(0.f)
, x570_glowDuration(f1)
, x574_attackDelay(f2)
, x578_turnDelay(f3)
, x57c_(f4)
, x580_knockPlayerImpulse(f5)
, x584_bodyOrigin(pInfo.xcc_bodyOrigin)
, x590_halfExtent(pInfo.xc4_halfExtent)
, x594_height(pInfo.xc8_height)
, x598_heightOffset(0.f)
, x59c_heightScale(1.f)
, x5a0_collisionPrimitive(CalculateBoundingBox(), GetMaterialList())
, x5c8_collisionState(0)
, x5cc_animProgress(0)
, x5d0_glowDescription(gpSimplePool->GetObj(SObjectTag('PART', glowFx)))
, x5ec_projectileInfo(weapon, dInfo)
, x614_24_updateStateTime(false)
, x614_25_updateGlowTime(false) {
  x5dc_particles.reserve(kEyeCount);
  for (int i = 0; i < kEyeCount; ++i) {
    x5dc_particles.push_back(x5d0_glowDescription);
  }

  x5ec_projectileInfo.Token().Lock();
  KnockBackCtrl().SetAutoResetImpulse(false);
}

ENTITY_ACCEPT_IMPL(CPuddleSpore)

void CPuddleSpore::PreThink(float dt, CStateManager& mgr) {
  const CAABox box = CalculateBoundingBox();
  if (x5c8_collisionState == 2) {
    AddMaterial(kMT_SolidCharacter, mgr);
  } else {
    RemoveMaterial(kMT_SolidCharacter, mgr);
  }
  UpdateBoundingState(box, mgr, dt);
  CPatterned::PreThink(dt, mgr);
}

void CPuddleSpore::Think(float dt, CStateManager& mgr) {
  if (x614_25_updateGlowTime) {
    x56c_glowTime += dt;
  }
  if (x614_24_updateStateTime) {
    x568_stateTime += dt;
  }
  HealthInfo(mgr)->SetHP(1000000.f);
  const CColor color = CColor::Lerp(CColor(1.f, 1.f, 1.f, 0.f), CColor(1.f, 1.f, 1.f, 1.f),
                                    CMath::FastMin(x56c_glowTime / x570_glowDuration, 1.f));
  for (int i = 0; i < kEyeCount; ++i) {
    const CTransform4f xf = GetLctrTransform(rstl::string(kEyeLocators[i]));
    x5dc_particles[i].SetModulationColor(color);
    x5dc_particles[i].SetGlobalTranslation(xf.GetTranslation());
    x5dc_particles[i].Update(dt);
  }
  CPatterned::Think(dt, mgr);
}

void CPuddleSpore::Render(const CStateManager& mgr) const {
  CPatterned::Render(mgr);
  if (x56c_glowTime > 0.01f) {
    for (AUTO(it, x5dc_particles.begin()); it != x5dc_particles.end(); it++) {
      const_cast< CElementGen& >(*it).Render();
    }
  }
}

void CPuddleSpore::FluidFXThink(EFluidState state, CScriptWater& water, CStateManager& mgr) {
  if (state == kFS_InFluid) {
    if (mgr.GetFluidPlaneManager()->GetLastRippleDeltaTime(GetUniqueId()) >= 2.9f) {
      const float surfaceZ = water.GetTriggerBoundsWR().GetMaxPoint().GetZ();
      const CVector3f point(GetTranslation().GetX(), GetTranslation().GetY(), surfaceZ);
      water.FluidPlane().AddRipple(2.f, GetUniqueId(), point, water, mgr);
    }
  }
}

void CPuddleSpore::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid, CStateManager& mgr) {
  bool handled = false;
  switch (msg) {
  case kSM_InvulnDamage:
    handled = true;
    break;
  case kSM_Registered:
    BodyCtrl()->Activate(mgr);
    break;
  }
  if (!handled) {
    CPatterned::AcceptScriptMsg(msg, uid, mgr);
  }
}

CProjectileInfo* CPuddleSpore::ProjectileInfo() { return &x5ec_projectileInfo; }

bool CPuddleSpore::HitShell(const CVector3f& point) const {
  if (x5c8_collisionState == 1) {
    const CUnitVector3f direction(point - GetBoundingBox().GetCenterPoint());
    const CVector3f up = GetTransform().GetUp();
    const float distance = CVector3f::Dot(up, direction);
    if (distance > -0.5f && distance < 0.5f) {
      return false;
    }
  }
  return true;
}

EWeaponCollisionResponseTypes CPuddleSpore::GetCollisionResponseType(const CVector3f& point,
                                                                     const CVector3f&,
                                                                     const CWeaponMode&,
                                                                     int) const {
  EWeaponCollisionResponseTypes response = kWCR_Unknown34;
  if (HitShell(point)) {
    response = kWCR_Unknown84;
  }
  return response;
}

void CPuddleSpore::Touch(CActor& actor, CStateManager& mgr) {
  if (IsAlive()) {
    if (CGameProjectile* projectile = TCastToPtr< CGameProjectile >(actor)) {
      if (projectile->GetOwnerId() == mgr.GetPlayer()->GetUniqueId()) {
        SetWasHit(!HitShell(projectile->GetTranslation()));
      }
    }
  }
}

void CPuddleSpore::KnockBack(const CVector3f& direction, CStateManager& mgr,
                             const CDamageInfo& info, float magnitude, bool direct,
                             const bool inDeferred) {
  if (x5c8_collisionState != 1) {
    CPatterned::KnockBack(direction, mgr, info, magnitude, direct, inDeferred);
  }
}

void CPuddleSpore::DoUserAnimEvent(CStateManager& mgr, const CInt32POINode& node,
                                   EUserEventType type, float dt) {
  bool handled = false;
  switch (type) {
  case kUE_Projectile: {
    x56c_glowTime = 0.f;
    if (ProjectileInfo()->Token().TryCache() &&
        mgr.CanCreateProjectile(GetUniqueId(), kWT_AI, kEyeCount)) {
      const CTransform4f xf = GetLctrTransform(node.GetLocatorName());
      CEnergyProjectile* projectile = rs_new CEnergyProjectile(
          true, ProjectileInfo()->Token(), kWT_AI, CTransform4f::Translate(xf.GetTranslation()),
          kMT_Character, ProjectileInfo()->GetDamage(), mgr.AllocateUniqueId(), GetCurrentAreaId(),
          GetUniqueId(), kInvalidUniqueId, 0, false, CVector3f(1.f, 1.f, 1.f),
          rstl::optional_object_null(), CSfxManager::kInternalInvalidSfxId, false);
      CProjectileWeapon& weapon = projectile->Projectile();
      weapon.SetVelocity(xf.Rotate(weapon.GetVelocity()));
      mgr.AddObject(projectile);
    }
    handled = true;
    break;
  }
  }
  if (!handled) {
    CPatterned::DoUserAnimEvent(mgr, node, type, dt);
  }
}

void CPuddleSpore::CollidedWith(const TUniqueId& uid, const CCollisionInfoList& list,
                                CStateManager& mgr) {
  if (x5c8_collisionState != 2) {
    CPatterned::CollidedWith(uid, list, mgr);
  }
}

void CPuddleSpore::KnockPlayer(CStateManager& mgr, float magnitude) {
  const CAABox& selfBox = GetBoundingBox();
  const CAABox& playerBox = mgr.GetPlayer()->GetBoundingBox();
  const float selfMaxZ = selfBox.GetMaxPoint().GetZ();
  if (selfMaxZ < (playerBox.GetMinPoint().GetZ() + playerBox.GetMaxPoint().GetZ()) / 2.f &&
      selfBox.GetMaxPoint().GetX() >= playerBox.GetMinPoint().GetX() &&
      selfBox.GetMaxPoint().GetY() >= playerBox.GetMinPoint().GetY() &&
      playerBox.GetMaxPoint().GetX() >= selfBox.GetMinPoint().GetX() &&
      playerBox.GetMaxPoint().GetY() >= selfBox.GetMinPoint().GetY() &&
      playerBox.GetMinPoint().GetZ() - selfMaxZ < 0.2f) {
    float scale = 1.f;
    const CVector3f direction(1.f, 0.f, 0.3f);
    CPlayer& player = *mgr.Player();
    if (player.GetMorphballTransitionState() == CPlayer::kMS_Morphed) {
      scale = 1.5f;
    }
    player.ApplyImpulseWR(scale * (magnitude * player.GetMass()) * GetTransform().Rotate(direction),
                          CAxisAngle::Identity());
    player.SetMoveState(NPlayer::kMS_ApplyJump, mgr);
  }
}

const CCollisionPrimitive* CPuddleSpore::GetCollisionPrimitive() const {
  return &x5a0_collisionPrimitive;
}

const CAABox CPuddleSpore::CalculateBoundingBox() const {
  const float offset = x598_heightOffset;
  const CVector3f min(-x590_halfExtent + x584_bodyOrigin.GetX(),
                      -x590_halfExtent + x584_bodyOrigin.GetY(), offset + x584_bodyOrigin.GetZ());
  const CVector3f max(x590_halfExtent + x584_bodyOrigin.GetX(),
                      x590_halfExtent + x584_bodyOrigin.GetY(),
                      x594_height * x59c_heightScale + offset + x584_bodyOrigin.GetZ());
  return CAABox(CVector3f::Lerp(min, GetBaseBoundingBox().GetMinPoint(), 0.95f),
                CVector3f::Lerp(max, GetBaseBoundingBox().GetMaxPoint(), 0.95f));
}

void CPuddleSpore::UpdateBoundingState(const CAABox& box, CStateManager& mgr, float dt) {
  SetBoundingBox(box);
  x5a0_collisionPrimitive = CCollidableAABox(box, GetMaterialList());
  CPlayer& player = *mgr.Player();
  CAABox playerBox(CVector3f::Zero(), CVector3f::Zero());
  const CAABox& selfBox = GetBoundingBox();
  if (player.GetMorphballTransitionState() == CPlayer::kMS_Morphed) {
    const float radius = player.GetMorphBall()->GetBallRadius();
    const CVector3f center = player.GetTranslation() + CVector3f(0.f, 0.f, radius);
    const float negativeRadius = -radius;
    playerBox = CAABox(center + CVector3f(negativeRadius, negativeRadius, negativeRadius),
                       center + CVector3f(radius, radius, radius));
  } else {
    playerBox = player.GetBoundingBox();
  }
  if (selfBox.DoBoundsOverlap(playerBox)) {
    const float epsilon = x5c8_collisionState == 2 ? 0.001f : -0.0001f;
    const float bias = epsilon + (selfBox.GetMaxPoint().GetZ() - playerBox.GetMinPoint().GetZ());
    if (bias > 0.f && selfBox.GetMaxPoint().GetZ() < playerBox.GetMaxPoint().GetZ()) {
      const bool hadGroundCollider = player.GetMaterialList().HasMaterial(kMT_GroundCollider);
      if (hadGroundCollider) {
        player.RemoveMaterial(kMT_GroundCollider, mgr);
      }
      player.RemoveMaterial(kMT_Player, mgr);
      CPhysicsState state = player.GetPhysicsState();
      player.MoveToOR(bias * CVector3f(0.f, 0.f, 1.f), dt);
      CGameCollision::Move(mgr, player, dt, nullptr);
      state.SetTranslation(player.GetTranslation());
      player.SetPhysicsState(state);
      if (hadGroundCollider) {
        player.AddMaterial(kMT_GroundCollider, mgr);
      }
      player.AddMaterial(kMT_Player, mgr);
    }
  }
}

bool CPuddleSpore::InAttackPosition(CStateManager&, float) {
  return x568_stateTime >= x570_glowDuration;
}

bool CPuddleSpore::ShouldAttack(CStateManager&, float) {
  return x568_stateTime >= x574_attackDelay;
}

bool CPuddleSpore::ShouldTurn(CStateManager& mgr, float) {
  const CPlayer& player = *mgr.GetPlayer();
  const CAABox& selfBox = GetBoundingBox();
  const CAABox& playerBox = player.GetBoundingBox();
  if (selfBox.GetMaxPoint().GetZ() <
          (playerBox.GetMinPoint().GetZ() + playerBox.GetMaxPoint().GetZ()) / 2.f &&
      selfBox.GetMaxPoint().GetX() >= playerBox.GetMinPoint().GetX() &&
      selfBox.GetMaxPoint().GetY() >= playerBox.GetMinPoint().GetY() &&
      playerBox.GetMaxPoint().GetX() >= selfBox.GetMinPoint().GetX() &&
      playerBox.GetMaxPoint().GetY() >= selfBox.GetMinPoint().GetY() &&
      player.GetMorphballTransitionState() == CPlayer::kMS_Morphed) {
    return true;
  }
  return x568_stateTime >= x578_turnDelay;
}

bool CPuddleSpore::AnimOver(CStateManager&, float) { return x5cc_animProgress == 2; }

void CPuddleSpore::InActive(CStateManager&, EStateMsg msg, float) {
  switch (msg) {
  case kStateMsg_Update:
    return;
  case kStateMsg_Activate:
    BodyCtrl()->SetLocomotionType(pas::kLT_Relaxed);
    x56c_glowTime = 0.f;
    x598_heightOffset = -1.f;
    break;
  }
}

void CPuddleSpore::Active(CStateManager&, EStateMsg msg, float) {
  switch (msg) {
  case kStateMsg_Activate:
    BodyCtrl()->SetLocomotionType(pas::kLT_Lurk);
    x568_stateTime = 0.f;
    x56c_glowTime = 0.f;
    x598_heightOffset = 0.f;
    x614_24_updateStateTime = true;
    x614_25_updateGlowTime = true;
    break;
  case kStateMsg_Update:
    break;
  case kStateMsg_Deactivate:
    x614_24_updateStateTime = false;
    x614_25_updateGlowTime = false;
    break;
  }
}

void CPuddleSpore::Run(CStateManager&, EStateMsg msg, float) {
  switch (msg) {
  case kStateMsg_Activate:
    x5c8_collisionState = 1;
    x5cc_animProgress = 0;
    x568_stateTime = 0.f;
    x614_24_updateStateTime = false;
    x598_heightOffset = 0.f;
    x59c_heightScale = 1.5f;
    break;
  case kStateMsg_Update:
    switch (x5cc_animProgress) {
    case 2:
      break;
    case 0:
      if (GetBodyCtrl()->GetCurrentStateId() == pas::kAS_LoopReaction) {
        x5cc_animProgress = 1;
        x614_24_updateStateTime = true;
      } else {
        BodyCtrl()->CommandMgr().DeliverCmd(CBCLoopReactionCmd(pas::kRT_Zero));
      }
      break;
    case 1:
      if (GetBodyCtrl()->GetCurrentStateId() != pas::kAS_LoopReaction) {
        x5cc_animProgress = 2;
      }
      break;
    }
    break;
  case kStateMsg_Deactivate:
    BodyCtrl()->CommandMgr().DeliverCmd(CBodyStateCmd(kBSC_ExitState));
    x5c8_collisionState = 0;
    x59c_heightScale = 1.f;
    x614_24_updateStateTime = false;
    break;
  }
}

void CPuddleSpore::TurnAround(CStateManager&, EStateMsg msg, float) {
  switch (msg) {
  case kStateMsg_Activate:
    x568_stateTime = 0.f;
    x56c_glowTime = 0.f;
    SetWasHit(false);
    x598_heightOffset = -2.5f;
    x5c8_collisionState = 2;
    x5cc_animProgress = 0;
    x614_24_updateStateTime = false;
    break;
  case kStateMsg_Update:
    switch (x5cc_animProgress) {
    case 2:
      break;
    case 0:
      if (GetBodyCtrl()->GetCurrentStateId() == pas::kAS_LieOnGround) {
        x5cc_animProgress = 1;
        x614_24_updateStateTime = true;
      } else {
        BodyCtrl()->CommandMgr().DeliverCmd(CBCKnockDownCmd(CVector3f(1.f, 0.f, 0.f), pas::kS_One));
      }
      break;
    case 1:
      if (GetBodyCtrl()->GetCurrentStateId() != pas::kAS_LieOnGround) {
        x5cc_animProgress = 2;
      }
      break;
    }
    break;
  case kStateMsg_Deactivate:
    x614_24_updateStateTime = false;
    break;
  }
}

void CPuddleSpore::GetUp(CStateManager& mgr, EStateMsg msg, float) {
  switch (msg) {
  case kStateMsg_Activate:
    BodyCtrl()->CommandMgr().DeliverCmd(CBCGetupCmd(pas::kGetup_Zero));
    KnockPlayer(mgr, x580_knockPlayerImpulse);
    x56c_glowTime = 0.f;
    x598_heightOffset = -1.f;
    x5cc_animProgress = 0;
    break;
  case kStateMsg_Update:
    KnockPlayer(mgr, x580_knockPlayerImpulse / 4.f);
    switch (x5cc_animProgress) {
    case 0:
      BodyCtrl()->CommandMgr().DeliverCmd(CBCGetupCmd(pas::kGetup_Zero));
      if (GetBodyCtrl()->GetCurrentStateId() == pas::kAS_Getup) {
        x5cc_animProgress = 1;
      }
      break;
    case 1:
      if (GetBodyCtrl()->GetCurrentStateId() != pas::kAS_Getup) {
        x5cc_animProgress = 1;
      }
      break;
    }
    break;
  case kStateMsg_Deactivate:
    x5c8_collisionState = 0;
    break;
  }
}

void CPuddleSpore::Attack(CStateManager& mgr, EStateMsg msg, float) {
  switch (msg) {
  case kStateMsg_Activate:
    x32c_animState = kAS_Ready;
    x598_heightOffset = 0.f;
    break;
  case kStateMsg_Update:
    TryCommand(mgr, pas::kAS_MeleeAttack, &CPatterned::TryMeleeAttack, 1);
    break;
  case kStateMsg_Deactivate:
    x32c_animState = kAS_NotReady;
    break;
  }
}
