#include "MetroidPrime/Enemies/CPuddleToadGamma.hpp"

#include "Kyoto/CResFactory.hpp"
#include "Kyoto/Math/CAbsAngle.hpp"
#include "Kyoto/Math/CRelAngle.hpp"
#include "Kyoto/Math/CUnitVector3f.hpp"
#include "MetroidPrime/CGameCollision.hpp"
#include "MetroidPrime/CPhysicsState.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/Player/CMorphBall.hpp"
#include "MetroidPrime/TCastTo.hpp"
#include "MetroidPrime/Weapons/CBomb.hpp"
#include "MetroidPrime/Player/CPlayer.hpp"

const CVector3f CPuddleToadGamma::skBellyOffset(0.f, 0.1f, -0.3f);
const char* CPuddleToadGamma::mMouthLocatorName = "MOUTH_LCTR_SDK";
const char* CPuddleToadGamma::mBellyLocatorName = "SAMUS_POS_LCTR_SDK";

CPuddleToadGamma::CPuddleToadGamma(
    const TUniqueId uid, const rstl::string& name, const EFlavorType flavor,
    const CEntityInfo& info, const CTransform4f& xf, const CModelData& mData,
    const CPatternedInfo& pInfo, const CActorParameters& aParms, const float suckForceMultiplier,
    const float suckAngle, const float playerSuckRange, const CVector3f localShootDir,
    const float playerShootSpeed, const float shouldAttackWaitTime, const float spotPlayerWaitTime,
    const CDamageInfo& playerShootDamage, const CDamageInfo& dInfo2, const CAssetId dcln)
: CPatterned(kC_PuddleToad, uid, name, flavor, info, xf, mData, pInfo, kMT_Flyer, kCT_Zero,
             kBT_Restricted, aParms, kCS_Large)
, x568_stateProg(0.f)
, x56c_waitTimer(0.f)
, x570_playerShootDamage(playerShootDamage)
, x58c_dInfo2(dInfo2)
, x5a8_suckForceMultiplier(suckForceMultiplier)
, x5ac_minSuckAngleProj(cosine(CRelAngle::FromDegrees(suckAngle / 2.f)))
, x5b0_playerSuckRange(playerSuckRange)
, x5b4_localShootDir(localShootDir)
, x5c0_playerShootSpeed(playerShootSpeed)
, x5c4_shouldAttackWaitTime(shouldAttackWaitTime)
, x5c8_spotPlayerWaitTime(spotPlayerWaitTime)
, x5cc_suckPoint(CVector3f::Zero())
, x5d8_damageablePoint(CVector3f::Zero())
, x5e8_24_playerInside(false)
, x5e8_25_waitTimerActive(false)
, x5e8_26_shotPlayer(false) {
  x401_26_disableMove = true;
  KnockBackCtrl().SetEnableBurn(false);
  KnockBackCtrl().SetEnableLaggedBurnDeath(false);
  KnockBackCtrl().SetEnableShock(false);
  KnockBackCtrl().SetEnableFreeze(false);
  KnockBackCtrl().SetX81_31(false);
  SetMovable(false);

  if (dcln != kInvalidAssetId && gpResourceFactory->GetResourceTypeById(dcln) != 0) {
    TLockedToken< CCollidableOBBTreeGroupContainer > container =
        TLockedToken< CCollidableOBBTreeGroupContainer >(
            gpSimplePool->GetObj(SObjectTag('DCLN', dcln)));

    x5e4_collisionTreePrim = rs_new CCollidableOBBTreeGroup(*container, GetMaterialList());
  }
}

void CPuddleToadGamma::Think(float dt, CStateManager& mgr) {
  CPatterned::Think(dt, mgr);
  if (x5e8_25_waitTimerActive) {
    x56c_waitTimer += dt;
  }
}

