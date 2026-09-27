#include "MetroidPrime/Enemies/CTryclops.hpp"

#include "Collision/CCollidableSphere.hpp"
#include "Kyoto/Math/CRelAngle.hpp"
#include "MetroidPrime/CGameCollision.hpp"
#include "MetroidPrime/CWorld.hpp"
#include "MetroidPrime/Player/CMorphBall.hpp"
#include "MetroidPrime/Player/CPlayer.hpp"
#include "MetroidPrime/TCastTo.hpp"
#include "MetroidPrime/Weapons/CBomb.hpp"

const CDamageVulnerability CTryclops::skPowerBombVulnerability =
    CDamageVulnerability(kVN_Deflect, kVN_Deflect, kVN_Deflect, kVN_Deflect, kVN_Deflect,
                         kVN_Normal, kVN_Deflect, kVN_Deflect, kVN_Deflect, kVN_Deflect,
                         kVN_Deflect, kVN_Deflect, kVN_Deflect, kVN_Deflect, kVN_Deflect, kDT_None);

CVector3f CTryclops::kBombPosOffset(0.f, 0.f, -0.3f);
const char* const CTryclops::kMouthLctr = "ballGrab_locator";
static const char* const skRootLocator = "Skeleton_Root";

CTryclops::CTryclops(TUniqueId uid, const rstl::string& name, const CEntityInfo& info,
                     const CTransform4f& xf, const CModelData& mData, const CPatternedInfo& pInfo,
                     const CActorParameters& actParms, float suckForceMultiplier, float suckAngle,
                     float suckRange, float launchSpeed)
: CPatterned(kC_Tryclops, uid, name, kFT_Zero, info, xf, mData, pInfo, kMT_Ground, kCT_One,
             kBT_BiPedal, actParms, kCS_Small)
, mPathFindSearch(nullptr, 1, pInfo.GetPathfindingIndex(), 1.f, 1.f)
, mPlayerRotation(CTransform4f::Identity())
, mSuckForceMultiplier(suckForceMultiplier)
, mMinSuckAngleProj(cosf(CRelAngle::FromDegrees(suckAngle * 0.5f).AsRadians()))
, mSuckRange(suckRange)
, mLaunchSpeed(launchSpeed)
, mIgnoreMorphballTimer(0.f)
, x690_(0)
, mBombId(kInvalidUniqueId)
, x696_(kInvalidUniqueId)
, mShotTarget(false)
, mTargetingBomb(false)
, mVulnerable(false)
, mDizzy(false) {
  SetDrawShadow(false);
  MakeThermalColdAndHot();
  KnockBackCtrl().SetAutoResetImpulse(false);
  mLookAtDeathDir = false;
}

CTryclops::~CTryclops() {}

void CTryclops::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid, CStateManager& mgr) {
  CPatterned::AcceptScriptMsg(msg, uid, mgr);
  switch (msg) {
  case kSM_Registered:
    BodyCtrl()->Activate(mgr);
    break;
  case kSM_InitializedInArea: {
    const TAreaId areaId = GetCurrentAreaId();
    const CGameArea::CPostConstructed* constructed =
        mgr.GetWorld()->GetAreaAlways(areaId).GetPostConstructed();
    mPathFindSearch.SetArea(constructed->mPathArea);
  } break;
  }
}

bool CTryclops::InMaxRange(CStateManager& mgr, float arg) {
  if (mBombId != kInvalidUniqueId) {
    return true;
  }

  const float detectionRange = mDetectionRange;
  const float detectionHeight = mDetectionHeightRange;
  float nearestDistSq = detectionRange * detectionRange;
  const float heightSq = detectionHeight * detectionHeight;
  const float negRange = -detectionRange;
  const CAABox bounds(GetTranslation() + CVector3f(negRange, negRange, 0.f),
                      GetTranslation() +
                          CVector3f(detectionRange, detectionRange, detectionHeight));
  TEntityList nearList;
  mgr.BuildNearList(nearList, bounds, CMaterialFilter::MakeInclude(CMaterialList(kMT_Bomb)), this);
  mBombId = kInvalidUniqueId;
  for (AUTO(it, nearList.begin()); it != nearList.end(); ++it) {
    if (const CBomb* const bomb = TCastToConstPtr< CBomb >(mgr.GetObjectById(*it))) {
      if (!bomb->IsBeingDragged()) {
        const CVector3f delta = bomb->GetTranslation() - GetTranslation();
        const float distSq = delta.MagSquared();
        if (distSq < nearestDistSq) {
          bool inHeightRange = true;
          if (detectionHeight > 0.f) {
            inHeightRange = delta.GetZ() * delta.GetZ() < heightSq;
          }
          if (inHeightRange &&
              mPathFindSearch.OnPath(bomb->GetTranslation()) == CPathFindSearch::kR_Success) {
            mBombId = bomb->GetUniqueId();
            nearestDistSq = distSq;
          }
        }
      }
    }
  }

  if (mBombId != kInvalidUniqueId) {
    if (CBomb* bomb = TCastToPtr< CBomb >(mgr.ObjectById(mBombId))) {
      bomb->SetFuseDisabled(true);
      bomb->SetIsBeingDragged(true);

      return true;
    }
  }

  return false;
}

