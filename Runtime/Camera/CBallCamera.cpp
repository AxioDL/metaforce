#include "Runtime/Camera/CBallCamera.hpp"

#include <algorithm>
#include <cmath>

#include "Runtime/CStateManager.hpp"
#include "Runtime/GameGlobalObjects.hpp"
#include "Runtime/rstl.hpp"
#include "Runtime/Camera/CFirstPersonCamera.hpp"
#include "Runtime/Camera/CPathCamera.hpp"
#include "Runtime/Collision/CCollisionActor.hpp"
#include "Runtime/Collision/CGameCollision.hpp"
#include "Runtime/Input/ControlMapper.hpp"
#include "Runtime/World/CPlayer.hpp"
#include "Runtime/World/CScriptCameraHint.hpp"
#include "Runtime/World/CScriptDock.hpp"
#include "Runtime/World/CScriptDoor.hpp"
#include "Runtime/World/CScriptSpindleCamera.hpp"
#include "Runtime/World/CScriptWater.hpp"

#include "TCastTo.hpp" // Generated file, do not modify include path

namespace urde {

void CCameraSpring::Reset() {
  x4_k2Sqrt = 2.f * std::sqrt(x0_k);
  x10_dx = 0.f;
}

float CCameraSpring::ApplyDistanceSpringNoMax(float targetX, float curX, float dt) {
  float useX = xc_tardis * x10_dx * dt + curX;
  x10_dx += xc_tardis * (x0_k * (targetX - curX) - x4_k2Sqrt * x10_dx) * dt;
  return std::max(useX, targetX);
}

float CCameraSpring::ApplyDistanceSpring(float targetX, float curX, float dt) {
  float useX = xc_tardis * x10_dx * dt + curX;
  x10_dx += xc_tardis * (x0_k * (targetX - curX) - x4_k2Sqrt * x10_dx) * dt;
  useX = std::max(useX, targetX);
  if (useX - targetX > x8_max)
    useX = targetX + x8_max;
  return useX;
}

CBallCamera::CBallCamera(TUniqueId uid, TUniqueId watchedId, const zeus::CTransform& xf, float fovy, float znear,
                         float zfar, float aspect)
: CGameCamera(uid, true, "Ball Camera", CEntityInfo(kInvalidAreaId, CEntity::NullConnectionList), xf, fovy, znear, zfar,
              aspect, watchedId, false, 0)
, x214_ballCameraSpring(g_tweakBall->GetBallCameraSpringConstant(), g_tweakBall->GetBallCameraSpringMax(),
                        g_tweakBall->GetBallCameraSpringTardis())
, x228_ballCameraCentroidSpring(g_tweakBall->GetBallCameraCentroidSpringConstant(),
                                g_tweakBall->GetBallCameraCentroidSpringMax(),
                                g_tweakBall->GetBallCameraCentroidSpringTardis())
, x23c_ballCameraLookAtSpring(g_tweakBall->GetBallCameraLookAtSpringConstant(),
                              g_tweakBall->GetBallCameraLookAtSpringMax(),
                              g_tweakBall->GetBallCameraLookAtSpringTardis())
, x250_ballCameraCentroidDistanceSpring(g_tweakBall->GetBallCameraCentroidDistanceSpringConstant(),
                                        g_tweakBall->GetBallCameraCentroidDistanceSpringMax(),
                                        g_tweakBall->GetBallCameraCentroidDistanceSpringTardis())
, x41c_ballCameraChaseSpring(g_tweakBall->GetBallCameraChaseSpringConstant(),
                             g_tweakBall->GetBallCameraChaseSpringMax(), g_tweakBall->GetBallCameraChaseSpringTardis())
, x448_ballCameraBoostSpring(g_tweakBall->GetBallCameraBoostSpringConstant(),
                             g_tweakBall->GetBallCameraBoostSpringMax(),
                             g_tweakBall->GetBallCameraBoostSpringTardis()) {
  x190_curMinDistance = g_tweakBall->GetBallCameraMinSpeedDistance();
  x194_targetMinDistance = g_tweakBall->GetBallCameraMinSpeedDistance();
  x198_maxDistance = g_tweakBall->GetBallCameraMaxSpeedDistance();
  x19c_backwardsDistance = g_tweakBall->GetBallCameraBackwardsDistance();

  x1a0_elevation = g_tweakBall->GetBallCameraElevation();
  x1a4_curAnglePerSecond = g_tweakBall->GetBallCameraAnglePerSecond();
  x1a8_targetAnglePerSecond = g_tweakBall->GetBallCameraAnglePerSecond();
  x1b4_lookAtOffset = g_tweakBall->GetBallCameraOffset();

  x404_chaseElevation = g_tweakBall->GetBallCameraChaseElevation();
  x408_chaseDistance = g_tweakBall->GetBallCameraChaseDistance();
  x40c_chaseAnglePerSecond = g_tweakBall->GetBallCameraChaseAnglePerSecond();
  x410_chaseLookAtOffset = g_tweakBall->GetBallCameraChaseLookAtOffset();

  x430_boostElevation = g_tweakBall->GetBallCameraBoostElevation();
  x434_boostDistance = g_tweakBall->GetBallCameraBoostDistance();
  x438_boostAnglePerSecond = g_tweakBall->GetBallCameraBoostAnglePerSecond();
  x43c_boostLookAtOffset = g_tweakBall->GetBallCameraBoostLookAtOffset();

  x468_conservativeDoorCamDistance = g_tweakBall->GetConservativeDoorCameraDistance();

  x47c_failsafeState = std::make_unique<SFailsafeState>();
  x480_ = std::make_unique<u32>();

  SetupColliders(x264_smallColliders, 2.31f, 2.31f, 0.1f, 3, 2.f, 0.5f, -M_PIF / 2.f);
  SetupColliders(x274_mediumColliders, 4.62f, 4.62f, 0.1f, 6, 2.f, 0.5f, -M_PIF / 2.f);
  SetupColliders(x284_largeColliders, 7.f, 7.f, 0.1f, 12, 2.f, 0.5f, -M_PIF / 2.f);
}

void CBallCamera::SetupColliders(std::vector<CCameraCollider>& out, float xMag, float zMag, float radius, int count,
                                 float k, float max, float startAngle) {
  out.reserve(count);
  float theta = startAngle;
  for (int i = 0; i < count; ++i) {
    float z = std::cos(theta) * zMag;
    if (theta > M_PIF / 2.f)
      z *= 0.25f;
    out.emplace_back(radius, zeus::CVector3f{std::sin(theta) * xMag, 0.f, z}, CCameraSpring{k, max, 1.f}, 1.f);
    theta += 2.f * M_PIF / float(count);
  }
}

void CBallCamera::Accept(IVisitor& visitor) { visitor.Visit(this); }

void CBallCamera::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId objId, CStateManager& stateMgr) {
  CGameCamera::AcceptScriptMsg(msg, objId, stateMgr);
  switch (msg) {
  case EScriptObjectMessage::Registered: {
    x46c_collisionActorId = stateMgr.AllocateUniqueId();
    CCollisionActor* colAct =
        new CCollisionActor(x46c_collisionActorId, GetAreaId(), kInvalidUniqueId, true, 0.3f, 1.f, "BallCamera"sv);
    colAct->SetMaterialFilter(CMaterialFilter::MakeIncludeExclude(
        {EMaterialTypes::Solid}, {EMaterialTypes::Player, EMaterialTypes::CameraPassthrough}));
    colAct->SetMaterialList({EMaterialTypes::ProjectilePassthrough, EMaterialTypes::ScanPassthrough,
                             EMaterialTypes::SeeThrough, EMaterialTypes::CameraPassthrough});
    colAct->SetTranslation(GetTranslation());
    stateMgr.AddObject(colAct);
    colAct->SetMovable(false);
    CMotionState mState(GetTranslation(), zeus::CNUQuaternion::fromAxisAngle(zeus::skForward, 0.f),
                        zeus::skZero3f, zeus::CAxisAngle());
    colAct->SetLastNonCollidingState(mState);
    SetMaterialFilter(CMaterialFilter::MakeIncludeExclude(
        {}, {EMaterialTypes::Solid, EMaterialTypes::ProjectilePassthrough, EMaterialTypes::Player,
             EMaterialTypes::Character, EMaterialTypes::CameraPassthrough}));
    RemoveMaterial(EMaterialTypes::Solid, stateMgr);
    break;
  }
  case EScriptObjectMessage::Deleted:
    stateMgr.FreeScriptObject(x46c_collisionActorId);
    x46c_collisionActorId = kInvalidUniqueId;
    break;
  default:
    break;
  }
}

void CBallCamera::ProcessInput(const CFinalInput& input, CStateManager& mgr) {
  if (input.ControllerIdx() != 0)
    return;

  if (TCastToConstPtr<CPlayer> player = mgr.GetObjectById(xe8_watchedObject)) {
    if (player->GetMorphballTransitionState() == CPlayer::EPlayerMorphBallState::Morphed) {
      switch (x400_state) {
      case EBallCameraState::Chase:
        if (!ControlMapper::GetDigitalInput(ControlMapper::ECommands::ChaseCamera, input) || player->IsInFreeLook())
          SetState(EBallCameraState::Default, mgr);
        break;
      case EBallCameraState::Boost:
        if (!player->GetMorphBall()->IsInBoost())
          SetState(EBallCameraState::Default, mgr);
        break;
      case EBallCameraState::Default:
        if (x18c_25_chaseAllowed && ControlMapper::GetPressInput(ControlMapper::ECommands::ChaseCamera, input))
          SetState(EBallCameraState::Chase, mgr);
        break;
      default:
        break;
      }

      if (x18c_26_boostAllowed && x400_state != EBallCameraState::Boost &&
          (player->GetMorphBall()->IsInBoost() || player->GetMorphBall()->GetBoostChargeTime() > 0.f))
        SetState(EBallCameraState::Boost, mgr);
    }
  }
}

void CBallCamera::Reset(const zeus::CTransform& xf, CStateManager& mgr) {
  x214_ballCameraSpring.Reset();
  x228_ballCameraCentroidSpring.Reset();
  x23c_ballCameraLookAtSpring.Reset();
  x250_ballCameraCentroidDistanceSpring.Reset();
  x41c_ballCameraChaseSpring.Reset();
  x448_ballCameraBoostSpring.Reset();

  zeus::CVector3f desiredPos = FindDesiredPosition(x190_curMinDistance, x1a0_elevation, xf.basis[1], mgr, false);

  if (TCastToConstPtr<CPlayer> player = mgr.GetObjectById(xe8_watchedObject)) {
    ResetPosition(mgr);
    x310_idealLookVec = x1b4_lookAtOffset;
    x31c_predictedLookPos = x1d8_lookPos;
    if ((x1d8_lookPos - desiredPos).canBeNormalized()) {
      TeleportCamera(zeus::lookAt(desiredPos, x1d8_lookPos), mgr);
    } else {
      zeus::CTransform camXf = player->CreateTransformFromMovementDirection();
      camXf.origin = desiredPos;
      TeleportCamera(camXf, mgr);
      mgr.GetCameraManager()->SetPlayerCamera(mgr, GetUniqueId());
    }

    x2e8_ballVelFlat = 0.f;
    x2ec_maxBallVel = 0.f;
    x190_curMinDistance = x194_targetMinDistance;
    x2fc_ballDeltaFlat = zeus::skZero3f;
    x2f0_ballDelta = zeus::skZero3f;
    x18d_28_obtuseDirection = false;
    x308_speedFactor = 0.f;
    x2dc_prevBallPos = player->GetBallPosition();
    x294_dampedPos = GetTranslation();
    x2a0_smallCentroid = zeus::skZero3f;
    x2ac_mediumCentroid = zeus::skZero3f;
    x2b8_largeCentroid = zeus::skZero3f;
    x2c4_smallCollidersObsCount = 0;
    x2c8_mediumCollidersObsCount = 0;
    x2cc_largeCollidersObsCount = 0;
    x2d0_smallColliderIt = 0;
    x2d4_mediumColliderIt = 0;
    x2d8_largeColliderIt = 0;
    x32c_colliderMag = 1.f;
    x18d_25_avoidGeometryFull = true;
    x18d_27_forceProcessing = true;
    Think(0.1f, mgr);
    x18d_25_avoidGeometryFull = false;
    x18d_27_forceProcessing = false;
  }
}

void CBallCamera::Render(CStateManager& mgr) {
  // Empty
}

void CBallCamera::SetState(EBallCameraState state, CStateManager& mgr) {
  switch (state) {
  case EBallCameraState::ToBall: {
    zeus::CTransform xf = mgr.GetCameraManager()->GetFirstPersonCamera()->GetTransform();
    SetTransform(xf);
    TeleportCamera(xf.origin, mgr);
    SetFovInterpolation(mgr.GetCameraManager()->GetFirstPersonCamera()->GetFov(), CCameraManager::ThirdPersonFOV(), 1.f,
                        0.f);
    x36c_splineState = ESplineState::Invalid;
    [[fallthrough]];
  }
  case EBallCameraState::Default:
  case EBallCameraState::Chase:
  case EBallCameraState::Boost:
    mgr.SetGameState(CStateManager::EGameState::Running);
    break;
  case EBallCameraState::FromBall:
    mgr.GetCameraManager()->SetPlayerCamera(mgr, GetUniqueId());
    mgr.SetGameState(CStateManager::EGameState::Running);
    SetFovInterpolation(GetFov(), CCameraManager::FirstPersonFOV(), 1.f, 0.f);
    x36c_splineState = ESplineState::Invalid;
    break;
  default:
    break;
  }

  x400_state = state;
}

constexpr CMaterialFilter BallCameraFilter = CMaterialFilter::MakeIncludeExclude(
    {EMaterialTypes::Solid}, {EMaterialTypes::ProjectilePassthrough, EMaterialTypes::Player, EMaterialTypes::Character,
                              EMaterialTypes::CameraPassthrough});