bool CPuddleToadGamma::PlayerInVortexArea(const CStateManager& mgr) {
  const CPlayer& player = *mgr.GetPlayer();
  const CTransform4f xf = GetLctrTransform(rstl::string_l(mMouthLocatorName));
  const CVector3f playerCenter =
      player.GetTranslation() + CVector3f(0.f, 0.f, player.GetMorphBall()->GetBallRadius());
  const CVector3f forward = GetTransform().Rotate(CVector3f(0.f, 1.f, 0.f));
  const CVector3f delta = playerCenter - (xf.GetTranslation() - 1.f * forward);
  const CVector3f angleDelta = player.GetTranslation() - (xf.GetTranslation() - 4.f * forward);
  const float projection = CVector3f::Dot(delta.AsNormalized(), forward);
  const float angleProjection = CVector3f::Dot(angleDelta.AsNormalized(), forward);
  const float distance = delta.Magnitude();
  static const CMaterialFilter kSolidFilter = CMaterialFilter::MakeIncludeExclude(
      CMaterialList(kMT_Solid),
      CMaterialList(kMT_Character, kMT_Player, kMT_ProjectilePassthrough));
  if (distance > 2.f) {
    const CRayCastResult result =
        mgr.RayStaticIntersection(xf.GetTranslation(), (1.f / distance) * delta,
                                  distance - player.GetMorphBall()->GetBallRadius(), kSolidFilter);
    if (result.IsValid()) {
      return false;
    }
  }
  if (distance < x5b0_playerSuckRange) {
    return projection > 0.f && angleProjection > x5ac_minSuckAngleProj;
  }
  return false;
}

void CPuddleToadGamma::SuckPlayer(CStateManager& mgr, float arg) {
  CPlayer* player = mgr.Player();
  const CVector3f playerPos = player->GetTranslation();
  const CVector3f posDiff = playerPos - x5cc_suckPoint;
  if (player->GetMorphballTransitionState() == CPlayer::kMS_Morphed) {
    const float mag = posDiff.Magnitude();
    if (mag < 3.f) {
      player->Stop();
      CenterPlayer(mgr, x5cc_suckPoint, arg);
    } else {
      float force = x5a8_suckForceMultiplier * (x5b0_playerSuckRange / (mag * mag));
      CVector3f forceVec = -posDiff * player->GetMass() * force;
      player->ApplyForceWR(forceVec, CAxisAngle::Identity());
    }
  }
}

const CDamageVulnerability* CPuddleToadGamma::GetDamageVulnerability() const {
  return &CDamageVulnerability::ImmuneVulnerability();
}

const CDamageVulnerability* CPuddleToadGamma::GetDamageVulnerability(const CVector3f& pos,
                                                                     const CVector3f&,
                                                                     const CDamageInfo&) const {
  if (x5e8_24_playerInside && (x5d8_damageablePoint - pos).MagSquared() < 4.f) {
    return CAi::GetDamageVulnerability();
  }
  return &CDamageVulnerability::ImmuneVulnerability();
}

void CPuddleToadGamma::ShootPlayer(CStateManager& mgr, float speed) {
  CPlayer& player = *mgr.Player();
  const CVector3f shootDir = GetTransform().Rotate(x5b4_localShootDir.AsNormalized());
  player.EnableLeaveMorphBall(true);
  if (player.GetMorphballTransitionState() == CPlayer::kMS_Morphed) {
    x5e8_26_shotPlayer = true;
    player.Stop();
    player.SetVelocityWR(CVector3f::Zero());
    player.ApplyImpulseWR(speed * (player.GetMass() * shootDir), CAxisAngle::Identity());
    player.SetMoveState(NPlayer::kMS_ApplyJump, mgr);
    mgr.ApplyDamage(
        GetUniqueId(), mgr.GetPlayer()->GetUniqueId(), GetUniqueId(), x570_playerShootDamage,
        CMaterialFilter::MakeIncludeExclude(CMaterialList(SolidMaterial), CMaterialList()),
        CVector3f::Zero());
    player.MorphBall()->SetAsProjectile();
    TEntityList nearList;
    mgr.BuildNearList(nearList, GetBoundingBox(),
                      CMaterialFilter::MakeInclude(CMaterialList(kMT_Bomb)), this);
    for (AUTO(it, nearList.begin()); it != nearList.end(); it++) {
      if (CBomb* const bomb = TCastToPtr< CBomb >(mgr.ObjectById(*it))) {
        bomb->SetVelocityWR((mgr.Random()->Float() * 5.f + 20.f) * shootDir);
        bomb->SetConstantAccelerationWR(CVector3f(0.f, 0.f, -CPhysicsActor::GravityConstant()));
      }
    }
  }
}