bool CTryclops::InDetectionRange(CStateManager& mgr, float arg) {
  const CPlayer& player = *mgr.GetPlayer();
  const bool unattached = player.GetAttachedActor() == kInvalidUniqueId;
  const bool ignore = mIgnoreMorphballTimer > 0.f;
  if (player.GetMorphballTransitionState() == CPlayer::kMS_Morphed && unattached && !ignore &&
      CPatterned::InDetectionRange(mgr, arg)) {
    return mPathFindSearch.OnPath(player.GetBallPosition()) == CPathFindSearch::kR_Success;
  }
  return false;
}

void CTryclops::Think(float dt, CStateManager& mgr) {
  CPatterned::Think(dt, mgr);
  if (mAlive && mIgnoreMorphballTimer > 0.f) {
    mIgnoreMorphballTimer -= dt;
  }
  if (mgr.GetPlayer()->GetAttachedActor() == GetUniqueId() && !mDizzy) {
    mDizzy =
        mgr.GetPlayer()->GetAttachedActorStruggle() == 1.f && !BallCloseToCollision(mgr);
  }
}

void CTryclops::TargetCover(CStateManager& mgr, EStateMsg msg, float) {
  switch (msg) {
  case kStateMsg_Activate:
    BodyCtrl()->SetLocomotionType(pas::kLT_Relaxed);
    if (mBombId != kInvalidUniqueId) {
      if (const CBomb* bomb = TCastToConstPtr< CBomb >(mgr.GetObjectById(mBombId))) {
        SetDestPos(bomb->GetTranslation());
      } else {
        mBombId = kInvalidUniqueId;
      }
    }
    break;
  }
}

bool CTryclops::InAttackPosition(CStateManager& mgr, float) {
  const CPlayer& player = *mgr.GetPlayer();
  if (player.GetMorphballTransitionState() == CPlayer::kMS_Morphed) {
    const CVector3f pos = player.GetTranslation();
    const CVector3f center =
        player.GetTranslation() + CVector3f(0.f, 0.f, player.GetMorphBall()->GetBallRadius());
    const CAABox bounds = player.GetBoundingBox();
    return ObjectInVortexArea(pos, center, bounds, mgr);
  }
  return false;
}

bool CTryclops::InRange(CStateManager& mgr, float) {
  if (mBombId != kInvalidUniqueId) {
    if (const CBomb* bomb = TCastToConstPtr< CBomb >(mgr.GetObjectById(mBombId))) {
      const CVector3f pos = bomb->GetTranslation();
      return ObjectInVortexArea(pos, pos, *bomb->GetTouchBounds(), mgr);
    }
  }
  return false;
}

bool CTryclops::ObjectInVortexArea(const CVector3f& pos, const CVector3f& center,
                                   const CAABox& bounds, const CStateManager& mgr) {
  const CAABox ownBounds = GetBoundingBox();
  if (bounds.DoBoundsOverlap(ownBounds)) {
    return true;
  }
  const CTransform4f xf = GetLctrTransform(rstl::string_l(kMouthLctr));
  const CVector3f forward = GetTransform().GetForward();
  const CVector3f delta = center - (xf.GetTranslation() - 1.f * forward);
  const CVector3f angleDelta = pos - (xf.GetTranslation() - 4.f * forward);
  const float projection = CVector3f::Dot(delta.AsNormalized(), forward);
  const float angleProjection = CVector3f::Dot(angleDelta.AsNormalized(), forward);
  const float distance = delta.Magnitude();
  static const CMaterialFilter kSolidFilter = CMaterialFilter::MakeIncludeExclude(
      CMaterialList(kMT_Solid),
      CMaterialList(kMT_Character, kMT_Player, kMT_ProjectilePassthrough));
  if (distance > 2.f) {
    const CRayCastResult result = mgr.RayStaticIntersection(
        xf.GetTranslation(), (1.f / distance) * delta,
        distance - mgr.GetPlayer()->GetMorphBall()->GetBallRadius(), kSolidFilter);
    if (result.IsValid()) {
      return false;
    }
  }
  if (distance < mSuckRange) {
    return projection > 0.f && angleProjection > mMinSuckAngleProj;
  }
  return false;
}