void CBallCamera::BuildSplineNav(CStateManager& mgr) {
  zeus::CVector3f ballPos = mgr.GetPlayer().GetBallPosition();
  TUniqueId intersectId = kInvalidUniqueId;
  rstl::reserved_vector<TUniqueId, 1024> nearList;
  CRayCastResult result =
      mgr.RayWorldIntersection(intersectId, ballPos, zeus::skDown, 20.f, BallCameraFilter, nearList);
  float downFactor = result.IsValid() ? zeus::clamp(0.f, result.GetT() / 20.f, 1.f) : 1.f;
  x36c_splineState = ESplineState::Nav;
  x370_24_reevalSplineEnd = true;
  x3d0_24_camBehindFloorOrWall = false;
  x37c_camSpline.Reset(4);
  x37c_camSpline.AddKnot(GetTranslation(), zeus::skForward);
  float elevation = x1a0_elevation;
  float distance = x190_curMinDistance;
  ConstrainElevationAndDistance(elevation, distance, 0.f, mgr);
  zeus::CVector3f pt1(x35c_splineIntermediatePos.x(), x35c_splineIntermediatePos.y(), GetTranslation().z());
  x37c_camSpline.AddKnot(pt1, zeus::skForward);
  zeus::CVector3f pt2 = pt1 + (x35c_splineIntermediatePos - GetTranslation()) * (0.5f + downFactor);
  x37c_camSpline.AddKnot(pt2, zeus::skForward);
  zeus::CVector3f pt2Ball = ballPos - pt2;
  if (pt2Ball.canBeNormalized())
    pt2Ball.normalize();
  else
    pt2Ball = mgr.GetPlayer().GetMoveDir();
  zeus::CVector3f desiredPosition = FindDesiredPosition(distance, elevation, pt2Ball, mgr, false);
  x37c_camSpline.AddKnot(desiredPosition, zeus::skForward);
  x37c_camSpline.UpdateSplineLength();
  x3d0_24_camBehindFloorOrWall = false;
  CMaterialList intersectMat;
  x3c8_collisionExcludeList = CMaterialList(EMaterialTypes::Floor, EMaterialTypes::Ceiling);
  if (!SplineIntersectTest(intersectMat, mgr)) {
    if (intersectMat.HasMaterial(EMaterialTypes::Floor) || intersectMat.HasMaterial(EMaterialTypes::Wall)) {
      x3d0_24_camBehindFloorOrWall = true;
      x3c8_collisionExcludeList = CMaterialList();
    }
  }
  x374_splineCtrl = 0.5f * downFactor + 2.f;
  x378_splineCtrlRange = 2.5f;
}

void CBallCamera::BuildSplineArc(CStateManager& mgr) {
  zeus::CVector3f ballPos = mgr.GetPlayer().GetBallPosition();
  x36c_splineState = ESplineState::Arc;
  x370_24_reevalSplineEnd = false;
  x37c_camSpline.Reset(4);
  x37c_camSpline.AddKnot(GetTranslation(), zeus::skForward);
  float elevation = x1a0_elevation;
  float distance = x190_curMinDistance;
  ConstrainElevationAndDistance(elevation, distance, 0.f, mgr);
  zeus::CVector3f halfwayPoint = (ballPos.toVec2f() - GetTranslation().toVec2f()) * 0.5f + GetTranslation().toVec2f();
  halfwayPoint.z() = GetTranslation().z();
  zeus::CVector3f delta = GetTranslation() - halfwayPoint;
  zeus::CQuaternion rot;
  rot.rotateZ(zeus::degToRad(45.f));
  if (mgr.GetPlayer().GetMoveDir().cross(x34_transform.basis[1]).z() >= 0.f) {
    rot = zeus::CQuaternion();
    rot.rotateZ(zeus::degToRad(-45.f));
  }
  delta = rot.transform(delta);
  zeus::CVector3f pt1 = halfwayPoint + delta;
  TUniqueId intersectId = kInvalidUniqueId;
  rstl::reserved_vector<TUniqueId, 1024> nearList;
  CRayCastResult result =
      mgr.RayWorldIntersection(intersectId, pt1, -delta.normalized(), delta.magnitude(), BallCameraFilter, nearList);
  if (result.IsValid())
    pt1 = delta.normalized() * 1.5f + result.GetPoint();
  else
    pt1 = halfwayPoint + delta;
  x37c_camSpline.AddKnot(pt1, zeus::skForward);
  FindDesiredPosition(distance, elevation, mgr.GetPlayer().GetMoveDir(), mgr, false);
  delta = rot.transform(delta);
  zeus::CVector3f pt2 = halfwayPoint + delta;
  result =
      mgr.RayWorldIntersection(intersectId, pt2, -delta.normalized(), delta.magnitude(), BallCameraFilter, nearList);
  if (result.IsValid())
    pt2 = delta.normalized() * 2.f + result.GetPoint();
  else
    pt2 = halfwayPoint + delta;
  x37c_camSpline.AddKnot(pt2, zeus::skForward);
  delta = rot.transform(delta);
  zeus::CVector3f pt3 = delta + halfwayPoint;
  result =
      mgr.RayWorldIntersection(intersectId, pt3, -delta.normalized(), delta.magnitude(), BallCameraFilter, nearList);
  if (result.IsValid())
    pt3 = delta.normalized() * 2.f + result.GetPoint();
  else
    pt3 = halfwayPoint + delta;
  x37c_camSpline.AddKnot(pt3, zeus::skForward);
  CMaterialList intersectMat;
  if (!SplineIntersectTest(intersectMat, mgr) && intersectMat.HasMaterial(EMaterialTypes::Wall)) {
    delta = pt1 - halfwayPoint;
    result =
        mgr.RayWorldIntersection(intersectId, pt1, -delta.normalized(), delta.magnitude(), BallCameraFilter, nearList);
    if (result.IsValid() && !result.GetMaterial().HasMaterial(EMaterialTypes::Pillar))
      x37c_camSpline.SetKnotPosition(1, result.GetPoint() - delta.normalized() * 0.3f * 1.25f);
    delta = pt2 - halfwayPoint;
    result =
        mgr.RayWorldIntersection(intersectId, pt2, -delta.normalized(), delta.magnitude(), BallCameraFilter, nearList);
    if (result.IsValid() && !result.GetMaterial().HasMaterial(EMaterialTypes::Pillar))
      x37c_camSpline.SetKnotPosition(2, result.GetPoint() - delta.normalized() * 0.3f * 1.25f);
    x37c_camSpline.UpdateSplineLength();
    if (!SplineIntersectTest(intersectMat, mgr)) {
      x36c_splineState = ESplineState::Invalid;
      return;
    }
  }
  x374_splineCtrl = 0.5f;
  x378_splineCtrlRange = 0.5f;
  x37c_camSpline.UpdateSplineLength();
  x3c8_collisionExcludeList = CMaterialList();
}

bool CBallCamera::ShouldResetSpline(CStateManager& mgr) const {
  return x400_state != EBallCameraState::ToBall && !mgr.GetCameraManager()->IsInterpolationCameraActive() &&
         mgr.GetPlayer().GetMorphBall()->GetSpiderBallState() != CMorphBall::ESpiderBallState::Active &&
         x36c_splineState == ESplineState::Invalid &&
         (x188_behaviour > EBallCameraBehaviour::SpindleCamera ||
          x188_behaviour < EBallCameraBehaviour::HintFixedPosition);
}

void CBallCamera::UpdatePlayerMovement(float dt, CStateManager& mgr) {
  x2ec_maxBallVel = std::fabs(mgr.GetPlayer().GetActualBallMaxVelocity(dt));
  zeus::CVector3f ballPos = mgr.GetPlayer().GetBallPosition();
  x2f0_ballDelta = ballPos - x2dc_prevBallPos;
  x2fc_ballDeltaFlat = x2f0_ballDelta;
  x2fc_ballDeltaFlat.z() = 0.f;
  if (x2fc_ballDeltaFlat.canBeNormalized())
    x2e8_ballVelFlat = x2fc_ballDeltaFlat.magnitude() / dt;
  else
    x2e8_ballVelFlat = 0.f;
  x2dc_prevBallPos = ballPos;
  x18d_28_obtuseDirection = false;
  zeus::CVector3f camToBallFlat = ballPos - GetTranslation();
  camToBallFlat.z() = 0.f;
  if (camToBallFlat.canBeNormalized()) {
    camToBallFlat.normalize();
    if (std::fabs(std::acos(zeus::clamp(-1.f, camToBallFlat.dot(mgr.GetPlayer().GetMoveDir()), 1.f))) >
        zeus::degToRad(100.f))
      x18d_28_obtuseDirection = true;
  }
  x308_speedFactor = 0.f;
  float tmpVel = x2e8_ballVelFlat - 4.f;
  if (tmpVel > 0.f)
    x308_speedFactor =
        zeus::clamp(-1.f, std::fabs(std::sin(zeus::degToRad(tmpVel / (x2ec_maxBallVel - 4.f) * 90.f))), 1.f);
  x190_curMinDistance = x308_speedFactor * (x198_maxDistance - x194_targetMinDistance) + x194_targetMinDistance;
  if (x308_speedFactor > 0.5f && mgr.GetPlayer().GetPlayerMovementState() == CPlayer::EPlayerMovementState::OnGround)
    x30c_speedingTime += dt * x308_speedFactor;
  else
    x30c_speedingTime = 0.f;
  x30c_speedingTime = zeus::clamp(0.f, x30c_speedingTime, 3.f);
}

void CBallCamera::UpdateTransform(const zeus::CVector3f& lookDir, const zeus::CVector3f& pos, float dt,
                                  CStateManager& mgr) {
  zeus::CVector3f useLookDir = lookDir;
  if (x18d_31_overrideLookDir)
    if (const CScriptCameraHint* hint = mgr.GetCameraManager()->GetCameraHint(mgr))
      useLookDir = hint->GetTransform().basis[1];
  zeus::CVector3f lookDirFlat = useLookDir;
  lookDirFlat.z() = 0.f;
  if (!lookDirFlat.canBeNormalized()) {
    SetTranslation(pos);
    return;
  }
  zeus::CVector3f curLookDir = x34_transform.basis[1];
  if (curLookDir.canBeNormalized()) {
    curLookDir.normalize();
  } else {
    SetTransform(zeus::lookAt(pos, pos + useLookDir));
    return;
  }
  float lookDirDot = zeus::clamp(-1.f, curLookDir.dot(useLookDir), 1.f);
  if (std::fabs(lookDirDot) >= 1.f) {
    SetTransform(zeus::lookAt(pos, pos + useLookDir));
  } else {
    float angleSpeedMul = zeus::clamp(0.f, std::acos(lookDirDot) / (zeus::degToRad(60.f) * dt), 1.f);
    float angleDelta = dt * x1a4_curAnglePerSecond * angleSpeedMul;
    float lookUpDot = std::fabs(zeus::clamp(-1.f, useLookDir.dot(zeus::skUp), 1.f));
    float maxAngleDelta = (1.f - lookUpDot) * zeus::degToRad(720.f) * dt;
    if (x36c_splineState == ESplineState::Nav) {
      maxAngleDelta = zeus::degToRad(240.f) * dt;
      if (angleDelta > maxAngleDelta)
        angleDelta = maxAngleDelta;
    }
    if (angleDelta > maxAngleDelta && !mgr.GetPlayer().IsMorphBallTransitioning() && lookUpDot > 0.999f)
      angleDelta = maxAngleDelta;
    switch (x400_state) {
    case EBallCameraState::Chase:
      if (x18c_25_chaseAllowed)
        angleDelta = dt * x40c_chaseAnglePerSecond * angleSpeedMul;
      break;
    case EBallCameraState::Boost:
      angleDelta = dt * x438_boostAnglePerSecond * angleSpeedMul;
      break;
    default:
      break;
    }
    if (x18d_26_lookAtBall || mgr.GetCameraManager()->IsInterpolationCameraActive()) {
      x18d_26_lookAtBall = false;
      SetTransform(zeus::CQuaternion::lookAt(curLookDir, useLookDir, 2.f * M_PIF).toTransform() *
                   x34_transform.getRotation());
    } else {
      SetTransform(zeus::CQuaternion::lookAt(curLookDir, useLookDir, angleDelta).toTransform() *
                   x34_transform.getRotation());
    }
  }
  SetTranslation(pos);
}

zeus::CVector3f CBallCamera::ConstrainYawAngle(const CPlayer& player, float distance, float yawSpeed, float dt,
                                               CStateManager& mgr) const {
  zeus::CVector3f playerToCamFlat = GetTranslation() - player.GetTranslation();
  playerToCamFlat.z() = 0.f;
  zeus::CVector3f lookDir = player.GetTransform().basis[1];
  if (player.GetMorphballTransitionState() == CPlayer::EPlayerMorphBallState::Morphed) {
    lookDir = player.GetMoveDir();
    TCastToConstPtr<CScriptDoor> door = mgr.GetObjectById(x3dc_tooCloseActorId);
    if ((!door || !door->x2a8_26_isOpen) &&
        (x400_state == EBallCameraState::Boost || x400_state == EBallCameraState::Chase))
      lookDir = player.GetLeaveMorphDir();
  }
  if (player.GetMorphballTransitionState() == CPlayer::EPlayerMorphBallState::Unmorphing)
    lookDir = player.GetLeaveMorphDir();
  if (lookDir.canBeNormalized())
    lookDir.normalize();
  else
    lookDir = -playerToCamFlat;
  if (playerToCamFlat.canBeNormalized())
    playerToCamFlat.normalize();
  else
    return -lookDir;
  float angleProj = zeus::clamp(-1.f, playerToCamFlat.dot(-lookDir), 1.f);
  if (angleProj >= 1.f)
    return -lookDir;
  return zeus::CQuaternion::lookAt(playerToCamFlat, -lookDir,
                                   distance * dt * zeus::clamp(0.f, std::acos(angleProj) / yawSpeed, 1.f))
      .transform(playerToCamFlat);
}

