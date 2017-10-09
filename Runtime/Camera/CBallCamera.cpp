#include "GameGlobalObjects.hpp"
#include "CBallCamera.hpp"
#include "TCastTo.hpp"
#include "CStateManager.hpp"
#include "Collision/CCollisionActor.hpp"
#include "World/CPlayer.hpp"
#include "Input/ControlMapper.hpp"
#include "Camera/CFirstPersonCamera.hpp"

namespace urde
{

void CCameraSpring::Reset()
{
    x4_tardis2Sqrt = 2.f * std::sqrt(x0_tardis);
    x10_dx = 0.f;
}

float CCameraSpring::ApplyDistanceSpringNoMax(float targetX, float curX, float dt)
{
    float usePos = xc_k * x10_dx * dt + curX;
    x10_dx += xc_k * (x0_tardis * (targetX - curX) - x4_tardis2Sqrt * x10_dx) * dt;
    return std::max(usePos, targetX);
}

float CCameraSpring::ApplyDistanceSpring(float targetX, float curX, float dt)
{
    float usePos = xc_k * x10_dx * dt + curX;
    x10_dx += xc_k * (x0_tardis * (targetX - curX) - x4_tardis2Sqrt * x10_dx) * dt;
    usePos = std::max(usePos, targetX);
    if (usePos - targetX > x8_max)
        usePos = targetX + x8_max;
    return usePos;
}

CBallCamera::CBallCamera(TUniqueId uid, TUniqueId watchedId, const zeus::CTransform& xf,
                         float fovy, float znear, float zfar, float aspect)
: CGameCamera(uid, true, "Ball Camera",
              CEntityInfo(kInvalidAreaId, CEntity::NullConnectionList),
              xf, fovy, znear, zfar, aspect, watchedId, false, 0),
  x214_ballCameraSpring(g_tweakBall->GetBallCameraSpringTardis(),
                        g_tweakBall->GetBallCameraSpringMax(),
                        g_tweakBall->GetBallCameraSpringConstant()),
  x228_ballCameraCentroidSpring(g_tweakBall->GetBallCameraCentroidSpringTardis(),
                                g_tweakBall->GetBallCameraCentroidSpringMax(),
                                g_tweakBall->GetBallCameraCentroidSpringConstant()),
  x23c_ballCameraLookAtSpring(g_tweakBall->GetBallCameraLookAtSpringTardis(),
                              g_tweakBall->GetBallCameraLookAtSpringMax(),
                              g_tweakBall->GetBallCameraLookAtSpringConstant()),
  x250_ballCameraCentroidDistanceSpring(g_tweakBall->GetBallCameraCentroidDistanceSpringTardis(),
                                        g_tweakBall->GetBallCameraCentroidDistanceSpringMax(),
                                        g_tweakBall->GetBallCameraCentroidDistanceSpringConstant()),
  x37c_camSpline(false),
  x41c_ballCameraChaseSpring(g_tweakBall->GetBallCameraChaseSpringTardis(),
                             g_tweakBall->GetBallCameraChaseSpringMax(),
                             g_tweakBall->GetBallCameraChaseSpringConstant()),
  x448_ballCameraBoostSpring(g_tweakBall->GetBallCameraBoostSpringTardis(),
                             g_tweakBall->GetBallCameraBoostSpringMax(),
                             g_tweakBall->GetBallCameraBoostSpringConstant())
{
    x18c_24_ = true;
    x18c_25_ = true;
    x18c_26_ = true;
    x18c_27_ = true;
    x18c_28_ = true;
    x18c_29_ = false;
    x18c_30_ = false;
    x18c_31_ = true;
    x18d_24_ = true;
    x18d_25_ = false;
    x18d_26_ = false;
    x18d_27_ = false;
    x18d_28_ = false;
    x18d_29_ = false;
    x18d_30_ = false;
    x18d_31_ = false;
    x18e_24_ = false;
    x18e_25_ = false;
    x18e_26_ = false;
    x18e_27_nearbyDoorClosed = false;
    x18e_28_nearbyDoorClosing = false;

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

    x468_ = g_tweakBall->x170_;

    x47c_failsafeState = std::make_unique<SFailsafeState>();
    x480_ = std::make_unique<u32>();

    SetupColliders(x264_smallColliders, 2.31f, 2.31f, 0.1f, 3, 2.f, 0.5f, -M_PIF / 2.f);
    SetupColliders(x274_mediumColliders, 4.62f, 4.62f, 0.1f, 6, 2.f, 0.5f, -M_PIF / 2.f);
    SetupColliders(x284_largeColliders, 7.f, 7.f, 0.1f, 12, 2.f, 0.5f, -M_PIF / 2.f);
}

void CBallCamera::SetupColliders(std::vector<CCameraCollider>& out, float xMag, float zMag, float radius, int count,
                                 float tardis, float max, float startAngle)
{
    out.reserve(count);
    float theta = startAngle;
    for (int i=0 ; i<count ; ++i)
    {
        float z = std::cos(theta) * zMag;
        if (theta > M_PIF / 2.f)
            z *= 0.25f;
        out.emplace_back(radius, zeus::CVector3f{std::sin(theta) * xMag, 0.f, z}, CCameraSpring{tardis, max, 1.f}, 1.f);
        theta += 2.f * M_PIF / float(count);
    }
}

void CBallCamera::Accept(IVisitor& visitor)
{
    visitor.Visit(this);
}

void CBallCamera::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId objId, CStateManager& stateMgr)
{
    CGameCamera::AcceptScriptMsg(msg, objId, stateMgr);
    switch (msg)
    {
    case EScriptObjectMessage::Registered:
    {
        x46c_collisionActorId = stateMgr.AllocateUniqueId();
        CCollisionActor* colAct = new CCollisionActor(x46c_collisionActorId, GetAreaId(), kInvalidUniqueId,
                                                      true, 0.3f, 1.f);
        colAct->SetMaterialFilter(CMaterialFilter::MakeIncludeExclude({EMaterialTypes::Solid},
                                                                      {EMaterialTypes::Player,
                                                                       EMaterialTypes::CameraPassthrough}));
        colAct->SetMaterialList({EMaterialTypes::ProjectilePassthrough, EMaterialTypes::ScanPassthrough,
                                 EMaterialTypes::SeeThrough, EMaterialTypes::CameraPassthrough});
        colAct->SetTranslation(GetTranslation());
        stateMgr.AddObject(colAct);
        colAct->SetMovable(false);
        CMotionState mState(GetTranslation(), zeus::CNUQuaternion::fromAxisAngle(zeus::CVector3f::skForward, 0.f),
                            zeus::CVector3f::skZero, zeus::CAxisAngle::sIdentity);
        colAct->SetLastNonCollidingState(mState);
        SetMaterialFilter(CMaterialFilter::MakeIncludeExclude({},
                                                              {EMaterialTypes::Solid,
                                                               EMaterialTypes::ProjectilePassthrough,
                                                               EMaterialTypes::Player,
                                                               EMaterialTypes::Character,
                                                               EMaterialTypes::CameraPassthrough}));
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

void CBallCamera::ProcessInput(const CFinalInput& input, CStateManager& mgr)
{
    if (input.ControllerIdx() != 0)
        return;

    if (TCastToConstPtr<CPlayer> player = mgr.GetObjectById(xe8_watchedObject))
    {
        if (player->GetMorphballTransitionState() == CPlayer::EPlayerMorphBallState::Morphed)
        {
            switch (x400_state)
            {
            case EBallCameraState::Two:
                if (!ControlMapper::GetDigitalInput(ControlMapper::ECommands::ChaseCamera, input) ||
                    player->IsInFreeLook())
                    SetState(EBallCameraState::Zero, mgr);
                break;
            case EBallCameraState::Three:
                if (!player->GetMorphBall()->IsInBoost())
                    SetState(EBallCameraState::Zero, mgr);
                break;
            case EBallCameraState::Zero:
                if (x18c_25_ && ControlMapper::GetPressInput(ControlMapper::ECommands::ChaseCamera, input))
                    SetState(EBallCameraState::Two, mgr);
                break;
            default:
                break;
            }

            if (x18c_26_ && x400_state != EBallCameraState::Three &&
                (player->GetMorphBall()->IsInBoost() || player->GetMorphBall()->GetBoostChargeTime() > 0.f))
                SetState(EBallCameraState::Three, mgr);
        }
    }
}

void CBallCamera::Reset(const zeus::CTransform& xf, CStateManager& mgr)
{
    x214_ballCameraSpring.Reset();
    x228_ballCameraCentroidSpring.Reset();
    x23c_ballCameraLookAtSpring.Reset();
    x250_ballCameraCentroidDistanceSpring.Reset();
    x41c_ballCameraChaseSpring.Reset();
    x448_ballCameraBoostSpring.Reset();

    zeus::CVector3f desiredPos = FindDesiredPosition(x190_curMinDistance, x1a0_elevation, xf.basis[1], mgr);

    if (TCastToConstPtr<CPlayer> player = mgr.GetObjectById(xe8_watchedObject))
    {
        ResetPosition();
        x310_ = x1b4_lookAtOffset;
        x31c_ = x1d8_;
        if ((x1d8_ - desiredPos).canBeNormalized())
        {
            TeleportCamera(zeus::lookAt(desiredPos, x1d8_), mgr);
        }
        else
        {
            zeus::CTransform camXf = player->CreateTransformFromMovementDirection();
            camXf.origin = desiredPos;
            TeleportCamera(camXf, mgr);
            mgr.GetCameraManager()->SetPlayerCamera(mgr, GetUniqueId());
        }

        x2e8_ = 0.f;
        x2ec_ = 0.f;
        x190_curMinDistance = x194_targetMinDistance;
        x2fc_ = zeus::CVector3f::skZero;
        x2f0_ = zeus::CVector3f::skZero;
        x18d_28_ = false;
        x308_ = 0.f;
        x2dc_ = player->GetBallPosition();
        x294_ = GetTranslation();
        x2a0_ = zeus::CVector3f::skZero;
        x2ac_ = zeus::CVector3f::skZero;
        x2b8_ = zeus::CVector3f::skZero;
        x2c4_ = 0;
        x2c8_ = 0;
        x2cc_ = 0;
        x2d0_ = 0;
        x2d4_ = 0;
        x2d8_ = 0;
        x32c_ = 1.f;
        x18d_25_ = true;
        x18d_27_ = true;
        Think(0.1f, mgr);
        x18d_25_ = false;
        x18d_27_ = false;
    }
}

void CBallCamera::Render(const CStateManager& mgr) const
{
    // Empty
}

void CBallCamera::SetState(EBallCameraState state, CStateManager& mgr)
{
    switch (state)
    {
    case EBallCameraState::Four:
    {
        zeus::CTransform xf = mgr.GetCameraManager()->GetFirstPersonCamera()->GetTransform();
        SetTransform(xf);
        TeleportCamera(xf.origin, mgr);
        SetFovInterpolation(mgr.GetCameraManager()->GetFirstPersonCamera()->GetFov(),
                            CCameraManager::ThirdPersonFOV(), 1.f, 0.f);
        x36c_ = 0;
    }
    case EBallCameraState::Zero:
    case EBallCameraState::Two:
    case EBallCameraState::Three:
        mgr.SetGameState(CStateManager::EGameState::Running);
        break;
    case EBallCameraState::Five:
        mgr.GetCameraManager()->SetPlayerCamera(mgr, GetUniqueId());
        mgr.SetGameState(CStateManager::EGameState::Running);
        SetFovInterpolation(GetFov(), CCameraManager::FirstPersonFOV(), 1.f, 0.f);
        x36c_ = 0;
        break;
    default:
        break;
    }

    x400_state = state;
}

static const CMaterialFilter BallCameraFilter =
    CMaterialFilter::MakeIncludeExclude({EMaterialTypes::Solid},
    {EMaterialTypes::ProjectilePassthrough, EMaterialTypes::Player, EMaterialTypes::Character,
     EMaterialTypes::CameraPassthrough});

void CBallCamera::ResetSpline(CStateManager& mgr)
{
    zeus::CVector3f ballPos = mgr.GetPlayer().GetBallPosition();
    TUniqueId intersectId = kInvalidUniqueId;
    rstl::reserved_vector<TUniqueId, 1024> nearList;
    CRayCastResult result = mgr.RayWorldIntersection(intersectId, ballPos, zeus::CVector3f::skDown, 20.f,
                                                     BallCameraFilter, nearList);
    float downFactor = result.IsValid() ? zeus::clamp(0.f, result.GetT() / 20.f, 1.f) : 1.f;
    x36c_ = 1;
    x370_24_ = true;
    x3d0_24_ = false;
    x37c_camSpline.Reset(4);
    x37c_camSpline.AddKnot(GetTranslation(), zeus::CVector3f::skForward);
    float elevation = x1a0_elevation;
    float distance = x190_curMinDistance;
    ConstrainElevationAndDistance(elevation, distance, 0.f, mgr);
    zeus::CVector3f tmpPoint(x35c_.x, x35c_.y, GetTranslation().z);
    x37c_camSpline.AddKnot(tmpPoint, zeus::CVector3f::skForward);
    zeus::CVector3f tmpPoint2 = tmpPoint + (x35c_ - GetTranslation()) * (0.5f + downFactor);
    x37c_camSpline.AddKnot(tmpPoint2, zeus::CVector3f::skForward);
    zeus::CVector3f tmpPoint2Ball = ballPos - tmpPoint2;
    if (tmpPoint2Ball.canBeNormalized())
        tmpPoint2Ball.normalize();
    else
        tmpPoint2Ball = mgr.GetPlayer().GetMoveDir();
    zeus::CVector3f desiredPosition = FindDesiredPosition(distance, elevation, tmpPoint2Ball, mgr);
    x37c_camSpline.AddKnot(desiredPosition, zeus::CVector3f::skForward);
    x37c_camSpline.x44_ = x37c_camSpline.CalculateSplineLength();
    x3d0_24_ = false;
    CMaterialList intersectMat;
    x3c8_ = CMaterialList(EMaterialTypes::Floor, EMaterialTypes::Ceiling);
    if (!SplineIntersectTest(intersectMat, mgr))
    {
        if (intersectMat.HasMaterial(EMaterialTypes::Floor) || intersectMat.HasMaterial(EMaterialTypes::Wall))
        {
            x3d0_24_ = true;
            x3c8_ = CMaterialList();
        }
    }
    x374_ = 0.5f * downFactor + 2.f;
    x378_ = 2.5f;
}

void CBallCamera::BuildSpline(CStateManager& mgr)
{

}

bool CBallCamera::ShouldResetSpline(CStateManager& mgr) const
{
    return false;
}

void CBallCamera::UpdatePlayerMovement(float dt, CStateManager& mgr)
{

}

void CBallCamera::UpdateTransform(const zeus::CVector3f& lookDir, const zeus::CVector3f& pos,
                                  float dt, CStateManager& mgr)
{

}

zeus::CVector3f CBallCamera::ConstrainYawAngle(const CPlayer& player, float f1, float f2, float dt) const
{
    return {};
}

void CBallCamera::CheckFailsafe(float dt, CStateManager& mgr)
{

}

void CBallCamera::UpdateObjectTooCloseId(CStateManager& mgr)
{

}

void CBallCamera::UpdateAnglePerSecond(float dt)
{
    float delta = x1a8_targetAnglePerSecond - x1a4_curAnglePerSecond;
    if (std::fabs(delta) >= M_PIF / 1800.f)
        x1a4_curAnglePerSecond += zeus::clamp(-1.f, delta / M_PIF, 1.f) * (10.471975f * dt);
    else
        x1a4_curAnglePerSecond = x1a8_targetAnglePerSecond;
}

void CBallCamera::UpdateUsingPathCameras(float dt, CStateManager& mgr)
{

}

zeus::CVector3f CBallCamera::GetFixedLookTarget(const zeus::CVector3f& pos, CStateManager& mgr) const
{
    return {};
}

void CBallCamera::UpdateUsingFixedCameras(float dt, CStateManager& mgr)
{

}

zeus::CVector3f CBallCamera::ComputeVelocity(const zeus::CVector3f& curVel, const zeus::CVector3f& posDelta) const
{
    return {};
}

zeus::CVector3f CBallCamera::TweenVelocity(const zeus::CVector3f& curVel, const zeus::CVector3f& newVel,
                                           float rate, float dt)
{
    return {};
}

zeus::CVector3f CBallCamera::MoveCollisionActor(const zeus::CVector3f& pos, float dt, CStateManager& mgr)
{
    return {};
}

void CBallCamera::UpdateUsingFreeLook(float dt, CStateManager& mgr)
{

}

zeus::CVector3f CBallCamera::InterpolateCameraElevation(const zeus::CVector3f& camPos) const
{
    return {};
}

zeus::CVector3f CBallCamera::CalculateCollidersCentroid(const std::vector<CCameraCollider>& colliderList, int w1) const
{
    return {};
}

zeus::CVector3f CBallCamera::ApplyColliders()
{
    return {};
}

void CBallCamera::UpdateColliders(const zeus::CTransform& xf, std::vector<CCameraCollider>& colliderList, int& r6,
                                  int r7, float f1, const rstl::reserved_vector<TUniqueId, 1024>& nearList, float f2,
                                  CStateManager& mgr)
{

}

void CBallCamera::AvoidGeometry(const zeus::CTransform& xf, const rstl::reserved_vector<TUniqueId, 1024>& nearList,
                                float dt, CStateManager& mgr)
{

}

void CBallCamera::AvoidGeometryFull(const zeus::CTransform& xf, const rstl::reserved_vector<TUniqueId, 1024>& nearList,
                                    float dt, CStateManager& mgr)
{

}

zeus::CAABox CBallCamera::CalculateCollidersBoundingBox(const std::vector<CCameraCollider>& colliderList,
                                                        CStateManager& mgr) const
{
    return {};
}

int CBallCamera::CountObscuredColliders(const std::vector<CCameraCollider>& colliderList) const
{
    int ret = 0;
    for (const CCameraCollider& c : colliderList)
        if (c.x4c_occlusionCount >= 2)
            ++ret;
    return ret;
}

void CBallCamera::UpdateCollidersDistances(std::vector<CCameraCollider>& colliderList, float f1, float f2, float f3)
{

}

void CBallCamera::UpdateUsingColliders(float dt, CStateManager& mgr)
{

}

void CBallCamera::UpdateUsingSpindleCameras(float dt, CStateManager& mgr)
{

}

zeus::CVector3f CBallCamera::ClampElevationToWater(zeus::CVector3f& pos, CStateManager& mgr) const
{
    return {};
}

void CBallCamera::UpdateUsingTransitions(float dt, CStateManager& mgr)
{

}

zeus::CTransform CBallCamera::UpdateCameraPositions(float dt, const zeus::CTransform& oldXf,
                                                    const zeus::CTransform& newXf)
{
    return {};
}

bool CBallCamera::CheckFailsafeFromMorphBallState(CStateManager& mgr) const
{
    return false;
}

bool CBallCamera::SplineIntersectTest(CMaterialList& intersectMat, CStateManager& mgr) const
{
    return false;
}

bool CBallCamera::IsBallNearDoor(CStateManager& mgr) const
{
    return false;
}

void CBallCamera::ActivateFailsafe(float dt, CStateManager& mgr)
{

}

void CBallCamera::ConstrainElevationAndDistance(float& elevation, float& distance, float f1, CStateManager& mgr)
{

}

zeus::CVector3f CBallCamera::FindDesiredPosition(float distance, float elevation,
                                                 const zeus::CVector3f& dir, CStateManager& mgr)
{
    return {};
}

bool CBallCamera::DetectCollision(const zeus::CVector3f& from, const zeus::CVector3f& to, float margin,
                                  float& d, CStateManager& mgr)
{
    return false;
}

void CBallCamera::Think(float dt, CStateManager& mgr)
{
    mgr.SetActorAreaId(*this, mgr.GetNextAreaId());
    UpdatePlayerMovement(dt, mgr);
    TCastToPtr<CCollisionActor> colAct = mgr.ObjectById(x46c_collisionActorId);
    if (colAct)
        mgr.SetActorAreaId(*colAct, mgr.GetNextAreaId());

    switch (mgr.GetPlayer().GetCameraState())
    {
    default:
        if (!x18d_27_)
        {
            if (colAct)
                colAct->SetActive(false);
            return;
        }
    case CPlayer::EPlayerCameraState::Ball:
    case CPlayer::EPlayerCameraState::Transitioning:
    case CPlayer::EPlayerCameraState::Two:
    {
        if (colAct)
            colAct->SetActive(true);
        zeus::CTransform oldXf = x34_transform;
        if (mgr.GetPlayer().GetBombJumpCount() != 1)
            UpdateLookAtPosition(dt, mgr);
        CheckFailsafe(dt, mgr);
        UpdateObjectTooCloseId(mgr);
        UpdateAnglePerSecond(dt);
        switch (x400_state)
        {
        case EBallCameraState::Zero:
        case EBallCameraState::Two:
        case EBallCameraState::Three:
            switch (x188_behaviour)
            {
            case EBallCameraBehaviour::Seven:
                UpdateUsingPathCameras(dt, mgr);
                break;
            case EBallCameraBehaviour::Four:
            case EBallCameraBehaviour::Five:
                UpdateUsingFixedCameras(dt, mgr);
                break;
            case EBallCameraBehaviour::Six:
            case EBallCameraBehaviour::Zero:
            case EBallCameraBehaviour::One:
            case EBallCameraBehaviour::Two:
                if (x36c_)
                    UpdateUsingFreeLook(dt, mgr);
                else
                    UpdateUsingColliders(dt, mgr);
                break;
            case EBallCameraBehaviour::Eight:
                UpdateUsingSpindleCameras(dt, mgr);
                break;
            default:
                break;
            }
            break;
        case EBallCameraState::Four:
        case EBallCameraState::Five:
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

bool CBallCamera::TransitionFromMorphBallState(CStateManager& mgr)
{
    return false;
}

void CBallCamera::TeleportCamera(const zeus::CVector3f& pos, CStateManager& mgr)
{

}

void CBallCamera::TeleportCamera(const zeus::CTransform& xf, CStateManager& mgr)
{

}

void CBallCamera::ResetToTweaks(CStateManager& mgr)
{

}

void CBallCamera::UpdateLookAtPosition(float dt, CStateManager& mgr)
{

}

zeus::CTransform CBallCamera::UpdateLookDirection(const zeus::CVector3f& dir, CStateManager& mgr)
{
    return {};
}

void CBallCamera::ApplyCameraHint(CStateManager& mgr)
{

}

void CBallCamera::ResetPosition()
{

}

void CBallCamera::DoorClosed(TUniqueId doorId)
{
    if (doorId != x3dc_tooCloseActorId)
        return;
    x18e_27_nearbyDoorClosed = true;
}

void CBallCamera::DoorClosing(TUniqueId doorId)
{
    if (doorId != x3dc_tooCloseActorId)
        return;
    x18e_28_nearbyDoorClosing = true;
}

}
