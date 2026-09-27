#include "MetroidPrime/BodyState/CBSLocomotion.hpp"

#include "Kyoto/Math/CVector3f.hpp"
#include "MetroidPrime/BodyState/CBodyController.hpp"
#include "MetroidPrime/BodyState/CBodyStateCmdMgr.hpp"
#include "MetroidPrime/CAnimPlaybackParms.hpp"
#include "MetroidPrime/CPhysicsActor.hpp"
#include "MetroidPrime/TCastTo.hpp"

#include "Kyoto/Animation/CPASAnimParmData.hpp"
#include "Kyoto/Math/CRelAngle.hpp"
#include "Kyoto/Math/CloseEnough.hpp"

#include "rstl/math.hpp"

static float skMaxPitchAngle = CRelAngle::FromDegrees(10.f).AsRadians();

const float CBSBiPedLocomotion::skMinWalkPercent = 0.5f;
static int skInvalidAnimId = -1;

bool CBSFlyerLocomotion::IsPitchable() const { return mPitchable; }

bool CBSFlyerLocomotion::IsBackPedal(CBodyController& bc) const {
  (void)bc;
  return false;
}

CBSFlyerLocomotion::~CBSFlyerLocomotion() {}

CBSWallWalkerLocomotion::~CBSWallWalkerLocomotion() {}

bool CBSBiPedLocomotion::IsMoving() const { return mAnim != pas::kLA_Idle; }

CBSAiMovedFlyerLocomotion::~CBSAiMovedFlyerLocomotion() {}

CBSLocomotion::CBSLocomotion() : mLocomotionType(pas::kLT_Invalid) {}

void CBSLocomotion::Start(CBodyController& bc, CStateManager& mgr) {
  mLocomotionType = bc.GetLocomotionType();

  if (bc.CommandMgr().GetCmd(kBSC_MaintainVelocity)) {
    ReStartBodyState(bc, true);
  } else {
    ReStartBodyState(bc, false);
  }

  (void)mgr;
}

pas::EAnimationState CBSLocomotion::UpdateBody(float dt, CBodyController& bc, CStateManager& mgr) {
  const pas::EAnimationState st = GetBodyStateTransition(dt, bc);
  if (st == pas::kAS_Invalid) {
    const float velMag = ApplyLocomotionPhysics(dt, bc);
    UpdateLocomotionAnimation(dt, velMag, bc, false);
  }

  (void)mgr;
  return st;
}

void CBSLocomotion::Shutdown(CBodyController& bc) { bc.MultiplyPlaybackRate(1.f); }

float CBSLocomotion::ApplyLocomotionPhysics(float dt, CBodyController& bc) {
  if (const CPhysicsActor* act = TCastToConstPtr< CPhysicsActor >(&bc.GetOwner())) {
    const CBodyStateCmdMgr& cmdMgr = bc.GetCommandMgr();
    const CVector3f& moveVec = cmdMgr.GetMoveVector();
    const CVector3f& faceVec = cmdMgr.GetFaceVector();
    const CVector3f vec =
        close_enough(faceVec, CVector3f::Zero(), vector3_epsilon()) ? moveVec : faceVec;

    if (vec.CanBeNormalized()) {
      if (IsPitchable()) {
        CVector3f lookForward = act->GetTransform().GetForward();
        CVector3f lookVec = lookForward;
        lookVec[kDZ] = 0.f;
        lookVec.Normalize();

        CVector3f tmp = vec;
        tmp[kDZ] = 0.f;
        bc.FaceDirection3D(tmp, lookVec, dt);

        CVector3f lookVec2 = lookForward;
        lookVec2[kDZ] = vec.GetZ();
        lookVec2.Normalize();
        if (!close_enough(lookVec, lookVec2, vector3_epsilon())) {
          CRelAngle pitchAngle(rstl::min_val(CVector3f::GetAngleDiff(vec, tmp),
                                             bc.GetBodyStateInfo().GetMaximumPitch()));
          lookVec2 = CVector3f::Slerp(lookVec, lookVec2, pitchAngle);
        }

        bc.FaceDirection3D(lookVec2, lookForward, dt);

        CVector3f right = act->GetTransform().GetRight();
        CVector3f lookVec3 = right;
        lookVec3[kDZ] = 0.f;
        bc.FaceDirection3D(lookVec3, right, dt);
      } else {
        bc.FaceDirection(vec.AsNormalized(), dt);
      }
    }

    return rstl::min_val(moveVec.Magnitude(), 1.f);
  }

  return 0.f;
}