void CBallCamera::CheckFailsafe(float dt, CStateManager& mgr) {
  zeus::CVector3f ballPos = mgr.GetPlayer().GetBallPosition();
  x18d_24_prevClearLOS = x18c_31_clearLOS;
  zeus::CVector3f camToBall = ballPos - GetTranslation();
  float camToBallMag = camToBall.magnitude();
  camToBall.normalize();
  rstl::reserved_vector<TUniqueId, 1024> nearList;
  mgr.BuildNearList(nearList, GetTranslation(), camToBall, camToBallMag, BallCameraFilter, nullptr);
  CRayCastResult result = mgr.RayWorldIntersection(x368_obscuringObjectId, GetTranslation(), camToBall, camToBallMag,
                                                   BallCameraFilter, nearList);
  if (result.IsValid()) {
    x350_obscuringMaterial = result.GetMaterial();
    if (!mgr.RayCollideWorld(GetTranslation(), ballPos, nearList, BallCameraFilter, &mgr.GetPlayer()) &&
        !mgr.RayCollideWorld(GetTranslation(), mgr.GetPlayer().GetTranslation(), nearList, BallCameraFilter,
                             &mgr.GetPlayer())) {
      x18c_31_clearLOS = false;
      if (x18d_24_prevClearLOS) {
        x35c_splineIntermediatePos = ballPos;
        if (ShouldResetSpline(mgr) && !x18e_25_noSpline && x350_obscuringMaterial.HasMaterial(EMaterialTypes::Floor) &&
            mgr.RayCollideWorld(ballPos, ballPos + zeus::CVector3f(0.f, 0.f, -2.5f), nearList, BallCameraFilter,
                                nullptr))
          BuildSplineNav(mgr);
      }
    }
  } else {
    x18c_31_clearLOS = true;
    x350_obscuringMaterial = CMaterialList(EMaterialTypes::NoStepLogic);
  }

  if (!x18c_31_clearLOS) {
    x34c_obscuredTime += dt;
    if (ShouldResetSpline(mgr) && !x18e_25_noSpline && x350_obscuringMaterial.HasMaterial(EMaterialTypes::Pillar))
      BuildSplineArc(mgr);
  } else {
    x34c_obscuredTime = 0.f;
  }

  x358_unobscureMag = zeus::clamp(0.f, x34c_obscuredTime * 0.5f, 1.f);

  x3e4_pendingFailsafe =
      x18c_27_obscureAvoidance &&
      (x34c_obscuredTime > 2.f || (x3dc_tooCloseActorId != kInvalidUniqueId && x34c_obscuredTime > 1.f)) &&
      !x18c_31_clearLOS && x36c_splineState == ESplineState::Invalid;

  bool doFailsafe = x3e4_pendingFailsafe;
  if ((GetTranslation() - ballPos).magnitude() < 0.3f + g_tweakPlayer->GetPlayerBallHalfExtent())
    doFailsafe = true;

  if (x18e_27_nearbyDoorClosed) {
    x18e_27_nearbyDoorClosed = false;
    if (result.IsValid())
      doFailsafe = true;
  }

  if (x18e_28_nearbyDoorClosing) {
    x18e_28_nearbyDoorClosing = false;
    if (IsBallNearDoor(GetTranslation(), mgr))
      doFailsafe = true;
  }

  if (doFailsafe)
    ActivateFailsafe(dt, mgr);
}

void CBallCamera::UpdateObjectTooCloseId(CStateManager& mgr) {
  x3e0_tooCloseActorDist = 1000000.f;
  x3dc_tooCloseActorId = kInvalidUniqueId;
  const zeus::CVector3f ballPos = mgr.GetPlayer().GetBallPosition();
  for (const CEntity* ent : mgr.GetPlatformAndDoorObjectList()) {
    if (const TCastToConstPtr<CScriptDoor> door = ent) {
      if (mgr.GetPlayer().GetAreaIdAlways() == door->GetAreaIdAlways()) {
        door->GetBoundingBox();
        const float minMag = std::min((door->GetTranslation() - GetTranslation()).magnitude(),
                                      (door->GetTranslation() - ballPos).magnitude());
        if (minMag < 30.f && minMag < x3e0_tooCloseActorDist) {
          x3dc_tooCloseActorId = door->GetUniqueId();
          x3e0_tooCloseActorDist = minMag;
        }
      }
    }
  }
}

void CBallCamera::UpdateAnglePerSecond(float dt) {
  float delta = x1a8_targetAnglePerSecond - x1a4_curAnglePerSecond;
  if (std::fabs(delta) >= M_PIF / 1800.f)
    x1a4_curAnglePerSecond += zeus::clamp(-1.f, delta / M_PIF, 1.f) * (10.471975f * dt);
  else
    x1a4_curAnglePerSecond = x1a8_targetAnglePerSecond;
}

void CBallCamera::UpdateUsingPathCameras(float dt, CStateManager& mgr) {
  if (const TCastToConstPtr<CPathCamera> cam = mgr.ObjectById(mgr.GetCameraManager()->GetPathCameraId())) {
    TeleportCamera(cam->GetTransform(), mgr);
    x18d_26_lookAtBall = true;
  }
}

zeus::CVector3f CBallCamera::GetFixedLookTarget(const zeus::CVector3f& hintToLookDir, CStateManager& mgr) const {
  const CScriptCameraHint* hint = mgr.GetCameraManager()->GetCameraHint(mgr);
  if (!hint)
    return hintToLookDir;
  zeus::CVector3f hintDir = hint->GetTransform().basis[1];
  zeus::CVector3f hintDirFlat = hintDir;
  hintDirFlat.z() = 0.f;
  if (hintDir.canBeNormalized() && hintDirFlat.canBeNormalized()) {
    hintDir.normalize();
    hintDirFlat.normalize();
  } else {
    hintDir = zeus::skForward;
    hintDirFlat = zeus::skForward;
  }

  zeus::CVector3f hintToLookDirFlat = hintToLookDir;
  hintToLookDirFlat.z() = 0.f;
  if (hintToLookDir.canBeNormalized() && hintToLookDirFlat.canBeNormalized())
    hintToLookDirFlat.normalize();
  else
    hintToLookDirFlat = hintDirFlat;

  float attitude = std::acos(zeus::clamp(-1.f, hintToLookDir.dot(hintToLookDirFlat), 1.f));
  if (x18c_29_clampAttitude) {
    float refAttitude = std::acos(zeus::clamp(-1.f, hintDir.dot(hintDirFlat), 1.f));
    attitude = refAttitude + zeus::clamp(-x1ac_attitudeRange, attitude - refAttitude, x1ac_attitudeRange);
  }

  if (hintToLookDir.z() >= 0.f)
    attitude = -attitude;

  float azimuth = std::acos(zeus::clamp(-1.f, hintToLookDirFlat.dot(hintDirFlat), 1.f));
  if (x18c_30_clampAzimuth)
    azimuth = zeus::clamp(-x1b0_azimuthRange, azimuth, x1b0_azimuthRange);

  if (hintToLookDirFlat.x() * hintDirFlat.y() - hintDirFlat.x() * hintToLookDirFlat.y() >= 0.f)
    azimuth = -azimuth;

  zeus::CQuaternion quat;
  quat.rotateZ(azimuth);
  zeus::CVector3f aziLookDirFlat = quat.transform(hintDirFlat);
  zeus::CVector3f attitudeAxis(aziLookDirFlat.y(), -aziLookDirFlat.x(), 0.f);
  attitudeAxis.normalize();
  return zeus::CQuaternion::fromAxisAngle(attitudeAxis, -attitude).transform(aziLookDirFlat);
}

void CBallCamera::UpdateUsingFixedCameras(float dt, CStateManager& mgr) {
  if (const CScriptCameraHint* hint = mgr.GetCameraManager()->GetCameraHint(mgr)) {
    switch (x188_behaviour) {
    case EBallCameraBehaviour::HintFixedPosition: {
      zeus::CVector3f hintToLookPos = x1d8_lookPos - hint->GetTranslation();
      if (hintToLookPos.canBeNormalized()) {
        hintToLookPos = GetFixedLookTarget(hintToLookPos.normalized(), mgr);
        if ((hint->GetHint().GetOverrideFlags() & 0x40) != 0)
          x18d_26_lookAtBall = true;
        UpdateTransform(hintToLookPos, hint->GetTranslation(), dt, mgr);
      }
      break;
    }
    case EBallCameraBehaviour::HintFixedTransform:
      SetTransform(hint->GetTransform());
      break;
    default:
      break;
    }
    TeleportCamera(GetTranslation(), mgr);
  }
}

zeus::CVector3f CBallCamera::ComputeVelocity(const zeus::CVector3f& curVel, const zeus::CVector3f& posDelta) const {
  zeus::CVector3f ret = posDelta;
  if (x470_clampVelTimer > 0.f && ret.canBeNormalized() && !x18d_28_obtuseDirection) {
    float mag = ret.magnitude();
    mag = zeus::clamp(-x474_clampVelRange, mag, x474_clampVelRange);
    ret = ret.normalized() * mag;
  }
  return ret;
}

zeus::CVector3f CBallCamera::TweenVelocity(const zeus::CVector3f& curVel, const zeus::CVector3f& newVel, float rate,
                                           float dt) {
  zeus::CVector3f velDelta = newVel - curVel;
  if (velDelta.canBeNormalized()) {
    float t = zeus::clamp(-1.f, velDelta.magnitude() / (rate * dt), 1.f);
    return velDelta.normalized() * rate * dt * t + curVel;
  }
  return newVel;
}

zeus::CVector3f CBallCamera::MoveCollisionActor(const zeus::CVector3f& pos, float dt, CStateManager& mgr) {
  if (const TCastToPtr<CPhysicsActor> act = mgr.ObjectById(x46c_collisionActorId)) {
    const zeus::CVector3f posDelta = pos - act->GetTranslation();
    if (!posDelta.canBeNormalized() || posDelta.magnitude() < 0.01f) {
      act->Stop();
      return act->GetTranslation();
    }
    const zeus::CVector3f oldTranslation = act->GetTranslation();
    const zeus::CVector3f oldVel = act->GetVelocity();
    const zeus::CVector3f newVel = ComputeVelocity(oldVel, posDelta * (1.f / dt));
    act->SetVelocityWR(newVel);
    act->SetMovable(true);
    act->AddMaterial(EMaterialTypes::Solid, mgr);
    CGameCollision::Move(mgr, *act, dt, nullptr);
    zeus::CVector3f posDelta2 = act->GetTranslation() - pos;
    if (posDelta2.canBeNormalized() && posDelta2.magnitude() > 0.1f) {
      act->SetTranslation(oldTranslation);
      act->SetVelocityWR(TweenVelocity(oldVel, newVel, 50.f, dt));
      CGameCollision::Move(mgr, *act, dt, nullptr);
      posDelta2 = act->GetTranslation() - pos;
      if (posDelta2.magnitude() > 0.1f)
        x478_shortMoveCount += 1;
      else
        x478_shortMoveCount = 0;
    } else {
      act->Stop();
      x478_shortMoveCount = 0;
    }
    act->SetMovable(false);
    act->RemoveMaterial(EMaterialTypes::Solid, mgr);
    return act->GetTranslation();
  }
  return pos;
}

void CBallCamera::UpdateUsingFreeLook(float dt, CStateManager& mgr) {
  if (x400_state == EBallCameraState::ToBall || x400_state == EBallCameraState::FromBall) {
    x36c_splineState = ESplineState::Invalid;
    return;
  }

  if (x36c_splineState == ESplineState::Nav && x188_behaviour <= EBallCameraBehaviour::SpindleCamera &&
      x188_behaviour >= EBallCameraBehaviour::HintFixedPosition) {
    x36c_splineState = ESplineState::Invalid;
    return;
  }

  float elevation = x1a0_elevation;
  float distance = x190_curMinDistance;
  ConstrainElevationAndDistance(elevation, distance, 0.f, mgr);

  zeus::CVector3f ballPos = mgr.GetPlayer().GetBallPosition();
  zeus::CVector3f knotToBall = ballPos - x37c_camSpline.GetKnotPosition(2);
  if (knotToBall.canBeNormalized())
    knotToBall.normalize();
  else
    knotToBall = mgr.GetPlayer().GetMoveDir();
  zeus::CVector3f knot3 = x37c_camSpline.GetKnotPosition(3);
  zeus::CVector3f desiredPos = FindDesiredPosition(distance, elevation, knotToBall, mgr, false);

  if (x370_24_reevalSplineEnd)
    x37c_camSpline.SetKnotPosition(3, desiredPos);

  x374_splineCtrl -= dt;

  float splineT = 1.f - zeus::clamp(0.f, x374_splineCtrl / x378_splineCtrlRange, 1.f);
  if (x36c_splineState == ESplineState::Nav) {
    CMaterialList intersectMat;
    if (!SplineIntersectTest(intersectMat, mgr)) {
      x37c_camSpline.SetKnotPosition(3, knot3);
      if (intersectMat.HasMaterial(EMaterialTypes::Floor)) {
        x36c_splineState = ESplineState::Invalid;
        return;
      }
    }
  }

  if (x374_splineCtrl <= 0.f || (splineT > 0.75f && x18c_31_clearLOS)) {
    if (x36c_splineState == ESplineState::Arc && !x18c_31_clearLOS) {
      CMaterialList intersectMat;
      if (!SplineIntersectTest(intersectMat, mgr)) {
        x36c_splineState = ESplineState::Invalid;
      } else {
        zeus::CVector3f oldKnot2 = x37c_camSpline.GetKnotPosition(2);
        zeus::CVector3f oldKnot1 = x37c_camSpline.GetKnotPosition(1);
        BuildSplineArc(mgr);
        x37c_camSpline.SetKnotPosition(3, x37c_camSpline.GetKnotPosition(1));
        x37c_camSpline.SetKnotPosition(2, x37c_camSpline.GetKnotPosition(0));
        x37c_camSpline.SetKnotPosition(1, oldKnot2);
        x37c_camSpline.SetKnotPosition(0, oldKnot1);
        x37c_camSpline.UpdateSplineLength();
        x374_splineCtrl =
            x378_splineCtrlRange - x378_splineCtrlRange * (x37c_camSpline.GetKnotT(2) / x37c_camSpline.x44_length);
        x374_splineCtrl -= dt;
        splineT = zeus::clamp(0.f, x374_splineCtrl / x378_splineCtrlRange, 1.f);
      }
    } else {
      x36c_splineState = ESplineState::Invalid;
    }
  }

  x37c_camSpline.UpdateSplineLength();
  const zeus::CVector3f pos = x37c_camSpline.GetInterpolatedSplinePointByLength(splineT * x37c_camSpline.x44_length).origin;
  if (const TCastToPtr<CPhysicsActor> act = mgr.ObjectById(x46c_collisionActorId)) {
    CMaterialFilter filter = act->GetMaterialFilter();
    CMaterialFilter tmpFilter = filter;
    tmpFilter.IncludeList().Add(EMaterialTypes::Wall);
    tmpFilter.ExcludeList().Add(x3c8_collisionExcludeList);
    act->SetMaterialFilter(tmpFilter);
    MoveCollisionActor(pos, dt, mgr);
    act->SetMaterialFilter(filter);
  }

  zeus::CVector3f lookDir = x1d8_lookPos - desiredPos;
  if (x18d_26_lookAtBall) {
    lookDir = ballPos - desiredPos;
  }

  if (lookDir.canBeNormalized()) {
    lookDir.normalize();
    UpdateTransform(lookDir, desiredPos, dt, mgr);
  }

  TeleportCamera(desiredPos, mgr);

  if (x3d0_24_camBehindFloorOrWall && x374_splineCtrl / x378_splineCtrlRange < 0.5f) {
    x36c_splineState = ESplineState::Invalid;
  }
}