bool CTryclops::TargetCaught(const CVector3f& pos, float range) {
  const CTransform4f xf = GetLctrTransform(rstl::string_l(kMouthLctr));
  const CVector3f delta = pos - xf.GetTranslation();
  return delta.MagSquared() <= range;
}

bool CTryclops::Inside(CStateManager& mgr, float arg) {
  const CPlayer& player = *mgr.GetPlayer();
  const CTransform4f& xf = player.GetTransform();
  const CVector3f pos =
      xf.GetTranslation() + CVector3f(0.f, 0.f, player.GetMorphBall()->GetBallRadius());
  mPlayerRotation = xf.GetRotation();
  return TargetCaught(pos, arg);
}

bool CTryclops::InPosition(CStateManager& mgr, float arg) {
  if (mBombId != kInvalidUniqueId) {
    if (const CBomb* bomb = TCastToConstPtr< CBomb >(mgr.GetObjectById(mBombId))) {
      return TargetCaught(bomb->GetTranslation(), arg);
    }
  }
  return false;
}

bool CTryclops::HearShot(CStateManager& mgr, float) {
  mVulnerable = false;
  if (mBombId != kInvalidUniqueId) {
    if (TCastToConstPtr< CBomb >(mgr.GetObjectById(mBombId))) {
      mVulnerable = true;
      return false;
    }
    mBombId = kInvalidUniqueId;
    return true;
  }
  return true;
}

void CTryclops::Suck(CStateManager& mgr, EStateMsg msg, float dt) {
  CPlayer& player = *mgr.Player();
  switch (msg) {
  case kStateMsg_Activate:
    player.EnableLeaveMorphBall(false);
    player.MorphBall()->DisableHalfPipeStatus();
    BodyCtrl()->SetLocomotionType(pas::kLT_Internal6);
    break;
  case kStateMsg_Update:
    AttractPlayer(mgr, dt);
    break;
  case kStateMsg_Deactivate:
    player.EnableLeaveMorphBall(true);
    player.AddMaterial(kMT_Solid, mgr);
    break;
  }
}

void CTryclops::SelectTarget(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Deactivate:
    break;
  case kStateMsg_Activate: {
    CPlayer& player = *mgr.Player();
    player.EnableLeaveMorphBall(true);
    player.AddMaterial(kMT_Solid, mgr);
    mTargetingBomb = true;
    BodyCtrl()->SetLocomotionType(pas::kLT_Internal6);
    break;
  }
  case kStateMsg_Update:
    AttractBomb(mgr, dt);
    break;
  }
}

void CTryclops::AttractPlayer(CStateManager& mgr, float dt) {
  CPlayer& player = *mgr.Player();
  const CTransform4f xf = GetLctrTransform(rstl::string_l(kMouthLctr));
  const CVector3f delta = player.GetTranslation() - xf.GetTranslation();
  if (player.GetMorphballTransitionState() == CPlayer::kMS_Morphed) {
    const float distance = delta.Magnitude();
    if (distance < 3.f) {
      player.Stop();
      CenterPlayer(mgr, xf.GetTranslation(), dt);
    } else {
      const float force = mSuckForceMultiplier * (mSuckRange / (distance * distance));
      const CVector3f massDirection = player.GetMass() * -delta;
      const CVector3f forceVector = force * massDirection;
      player.ApplyForceWR(forceVector, CAxisAngle::Identity());
    }
  }
}