void CBSLocomotion::ReStartBodyState(CBodyController& bc, bool maintainVel) {
  UpdateLocomotionAnimation(0.f, maintainVel ? GetStartVelocityMagnitude(bc) : 0.f, bc, true);
}

float CBSLocomotion::GetStartVelocityMagnitude(CBodyController& bc) const {
  if (const CPhysicsActor* act = TCastToConstPtr< CPhysicsActor >(&bc.GetOwner())) {
    const float velocityMag = act->GetVelocityWR().Magnitude();
    const float maxSpeed = bc.GetBodyStateInfo().GetMaxSpeed();
    float ret = maxSpeed > 0.f ? velocityMag / maxSpeed : 0.f;
    ret = rstl::min_val(ret, 1.f);
    return ret;
  }
  return 0.f;
}

float CBSLocomotion::ComputeWeightPercentage(const rstl::pair< int, float >& a,
                                             const rstl::pair< int, float >& b, float f) const {
  const float range = b.second - a.second;
  if (range > FLT_EPSILON) {
    return rstl::max_val(rstl::min_val((f - a.second) / range, 1.f), 0.f);
  }
  return 0.f;
}

pas::EAnimationState CBSLocomotion::GetBodyStateTransition(float dt, CBodyController& bc) {
  CBodyStateCmdMgr& cmdMgr = bc.CommandMgr();
  if (cmdMgr.GetCmd(kBSC_Hurled)) {
    return pas::kAS_Hurled;
  }
  if (cmdMgr.GetCmd(kBSC_KnockDown)) {
    return pas::kAS_Fall;
  }
  if (cmdMgr.GetCmd(kBSC_LoopHitReaction)) {
    return pas::kAS_LoopReaction;
  }
  if (cmdMgr.GetCmd(kBSC_KnockBack)) {
    return pas::kAS_KnockBack;
  }
  if (cmdMgr.GetCmd(kBSC_Locomotion)) {
    cmdMgr.ClearLocomotionCmds();
  } else {
    if (cmdMgr.GetCmd(kBSC_Slide)) {
      return pas::kAS_Slide;
    }
    if (cmdMgr.GetCmd(kBSC_Generate)) {
      return pas::kAS_Generate;
    }
    if (cmdMgr.GetCmd(kBSC_MeleeAttack)) {
      return pas::kAS_MeleeAttack;
    }
    if (cmdMgr.GetCmd(kBSC_ProjectileAttack)) {
      return pas::kAS_ProjectileAttack;
    }
    if (cmdMgr.GetCmd(kBSC_LoopAttack)) {
      return pas::kAS_LoopAttack;
    }
    if (cmdMgr.GetCmd(kBSC_LoopReaction)) {
      return pas::kAS_LoopReaction;
    }
    if (cmdMgr.GetCmd(kBSC_Jump)) {
      return pas::kAS_Jump;
    }
    if (cmdMgr.GetCmd(kBSC_Taunt)) {
      return pas::kAS_Taunt;
    }
    if (cmdMgr.GetCmd(kBSC_Step)) {
      return pas::kAS_Step;
    }
    if (cmdMgr.GetCmd(kBSC_Cover)) {
      return pas::kAS_Cover;
    }
    if (cmdMgr.GetCmd(kBSC_WallHang)) {
      return pas::kAS_WallHang;
    }
    if (cmdMgr.GetCmd(kBSC_Scripted)) {
      return pas::kAS_Scripted;
    }
    if (!cmdMgr.GetMoveVector().IsNonZero()) {
      if (cmdMgr.GetFaceVector().IsNonZero()) {
        if (!IsMoving()) {
          return pas::kAS_Turn;
        }
      }
    }
    if (mLocomotionType != bc.GetLocomotionType()) {
      return pas::kAS_Locomotion;
    }
  }

  (void)dt;
  return pas::kAS_Invalid;
}