zeus::CVector3f CBallCamera::InterpolateCameraElevation(const zeus::CVector3f& camPos, float dt) {
  if (x1a0_elevation < 2.f)
    return camPos;

  zeus::CVector3f ret = camPos;
  if (!x18c_31_clearLOS && x350_obscuringMaterial.HasMaterial(EMaterialTypes::Floor)) {
    x3d4_elevInterpTimer = 1.f;
    ret.z() = x3d8_elevInterpStart = GetTranslation().z();
  } else if (x3d4_elevInterpTimer > 0.f) {
    x3d4_elevInterpTimer -= dt;
    ret.z() = (camPos.z() - x3d8_elevInterpStart) * (1.f - zeus::clamp(0.f, x3d4_elevInterpTimer, 1.f)) +
              x3d8_elevInterpStart;
  }

  return ret;
}

zeus::CVector3f CBallCamera::CalculateCollidersCentroid(const std::vector<CCameraCollider>& colliderList,
                                                        int numObscured) const {
  if (colliderList.size() < 3)
    return zeus::skForward;

  int clearColliders = 0;
  const CCameraCollider* prevCol = &colliderList.back();
  float accumCross = 0.f;
  float accumX = 0.f;
  float accumZ = 0.f;
  for (const CCameraCollider& col : colliderList) {
    if (prevCol->x4c_occlusionCount < 2 && col.x4c_occlusionCount < 2) {
      float z0 = prevCol->x50_scale * prevCol->x8_lastLocalPos.z();
      float x1 = prevCol->x50_scale * col.x8_lastLocalPos.x();
      float x0 = prevCol->x50_scale * prevCol->x8_lastLocalPos.x();
      float z1 = prevCol->x50_scale * col.x8_lastLocalPos.z();

      float cross = x0 * z1 - x1 * z0;
      accumCross += cross;
      accumX += cross * (x1 + x0);
      accumZ += cross * (z1 + z0);
    } else {
      clearColliders += 1;
    }
    prevCol = &col;
  }

  if (clearColliders / float(colliderList.size()) <= x330_clearColliderThreshold) {
    return zeus::skForward;
  } else if (0.f != accumCross) {
    float baryCross = 3.f * accumCross;
    return {accumX / baryCross, 0.f, accumZ / baryCross};
  }

  return {0.f, 2.f, 0.f};
}

zeus::CVector3f CBallCamera::ApplyColliders() {
  zeus::CVector3f smallCentroid = CalculateCollidersCentroid(x264_smallColliders, x2c4_smallCollidersObsCount);
  zeus::CVector3f mediumCentroid = CalculateCollidersCentroid(x274_mediumColliders, x2c8_mediumCollidersObsCount);
  zeus::CVector3f largeCentroid = CalculateCollidersCentroid(x284_largeColliders, x2cc_largeCollidersObsCount);

  if (smallCentroid.y() == 0.f)
    x2a0_smallCentroid = smallCentroid;
  else
    x2a0_smallCentroid = zeus::skZero3f;

  float centroidX = x2a0_smallCentroid.x();
  float centroidZ = x2a0_smallCentroid.z();

  if (mediumCentroid.y() == 0.f)
    x2ac_mediumCentroid = mediumCentroid;
  else
    x2ac_mediumCentroid = zeus::skZero3f;

  centroidX += x2ac_mediumCentroid.x();
  centroidZ += x2ac_mediumCentroid.z();

  if (largeCentroid.y() == 0.f)
    x2b8_largeCentroid = largeCentroid;
  else
    x2b8_largeCentroid = zeus::skZero3f;

  centroidX += x2b8_largeCentroid.x();
  centroidZ += x2b8_largeCentroid.z();

  if (x18c_31_clearLOS)
    centroidX /= 1.5f;
  centroidZ /= 3.f;

  if (!x18c_31_clearLOS && x368_obscuringObjectId == kInvalidUniqueId) {
    float xMul = 1.5f;
    float zMul = 1.f;
    if (x350_obscuringMaterial.HasMaterial(EMaterialTypes::Floor))
      zMul += 2.f * x358_unobscureMag;
    if (x350_obscuringMaterial.HasMaterial(EMaterialTypes::Wall))
      xMul += 3.f * zeus::clamp(0.f, x358_unobscureMag - 0.25f, 1.f);
    centroidX *= xMul;
    centroidZ *= zMul;
  }

  if (!x18c_28_volumeCollider)
    return zeus::skZero3f;

  if (std::fabs(centroidX) < 0.05f)
    centroidX = 0.f;
  if (std::fabs(centroidZ) < 0.05f)
    centroidZ = 0.f;

  if (x18c_31_clearLOS)
    centroidZ *= 0.5f;

  return {centroidX, 0.f, centroidZ};
}

void CBallCamera::UpdateColliders(const zeus::CTransform& xf, std::vector<CCameraCollider>& colliderList, int& it,
                                  int count, float tolerance, const rstl::reserved_vector<TUniqueId, 1024>& nearList,
                                  float dt, CStateManager& mgr) {
  if (it < colliderList.size()) {
    x310_idealLookVec = {0.f, g_tweakBall->GetBallCameraOffset().y(), g_tweakPlayer->GetPlayerBallHalfExtent()};
    x310_idealLookVec.y() *= x308_speedFactor;
    x31c_predictedLookPos = mgr.GetPlayer().GetMoveDir() * x310_idealLookVec.y();
    x31c_predictedLookPos.z() = float(x310_idealLookVec.z());
    x31c_predictedLookPos += mgr.GetPlayer().GetTranslation();
    zeus::CTransform predictedLookXf = zeus::lookAt(xf.origin, x31c_predictedLookPos);
    float toleranceRecip = 1.f / tolerance;
    for (int i = 0; i < count; ++i) {
      zeus::CVector3f localPos = colliderList[it].x14_localPos;
      zeus::CVector3f worldPos = predictedLookXf.rotate(localPos) + predictedLookXf.origin;
      if ((colliderList[it].x2c_lastWorldPos - worldPos).magnitude() < 0.1f) {
        localPos = colliderList[it].x8_lastLocalPos;
        worldPos = colliderList[it].x2c_lastWorldPos;
      }
      zeus::CVector3f centerToCollider = worldPos - predictedLookXf.origin;
      float mag = centerToCollider.magnitude();
      if (centerToCollider.canBeNormalized()) {
        centerToCollider.normalize();
        TUniqueId intersectId = kInvalidUniqueId;
        CRayCastResult result = mgr.RayWorldIntersection(intersectId, predictedLookXf.origin, centerToCollider,
                                                         mag + colliderList[it].x4_radius, BallCameraFilter, nearList);
        if (result.IsValid()) {
          zeus::CVector3f centerToPoint = centerToCollider * (result.GetT() - colliderList[it].x4_radius);
          worldPos = centerToPoint + predictedLookXf.origin;
          localPos = predictedLookXf.getRotation().inverse() * centerToPoint;
        }
      }
      colliderList[it].x2c_lastWorldPos = worldPos;
      colliderList[it].x8_lastLocalPos = localPos;
      zeus::CVector3f scaledWorldColliderPos = centerToCollider * mag * toleranceRecip;
      scaledWorldColliderPos = scaledWorldColliderPos * x308_speedFactor + x31c_predictedLookPos;
      colliderList[it].x20_scaledWorldPos = scaledWorldColliderPos;
      if (mgr.RayCollideWorld(worldPos, scaledWorldColliderPos, nearList, BallCameraFilter, nullptr))
        colliderList[it].x4c_occlusionCount = 0;
      else
        colliderList[it].x4c_occlusionCount += 1;
      it += 1;
      if (it == colliderList.size())
        it = 0;
    }
  }
}

zeus::CVector3f CBallCamera::AvoidGeometry(const zeus::CTransform& xf,
                                           const rstl::reserved_vector<TUniqueId, 1024>& nearList, float dt,
                                           CStateManager& mgr) {
  switch (x328_avoidGeomCycle) {
  case 0:
    UpdateColliders(xf, x264_smallColliders, x2d0_smallColliderIt, 1, 4.f, nearList, dt, mgr);
    break;
  case 1:
    UpdateColliders(xf, x274_mediumColliders, x2d4_mediumColliderIt, 3, 4.f, nearList, dt, mgr);
    break;
  case 2:
  case 3:
    UpdateColliders(xf, x284_largeColliders, x2d8_largeColliderIt, 4, 4.f, nearList, dt, mgr);
    break;
  default:
    break;
  }

  x328_avoidGeomCycle += 1;
  if (x328_avoidGeomCycle >= 4)
    x328_avoidGeomCycle = 0;

  return ApplyColliders();
}

zeus::CVector3f CBallCamera::AvoidGeometryFull(const zeus::CTransform& xf,
                                               const rstl::reserved_vector<TUniqueId, 1024>& nearList, float dt,
                                               CStateManager& mgr) {
  UpdateColliders(xf, x264_smallColliders, x2d0_smallColliderIt, x264_smallColliders.size(), 4.f, nearList, dt, mgr);
  UpdateColliders(xf, x274_mediumColliders, x2d4_mediumColliderIt, x274_mediumColliders.size(), 4.f, nearList, dt, mgr);
  UpdateColliders(xf, x284_largeColliders, x2d8_largeColliderIt, x284_largeColliders.size(), 4.f, nearList, dt, mgr);
  return ApplyColliders();
}

zeus::CAABox CBallCamera::CalculateCollidersBoundingBox(const std::vector<CCameraCollider>& colliderList,
                                                        CStateManager& mgr) const {
  zeus::CAABox aabb;
  for (const CCameraCollider& col : colliderList)
    aabb.accumulateBounds(col.x2c_lastWorldPos);
  aabb.accumulateBounds(mgr.GetPlayer().GetTranslation());
  return aabb;
}

int CBallCamera::CountObscuredColliders(const std::vector<CCameraCollider>& colliderList) const {
  int ret = 0;
  for (const CCameraCollider& c : colliderList)
    if (c.x4c_occlusionCount >= 2)
      ++ret;
  return ret;
}

void CBallCamera::UpdateCollidersDistances(std::vector<CCameraCollider>& colliderList, float xMag, float zMag,
                                           float angOffset) {
  float theta = angOffset;
  for (CCameraCollider& col : colliderList) {
    float z = std::cos(theta) * zMag;
    if (theta > M_PIF / 2.f)
      z *= 0.25f;
    col.x14_localPos = {std::sin(theta) * xMag, 0.f, z};
    theta += 2.f * M_PIF / float(colliderList.size());
  }
}