void CTryclops::AttractBomb(CStateManager& mgr, float dt) {
  if (CBomb* bomb = TCastToPtr< CBomb >(mgr.ObjectById(mBombId))) {
    const CTransform4f xf = GetLctrTransform(rstl::string_l(kMouthLctr));
    const CVector3f target = xf.GetTranslation() + kBombPosOffset;
    const CVector3f position = bomb->GetTranslation();
    const CVector3f direction = (target - position).AsNormalized();
    bomb->SetVelocityWR((1.f / (2.f * dt)) * direction);
  }
}

void CTryclops::CenterPlayer(CStateManager& mgr, const CVector3f& pos, float dt) {
  CPlayer& player = *mgr.Player();
  const float ballRadius = player.GetMorphBall()->GetBallRadius();
  const CVector3f center = player.GetTranslation() + CVector3f(0.f, 0.f, ballRadius);
  const CVector3f direction = (pos - center).AsNormalized();
  const CVector3f velocity = (1.f / (2.f * dt)) * direction;
  player.SetVelocityWR(velocity);
}

void CTryclops::Crouch(CStateManager& mgr, EStateMsg msg, float) {
  CPlayer* const player = mgr.Player();
  switch (msg) {
  case kStateMsg_Activate: {
    const TUniqueId waypointId = GetConnectedObject(mgr, kSS_Retreat, kSM_Follow);
    if (const CActor* waypoint = TCastToConstPtr< CActor >(mgr.GetObjectById(waypointId))) {
      SetDestPos(waypoint->GetTranslation());
    }
    CPlayer& currentPlayer = *mgr.Player();
    currentPlayer.Stop();
    currentPlayer.RemoveMaterial(kMT_Solid, mgr);
    SendScriptMsgs(kSS_Inside, mgr, kSM_None);
    mgr.Player()->AttachActorToPlayer(GetUniqueId(), true);
    player->EnableLeaveMorphBall(false);
    player->MorphBall()->DisableHalfPipeStatus();
    BodyCtrl()->SetLocomotionType(pas::kLT_Combat);
    break;
  }
  case kStateMsg_Update: {
    const CTransform4f xf = GetLctrTransform(rstl::string_l(kMouthLctr));
    SetPlayerPosition(mgr, xf.GetTranslation());
    break;
  }
  case kStateMsg_Deactivate: {
    if (player->GetAttachedActor() == GetUniqueId()) {
      player->DetachActorFromPlayer();
    }
    CPlayer& currentPlayer = *mgr.Player();
    currentPlayer.EnableLeaveMorphBall(true);
    currentPlayer.AddMaterial(kMT_Solid, mgr);
    break;
  }
  }
}

void CTryclops::JumpBack(CStateManager& mgr, EStateMsg msg, float) {
  switch (msg) {
  case kStateMsg_Deactivate:
    break;
  case kStateMsg_Activate: {
    const TUniqueId waypointId = GetConnectedObject(mgr, kSS_Retreat, kSM_Follow);
    if (const CActor* waypoint = TCastToConstPtr< CActor >(mgr.GetObjectById(waypointId))) {
      SetDestPos(waypoint->GetTranslation());
    }
    if (mBombId != kInvalidUniqueId) {
      if (CBomb* bomb = TCastToPtr< CBomb >(mgr.ObjectById(mBombId))) {
        bomb->SetFuseDisabled(false);
      } else {
        mBombId = kInvalidUniqueId;
      }
    }
    SendScriptMsgs(kSS_Inside, mgr, kSM_None);
    BodyCtrl()->SetLocomotionType(pas::kLT_Combat);
    break;
  }
  case kStateMsg_Update:
    SetBombPosition(mgr);
    break;
  }
}

void CTryclops::PathFind(CStateManager& mgr, EStateMsg msg, float dt) {
  CPatterned::PathFind(mgr, msg, dt);
  const CVector3f forward = GetTransform().GetForward();
  const CVector3f move = GetBodyCtrl()->GetCommandMgr().GetMoveVector();
  if (CVector3f::Dot(forward, move) < 0.f && move.CanBeNormalized()) {
    BodyCtrl()->CommandMgr().ClearLocomotionCmds();
    BodyCtrl()->CommandMgr().DeliverCmd(
        CBCLocomotionCmd(CVector3f::Zero(), move.AsNormalized(), 1.f));
  }
  ApplySeparationBehavior(mgr);
}

void CTryclops::Shuffle(CStateManager& mgr, EStateMsg msg, float dt) { PathFind(mgr, msg, dt); }