CBSBiPedLocomotion::CBSBiPedLocomotion(CActor& actor)
: mAnims(14, rstl::reserved_vector< rstl::pair< int, float >, 8 >(
                   8, rstl::pair< int, float >(0, 0.f)))
, mAnim(pas::kLA_Invalid) {
  const CPASDatabase& pasDatabase = actor.GetAnimationData()->GetCharacterInfo().GetPASDatabase();
  for (int i = 0; i < 14; ++i) {
    for (int j = 0; j < 8; ++j) {
      const CPASAnimParmData parms(pas::kAS_Locomotion, CPASAnimParm::FromEnum(j),
                                   CPASAnimParm::FromEnum(i));
      rstl::pair< float, int > best = pasDatabase.FindBestAnimation(parms, -1);
      float avgVel = 0.f;
      if (best.second != -1) {
        avgVel = actor.GetAverageAnimVelocity(best.second);
        avgVel = j != 0 ? avgVel : 0.f;
      }
      mAnims[static_cast< pas::ELocomotionType >(i)][static_cast< pas::ELocomotionAnim >(j)] =
          rstl::pair< int, float >(best.second, avgVel);
    }
  }
}

float CBSBiPedLocomotion::GetLocomotionSpeed(pas::ELocomotionType type,
                                             pas::ELocomotionAnim anim) const {
  return GetLocoAnimation(type, anim).second;
}

void CBSBiPedLocomotion::Start(CBodyController& bc, CStateManager& mgr) {
  mPrimeTime = 0.f;
  CBSLocomotion::Start(bc, mgr);
}

pas::EAnimationState CBSBiPedLocomotion::UpdateBody(float dt, CBodyController& bc,
                                                    CStateManager& mgr) {
  if (mPrimeTime < 0.2f) {
    mPrimeTime += dt;
  }
  return CBSLocomotion::UpdateBody(dt, bc, mgr);
}

float CBSBiPedLocomotion::UpdateLocomotionAnimation(float dt, float velMag, CBodyController& bc,
                                                    bool init) {
  float ret = 1.f;

  if (init || mPrimeTime >= 0.2f) {
    const pas::ELocomotionAnim anim = init ? pas::kLA_Invalid : mAnim;
    const float maxSpeed = velMag * GetLocomotionSpeed(mLocomotionType, pas::kLA_Run);
    if (IsStrafing(bc) && velMag >= 0.01f) {
      ret = UpdateStrafe(velMag, bc, anim);
    } else if (maxSpeed < 0.01f) {
      if (anim != pas::kLA_Idle || init) {
        if (!bc.GetBodyStateInfo().GetLocoAnimChangeAtEndOfAnimOnly() ||
            bc.GetAnimTimeRemaining() <= dt || init) {
          const rstl::pair< int, float >& best = GetLocoAnimation(mLocomotionType, pas::kLA_Idle);
          if (bc.GetCurrentAnimId() != best.first) {
            const CAnimPlaybackParms playParms(best.first, -1, 1.f, true);
            bc.SetCurrentAnimation(playParms, true, false);
            mPrimeTime = 0.f;
          }
          mAnim = pas::kLA_Idle;
        }
      }
    } else {
      const rstl::pair< int, float >& best = GetLocoAnimation(mLocomotionType, pas::kLA_Walk);
      if (maxSpeed < best.second) {
        ret = UpdateWalk(maxSpeed, bc, anim);
      } else {
        ret = UpdateRun(maxSpeed, bc, anim);
      }
    }
  }

  return ret;
}

const rstl::pair< int, float >&
CBSBiPedLocomotion::GetLocoAnimation(pas::ELocomotionType type, pas::ELocomotionAnim anim) const {
  return mAnims[static_cast< int >(type)][static_cast< int >(anim)];
}

bool CBSBiPedLocomotion::IsStrafing(CBodyController& bc) const {
  const CBodyStateCmdMgr& cmdMgr = bc.GetCommandMgr();
  const CVector3f& moveVec = cmdMgr.GetMoveVector();
  const CVector3f& faceVec = cmdMgr.GetFaceVector();
  return !close_enough(moveVec, CVector3f::Zero()) && !close_enough(faceVec, CVector3f::Zero());
}