void CBallCamera::UpdateUsingColliders(float dt, CStateManager& mgr) {
  if (mgr.GetPlayer().GetBombJumpCount() == 1)
    return;

  zeus::CVector3f ballPos = mgr.GetPlayer().GetBallPosition();

  if (mgr.GetPlayer().GetBombJumpCount() == 2) {
    zeus::CVector3f camToLookDir = x1d8_lookPos - GetTranslation();
    if (x18d_26_lookAtBall)
      camToLookDir = ballPos - GetTranslation();

    if (camToLookDir.canBeNormalized()) {
      camToLookDir.normalize();
      UpdateTransform(camToLookDir, GetTranslation(), dt, mgr);
    }
  } else if (mgr.GetPlayer().GetMorphballTransitionState() != CPlayer::EPlayerMorphBallState::Unmorphed ||
             x18d_25_avoidGeometryFull) {
    //zeus::CTransform oldXf = x34_transform;
    zeus::CVector3f oldPos = GetTranslation();
    x2c4_smallCollidersObsCount = CountObscuredColliders(x264_smallColliders);
    x2c8_mediumCollidersObsCount = CountObscuredColliders(x274_mediumColliders);
    x2cc_largeCollidersObsCount = CountObscuredColliders(x284_largeColliders);
    zeus::CVector3f posAtBallLevel = {0.f, 0.f, GetTranslation().z() - ballPos.z()};
    zeus::CVector3f ballToCamFlat = GetTranslation() - ballPos;
    ballToCamFlat.z() = 0.f;
    float ballToCamFlatMag = 0.f;
    if (ballToCamFlat.canBeNormalized())
      ballToCamFlatMag = ballToCamFlat.magnitude();
    else
      ballToCamFlat = -mgr.GetPlayer().GetMoveDir();
    posAtBallLevel = GetTranslation() - posAtBallLevel;
    zeus::CTransform ballToUnderCamLook;
    if ((posAtBallLevel - ballPos).canBeNormalized())
      ballToUnderCamLook = zeus::lookAt(ballPos, posAtBallLevel);
    float distance =
        x214_ballCameraSpring.ApplyDistanceSpring(x190_curMinDistance, ballToCamFlatMag, (3.f + x308_speedFactor) * dt);
    zeus::CVector3f camToBall = ballPos - GetTranslation();
    camToBall.z() = 0.f;
    if (camToBall.canBeNormalized()) {
      camToBall.normalize();
      if (std::fabs(std::acos(zeus::clamp(-1.f, camToBall.dot(mgr.GetPlayer().GetMoveDir()), 1.f))) >
              zeus::degToRad(150.f) &&
          mgr.GetPlayer().GetVelocity().canBeNormalized()) {
        distance = x214_ballCameraSpring.ApplyDistanceSpring(
            x308_speedFactor * (x19c_backwardsDistance - x190_curMinDistance) + x190_curMinDistance, ballToCamFlatMag,
            3.f * dt);
      }
    }
    x334_collidersAABB = CalculateCollidersBoundingBox(x284_largeColliders, mgr);
    rstl::reserved_vector<TUniqueId, 1024> nearList;
    mgr.BuildNearList(nearList, x334_collidersAABB, BallCameraFilter,
                      TCastToConstPtr<CActor>(mgr.GetObjectById(x46c_collisionActorId)).GetPtr());
    if (!x18c_31_clearLOS && x368_obscuringObjectId == kInvalidUniqueId) {
      if (x34c_obscuredTime > 0.f || x350_obscuringMaterial.HasMaterial(EMaterialTypes::Floor) ||
          x350_obscuringMaterial.HasMaterial(EMaterialTypes::Wall)) {
        x32c_colliderMag += 2.f * dt;
        if (x32c_colliderMag < 2.f)
          x32c_colliderMag = 2.f;
        if (x32c_colliderMag > 2.f)
          x32c_colliderMag = 2.f;
        UpdateCollidersDistances(x264_smallColliders, 2.31f * x32c_colliderMag, 2.31f * x32c_colliderMag * 0.5f,
                                 -M_PIF / 2.f);
        UpdateCollidersDistances(x274_mediumColliders, 4.62f * x32c_colliderMag, 4.62f * x32c_colliderMag * 0.5f,
                                 -M_PIF / 2.f);
        UpdateCollidersDistances(x284_largeColliders, 7.f * x32c_colliderMag, 7.f * x32c_colliderMag * 0.5f,
                                 -M_PIF / 2.f);
      }
    } else {
      float targetColliderMag = 1.f;
      if (x18d_24_prevClearLOS && mgr.GetPlayer().GetMoveSpeed() < 1.f)
        targetColliderMag = 0.25f;
      x32c_colliderMag += (targetColliderMag - x32c_colliderMag) * dt * 2.f;
      UpdateCollidersDistances(x264_smallColliders, x32c_colliderMag * 2.31f, x32c_colliderMag * 2.31f, -M_PIF / 2.f);
      UpdateCollidersDistances(x274_mediumColliders, x32c_colliderMag * 4.62f, x32c_colliderMag * 4.62f, -M_PIF / 2.f);
      UpdateCollidersDistances(x284_largeColliders, x32c_colliderMag * 7.f, x32c_colliderMag * 7.f, -M_PIF / 2.f);
    }

    float elevation = x1a0_elevation;
    bool noDoor = !ConstrainElevationAndDistance(elevation, distance, dt, mgr);
    zeus::CVector3f desiredBallToCam = ballToUnderCamLook.rotate({0.f, distance, elevation});

    if (TCastToConstPtr<CScriptDoor> door = mgr.GetObjectById(x3dc_tooCloseActorId)) {
      if (!door->x2a8_26_isOpen) {
        if (x400_state == EBallCameraState::Boost) {
          zeus::CVector3f ballToCam = GetTranslation() - ballPos;
          if (ballToCam.canBeNormalized())
            ballToCam.normalize();
          else
            ballToCam = GetTransform().basis[1];
          if (std::fabs(ballToCamFlatMag - x430_boostElevation) < 1.f) {
            ballToCam = ConstrainYawAngle(mgr.GetPlayer(), g_tweakBall->GetBallCameraBoostDistance(),
                                          g_tweakBall->GetBallCameraBoostYawSpeed(), dt, mgr);
          }
          ballToCam.normalize();
          ballToCam.z() = 0.f;
          ballToCam = ballToCam * distance;
          ballToCam.z() = 1.f;
          desiredBallToCam = ballToCam;
          noDoor = false;
        }
        if (x18c_25_chaseAllowed &&
            (x400_state == EBallCameraState::Chase || x188_behaviour == EBallCameraBehaviour::FreezeLookPosition)) {
          zeus::CVector3f ballToCam = GetTranslation() - ballPos;
          if (ballToCam.canBeNormalized())
            ballToCam.normalize();
          else
            ballToCam = GetTransform().basis[1];
          if (std::fabs(ballToCamFlatMag - x404_chaseElevation) < 3.f) {
            ballToCam = ConstrainYawAngle(mgr.GetPlayer(), g_tweakBall->GetBallCameraChaseDistance(),
                                          g_tweakBall->GetBallCameraChaseYawSpeed(), dt, mgr);
          }
          ballToCam.z() = 0.f;
          ballToCam.normalize();
          ballToCam = ballToCam * distance;
          ballToCam.z() = g_tweakBall->GetBallCameraElevation();
          desiredBallToCam = ballToCam;
          noDoor = false;
        }
      }
    }

    if (x188_behaviour == EBallCameraBehaviour::HintBallToCam) {
      desiredBallToCam = x45c_overrideBallToCam;
      if (x18c_27_obscureAvoidance) {
        zeus::CVector3f ballToCamDir = x45c_overrideBallToCam;
        if (ballToCamDir.canBeNormalized())
          ballToCamDir.normalize();
        else
          ballToCamDir = -mgr.GetPlayer().GetMoveDir();
        TUniqueId intersectId = kInvalidUniqueId;
        CRayCastResult result =
            mgr.RayWorldIntersection(intersectId, ballPos, ballToCamDir, distance, BallCameraFilter, nearList);
        if (result.IsValid())
          desiredBallToCam = ballToCamDir * result.GetT() * 0.9f;
      }
      noDoor = false;
    }

    distance = desiredBallToCam.magnitude();
    zeus::CVector3f desiredCamPos = ballPos + desiredBallToCam;
    float d = 0.f;
    if (DetectCollision(ballPos, desiredCamPos, 0.3f, d, mgr)) {
      if (d >= 1.f) {
        desiredBallToCam = desiredBallToCam.normalized() * d;
        desiredCamPos = ballPos + desiredBallToCam;
      } else {
        desiredBallToCam = ballPos + GetTranslation();
        desiredCamPos = GetTranslation();
      }
    }

    zeus::CTransform lookXf = zeus::lookAt(desiredCamPos, x1d8_lookPos);
    zeus::CTransform oldLookXf = zeus::lookAt(GetTranslation(), x1d8_lookPos);
    x1e4_nextLookXf = lookXf;
    lookXf = oldLookXf;
    zeus::CVector3f colliderPointLocal;
    if (x18d_25_avoidGeometryFull || !x18c_31_clearLOS)
      colliderPointLocal = AvoidGeometryFull(lookXf, nearList, dt, mgr);
    else
      colliderPointLocal = AvoidGeometry(lookXf, nearList, dt, mgr);

    zeus::CVector3f ballToCam2 = GetTranslation() - ballPos;
    ballToCam2.z() = 0.f;
    if (ballToCam2.magnitude() < 2.f) {
      if (x18c_31_clearLOS && x478_shortMoveCount > 2)
        colliderPointLocal = colliderPointLocal / float(x478_shortMoveCount);
      if (d < 3.f) {
        colliderPointLocal = colliderPointLocal * 0.25f;
        if (x18c_31_clearLOS && x478_shortMoveCount > 0)
          colliderPointLocal = colliderPointLocal * x308_speedFactor;
      }
      if (d < 1.f)
        colliderPointLocal = zeus::skZero3f;
    }

    zeus::CVector3f camDelta = lookXf.rotate(colliderPointLocal) + desiredCamPos - ballPos;
    if (camDelta.canBeNormalized())
      camDelta.normalize();
    zeus::CVector3f desiredPos = camDelta * distance + ballPos;

    if (x188_behaviour == EBallCameraBehaviour::PathCameraDesiredPos)
      if (TCastToConstPtr<CPathCamera> cam = mgr.GetObjectById(mgr.GetCameraManager()->GetPathCameraId()))
        desiredPos = cam->GetTranslation();

    camDelta = x294_dampedPos - desiredPos;
    float camDeltaMag = camDelta.magnitude();
    if (camDelta.canBeNormalized())
      camDelta.normalize();

    x294_dampedPos = camDelta * x228_ballCameraCentroidSpring.ApplyDistanceSpring(0.f, camDeltaMag, dt) + desiredPos;
    zeus::CVector3f posDelta = oldPos - x294_dampedPos;
    camDeltaMag = posDelta.magnitude();
    if (posDelta.canBeNormalized())
      posDelta.normalize();

    float cDistSpringMag = x250_ballCameraCentroidDistanceSpring.ApplyDistanceSpring(
        0.f, camDeltaMag, (x18d_28_obtuseDirection ? 3.f : 1.f) * dt);
    if (x400_state == EBallCameraState::Boost)
      cDistSpringMag = x448_ballCameraBoostSpring.ApplyDistanceSpring(0.f, camDeltaMag, dt);
    else if (x18c_25_chaseAllowed &&
             (x400_state == EBallCameraState::Chase || x188_behaviour == EBallCameraBehaviour::FreezeLookPosition))
      cDistSpringMag = x41c_ballCameraChaseSpring.ApplyDistanceSpring(0.f, camDeltaMag, dt);

    zeus::CVector3f finalPos = posDelta * cDistSpringMag + x294_dampedPos;
    if (mgr.GetPlayer().GetMorphBall()->GetSpiderBallState() != CMorphBall::ESpiderBallState::Active &&
        !x18e_24_noElevationVelClamp && mgr.GetPlayer().GetVelocity().z() > 8.f) {
      zeus::CVector3f delta = finalPos - oldPos;
      delta.z() = zeus::clamp(-0.1f * dt, float(delta.z()), 0.1f * dt);
      finalPos = oldPos + delta;
    }

    if (noDoor && x400_state != EBallCameraState::ToBall)
      finalPos = InterpolateCameraElevation(finalPos, dt);

    if (x18d_29_noElevationInterp)
      finalPos.z() = elevation + ballPos.z();

    if (ballToCam2.magnitude() < 2.f) {
      if (finalPos.z() < 2.f + ballPos.z())
        finalPos.z() = 2.f + ballPos.z();
      x214_ballCameraSpring.Reset();
    }

    finalPos = ClampElevationToWater(finalPos, mgr);
    if (ballToCam2.magnitude() < 2.f && x3dc_tooCloseActorId != kInvalidUniqueId && x3e0_tooCloseActorDist < 5.f)
      if (TCastToConstPtr<CScriptDoor> door = mgr.GetObjectById(x3dc_tooCloseActorId))
        if (!door->x2a8_26_isOpen)
          finalPos = GetTranslation();

    float backupZ = finalPos.z();
    finalPos = MoveCollisionActor(finalPos, dt, mgr);

    if (x18c_31_clearLOS && x478_shortMoveCount > 0) {
      finalPos.z() = backupZ;
      finalPos = MoveCollisionActor(finalPos, dt, mgr);
    }

    zeus::CVector3f lookDir = x1d8_lookPos - finalPos;
    if (x18d_26_lookAtBall)
      lookDir = ballPos - finalPos;
    if (lookDir.canBeNormalized()) {
      lookDir.normalize();
      UpdateTransform(lookDir, finalPos, dt, mgr);
    }

    if (x470_clampVelTimer > 0.f)
      x470_clampVelTimer -= dt;
  }
}

void CBallCamera::UpdateUsingSpindleCameras(float dt, CStateManager& mgr) {
  if (const TCastToConstPtr<CScriptSpindleCamera> cam = mgr.ObjectById(mgr.GetCameraManager()->GetSpindleCameraId())) {
    TeleportCamera(cam->GetTransform(), mgr);
    x18d_26_lookAtBall = true;
  }
}

zeus::CVector3f CBallCamera::ClampElevationToWater(zeus::CVector3f& pos, CStateManager& mgr) const {
  zeus::CVector3f ret = pos;
  if (const TCastToConstPtr<CScriptWater> water = mgr.GetObjectById(mgr.GetPlayer().GetFluidId())) {
    const float waterZ = water->GetTriggerBoundsWR().max.z();
    if (pos.z() >= waterZ && pos.z() - waterZ <= 0.25f) {
      ret.z() = 0.25f + waterZ;
    } else if (pos.z() < waterZ && pos.z() - waterZ >= -0.12f) {
      ret.z() = waterZ - 0.12f;
    }
  }
  return ret;
}

void CBallCamera::UpdateTransitionFromBallCamera(CStateManager& mgr) {
  float morphFactor = mgr.GetPlayer().GetMorphFactor();
  zeus::CVector3f eyePos = mgr.GetPlayer().GetEyePosition();
  zeus::CVector3f delta = mgr.GetPlayer().GetTranslation() - x47c_failsafeState->x84_playerPos;
  x47c_failsafeState->x90_splinePoints[1] += delta;
  x47c_failsafeState->x90_splinePoints[2] += delta;
  x47c_failsafeState->x90_splinePoints[3] += delta;
  zeus::CVector3f splinePoint = GetFailsafeSplinePoint(x47c_failsafeState->x90_splinePoints, morphFactor);
  splinePoint.z() = (splinePoint.z() - eyePos.z()) * zeus::clamp(0.f, 1.f - 1.5f * morphFactor, 1.f) + eyePos.z();
  zeus::CVector3f deltaFlat = eyePos - splinePoint;
  deltaFlat.z() = 0.f;
  if (deltaFlat.magnitude() > 0.001f) {
    SetTransform(zeus::lookAt(splinePoint, zeus::CVector3f::lerp(x1d8_lookPos, eyePos, morphFactor)));
  } else {
    SetTransform(mgr.GetCameraManager()->GetFirstPersonCamera()->GetTransform());
    SetTranslation(splinePoint);
  }
  mgr.GetCameraManager()->GetFirstPersonCamera()->Reset(x34_transform, mgr);
  x47c_failsafeState->x84_playerPos = mgr.GetPlayer().GetTranslation();
}