void CPuddleToadGamma::SetSolid(CStateManager& mgr, bool solid) {
  if (solid) {
    AddMaterial(kMT_Solid, mgr);
    RemoveMaterial(kMT_NonSolidDamageable, mgr);
  } else {
    RemoveMaterial(kMT_Solid, mgr);
    AddMaterial(kMT_NonSolidDamageable, mgr);
  }
}

void CPuddleToadGamma::DoUserAnimEvent(CStateManager& mgr, const CInt32POINode& node,
                                       EUserEventType type, float dt) {
  bool handled = false;
  switch (type) {
  case kUE_Projectile:
    ShootPlayer(mgr, x5c0_playerShootSpeed);
    handled = true;
    break;
  }
  if (!handled) {
    CPatterned::DoUserAnimEvent(mgr, node, type, dt);
  }
}

void CPuddleToadGamma::CenterPlayer(CStateManager& mgr, const CVector3f& pos, float dt) {
  CPlayer& player = *mgr.Player();
  const CVector3f direction = (pos - player.GetTranslation()).AsNormalized();
  const CVector3f velocity = (1.f / (2.f * dt)) * direction;
  player.SetVelocityWR(velocity);
}

ENTITY_ACCEPT_IMPL(CPuddleToadGamma)

void CPuddleToadGamma::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid,
                                       CStateManager& mgr) {
  CPatterned::AcceptScriptMsg(msg, uid, mgr);
  switch (msg) {
  case kSM_Activate:
    break;
  case kSM_Registered: {
    BodyCtrl()->Activate(mgr);
    const CTransform4f xf = GetLctrTransform(rstl::string_l(mBellyLocatorName));
    x5cc_suckPoint = xf.GetTranslation() + GetTransform().Rotate(skBellyOffset);
    x5d8_damageablePoint = x5cc_suckPoint;
    RemoveMaterial(kMT_Target, kMT_Orbit, mgr);
    AddMaterial(kMT_Immovable, mgr);
    AddMaterial(kMT_SolidCharacter, mgr);
    break;
  }
  }
}

void CPuddleToadGamma::SetPlayerPosition(CStateManager& mgr, const CVector3f& pos) {
  const float dt = x500_preThinkDt;
  CPlayer& player = *mgr.Player();
  player.Stop();
  player.SetVelocityWR(CVector3f::Zero());
  const bool hadGroundCollider = player.GetMaterialList().HasMaterial(kMT_GroundCollider);
  if (hadGroundCollider) {
    player.RemoveMaterial(kMT_GroundCollider, mgr);
  }
  player.RemoveMaterial(kMT_Player, mgr);
  const bool hadSolid = GetMaterialList().HasMaterial(kMT_Solid);
  if (hadSolid) {
    RemoveMaterial(kMT_Solid, mgr);
  }
  CPhysicsState state = player.GetPhysicsState();
  player.Stop();
  player.MoveToWR(pos, dt);
  CGameCollision::Move(mgr, player, dt, nullptr);
  state.SetTranslation(player.GetTranslation());
  player.SetPhysicsState(state);
  if (hadGroundCollider) {
    player.AddMaterial(kMT_GroundCollider, mgr);
  }
  player.AddMaterial(kMT_Player, mgr);
  if (hadSolid) {
    AddMaterial(kMT_Solid, mgr);
  }
}