float CBSBiPedLocomotion::UpdateStrafe(float vel, CBodyController& bc, pas::ELocomotionAnim anim) {
  static pas::ELocomotionAnim strafes[6] = {
      pas::kLA_StrafeRight, pas::kLA_StrafeLeft, pas::kLA_Walk,
      pas::kLA_BackUp,      pas::kLA_StrafeUp,   pas::kLA_StrafeDown,
  };

  if (CPhysicsActor* act = TCastToPtr< CPhysicsActor >(&bc.GetOwner())) {
    CVector3f localVec = bc.GetCommandMgr().GetMoveVector();
    localVec = act->TransformWorldToLocalRotation(localVec);
    const CVector3f localVecSq = CVector3f::ByElementMultiply(localVec, localVec);
    int maxComp = 0;
    for (int i = 0; i < 3; ++i) {
      if (localVecSq[i] >= localVecSq[maxComp]) {
        maxComp = i;
      }
    }

    const int side = localVec[maxComp] > 0.f ? 0 : 1;
    const int strafeKey = maxComp * 2 + side;
    const pas::ELocomotionAnim strafeType = strafes[strafeKey];
    const float rate = vel * GetLocomotionSpeed(mLocomotionType, strafeType);
    if (anim != strafeType) {
      const rstl::pair< int, float >& strafe = GetLocoAnimation(mLocomotionType, strafeType);
      if (bc.GetCurrentAnimId() != strafe.first) {
        const CAnimPlaybackParms playParms(strafe.first, -1, 1.f, true);
        bc.SetCurrentAnimation(playParms, true, false);
        mPrimeTime = 0.f;
      }
      mAnim = strafeType;
    }

    const rstl::pair< int, float >& idle = GetLocoAnimation(mLocomotionType, pas::kLA_Idle);
    const rstl::pair< int, float >& strafe = GetLocoAnimation(mLocomotionType, strafeType);
    const float perc = rstl::max_val(skMinWalkPercent, ComputeWeightPercentage(idle, strafe, rate));
    bc.MultiplyPlaybackRate(perc);
  }

  return 1.f;
}

float CBSBiPedLocomotion::UpdateWalk(float vel, CBodyController& bc, pas::ELocomotionAnim anim) {
  if (anim != pas::kLA_Walk) {
    const rstl::pair< int, float >& walk = GetLocoAnimation(mLocomotionType, pas::kLA_Walk);
    if (bc.GetCurrentAnimId() != walk.first) {
      const CAnimPlaybackParms playParms(walk.first, -1, 1.f, true);
      bc.SetCurrentAnimation(playParms, true, false);
      mPrimeTime = 0.f;
    }
    mAnim = pas::kLA_Walk;
  }

  const rstl::pair< int, float >& idle = GetLocoAnimation(mLocomotionType, pas::kLA_Idle);
  const rstl::pair< int, float >& walk = GetLocoAnimation(mLocomotionType, pas::kLA_Walk);
  const float perc = rstl::max_val(skMinWalkPercent, ComputeWeightPercentage(idle, walk, vel));
  bc.MultiplyPlaybackRate(perc);
  return perc;
}

float CBSBiPedLocomotion::UpdateRun(float vel, CBodyController& bc, pas::ELocomotionAnim anim) {
  const rstl::pair< int, float >& walk = GetLocoAnimation(mLocomotionType, pas::kLA_Walk);
  const rstl::pair< int, float >& run = GetLocoAnimation(mLocomotionType, pas::kLA_Run);
  const float perc = ComputeWeightPercentage(walk, run, vel);
  const int walkAnim = walk.first;
  const int runAnim = run.first;
  float rate;

  if (perc < 0.4f) {
    rate = walk.second > 0.f ? vel / walk.second : 1.f;
    if (anim != pas::kLA_Walk && bc.GetCurrentAnimId() != walkAnim) {
      const CAnimPlaybackParms playParms(walkAnim, -1, 1.f, true);
      bc.SetCurrentAnimation(playParms, true, false);
      mPrimeTime = 0.f;
    }
    bc.MultiplyPlaybackRate(rate);
    mAnim = pas::kLA_Walk;
  } else {
    rate = rstl::min_val(vel / run.second, 1.f);
    if (anim != pas::kLA_Run && bc.GetCurrentAnimId() != runAnim) {
      const CAnimPlaybackParms playParms(runAnim, -1, 1.f, true);
      bc.SetCurrentAnimation(playParms, true, false);
      mPrimeTime = 0.f;
    }
    bc.MultiplyPlaybackRate(rate);
    mAnim = pas::kLA_Run;
  }

  return rate;
}