void CBallCamera::UpdateUsingTransitions(float dt, CStateManager& mgr) {
  if (x400_state == EBallCameraState::FromBall) {
    UpdateTransitionFromBallCamera(mgr);
    return;
  }

  x18d_26_lookAtBall = false;
  zeus::CVector3f ballPos = mgr.GetPlayer().GetBallPosition();
  zeus::CVector3f eyePos = mgr.GetPlayer().GetEyePosition();
  ballPos.z() += x1b4_lookAtOffset.z();

  zeus::CVector3f lookDir = x34_transform.basis[1];
  zeus::CTransform xe8 = x34_transform;

  switch (x400_state) {
  case EBallCameraState::ToBall: {
    float elevation = x1a0_elevation;
    float distance = x194_targetMinDistance;
    ConstrainElevationAndDistance(elevation, distance, dt, mgr);
    distance = x194_targetMinDistance;
    const bool r28 = IsBallNearDoor(GetTranslation(), mgr) || x478_shortMoveCount > 2;
    const zeus::CVector3f toDesired =
        FindDesiredPosition(distance, elevation, mgr.GetPlayer().GetMoveDir(), mgr, r28) - eyePos;
    zeus::CVector3f finalPos = toDesired * mgr.GetPlayer().GetMorphFactor() + eyePos;
    if (const TCastToPtr<CPhysicsActor> act = mgr.ObjectById(x46c_collisionActorId)) {
      act->SetTranslation(GetTranslation());
      finalPos = ClampElevationToWater(finalPos, mgr);
      finalPos = MoveCollisionActor(finalPos, dt, mgr);
      zeus::CVector3f camToLookDir = x1d8_lookPos - finalPos;
      if (camToLookDir.canBeNormalized()) {
        camToLookDir.normalize();
        const float devDot = std::fabs(zeus::clamp(-1.f, lookDir.dot(camToLookDir), 1.f));
        const float devAngle = zeus::clamp(-1.f, mgr.GetPlayer().GetMorphFactor() * 0.5f, 1.f) * std::acos(devDot);
        if (devDot < 1.f) {
          SetTransform(zeus::CQuaternion::lookAt(xe8.basis[1], camToLookDir, devAngle).toTransform() *
                       xe8.getRotation());
        } else {
          SetTransform(zeus::lookAt(zeus::skZero3f, camToLookDir));
        }
      }
    }
    SetTransform(ValidateCameraTransform(x34_transform, xe8));
    SetTranslation(finalPos);
    TeleportCamera(finalPos, mgr);
    break;
  }
  case EBallCameraState::FromBall: {
    if (std::fabs(mgr.GetPlayer().GetMorphFactor() - 1.f) < 0.00001f) {
      SetTransform(mgr.GetPlayer().GetTransform());
      SetTranslation(mgr.GetPlayer().GetEyePosition());
    } else {
      float morphT = zeus::clamp(-1.f, mgr.GetPlayer().GetMorphFactor() / 0.9f, 1.f);
      zeus::CVector3f finalPos = GetTranslation();
      zeus::CVector3f eyeToCam = GetTranslation() - eyePos;
      if (eyeToCam.canBeNormalized()) {
        float distance = eyeToCam.magnitude();
        distance = std::min(distance, (1.f - mgr.GetPlayer().GetMorphFactor()) * x190_curMinDistance);
        float yawSpeed = M_PIF;
        zeus::CVector3f playerToCamDir = GetTranslation() - mgr.GetPlayer().GetTranslation();
        zeus::CVector3f moveDir = mgr.GetPlayer().GetMoveDir();
        if (playerToCamDir.canBeNormalized())
          playerToCamDir.normalize();
        else
          playerToCamDir = -moveDir;
        if (moveDir.canBeNormalized()) {
          moveDir.normalize();
          yawSpeed = std::fabs(std::acos(zeus::clamp(-1.f, playerToCamDir.dot(-moveDir), 1.f))) * morphT / dt;
        }
        zeus::CVector3f useLookDir = ConstrainYawAngle(mgr.GetPlayer(), yawSpeed, zeus::degToRad(10.f), dt, mgr);
        useLookDir.z() = 0.f;
        useLookDir.normalize();
        zeus::CVector3f camPos = useLookDir * distance + eyePos;
        camPos.z() = (GetTranslation().z() - eyePos.z()) * morphT + eyePos.z();
        finalPos = ClampElevationToWater(camPos, mgr);
        finalPos = MoveCollisionActor(finalPos, dt, mgr);
        zeus::CVector3f finalToBall = ballPos - finalPos;
        finalToBall.z() = 0.f;
        zeus::CVector3f lookPos = ballPos;
        lookPos.z() = morphT * (eyePos.z() - ballPos.z()) + ballPos.z();
        if (finalToBall.canBeNormalized())
          SetTransform(zeus::lookAt(finalPos, lookPos));
        else
          SetTransform(mgr.GetCameraManager()->GetFirstPersonCamera()->GetTransform());
      } else {
        SetTransform(mgr.GetCameraManager()->GetFirstPersonCamera()->GetTransform());
      }
      SetTranslation(finalPos);
    }
    break;
  }
  default:
    break;
  }

  mgr.GetCameraManager()->GetFirstPersonCamera()->Reset(x34_transform, mgr);
}

zeus::CTransform CBallCamera::UpdateCameraPositions(float dt, const zeus::CTransform& oldXf,
                                                    const zeus::CTransform& newXf) {
  zeus::CTransform useXf = newXf;
  if (std::fabs(oldXf.basis[1].z()) > 0.9f && std::fabs(newXf.basis[1].z()) > 0.9f &&
      oldXf.basis[0].dot(newXf.basis[0]) <= 0.999f) {
    zeus::CVector3f newRight =
        zeus::CQuaternion::clampedRotateTo(oldXf.basis[0], newXf.basis[0], zeus::degToRad(2.f * dt)).toTransform() *
        oldXf.basis[0];
    if (newRight.dot(newXf.basis[1]) <= 0.999f) {
      zeus::CVector3f newUp = newXf.basis[1].cross(newRight).normalized();
      zeus::CVector3f newForward = newXf.basis[1].normalized();
      useXf = {newUp.cross(newForward), newXf.basis[1], newUp, newXf.origin};
    }
  }
  return useXf;
}

zeus::CVector3f CBallCamera::GetFailsafeSplinePoint(const std::vector<zeus::CVector3f>& points, float t) {
  t *= (points.size() - 3);
  int baseIdx = 0;
  while (t > 1.f) {
    t -= 1.f;
    baseIdx += 1;
  }
  return zeus::getBezierPoint(points[baseIdx], points[baseIdx + 1], points[baseIdx + 2], points[baseIdx + 3], t);
}

bool CBallCamera::CheckFailsafeFromMorphBallState(CStateManager& mgr) const {
  TUniqueId xbb8 = kInvalidUniqueId;
  float curT = 0.f;
  rstl::reserved_vector<TUniqueId, 1024> nearList;
  rstl::reserved_vector<CRayCastResult, 6> resultsA;
  rstl::reserved_vector<CRayCastResult, 6> resultsB;
  while (curT < 6.f) {
    zeus::CVector3f pointA = GetFailsafeSplinePoint(x47c_failsafeState->x90_splinePoints, curT / 6.f);
    zeus::CVector3f pointB = GetFailsafeSplinePoint(x47c_failsafeState->x90_splinePoints, (1.f + curT) / 6.f);
    zeus::CVector3f pointDelta = pointB - pointA;
    if (pointDelta.magnitude() > 0.1f) {
      resultsA.push_back(mgr.RayWorldIntersection(xbb8, pointA, pointDelta.normalized(), pointDelta.magnitude(),
                                                  BallCameraFilter, nearList));
      resultsB.push_back(mgr.RayWorldIntersection(xbb8, pointB, -pointDelta.normalized(), pointDelta.magnitude(),
                                                  BallCameraFilter, nearList));
    } else {
      resultsA.push_back({});
      resultsB.push_back({});
    }
    curT += 1.f;
  }
  for (size_t i = 0; i < resultsA.size(); ++i) {
    const CRayCastResult& resA = resultsA[i];
    const CRayCastResult& resB = resultsB[i];
    if (resA.IsValid()) {
      zeus::CVector3f separation = resA.GetPoint() - resB.GetPoint();
      if (separation.magnitude() < 0.00001f)
        separation = GetFailsafeSplinePoint(x47c_failsafeState->x90_splinePoints, (1.f + i) / 6.f) - resA.GetPoint();
      if (separation.magnitude() > 0.3f)
        return false;
    }
  }
  return true;
}

bool CBallCamera::SplineIntersectTest(CMaterialList& intersectMat, CStateManager& mgr) const {
  rstl::reserved_vector<TUniqueId, 1024> nearList;
  TUniqueId xe38 = kInvalidUniqueId;
  rstl::reserved_vector<CRayCastResult, 12> xacc;
  rstl::reserved_vector<CRayCastResult, 12> xd10;
  constexpr auto filter =
      CMaterialFilter::MakeIncludeExclude({EMaterialTypes::Solid, EMaterialTypes::Floor, EMaterialTypes::Wall},
                                          {EMaterialTypes::ProjectilePassthrough, EMaterialTypes::Player,
                                           EMaterialTypes::Character, EMaterialTypes::CameraPassthrough});
  float curT = 0.f;
  while (curT < 12.f) {
    zeus::CVector3f xdb0 = x37c_camSpline.GetInterpolatedSplinePointByTime(curT, 12.f);
    zeus::CVector3f xdbc = x37c_camSpline.GetInterpolatedSplinePointByTime(curT, 12.f);
    zeus::CVector3f xdc8 = xdbc - xdb0;
    if (xdc8.magnitude() > 0.1f) {
      xacc.push_back(mgr.RayWorldIntersection(xe38, xdb0, xdc8.normalized(), xdc8.magnitude(), filter, nearList));
      xd10.push_back(mgr.RayWorldIntersection(xe38, xdbc, -xdc8.normalized(), xdc8.magnitude(), filter, nearList));
    } else {
      xacc.push_back({});
      xd10.push_back({});
    }
    curT += 1.f;
  }
  for (size_t i = 0; i < xacc.size(); ++i) {
    const CRayCastResult& resA = xacc[i];
    const CRayCastResult& resB = xd10[i];
    if (resA.IsValid()) {
      zeus::CVector3f xdd4 = resA.GetPoint() - resB.GetPoint();
      if (xdd4.magnitude() < 0.00001f)
        xdd4 = x37c_camSpline.GetInterpolatedSplinePointByTime(1.f + i, 12.f) - resA.GetPoint();
      if (xdd4.magnitude() > 0.3f) {
        intersectMat = resA.GetMaterial();
        return false;
      }
    }
  }
  return true;
}

bool CBallCamera::IsBallNearDoor(const zeus::CVector3f& pos, CStateManager& mgr) {
  const TCastToConstPtr<CScriptDoor> door =
      mgr.GetObjectById(mgr.GetCameraManager()->GetBallCamera()->x3dc_tooCloseActorId);
  if (!door || door->x2a8_26_isOpen) {
    return false;
  }

  const auto tb = door->GetTouchBounds();
  const zeus::CAABox testAABB(pos - 0.3f, pos + 0.3f);
  if (!tb || !tb->intersects(testAABB)) {
    return false;
  }

  if (const TCastToConstPtr<CScriptDock> dock = mgr.GetObjectById(door->x282_dockId)) {
    if (std::fabs(dock->GetPlane(mgr).pointToPlaneDist(pos)) < 1.15f) {
      return true;
    }
  }

  return false;
}

void CBallCamera::ActivateFailsafe(float dt, CStateManager& mgr) {
  float elevation = x1a0_elevation;
  float distance = x194_targetMinDistance;
  ConstrainElevationAndDistance(elevation, distance, dt, mgr);
  zeus::CVector3f desiredPos = FindDesiredPosition(distance, elevation, mgr.GetPlayer().GetMoveDir(), mgr, true);
  SetTranslation(desiredPos);
  ResetPosition(mgr);
  TeleportCamera(zeus::lookAt(desiredPos, x1d8_lookPos), mgr);
  mgr.GetCameraManager()->SetPlayerCamera(mgr, GetUniqueId());
  x3e4_pendingFailsafe = false;
  x34c_obscuredTime = 0.f;
}

bool CBallCamera::ConstrainElevationAndDistance(float& elevation, float& distance, float dt, CStateManager& mgr) {
  zeus::CVector3f ballToCam = GetTranslation() - mgr.GetPlayer().GetBallPosition();
  float ballToCamMag = 0.f;
  if (ballToCam.canBeNormalized())
    ballToCamMag = ballToCam.toVec2f().magnitude();
  else
    ballToCam = -mgr.GetPlayer().GetMoveDir();

  bool doorClose = false;
  float stretchFac = 1.f;
  float newDistance = distance;
  float baseElevation = elevation;
  float springSpeed = 1.f;
  if (const TCastToConstPtr<CScriptDoor> door = mgr.GetObjectById(x3dc_tooCloseActorId)) {
    if (!door->x2a8_29_ballDoor) {
      stretchFac = zeus::clamp(-1.f, std::fabs(x3e0_tooCloseActorDist / (3.f * distance)), 1.f);
      if (x3e0_tooCloseActorDist < 3.f * distance) {
        doorClose = true;
      }
      if (door->x2a8_26_isOpen) {
        newDistance = stretchFac * (distance - x468_conservativeDoorCamDistance) + x468_conservativeDoorCamDistance;
      } else {
        newDistance = stretchFac * (distance - 5.f) + 5.f;
      }
      if (x18d_28_obtuseDirection) {
        newDistance *= 1.f + x308_speedFactor;
      }
      baseElevation = door->x2a8_26_isOpen ? 0.75f : 1.5f;
      springSpeed = 4.f;
    }
  }

  x214_ballCameraSpring.ApplyDistanceSpring(newDistance, ballToCamMag, dt * springSpeed);
  distance = newDistance;
  elevation = (elevation - baseElevation) * stretchFac + baseElevation;

  return doorClose;
}