bool CPuddleToadGamma::InAttackPosition(CStateManager& mgr, float) {
  if (mgr.GetPlayer()->GetMorphballTransitionState() == CPlayer::kMS_Morphed) {
    return PlayerInVortexArea(mgr);
  }
  return false;
}

bool CPuddleToadGamma::Inside(CStateManager& mgr, float) {
  if (mgr.GetPlayer()->GetMorphballTransitionState() == CPlayer::kMS_Morphed) {
    const CVector3f delta = mgr.GetPlayer()->GetTranslation() - x5cc_suckPoint;
    if (CVector3f::Dot(delta, GetTransform().GetForward()) <= 0.f && delta.MagSquared() < 2.f) {
      return true;
    }
  }
  return false;
}

bool CPuddleToadGamma::ShouldAttack(CStateManager&, float) {
  return x56c_waitTimer >= x5c4_shouldAttackWaitTime;
}

bool CPuddleToadGamma::SpotPlayer(CStateManager&, float) {
  return x56c_waitTimer >= x5c8_spotPlayerWaitTime;
}

void CPuddleToadGamma::InActive(CStateManager& mgr, EStateMsg msg, float) {
  switch (msg) {
  case kStateMsg_Update:
  case kStateMsg_Deactivate:
    break;
  case kStateMsg_Activate:
    BodyCtrl()->SetLocomotionType(pas::kLT_Relaxed);
    SetSolid(mgr, true);
    mgr.Player()->EnableLeaveMorphBall(true);
    StateMachineState().SetDelay(2.f);
    break;
  }
}

void CPuddleToadGamma::Active(CStateManager& mgr, EStateMsg msg, float) {
  switch (msg) {
  case kStateMsg_Activate: {
    BodyCtrl()->SetLocomotionType(pas::kLT_Lurk);
    const CTransform4f xf = GetLctrTransform(rstl::string_l(mBellyLocatorName));
    x5cc_suckPoint = xf.GetTranslation() + GetTransform().Rotate(skBellyOffset);
    x56c_waitTimer = 0.f;
    x5e8_25_waitTimerActive = true;
    SetSolid(mgr, true);
    mgr.Player()->EnableLeaveMorphBall(true);
    break;
  }
  case kStateMsg_Update:
    break;
  case kStateMsg_Deactivate:
    x5e8_25_waitTimerActive = false;
    break;
  }
}

void CPuddleToadGamma::Suck(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    x568_stateProg = 0;
    SetSolid(mgr, false);
    mgr.Player()->EnableLeaveMorphBall(false);
    mgr.Player()->MorphBall()->DisableHalfPipeStatus();
    break;
  case kStateMsg_Update:
    switch (x568_stateProg) {
    case 0:
      if (GetBodyCtrl()->GetCurrentStateId() == pas::kAS_LoopReaction) {
        x568_stateProg = 1;
      } else {
        BodyCtrl()->CommandMgr().DeliverCmd(CBCLoopReactionCmd(pas::kRT_Zero));
      }
      break;
    case 1:
      SuckPlayer(mgr, dt);
      break;
    }
    break;
  case kStateMsg_Deactivate:
    BodyCtrl()->CommandMgr().DeliverCmd(CBodyStateCmd(kBSC_ExitState));
    break;
  }
}

bool CPuddleToadGamma::LostInterest(CStateManager& mgr, float) {
  return !GetTouchBounds()->DoBoundsOverlap(mgr.GetPlayer()->GetBoundingBox());
}