void CTryclops::Approach(CStateManager& mgr, EStateMsg msg, float dt) {
  CPatterned::PathFind(mgr, msg, dt);
  ApplySeparationBehavior(mgr);
  switch (msg) {
  case kStateMsg_Activate:
  case kStateMsg_Deactivate:
    break;
  case kStateMsg_Update:
    SetBombPosition(mgr);
    break;
  }
}

void CTryclops::PathFindEx(CStateManager& mgr, EStateMsg msg, float dt) {
  CPatterned::PathFind(mgr, msg, dt);
  ApplySeparationBehavior(mgr);
  CPlayer& player = *mgr.Player();
  switch (msg) {
  case kStateMsg_Deactivate:
    break;
  case kStateMsg_Activate:
    player.Stop();
    player.RemoveMaterial(kMT_Solid, mgr);
    player.EnableLeaveMorphBall(false);
    player.MorphBall()->DisableHalfPipeStatus();
    mgr.Player()->AttachActorToPlayer(GetUniqueId(), true);
    break;
  case kStateMsg_Update: {
    const CTransform4f xf = GetLctrTransform(rstl::string_l(kMouthLctr));
    SetPlayerPosition(mgr, xf.GetTranslation());
    break;
  }
  }
}

void CTryclops::SetPlayerPosition(CStateManager& mgr, const CVector3f&) {
  CPlayer& player = *mgr.Player();
  player.Stop();
  player.RemoveMaterial(kMT_Solid, mgr);
  CTransform4f xf = GetLctrTransform(rstl::string_l(kMouthLctr)) * mPlayerRotation;
  xf.AddTranslation(CVector3f(0.f, 0.f, -0.5f));
  player.SetTransform(xf);
}

void CTryclops::SetBombPosition(CStateManager& mgr) {
  if (CBomb* bomb = TCastToPtr< CBomb >(mgr.ObjectById(mBombId))) {
    CTransform4f xf = GetLctrTransform(rstl::string_l(kMouthLctr));
    xf.AddTranslation(kBombPosOffset);
    bomb->SetTransform(xf);
  }
}

void CTryclops::Attack(CStateManager& mgr, EStateMsg msg, float) {
  CPlayer& player = *mgr.Player();
  switch (msg) {
  case kStateMsg_Activate:
    player.Stop();
    player.RemoveMaterial(kMT_Solid, mgr);
    mgr.Player()->EnableLeaveMorphBall(false);
    mAnimState = kAS_Ready;
    mShotTarget = false;
    break;
  case kStateMsg_Update:
    TryCommand(mgr, pas::kAS_MeleeAttack, &CPatterned::TryMeleeAttack, 1);
    if (!mShotTarget) {
      const CTransform4f xf = GetLctrTransform(rstl::string_l(kMouthLctr));
      SetPlayerPosition(mgr, xf.GetTranslation());
    }
    break;
  case kStateMsg_Deactivate: {
    mAnimState = kAS_NotReady;
    if (mgr.GetPlayer()->GetAttachedActor() == GetUniqueId()) {
      player.DetachActorFromPlayer();
    }
    CPlayer& currentPlayer = *mgr.Player();
    currentPlayer.EnableLeaveMorphBall(true);
    currentPlayer.AddMaterial(kMT_Solid, mgr);
    break;
  }
  }
}

void CTryclops::GetUp(CStateManager& mgr, EStateMsg msg, float) {
  switch (msg) {
  case kStateMsg_Activate:
    mAnimState = kAS_Ready;
    mShotTarget = false;
    break;
  case kStateMsg_Update:
    TryCommand(mgr, pas::kAS_MeleeAttack, &CPatterned::TryMeleeAttack, 1);
    if (!mShotTarget) {
      SetBombPosition(mgr);
    }
    break;
  case kStateMsg_Deactivate:
    mAnimState = kAS_NotReady;
    break;
  }
}