zeus::CVector3f CBallCamera::FindDesiredPosition(float distance, float elevation, const zeus::CVector3f& dir,
                                                 CStateManager& mgr, bool fullTest) {
  TCastToConstPtr<CPlayer> player = mgr.GetObjectById(xe8_watchedObject);
  if (!player)
    return {};

  zeus::CVector3f useDir = dir;
  if (!dir.canBeNormalized())
    useDir = zeus::skForward;

  zeus::CTransform lookDirXf = zeus::lookAt(zeus::skZero3f, useDir);
  zeus::CVector3f ballPos = player->GetBallPosition();
  float elev = elevation;
  float dist = distance;
  ConstrainElevationAndDistance(elev, dist, 0.f, mgr);
  zeus::CVector3f eyePos = player->GetEyePosition();
  if (!mgr.RayCollideWorld(ballPos, eyePos, BallCameraFilter, nullptr))
    eyePos = ballPos;

  zeus::CVector3f idealLookVec(0.f, -dist, elev - (eyePos.z() - ballPos.z()));
  idealLookVec = lookDirXf.getRotation() * idealLookVec;
  zeus::CVector3f lookVec(0.f, distance, elev - (eyePos.z() - ballPos.z()));
  float idealLookDist = idealLookVec.magnitude();
  float resolveLOSIntervalAng = zeus::degToRad(30.f);
  bool foundClear = false;
  bool clear = !DetectCollision(eyePos, eyePos + idealLookVec, 0.3f, idealLookDist, mgr);
  if (!clear && idealLookDist <= 0.f) {
    zeus::CAABox x13ac(ballPos - distance, ballPos + distance);
    x13ac.min.z() = float(ballPos.z());
    x13ac.max.z() = elev + ballPos.z();
    rstl::reserved_vector<TUniqueId, 1024> nearList;
    mgr.BuildNearList(nearList, x13ac, BallCameraFilter,
                      TCastToConstPtr<CActor>(mgr.GetObjectById(x46c_collisionActorId)).GetPtr());
    zeus::CQuaternion rotNeg;
    rotNeg.rotateZ(-resolveLOSIntervalAng);
    zeus::CTransform xfNeg = rotNeg.toTransform();
    zeus::CQuaternion rotPos;
    rotPos.rotateZ(resolveLOSIntervalAng);
    zeus::CTransform xfPos = rotPos.toTransform();
    while (!foundClear && idealLookDist > dist) {
      idealLookVec.normalize();
      idealLookVec = idealLookVec * idealLookDist;
      zeus::CVector3f lookVecNeg = xfNeg.rotate(idealLookVec);
      zeus::CVector3f lookVecPos = xfPos.rotate(idealLookVec);
      for (int i = 0; float(i) < 180.f / zeus::radToDeg(resolveLOSIntervalAng); ++i) {
        if (mgr.RayCollideWorld(eyePos, eyePos + lookVecNeg, nearList, BallCameraFilter, nullptr)) {
          foundClear = true;
          lookVec = lookVecNeg;
          break;
        } else if (mgr.RayCollideWorld(eyePos, eyePos + lookVecPos, nearList, BallCameraFilter, nullptr)) {
          foundClear = true;
          lookVec = lookVecPos;
          break;
        } else {
          lookVecNeg = xfNeg * lookVecNeg;
          lookVecPos = xfPos * lookVecPos;
        }
      }
      idealLookDist -= 0.3f;
    }
  } else {
    if (idealLookDist < 2.f) {
      idealLookVec.normalize();
      idealLookVec = idealLookVec * 2.f;
    }
    zeus::CQuaternion rotNeg;
    rotNeg.rotateZ(-resolveLOSIntervalAng);
    zeus::CTransform xfNeg = rotNeg.toTransform();
    zeus::CVector3f lookVecNeg = xfNeg * idealLookVec;
    zeus::CQuaternion rotPos;
    rotPos.rotateZ(resolveLOSIntervalAng);
    zeus::CTransform xfPos = rotPos.toTransform();
    zeus::CVector3f lookVecPos = xfPos * idealLookVec;
    if (clear || (!fullTest && (idealLookDist > 2.f || x2e8_ballVelFlat > 1.25f))) {
      idealLookVec.normalize();
      lookVec = idealLookVec * idealLookDist;
      foundClear = true;
    } else {
      for (int i = 0; float(i) < 180.f / zeus::radToDeg(resolveLOSIntervalAng); ++i) {
        idealLookDist = lookVecNeg.magnitude();
        if (!DetectCollision(eyePos, eyePos + lookVecNeg, 0.3f, idealLookDist, mgr) || idealLookDist > 2.f) {
          lookVecNeg.normalize();
          lookVec = lookVecNeg * idealLookDist;
          foundClear = true;
          break;
        }
        idealLookDist = lookVecPos.magnitude();
        if (!DetectCollision(eyePos, eyePos + lookVecPos, 0.3f, idealLookDist, mgr) || idealLookDist > 2.f) {
          lookVecPos.normalize();
          lookVec = lookVecPos * idealLookDist;
          foundClear = true;
          break;
        }
        lookVecNeg = xfNeg * lookVecNeg;
        lookVecPos = xfPos * lookVecPos;
      }
      if (!foundClear) {
        zeus::CAABox findBounds(ballPos - distance, ballPos + distance);
        findBounds.min.z() = float(ballPos.z());
        findBounds.max.z() = elev + ballPos.z();
        rstl::reserved_vector<TUniqueId, 1024> nearList;
        mgr.BuildNearList(nearList, findBounds, BallCameraFilter,
                          TCastToConstPtr<CActor>(mgr.GetObjectById(x46c_collisionActorId)).GetPtr());
        zeus::CQuaternion rotNeg2;
        rotNeg2.rotateZ(-resolveLOSIntervalAng);
        zeus::CTransform xfNeg2 = rotNeg2.toTransform();
        zeus::CQuaternion rotPos2;
        rotPos2.rotateZ(resolveLOSIntervalAng);
        zeus::CTransform xfPos2 = rotPos2.toTransform();
        while (!foundClear && idealLookDist > dist) {
          idealLookVec.normalize();
          idealLookVec = idealLookVec * idealLookDist;
          zeus::CVector3f lookVecNeg2 = xfNeg2.rotate(idealLookVec);
          zeus::CVector3f lookVecPos2 = xfPos2.rotate(idealLookVec);
          for (int i = 0; float(i) < 180.f / zeus::radToDeg(resolveLOSIntervalAng); ++i) {
            if (mgr.RayCollideWorld(eyePos, eyePos + lookVecNeg2, nearList, BallCameraFilter, nullptr)) {
              foundClear = true;
              lookVec = lookVecNeg2;
              break;
            } else if (mgr.RayCollideWorld(eyePos, eyePos + lookVecPos2, nearList, BallCameraFilter, nullptr)) {
              foundClear = true;
              lookVec = lookVecPos2;
              break;
            } else {
              lookVecNeg2 = xfNeg2 * lookVecNeg2;
              lookVecPos2 = xfPos2 * lookVecPos2;
            }
          }
          idealLookDist -= 0.3f;
        }
      }
    }
  }

  if (!foundClear)
    return GetTranslation();
  else
    return eyePos + lookVec;
}

bool CBallCamera::DetectCollision(const zeus::CVector3f& from, const zeus::CVector3f& to, float radius, float& d,
                                  CStateManager& mgr) {
  zeus::CVector3f delta = to - from;
  float deltaMag = delta.magnitude();
  zeus::CVector3f deltaNorm = delta * (1.f / deltaMag);
  bool clear = true;

  if (deltaMag > 0.000001f) {
    float margin = 2.f * radius;
    zeus::CAABox aabb;
    aabb.accumulateBounds(from);
    aabb.accumulateBounds(to);
    aabb = zeus::CAABox(aabb.min - margin, aabb.max + margin);
    rstl::reserved_vector<TUniqueId, 1024> nearList;
    mgr.BuildColliderList(nearList, mgr.GetPlayer(), aabb);
    CAreaCollisionCache cache(aabb);
    CGameCollision::BuildAreaCollisionCache(mgr, cache);
    if (cache.HasCacheOverflowed())
      clear = false;
    CCollidableSphere cSphere({zeus::skZero3f, radius}, {EMaterialTypes::Solid});
    if (CGameCollision::DetectCollisionBoolean_Cached(
            mgr, cache, cSphere, zeus::CTransform::Translate(from),
            CMaterialFilter::MakeIncludeExclude({EMaterialTypes::Solid},
                                                {EMaterialTypes::ProjectilePassthrough, EMaterialTypes::Player,
                                                 EMaterialTypes::Character, EMaterialTypes::CameraPassthrough}),
            nearList)) {
      d = -1.f;
      return true;
    }
    if (clear) {
      TUniqueId intersectId = kInvalidUniqueId;
      CCollisionInfo info;
      double dTmp = deltaMag;
      if (CGameCollision::DetectCollision_Cached_Moving(
              mgr, cache, cSphere, zeus::CTransform::Translate(from),
              CMaterialFilter::MakeIncludeExclude({EMaterialTypes::Solid},
                                                  {EMaterialTypes::ProjectilePassthrough, EMaterialTypes::Player,
                                                   EMaterialTypes::Character, EMaterialTypes::CameraPassthrough}),
              nearList, deltaNorm, intersectId, info, dTmp)) {
        d = float(dTmp);
        clear = false;
      }
    }
  }
  return !clear;
}

void CBallCamera::Think(float dt, CStateManager& mgr) {
  mgr.SetActorAreaId(*this, mgr.GetNextAreaId());
  UpdatePlayerMovement(dt, mgr);
  const TCastToPtr<CCollisionActor> colAct = mgr.ObjectById(x46c_collisionActorId);
  if (colAct) {
    mgr.SetActorAreaId(*colAct, mgr.GetNextAreaId());
  }

  switch (mgr.GetPlayer().GetCameraState()) {
  default:
    if (!x18d_27_forceProcessing) {
      if (colAct)
        colAct->SetActive(false);
      return;
    }
    [[fallthrough]];
  case CPlayer::EPlayerCameraState::Ball:
  case CPlayer::EPlayerCameraState::Transitioning:
  case CPlayer::EPlayerCameraState::Two: {
    if (colAct)
      colAct->SetActive(true);
    zeus::CTransform oldXf = x34_transform;
    if (mgr.GetPlayer().GetBombJumpCount() != 1)
      UpdateLookAtPosition(dt, mgr);
    CheckFailsafe(dt, mgr);
    UpdateObjectTooCloseId(mgr);
    UpdateAnglePerSecond(dt);
    switch (x400_state) {
    case EBallCameraState::Default:
    case EBallCameraState::Chase:
    case EBallCameraState::Boost:
      switch (x188_behaviour) {
      case EBallCameraBehaviour::PathCamera:
        UpdateUsingPathCameras(dt, mgr);
        break;
      case EBallCameraBehaviour::HintFixedPosition:
      case EBallCameraBehaviour::HintFixedTransform:
        UpdateUsingFixedCameras(dt, mgr);
        break;
      case EBallCameraBehaviour::PathCameraDesiredPos:
      case EBallCameraBehaviour::Default:
      case EBallCameraBehaviour::FreezeLookPosition:
      case EBallCameraBehaviour::HintBallToCam:
        if (x36c_splineState != ESplineState::Invalid)
          UpdateUsingFreeLook(dt, mgr);
        else
          UpdateUsingColliders(dt, mgr);
        break;
      case EBallCameraBehaviour::SpindleCamera:
        UpdateUsingSpindleCameras(dt, mgr);
        break;
      default:
        break;
      }
      break;
    case EBallCameraState::ToBall:
    case EBallCameraState::FromBall:
      UpdateUsingTransitions(dt, mgr);
      break;
    default:
      break;
    }

    SetTransform(ValidateCameraTransform(UpdateCameraPositions(dt, oldXf, x34_transform), oldXf));
    break;
  }
  }
}

bool CBallCamera::CheckTransitionLineOfSight(const zeus::CVector3f& eyePos, const zeus::CVector3f& behindPos,
                                             float& eyeToOccDist, float colRadius, CStateManager& mgr) {
  zeus::CVector3f eyeToBehind = behindPos - eyePos;
  float eyeToBehindMag = eyeToBehind.magnitude();
  zeus::CVector3f eyeToBehindNorm = eyeToBehind * (1.f / eyeToBehindMag);
  bool clear = true;
  if (eyeToBehindMag > 0.000001f) {
    float margin = 2.f * colRadius;
    zeus::CAABox aabb;
    aabb.accumulateBounds(eyePos);
    aabb.accumulateBounds(behindPos);
    aabb = zeus::CAABox(aabb.min - margin, aabb.max + margin);
    rstl::reserved_vector<TUniqueId, 1024> nearList;
    mgr.BuildColliderList(nearList, mgr.GetPlayer(), aabb);
    CAreaCollisionCache cache(aabb);
    CGameCollision::BuildAreaCollisionCache(mgr, cache);
    if (cache.HasCacheOverflowed())
      clear = false;
    if (clear) {
      CCollisionInfo cinfo;
      double d = eyeToBehindMag;
      TUniqueId intersectId = kInvalidUniqueId;
      CCollidableSphere cSphere({zeus::skZero3f, colRadius}, {EMaterialTypes::Solid});
      if (CGameCollision::DetectCollision_Cached_Moving(
              mgr, cache, cSphere, zeus::CTransform::Translate(eyePos),
              CMaterialFilter::MakeIncludeExclude({EMaterialTypes::Solid},
                                                  {EMaterialTypes::ProjectilePassthrough, EMaterialTypes::Player,
                                                   EMaterialTypes::Character, EMaterialTypes::CameraPassthrough}),
              nearList, eyeToBehindNorm, intersectId, cinfo, d)) {
        eyeToOccDist = float(d);
        clear = false;
      }
    }
  }
  return !clear;
}

bool CBallCamera::TransitionFromMorphBallState(CStateManager& mgr) {
  x47c_failsafeState->x0_playerXf = mgr.GetPlayer().GetTransform();
  x47c_failsafeState->x30_camXf = GetTransform();
  x47c_failsafeState->x60_lookPos = x1d8_lookPos;
  x47c_failsafeState->x84_playerPos = x47c_failsafeState->x0_playerXf.origin;
  zeus::CVector3f eyePos = mgr.GetPlayer().GetEyePosition();
  float lookDist = (x47c_failsafeState->x60_lookPos - x47c_failsafeState->x30_camXf.origin).magnitude();
  zeus::CVector3f behindPos = x47c_failsafeState->x0_playerXf.basis[1] * (0.6f * -lookDist) + eyePos;
  float eyeToOccDist;
  if (CheckTransitionLineOfSight(eyePos, behindPos, eyeToOccDist, 0.6f, mgr))
    x47c_failsafeState->x6c_behindPos = x47c_failsafeState->x0_playerXf.basis[1] * -eyeToOccDist + eyePos;
  else
    x47c_failsafeState->x6c_behindPos = behindPos;
  x47c_failsafeState->x90_splinePoints.clear();
  x47c_failsafeState->x90_splinePoints.reserve(4);
  x47c_failsafeState->x90_splinePoints.push_back(x47c_failsafeState->x30_camXf.origin);
  x47c_failsafeState->x90_splinePoints.push_back(x47c_failsafeState->x6c_behindPos);
  x47c_failsafeState->x90_splinePoints.push_back(x47c_failsafeState->x6c_behindPos);
  x47c_failsafeState->x90_splinePoints.push_back(eyePos);
  return CheckFailsafeFromMorphBallState(mgr);
}

void CBallCamera::TeleportColliders(std::vector<CCameraCollider>& colliderList, const zeus::CVector3f& pos) {
  for (CCameraCollider& collider : colliderList) {
    collider.x2c_lastWorldPos = pos;
    collider.x14_localPos = pos;
    collider.x20_scaledWorldPos = pos;
  }
}

void CBallCamera::TeleportCamera(const zeus::CVector3f& pos, CStateManager& mgr) {
  x294_dampedPos = pos;
  TeleportColliders(x264_smallColliders, pos);
  TeleportColliders(x274_mediumColliders, pos);
  TeleportColliders(x284_largeColliders, pos);
  if (const TCastToPtr<CCollisionActor> act = mgr.ObjectById(x46c_collisionActorId)) {
    act->SetTranslation(pos);
  }
}

void CBallCamera::TeleportCamera(const zeus::CTransform& xf, CStateManager& mgr) {
  SetTransform(xf);
  TeleportCamera(xf.origin, mgr);
}