CBSRestrictedLocomotion::CBSRestrictedLocomotion(CActor& actor)
: mAnims(14, skInvalidAnimId)
, mAnim(pas::kLA_Invalid) {
  const CPASDatabase& pasDatabase = actor.GetAnimationData()->GetCharacterInfo().GetPASDatabase();
  for (int i = 0; i < 14; ++i) {
    CPASAnimParmData parms(pas::kAS_Locomotion, CPASAnimParm::FromEnum(0),
                           CPASAnimParm::FromEnum(i));
    rstl::pair< float, int > best = pasDatabase.FindBestAnimation(parms, -1);
    mAnims[static_cast< pas::ELocomotionType >(i)] = best.second;
  }
}

float CBSRestrictedLocomotion::UpdateLocomotionAnimation(float dt, float velMag,
                                                         CBodyController& bc, bool init) {
  const pas::ELocomotionAnim anim = init ? pas::kLA_Invalid : mAnim;
  if (anim != pas::kLA_Idle) {
    const int newAnim = mAnims[static_cast< int >(mLocomotionType)];
    if (newAnim != bc.GetCurrentAnimId()) {
      const CAnimPlaybackParms playParms(newAnim, -1, 1.f, true);
      bc.SetCurrentAnimation(playParms, true, false);
    }
    mAnim = pas::kLA_Idle;
  }

  (void)dt;
  (void)velMag;
  return 1.f;
}

CBSFlyerLocomotion::CBSFlyerLocomotion(CActor& actor, const bool pitchable)
: CBSBiPedLocomotion(actor), mPitchable(pitchable) {}

float CBSFlyerLocomotion::ApplyLocomotionPhysics(float dt, CBodyController& bc) {
  const float ret = CBSLocomotion::ApplyLocomotionPhysics(dt, bc);

  if (CPhysicsActor* act = TCastToPtr< CPhysicsActor >(&bc.GetOwner())) {
    if (CMath::AbsF(bc.GetCommandMgr().GetMoveVector()[kDZ]) > 0.01f &&
        (!mPitchable || bc.GetBodyStateInfo().GetMaximumPitch() < skMaxPitchAngle)) {
      const float maxSpeed = bc.GetBodyStateInfo().GetMaxSpeed();
      CVector3f dir(0.f, 0.f, dt * (maxSpeed * bc.GetCommandMgr().GetMoveVector()[kDZ]));
      CVector3f impulse = act->GetMoveToORImpulseWR(dir, dt);
      act->ApplyImpulseWR(impulse, CAxisAngle::Identity());
    }
  }

  return ret;
}

CBSWallWalkerLocomotion::CBSWallWalkerLocomotion(CActor& actor) : CBSBiPedLocomotion(actor) {}

float CBSWallWalkerLocomotion::ApplyLocomotionPhysics(float dt, CBodyController& bc) {
  if (CPhysicsActor* act = TCastToPtr< CPhysicsActor >(&bc.GetOwner())) {
    const float maxSpeed = bc.GetBodyStateInfo().GetMaxSpeed();
    const CVector3f scaledMove = bc.CommandMgr().GetMoveVector() * maxSpeed;

    const CVector3f tmp =
        CVector3f::GetAngleDiff(bc.CommandMgr().GetFaceVector(), scaledMove) < M_PIF / 2.f
            ? scaledMove
            : bc.CommandMgr().GetFaceVector();
    if (tmp.CanBeNormalized()) {
      bc.FaceDirection3D(scaledMove.AsNormalized(), act->GetTransform().GetForward(), dt);
    }

    const CVector3f moveDt = scaledMove * dt;
    CVector3f moveImpulse =
        act->GetMoveToORImpulseWR(act->TransformWorldToLocalRotation(moveDt), dt);
    CVector3f impulse;
    impulse = act->GetMass() > FLT_EPSILON ? moveImpulse / act->GetMass()
                                           : CVector3f(0.f, act->GetVelocityWR().Magnitude(), 0.f);

    if (maxSpeed > FLT_EPSILON) {
      return rstl::min_val(impulse.Magnitude() / maxSpeed, 1.f);
    }
  }

  return 0.f;
}