void CTryclops::ShootPlayer(CStateManager& mgr, const CTransform4f& xf, float speed) {
  CPlayer& player = *mgr.Player();
  const CVector3f direction = xf.GetForward().AsNormalized();
  player.EnableLeaveMorphBall(true);
  if (player.GetMorphballTransitionState() == CPlayer::kMS_Morphed) {
    mShotTarget = true;
    mIgnoreMorphballTimer = 1.5f;
    player.Stop();
    CTransform4f playerXf = xf * mPlayerRotation;
    playerXf.AddTranslation(CVector3f(0.f, 0.f, -0.5f));
    player.Teleport(playerXf, mgr, false);
    player.ApplyImpulseWR(speed * (player.GetMass() * direction), CAxisAngle::Identity());
    player.SetMoveState(NPlayer::kMS_ApplyJump, mgr);
    player.AddMaterial(kMT_Solid, mgr);
    mgr.ApplyDamage(
        GetUniqueId(), player.GetUniqueId(), GetUniqueId(), GetContactDamage(),
        CMaterialFilter::MakeIncludeExclude(CMaterialList(SolidMaterial), CMaterialList()),
        CVector3f::Zero());
  }
}

void CTryclops::ShootBomb(CStateManager& mgr, const CTransform4f& xf) {
  if (mBombId != kInvalidUniqueId) {
    const CVector3f direction = xf.GetForward().AsNormalized();
    if (CBomb* bomb = TCastToPtr< CBomb >(mgr.ObjectById(mBombId))) {
      bomb->SetVelocityWR((mgr.Random()->Float() * 5.f + 20.f) * direction);
      bomb->SetConstantAccelerationWR(CVector3f(0.f, 0.f, -CPhysicsActor::GravityConstant()));
    }
  }
  mVulnerable = false;
  mShotTarget = true;
  mBombId = kInvalidUniqueId;
}

bool CTryclops::SpotPlayer(CStateManager& mgr, float) {
  const CPlayer& player = *mgr.GetPlayer();
  if (mBombId != kInvalidUniqueId) {
    if (CBomb* bomb = TCastToPtr< CBomb >(mgr.ObjectById(mBombId))) {
      if (player.GetMorphballTransitionState() == CPlayer::kMS_Morphed) {
        const CVector3f playerDelta = player.GetTranslation() - GetTranslation();
        const CVector3f bombDelta = bomb->GetTranslation() - GetTranslation();
        const bool closer = playerDelta.MagSquared() < bombDelta.MagSquared();
        if (closer) {
          bomb->SetFuseDisabled(false);
          bomb->SetIsBeingDragged(false);
          mBombId = kInvalidUniqueId;
        }
        return closer;
      }
    }
  }
  return true;
}

void CTryclops::TurnAround(CStateManager& mgr, EStateMsg msg, float) {
  CPlayer& player = *mgr.Player();
  switch (msg) {
  case kStateMsg_Activate: {
    if (mBombId == kInvalidUniqueId) {
      CPlayer& activePlayer = *mgr.Player();
      activePlayer.Stop();
      activePlayer.RemoveMaterial(kMT_Solid, mgr);
      mgr.Player()->EnableLeaveMorphBall(false);
    }
    TUniqueId waypointId = GetConnectedObject(mgr, kSS_Modify, kSM_Follow);
    bool retreat = false;
    if (waypointId == kInvalidUniqueId) {
      waypointId = GetConnectedObject(mgr, kSS_Retreat, kSM_Follow);
      retreat = true;
    }
    if (const CActor* waypoint = TCastToConstPtr< CActor >(mgr.GetObjectById(waypointId))) {
      CVector3f direction = retreat ? waypoint->GetTransform().GetForward()
                                    : waypoint->GetTranslation() - GetTranslation();
      CVector3f forward = GetTransform().GetForward();
      forward.SetZ(0.f);
      direction.Normalize();
      direction.SetZ(0.f);
      SetDestPos(GetTranslation() + direction);
      if (CMath::AbsF(CVector3f::Dot(forward, direction)) < 0.9998f) {
        mAnimState = kAS_Ready;
      }
    }
    break;
  }
  case kStateMsg_Update:
    TryCommand(mgr, pas::kAS_Turn, &CPatterned::TryTurn, 0);
    if (mBombId == kInvalidUniqueId) {
      const CTransform4f xf = GetLctrTransform(rstl::string_l(kMouthLctr));
      SetPlayerPosition(mgr, xf.GetTranslation());
    } else {
      SetBombPosition(mgr);
    }
    break;
  case kStateMsg_Deactivate: {
    mAnimState = kAS_NotReady;
    CPlayer& currentPlayer = *mgr.Player();
    currentPlayer.EnableLeaveMorphBall(true);
    currentPlayer.AddMaterial(kMT_Solid, mgr);
    if (mgr.GetPlayer()->GetAttachedActor() == GetUniqueId()) {
      player.DetachActorFromPlayer();
    }
    break;
  }
  }
}