void CBallCamera::ResetToTweaks(CStateManager& mgr) {
  x188_behaviour = EBallCameraBehaviour::Default;
  x18c_25_chaseAllowed = true;
  x18c_26_boostAllowed = true;
  x18c_27_obscureAvoidance = true;
  x18c_28_volumeCollider = true;
  x18c_29_clampAttitude = false;
  x18c_30_clampAzimuth = false;
  x194_targetMinDistance = g_tweakBall->GetBallCameraMinSpeedDistance();
  x198_maxDistance = g_tweakBall->GetBallCameraMaxSpeedDistance();
  x19c_backwardsDistance = g_tweakBall->GetBallCameraBackwardsDistance();
  x214_ballCameraSpring =
      CCameraSpring(g_tweakBall->GetBallCameraSpringConstant(), g_tweakBall->GetBallCameraSpringMax(),
                    g_tweakBall->GetBallCameraSpringTardis());
  x250_ballCameraCentroidDistanceSpring = CCameraSpring(g_tweakBall->GetBallCameraCentroidDistanceSpringConstant(),
                                                        g_tweakBall->GetBallCameraCentroidDistanceSpringMax(),
                                                        g_tweakBall->GetBallCameraCentroidDistanceSpringTardis());
  x1b4_lookAtOffset = g_tweakBall->GetBallCameraOffset();
  x410_chaseLookAtOffset = g_tweakBall->GetBallCameraChaseLookAtOffset();
  x1a0_elevation = g_tweakBall->GetBallCameraElevation();
  x1ac_attitudeRange = M_PIF / 2.f;
  x1b0_azimuthRange = M_PIF / 2.f;
  SetFovInterpolation(x15c_currentFov, CCameraManager::ThirdPersonFOV(), 1.f, 0.f);
  x1a8_targetAnglePerSecond = g_tweakBall->GetBallCameraAnglePerSecond();
  x18d_29_noElevationInterp = false;
  x18d_30_directElevation = false;
  x18d_31_overrideLookDir = false;
  x18e_24_noElevationVelClamp = false;
  x18e_25_noSpline = false;
  x18e_26_ = false;
}

void CBallCamera::UpdateLookAtPosition(float dt, CStateManager& mgr) {
  if (TCastToConstPtr<CPlayer> player = mgr.GetObjectById(xe8_watchedObject)) {
    zeus::CVector3f ballPos = player->GetBallPosition();
    if (player->IsMorphBallTransitioning()) {
      x1d8_lookPos = ballPos;
      x1d8_lookPos.z() += x1b4_lookAtOffset.z();
      x1c0_lookPosAhead = x1d8_lookPos;
      x1cc_fixedLookPos = x1d8_lookPos;
    } else {
      zeus::CVector3f dirNorm = player->GetMoveDir();
      dirNorm.normalize();
      zeus::CVector3f lookAtOffsetAhead(x308_speedFactor * x1b4_lookAtOffset.x(),
                                        x308_speedFactor * x1b4_lookAtOffset.y(), x1b4_lookAtOffset.z());
      if (x18c_25_chaseAllowed && (x400_state == EBallCameraState::Chase || x400_state == EBallCameraState::One))
        lookAtOffsetAhead = zeus::CVector3f(x308_speedFactor * x410_chaseLookAtOffset.x(),
                                            x308_speedFactor * x410_chaseLookAtOffset.y(), x410_chaseLookAtOffset.z());
      if (mgr.GetCameraManager()->IsInterpolationCameraActive())
        lookAtOffsetAhead = zeus::CVector3f(0.f, 0.f, x1b4_lookAtOffset.z());
      zeus::CTransform moveXf = player->CreateTransformFromMovementDirection().getRotation();
      if (x2fc_ballDeltaFlat.canBeNormalized())
        lookAtOffsetAhead = moveXf * lookAtOffsetAhead;
      zeus::CVector3f lookAtPosAhead = ballPos + lookAtOffsetAhead;
      x1c0_lookPosAhead = lookAtPosAhead;
      x1cc_fixedLookPos = ballPos + zeus::CVector3f(0.f, 0.f, lookAtOffsetAhead.z());
      zeus::CVector3f aheadToCurrentLookDelta = x1d8_lookPos - lookAtPosAhead;
      float aheadToCurrentLookMag = aheadToCurrentLookDelta.magnitude();
      if (aheadToCurrentLookDelta.canBeNormalized())
        aheadToCurrentLookDelta.normalize();
      float lookAtSpringMag = x23c_ballCameraLookAtSpring.ApplyDistanceSpringNoMax(
          0.f, aheadToCurrentLookMag, (2.f * zeus::clamp(0.f, x30c_speedingTime / 3.f, 1.f) + 1.f) * dt);
      if (lookAtSpringMag > 0.0001f)
        lookAtPosAhead += aheadToCurrentLookDelta * lookAtSpringMag;
      aheadToCurrentLookDelta = lookAtPosAhead - x1d8_lookPos;
      if (x18d_26_lookAtBall) {
        x1d8_lookPos = ballPos;
        x1d8_lookPos.z() += x1b4_lookAtOffset.z();
      } else {
        x1d8_lookPos = lookAtPosAhead;
      }
      switch (x188_behaviour) {
      case EBallCameraBehaviour::Default:
      case EBallCameraBehaviour::FreezeLookPosition:
      case EBallCameraBehaviour::HintBallToCam:
      case EBallCameraBehaviour::HintInitializePosition:
      case EBallCameraBehaviour::PathCameraDesiredPos:
      case EBallCameraBehaviour::PathCamera:
        if (mgr.GetCameraManager()->IsInterpolationCameraActive()) {
          x1d8_lookPos = x1c0_lookPosAhead;
          x1cc_fixedLookPos = x1c0_lookPosAhead;
        }
        break;
      case EBallCameraBehaviour::HintFixedPosition:
        x1d8_lookPos = x1cc_fixedLookPos;
        x1c0_lookPosAhead = x1d8_lookPos;
        break;
      case EBallCameraBehaviour::HintFixedTransform:
      case EBallCameraBehaviour::SpindleCamera:
        x1d8_lookPos = x1cc_fixedLookPos;
        x1c0_lookPosAhead = x1cc_fixedLookPos;
        break;
      }
      if (x18d_30_directElevation) {
        x1d8_lookPos.z() = ballPos.z() + x1b4_lookAtOffset.z();
        x1c0_lookPosAhead.z() = float(x1d8_lookPos.z());
        x1cc_fixedLookPos.z() = float(x1d8_lookPos.z());
      }
      if (x18d_31_overrideLookDir) {
        if (const CScriptCameraHint* hint = mgr.GetCameraManager()->GetCameraHint(mgr)) {
          x1d8_lookPos = hint->GetTransform().basis[1] * 10.f + GetTranslation();
          x1c0_lookPosAhead = x1d8_lookPos;
          x1cc_fixedLookPos = x1d8_lookPos;
        }
      }
    }
  }
}

zeus::CTransform CBallCamera::UpdateLookDirection(const zeus::CVector3f& dir, CStateManager& mgr) {
  zeus::CVector3f useDir = dir;
  if (!dir.canBeNormalized())
    useDir = zeus::skForward;
  float elevation = x1a0_elevation;
  float distance = x190_curMinDistance;
  ConstrainElevationAndDistance(elevation, distance, 0.f, mgr);
  zeus::CVector3f pos = FindDesiredPosition(distance, elevation, useDir, mgr, false);
  UpdateLookAtPosition(0.f, mgr);
  return zeus::lookAt(pos, x1d8_lookPos);
}

void CBallCamera::ApplyCameraHint(CStateManager& mgr) {
  if (const CScriptCameraHint* hint = mgr.GetCameraManager()->GetCameraHint(mgr)) {
    ResetToTweaks(mgr);
    x188_behaviour = hint->GetHint().GetBehaviourType();
    x18c_25_chaseAllowed = (hint->GetHint().GetOverrideFlags() & 0x2) != 0;
    x18c_26_boostAllowed = (hint->GetHint().GetOverrideFlags() & 0x4) != 0;
    x18c_27_obscureAvoidance = (hint->GetHint().GetOverrideFlags() & 0x8) != 0;
    x18c_28_volumeCollider = (hint->GetHint().GetOverrideFlags() & 0x10) != 0;
    if ((hint->GetHint().GetOverrideFlags() & 0x40) != 0)
      x18d_26_lookAtBall = true;
    x18d_29_noElevationInterp = (hint->GetHint().GetOverrideFlags() & 0x4000) != 0;
    x18d_30_directElevation = (hint->GetHint().GetOverrideFlags() & 0x8000) != 0;
    x18d_31_overrideLookDir = (hint->GetHint().GetOverrideFlags() & 0x10000) != 0;
    x18e_24_noElevationVelClamp = (hint->GetHint().GetOverrideFlags() & 0x20000) != 0;
    x18e_25_noSpline = x18e_26_ = (hint->GetHint().GetOverrideFlags() & 0x80000) != 0;
    if ((hint->GetHint().GetOverrideFlags() & 0x400000) != 0)
      x194_targetMinDistance = hint->GetHint().GetMinDist();
    if ((hint->GetHint().GetOverrideFlags() & 0x800000) != 0)
      x198_maxDistance = hint->GetHint().GetMaxDist();
    if ((hint->GetHint().GetOverrideFlags() & 0x1000000) != 0)
      x19c_backwardsDistance = hint->GetHint().GetBackwardsDist();
    if ((hint->GetHint().GetOverrideFlags() & 0x80000000) != 0)
      x1a0_elevation = hint->GetHint().GetElevation();
    if ((hint->GetHint().GetOverrideFlags() & 0x2000000) != 0)
      x1b4_lookAtOffset = hint->GetHint().GetLookAtOffset();
    if ((hint->GetHint().GetOverrideFlags() & 0x4000000) != 0)
      x410_chaseLookAtOffset = hint->GetHint().GetChaseLookAtOffset();
    if ((hint->GetHint().GetOverrideFlags() & 0x10000000) != 0) {
      x18c_29_clampAttitude = true;
      x1ac_attitudeRange = hint->GetHint().GetAttitudeRange();
    } else {
      x18c_29_clampAttitude = false;
    }
    if ((hint->GetHint().GetOverrideFlags() & 0x20000000) != 0) {
      x18c_30_clampAzimuth = true;
      x1b0_azimuthRange = hint->GetHint().GetAzimuthRange();
    } else {
      x18c_30_clampAzimuth = false;
    }
    if ((hint->GetHint().GetOverrideFlags() & 0x8000000) != 0)
      SetFovInterpolation(x15c_currentFov, hint->GetHint().GetFov(), 1.f, 0.f);
    if ((hint->GetHint().GetOverrideFlags() & 0x40000000) != 0)
      x1a8_targetAnglePerSecond = hint->GetHint().GetAnglePerSecond();
    if ((hint->GetHint().GetOverrideFlags() & 0x200) != 0)
      mgr.GetPlayer().SetControlDirectionInterpolation(hint->GetHint().GetControlInterpDur());
    else
      mgr.GetPlayer().ResetControlDirectionInterpolation();
    switch (hint->GetHint().GetBehaviourType()) {
    case EBallCameraBehaviour::HintBallToCam: {
      x45c_overrideBallToCam = hint->GetHint().GetBallToCam();
      ResetPosition(mgr);
      zeus::CVector3f camPos = mgr.GetPlayer().GetBallPosition() + hint->GetHint().GetBallToCam();
      if ((hint->GetHint().GetOverrideFlags() & 0x1) != 0) {
        float distance = hint->GetHint().GetBallToCam().toVec2f().magnitude();
        zeus::CVector3f camToBall = -zeus::CVector3f(hint->GetHint().GetBallToCam().toVec2f()).normalized();
        camPos = FindDesiredPosition(distance, hint->GetHint().GetBallToCam().z(), camToBall, mgr, false);
      }
      TeleportCamera(zeus::lookAt(camPos, x1d8_lookPos), mgr);
      break;
    }
    case EBallCameraBehaviour::HintFixedTransform: {
      ResetPosition(mgr);
      TeleportCamera(hint->GetTransform(), mgr);
      break;
    }
    case EBallCameraBehaviour::Default: {
      if ((hint->GetHint().GetOverrideFlags() & 0x20) != 0) {
        ResetPosition(mgr);
        if ((hint->GetHint().GetOverrideFlags() & 0x40000) != 0) {
          zeus::CVector3f lookDir =
              mgr.GetPlayer().GetTranslation() - mgr.GetCameraManager()->GetCurrentCameraTransform(mgr).origin;
          lookDir.z() = 0.f;
          if (lookDir.canBeNormalized())
            lookDir.normalize();
          else
            lookDir = mgr.GetPlayer().GetMoveDir();
          TeleportCamera(UpdateLookDirection(lookDir, mgr), mgr);
        } else {
          TeleportCamera(zeus::lookAt(hint->GetTranslation(), x1d8_lookPos), mgr);
        }
      }
      break;
    }
    case EBallCameraBehaviour::HintFixedPosition: {
      ResetPosition(mgr);
      TeleportCamera(zeus::lookAt(hint->GetTranslation(), x1d8_lookPos), mgr);
      break;
    }
    case EBallCameraBehaviour::FreezeLookPosition:
    case EBallCameraBehaviour::HintInitializePosition: {
      if ((hint->GetHint().GetOverrideFlags() & 0x20) != 0) {
        ResetPosition(mgr);
        float elevation = x1a0_elevation;
        float distance = x190_curMinDistance;
        ConstrainElevationAndDistance(elevation, distance, 0.f, mgr);
        TeleportCamera(zeus::lookAt(FindDesiredPosition(distance, elevation, mgr.GetPlayer().GetMoveDir(), mgr, false),
                                    x1cc_fixedLookPos),
                       mgr);
      }
      break;
    }
    default:
      break;
    }

    if ((hint->GetHint().GetOverrideFlags() & 0x20) != 0)
      mgr.GetCameraManager()->SetPlayerCamera(mgr, GetUniqueId());
  }
}

void CBallCamera::ResetPosition(CStateManager& mgr) {
  x1d8_lookPos = mgr.GetPlayer().GetBallPosition();
  x1d8_lookPos.z() += x1b4_lookAtOffset.z();
  x1c0_lookPosAhead = x1d8_lookPos;
  x1cc_fixedLookPos = x1d8_lookPos;
}

void CBallCamera::DoorClosed(TUniqueId doorId) {
  if (doorId != x3dc_tooCloseActorId)
    return;
  x18e_27_nearbyDoorClosed = true;
}

void CBallCamera::DoorClosing(TUniqueId doorId) {
  if (doorId != x3dc_tooCloseActorId)
    return;
  x18e_28_nearbyDoorClosing = true;
}

} // namespace urde