CBSBiPedLocomotion::~CBSBiPedLocomotion() {}

CBSAiMovedFlyerLocomotion::CBSAiMovedFlyerLocomotion(CActor& actor) : CBSBiPedLocomotion(actor) {}

float CBSAiMovedFlyerLocomotion::ApplyLocomotionPhysics(float dt, CBodyController& bc) {
  if (TCastToPtr< CPhysicsActor >(&bc.GetOwner())) {
    bc.FaceDirection(bc.CommandMgr().GetFaceVector(), dt);
  }

  return 0.f;
}

float CBSAiMovedFlyerLocomotion::UpdateLocomotionAnimation(float dt, float velMag, CBodyController& bc,
                                                       bool init) {
  (void)dt;
  (void)velMag;

  static pas::ELocomotionAnim runStrafes[6] = {
      pas::kLA_StrafeRight, pas::kLA_StrafeLeft, pas::kLA_Run,
      pas::kLA_BackUp,      pas::kLA_StrafeUp,   pas::kLA_StrafeDown,
  };

  if (CPhysicsActor* act = TCastToPtr< CPhysicsActor >(&bc.GetOwner())) {
    pas::ELocomotionAnim strafeType = pas::kLA_Idle;
    const CBodyStateCmdMgr& cmdMgr = bc.GetCommandMgr();
    if (cmdMgr.GetMoveVector().CanBeNormalized()) {
      CVector3f localVec = act->TransformWorldToLocalRotation(cmdMgr.GetMoveVector());
      CVector3f localVecSq = CVector3f::ByElementMultiply(localVec, localVec);
      int maxComp = 0;
      for (int i = 0; i < 3; ++i) {
        if (localVecSq[i] >= localVecSq[maxComp]) {
          maxComp = i;
        }
      }

      const int side = localVec[maxComp] > 0.f ? 0 : 1;
      const int strafeKey = maxComp * 2 + side;
      strafeType = runStrafes[strafeKey];
    }

    if (init || strafeType != mAnim) {
      const rstl::pair< int, float >& strafe = GetLocoAnimation(mLocomotionType, strafeType);
      const int anim = strafe.first;
      if (init || bc.GetCurrentAnimId() != anim) {
        const CAnimPlaybackParms playParms(anim, -1, 1.f, true);
        bc.SetCurrentAnimation(playParms, true, false);
      }
      mAnim = strafeType;
    }
  }

  return 1.f;
}

CBSFloaterLocomotion::CBSFloaterLocomotion(CActor& actor)
: CBSRestrictedLocomotion(actor) {}

float CBSFloaterLocomotion::ApplyLocomotionPhysics(float dt, CBodyController& bc) {
  if (CPhysicsActor* act = TCastToPtr< CPhysicsActor >(bc.GetOwner())) {
    bc.FaceDirection(bc.GetCommandMgr().GetFaceVector(), dt);
    const float moveSpeed = bc.GetRestrictedFlyerMoveSpeed();
    const float mass = act->GetMass();
    CVector3f impulse = bc.GetCommandMgr().GetMoveVector() * moveSpeed * mass;
    act->ApplyImpulseWR(impulse, CAxisAngle::Identity());
  }

  return 0.f;
}

bool CBSLocomotion::IsPitchable() const { return false; }

bool CBSLocomotion::CanShoot() const { return true; }

float CBSRestrictedLocomotion::GetLocomotionSpeed(pas::ELocomotionType type,
                                                  pas::ELocomotionAnim anim) const {
  (void)type;
  (void)anim;
  return 0.f;
}

bool CBSRestrictedLocomotion::IsMoving() const { return false; }

CBSFloaterLocomotion::~CBSFloaterLocomotion() {}