void CTryclops::DoUserAnimEvent(CStateManager& mgr, const CInt32POINode& node, EUserEventType type,
                                float dt) {
  bool handled = false;
  switch (type) {
  case kUE_Projectile:
    if (mBombId == kInvalidUniqueId) {
      ShootPlayer(mgr, GetLctrTransform(node.GetLocatorName()),
                  mDizzy ? 5.f : mLaunchSpeed);
    } else {
      ShootBomb(mgr, GetLctrTransform(node.GetLocatorName()));
    }
    handled = true;
    break;
  }
  if (!handled) {
    CPatterned::DoUserAnimEvent(mgr, node, type, dt);
  }
}

const CDamageVulnerability* CTryclops::GetDamageVulnerability() const {
  if (mVulnerable) {
    return CAi::GetDamageVulnerability();
  }
  return &skPowerBombVulnerability;
}

const CDamageVulnerability* CTryclops::GetDamageVulnerability(const CVector3f&, const CVector3f&,
                                                              const CDamageInfo&) const {
  if (mVulnerable) {
    return CAi::GetDamageVulnerability();
  }
  return &skPowerBombVulnerability;
}

void CTryclops::Death(CStateManager& mgr, const CVector3f& direction, EScriptObjectState state) {
  CPlayer& player = *mgr.Player();
  if (mAlive) {
    if (player.GetAttachedActor() == GetUniqueId()) {
      player.EnableLeaveMorphBall(true);
      player.AddMaterial(kMT_Solid, mgr);
      player.DetachActorFromPlayer();
    } else if (mBombId != kInvalidUniqueId) {
      if (CBomb* bomb = TCastToPtr< CBomb >(mgr.ObjectById(mBombId))) {
        bomb->SetFuseDisabled(false);
        bomb->SetIsBeingDragged(false);
      }
      mBombId = kInvalidUniqueId;
    }
  }
  CPatterned::Death(mgr, direction, state);
}

void CTryclops::TargetPatrol(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Update:
  case kStateMsg_Deactivate:
    break;
  case kStateMsg_Activate:
    BodyCtrl()->SetLocomotionType(pas::kLT_Relaxed);
    break;
  }
  CPatterned::TargetPatrol(mgr, msg, dt);
}

void CTryclops::Cover(CStateManager& mgr, EStateMsg msg, float) {
  switch (msg) {
  case kStateMsg_Activate:
    if (!mTargetingBomb) {
      mIgnoreMorphballTimer = 1.5f;
    }
    CPlayer& player = *mgr.Player();
    if (player.GetAttachedActor() == GetUniqueId()) {
      player.EnableLeaveMorphBall(true);
      player.AddMaterial(kMT_Solid, mgr);
      player.DetachActorFromPlayer();
    }
    break;
  }
}

bool CTryclops::CoverBlown(CStateManager&, float) {
  switch (mPathFindSearch.OnPath(GetTranslation())) {
  case CPathFindSearch::kR_InvalidArea:
    return false;
  default:
    return true;
  }
}

bool CTryclops::IsDizzy(CStateManager&, float) { return mDizzy; }

bool CTryclops::ShouldRetreat(CStateManager& mgr, float) {
  const TUniqueId waypointId = GetConnectedObject(mgr, kSS_Modify, kSM_Next);
  if (const CActor* waypoint = TCastToConstPtr< CActor >(mgr.GetObjectById(waypointId))) {
    SetDestPos(waypoint->GetTranslation());
    return true;
  }
  return false;
}

void CTryclops::ApplySeparationBehavior(CStateManager& mgr) {
  const CObjectList& list = mgr.GetObjectListById(kOL_ListeningAi);
  for (int i = list.GetFirstObjectIndex(); i != -1; i = list.GetNextObjectIndex(i)) {
    if (const CPatterned* ai = TCastToConstPtr< CPatterned >(list[i])) {
      if (ai != this && ai->GetCurrentAreaId() == GetCurrentAreaId()) {
        const CVector3f separation =
            mSteeringBehaviors.Separation(*this, ai->GetTranslation(), 8.f);
        if (separation.IsNonZero()) {
          BodyCtrl()->CommandMgr().DeliverCmd(CBCLocomotionCmd(separation, CVector3f::Zero(), 1.f));
        }
      }
    }
  }
}