void CPuddleToadGamma::Crouch(CStateManager& mgr, EStateMsg msg, float) {
  switch (msg) {
  case kStateMsg_Activate:
    x568_stateProg = 0;
    x56c_waitTimer = 0.f;
    x5e8_25_waitTimerActive = true;
    x5e8_24_playerInside = true;
    mgr.Player()->Stop();
    mgr.Player()->SetVelocityWR(CVector3f::Zero());
    SendScriptMsgs(kSS_Inside, mgr, kSM_None);
    if (!mgr.Player()->AttachActorToPlayer(GetUniqueId(), false)) {
      x56c_waitTimer = 100.f;
    }
    SetSolid(mgr, false);
    mgr.Player()->EnableLeaveMorphBall(false);
    mgr.Player()->MorphBall()->DisableHalfPipeStatus();
    SetSolid(mgr, false);
    break;
  case kStateMsg_Update: {
    const CTransform4f xf = GetLctrTransform(rstl::string_l(mBellyLocatorName));
    x5cc_suckPoint = xf.GetTranslation() + GetTransform().Rotate(skBellyOffset);
    SetPlayerPosition(mgr, x5cc_suckPoint);
    switch (x568_stateProg) {
    case 0:
      if (GetBodyCtrl()->GetCurrentStateId() == pas::kAS_Locomotion) {
        x568_stateProg = 1;
      } else {
        BodyCtrl()->SetLocomotionType(pas::kLT_Combat);
      }
      break;
    case 1:
      break;
    }
    break;
  }
  case kStateMsg_Deactivate:
    if (mgr.GetPlayer()->GetAttachedActor() == GetUniqueId()) {
      mgr.Player()->DetachActorFromPlayer();
    }
    mgr.Player()->EnableLeaveMorphBall(true);
    x5e8_25_waitTimerActive = false;
    break;
  }
}

void CPuddleToadGamma::Attack(CStateManager& mgr, EStateMsg msg, float) {
  switch (msg) {
  case kStateMsg_Activate:
    mgr.Player()->Stop();
    mgr.Player()->SetVelocityWR(CVector3f::Zero());
    BodyCtrl()->CommandMgr().DeliverCmd(CBCMeleeAttackCmd(pas::kS_One));
    x5e8_26_shotPlayer = false;
    mgr.Player()->EnableLeaveMorphBall(false);
    mgr.Player()->MorphBall()->SetBombJumpState(CMorphBall::kBJS_BombJumpDisabled);
    break;
  case kStateMsg_Update:
    if (!x5e8_26_shotPlayer) {
      const CTransform4f xf = GetLctrTransform(rstl::string_l(mBellyLocatorName));
      x5cc_suckPoint = xf.GetTranslation() + GetTransform().Rotate(skBellyOffset);
      SetPlayerPosition(mgr, x5cc_suckPoint);
    } else if (LostInterest(mgr, 0.f)) {
      SetSolid(mgr, true);
    }
    break;
  case kStateMsg_Deactivate:
    SetSolid(mgr, true);
    mgr.Player()->EnableLeaveMorphBall(true);
    mgr.Player()->MorphBall()->SetBombJumpState(CMorphBall::kBJS_BombJumpAvailable);
    x5e8_24_playerInside = false;
    break;
  }
}

rstl::optional_object< CAABox > CPuddleToadGamma::GetTouchBounds() const {
  if (GetActive()) {
    if (x5e4_collisionTreePrim.get() != nullptr) {
      return x5e4_collisionTreePrim->CalculateAABox(GetTransform());
    }
    return GetBoundingBox();
  }
  return rstl::optional_object_null();
}

const CCollisionPrimitive* CPuddleToadGamma::GetCollisionPrimitive() const {
  if (x5e4_collisionTreePrim.get() == nullptr) {
    return CPhysicsActor::GetCollisionPrimitive();
  }
  return x5e4_collisionTreePrim.get();
}

CTransform4f CPuddleToadGamma::GetPrimitiveTransform() const {
  CTransform4f xf = GetTransform();
  xf.AddTranslation(GetPrimitiveOffset());
  return xf;
}

CPuddleToadGamma::~CPuddleToadGamma() {}