void CTryclops::Patrol(CStateManager& mgr, EStateMsg msg, float dt) {
  CPatterned::Patrol(mgr, msg, dt);
  switch (msg) {
  case kStateMsg_Deactivate:
    break;
  case kStateMsg_Activate:
    BodyCtrl()->SetLocomotionType(pas::kLT_Relaxed);
    break;
  case kStateMsg_Update:
    ApplySeparationBehavior(mgr);
    break;
  }
}

void CTryclops::TargetPlayer(CStateManager& mgr, EStateMsg msg, float dt) {
  CPatterned::TargetPlayer(mgr, msg, dt);
  switch (msg) {
  case kStateMsg_Activate:
    if (mBombId != kInvalidUniqueId) {
      if (CBomb* bomb = TCastToPtr< CBomb >(mgr.ObjectById(mBombId))) {
        bomb->SetFuseDisabled(false);
        bomb->SetIsBeingDragged(false);
        mBombId = kInvalidUniqueId;
      }
    }
    break;
  }
}

void CTryclops::Dizzy(CStateManager& mgr, EStateMsg msg, float) {
  CPlayer& player = *mgr.Player();
  switch (msg) {
  case kStateMsg_Activate:
    player.Stop();
    player.RemoveMaterial(kMT_Solid, mgr);
    mgr.Player()->EnableLeaveMorphBall(false);
    mAnimState = kAS_Ready;
    mShotTarget = false;
    break;
  case kStateMsg_Update:
    TryCommand(mgr, pas::kAS_MeleeAttack, &CPatterned::TryMeleeAttack, 0);
    if (!mShotTarget) {
      const CTransform4f xf = GetLctrTransform(rstl::string_l(kMouthLctr));
      SetPlayerPosition(mgr, xf.GetTranslation());
    }
    break;
  case kStateMsg_Deactivate: {
    mAnimState = kAS_NotReady;
    CPlayer& currentPlayer = *mgr.Player();
    if (currentPlayer.GetAttachedActor() == GetUniqueId()) {
      currentPlayer.EnableLeaveMorphBall(true);
      currentPlayer.AddMaterial(kMT_Solid, mgr);
      player.DetachActorFromPlayer();
      mDizzy = false;
    }
    break;
  }
  }
}

bool CTryclops::BallCloseToCollision(const CStateManager& mgr) const {
  const CVector3f rayStart = GetLctrTransform(rstl::string_l(skRootLocator)).GetTranslation();
  const CRayCastResult ray = mgr.RayStaticIntersection(rayStart, GetTransform().GetForward(), 3.f,
                                                       CMaterialFilter::GetPassEverything());
  if (ray.IsValid()) {
    return true;
  }
  const CPlayer& player = *mgr.GetPlayer();
  const float radius = player.GetMorphBall()->GetBallRadius();
  const CMaterialFilter filter = CMaterialFilter::MakeIncludeExclude(
      CMaterialList(kMT_Solid), CMaterialList(kMT_ProjectilePassthrough));
  const CCollidableSphere sphere(
      CSphere(player.GetTranslation() + CVector3f(0.f, 0.f, radius), radius),
      CMaterialList(kMT_Player, kMT_Solid));
  TEntityList nearList;
  mgr.BuildColliderList(nearList, player, sphere.CalculateLocalAABox());
  if (CGameCollision::DetectStaticCollisionBoolean(mgr, sphere, CTransform4f::Identity(), filter)) {
    return true;
  }
  for (AUTO(it, nearList.begin()); it != nearList.end(); ++it) {
    const TUniqueId& id = *it;
    if (id != GetUniqueId()) {
      if (const CPhysicsActor* const actor =
              TCastToConstPtr< CPhysicsActor >(mgr.GetObjectById(id))) {
        if (CCollisionPrimitive::CollideBoolean(
                CInternalCollisionStructure::CPrimDesc(sphere, filter, CTransform4f::Identity()),
                CInternalCollisionStructure::CPrimDesc(*actor->GetCollisionPrimitive(),
                                                       CMaterialFilter::GetPassEverything(),
                                                       actor->GetPrimitiveTransform()))) {
          return true;
        }
      }
    }
  }
  return false;
}
