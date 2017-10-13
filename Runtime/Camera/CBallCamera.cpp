#include "GameGlobalObjects.hpp"
#include "CBallCamera.hpp"
#include "TCastTo.hpp"
#include "CStateManager.hpp"
#include "Collision/CCollisionActor.hpp"
#include "World/CPlayer.hpp"
#include "Input/ControlMapper.hpp"
#include "Camera/CFirstPersonCamera.hpp"
#include "Camera/CPathCamera.hpp"
#include "World/CScriptSpindleCamera.hpp"
#include "World/CScriptCameraHint.hpp"
#include "World/CScriptDoor.hpp"
#include "World/CScriptWater.hpp"
#include "Collision/CGameCollision.hpp"

namespace urde
{

void CCameraSpring::Reset()
{
    x4_k2Sqrt = 2.f * std::sqrt(x0_k);
    x10_dx = 0.f;
}

float CCameraSpring::ApplyDistanceSpringNoMax(float targetX, float curX, float dt)
{
    float useX = xc_tardis * x10_dx * dt + curX;
    x10_dx += xc_tardis * (x0_k * (targetX - curX) - x4_k2Sqrt * x10_dx) * dt;
    return std::max(useX, targetX);
}

float CCameraSpring::ApplyDistanceSpring(float targetX, float curX, float dt)
{
    float useX = xc_tardis * x10_dx * dt + curX;
    x10_dx += xc_tardis * (x0_k * (targetX - curX) - x4_k2Sqrt * x10_dx) * dt;
    useX = std::max(useX, targetX);
    if (useX - targetX > x8_max)
        useX = targetX + x8_max;
    return useX;
}

CBallCamera::CBallCamera(TUniqueId uid, TUniqueId watchedId, const zeus::CTransform& xf,
                         float fovy, float znear, float zfar, float aspect)
: CGameCamera(uid, true, "Ball Camera",
              CEntityInfo(kInvalidAreaId, CEntity::NullConnectionList),
              xf, fovy, znear, zfar, aspect, watchedId, false, 0),
  x214_ballCameraSpring(g_tweakBall->GetBallCameraSpringConstant(),
                        g_tweakBall->GetBallCameraSpringMax(),
                        g_tweakBall->GetBallCameraSpringTardis()),
  x228_ballCameraCentroidSpring(g_tweakBall->GetBallCameraCentroidSpringConstant(),
                                g_tweakBall->GetBallCameraCentroidSpringMax(),
                                g_tweakBall->GetBallCameraCentroidSpringTardis()),
  x23c_ballCameraLookAtSpring(g_tweakBall->GetBallCameraLookAtSpringConstant(),
                              g_tweakBall->GetBallCameraLookAtSpringMax(),
                              g_tweakBall->GetBallCameraLookAtSpringTardis()),
  x250_ballCameraCentroidDistanceSpring(g_tweakBall->GetBallCameraCentroidDistanceSpringConstant(),
                                        g_tweakBall->GetBallCameraCentroidDistanceSpringMax(),
                                        g_tweakBall->GetBallCameraCentroidDistanceSpringTardis()),
  x37c_camSpline(false),
  x41c_ballCameraChaseSpring(g_tweakBall->GetBallCameraChaseSpringConstant(),
                             g_tweakBall->GetBallCameraChaseSpringMax(),
                             g_tweakBall->GetBallCameraChaseSpringTardis()),
  x448_ballCameraBoostSpring(g_tweakBall->GetBallCameraBoostSpringConstant(),
                             g_tweakBall->GetBallCameraBoostSpringMax(),
                             g_tweakBall->GetBallCameraBoostSpringTardis())
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
    x37c_camSpline.UpdateSplineLength();
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
    zeus::CVector3f ballPos = mgr.GetPlayer().GetBallPosition();
    x36c_ = 2;
    x370_24_ = false;
    x37c_camSpline.Reset(4);
    x37c_camSpline.AddKnot(GetTranslation(), zeus::CVector3f::skForward);
    float elevation = x1a0_elevation;
    float distance = x190_curMinDistance;
    ConstrainElevationAndDistance(elevation, distance, 0.f, mgr);
    zeus::CVector3f f30f31 = (ballPos.toVec2f() - GetTranslation().toVec2f()) * 0.5f + GetTranslation().toVec2f();
    f30f31.z = GetTranslation().z;
    zeus::CVector3f x978 = GetTranslation() - f30f31;
    zeus::CQuaternion rot;
    rot.rotateZ(zeus::degToRad(45.f));
    if (mgr.GetPlayer().GetMoveDir().cross(x34_transform.basis[1]).z >= 0.f)
    {
        rot = zeus::CQuaternion();
        rot.rotateZ(zeus::degToRad(-45.f));
    }
    x978 = rot.transform(x978);
    zeus::CVector3f x994 = f30f31 + x978;
    TUniqueId intersectId = kInvalidUniqueId;
    rstl::reserved_vector<TUniqueId, 1024> nearList;
    CRayCastResult result =
        mgr.RayWorldIntersection(intersectId, x994, -x978.normalized(), x978.magnitude(), BallCameraFilter, nearList);
    if (result.IsValid())
        x994 = x978.normalized() * 1.5f + result.GetPoint();
    else
        x994 = f30f31 + x978;
    x37c_camSpline.AddKnot(x994, zeus::CVector3f::skForward);
    FindDesiredPosition(distance, elevation, mgr.GetPlayer().GetMoveDir(), mgr);
    x978 = rot.transform(x978);
    zeus::CVector3f x9ac = f30f31 + x978;
    result =
        mgr.RayWorldIntersection(intersectId, x9ac, -x978.normalized(), x978.magnitude(), BallCameraFilter, nearList);
    if (result.IsValid())
        x9ac = x978.normalized() * 2.f + result.GetPoint();
    else
        x9ac = f30f31 + x978;
    x37c_camSpline.AddKnot(x9ac, zeus::CVector3f::skForward);
    x978 = rot.transform(x978);
    zeus::CVector3f x9b8 = x978 + f30f31;
    result =
        mgr.RayWorldIntersection(intersectId, x9b8, -x978.normalized(), x978.magnitude(), BallCameraFilter, nearList);
    if (result.IsValid())
        x9b8 = x978.normalized() * 2.f + result.GetPoint();
    else
        x9b8 = f30f31 + x978;
    x37c_camSpline.AddKnot(x9b8, zeus::CVector3f::skForward);
    CMaterialList intersectMat;
    if (!SplineIntersectTest(intersectMat, mgr) && intersectMat.HasMaterial(EMaterialTypes::Wall))
    {
        x978 = x994 - f30f31;
        result =
        mgr.RayWorldIntersection(intersectId, x994, -x978.normalized(), x978.magnitude(), BallCameraFilter, nearList);
        if (result.IsValid() && !result.GetMaterial().HasMaterial(EMaterialTypes::Pillar))
            x37c_camSpline.SetKnotPosition(1, result.GetPoint() - x978.normalized() * 0.3f * 1.25f);
        x978 = x9ac - f30f31;
        result =
        mgr.RayWorldIntersection(intersectId, x9ac, -x978.normalized(), x978.magnitude(), BallCameraFilter, nearList);
        if (result.IsValid() && !result.GetMaterial().HasMaterial(EMaterialTypes::Pillar))
            x37c_camSpline.SetKnotPosition(2, result.GetPoint() - x978.normalized() * 0.3f * 1.25f);
        x37c_camSpline.UpdateSplineLength();
        if (!SplineIntersectTest(intersectMat, mgr))
        {
            x36c_ = 0;
            return;
        }
    }
    x374_ = 0.5f;
    x378_ = 0.5f;
    x37c_camSpline.UpdateSplineLength();
    x3c8_ = CMaterialList();
}

bool CBallCamera::ShouldResetSpline(CStateManager& mgr) const
{
    return x400_state != EBallCameraState::Four && !mgr.GetCameraManager()->IsInterpolationCameraActive() &&
           mgr.GetPlayer().GetMorphBall()->GetSpiderBallState() != CMorphBall::ESpiderBallState::Active &&
           x36c_ == 0 && (x188_behaviour > EBallCameraBehaviour::Eight || x188_behaviour < EBallCameraBehaviour::Four);
}

void CBallCamera::UpdatePlayerMovement(float dt, CStateManager& mgr)
{
    x2ec_ = std::fabs(mgr.GetPlayer().GetActualBallMaxVelocity(dt));
    zeus::CVector3f ballPos = mgr.GetPlayer().GetBallPosition();
    x2f0_ = ballPos - x2dc_;
    x2fc_ = x2f0_;
    x2fc_.z = 0.f;
    if (x2fc_.canBeNormalized())
        x2e8_ = x2fc_.magnitude() / dt;
    else
        x2e8_ = 0.f;
    x2dc_ = ballPos;
    x18d_28_ = false;
    zeus::CVector3f x5c = ballPos - GetTranslation();
    x5c.z = 0.f;
    if (x5c.canBeNormalized())
    {
        x5c.normalize();
        if (std::fabs(std::acos(zeus::clamp(-1.f, x5c.dot(mgr.GetPlayer().GetMoveDir()), 1.f))) >
            zeus::degToRad(100.f))
            x18d_28_ = true;
    }
    x308_ = 0.f;
    float f3 = x2e8_ - 4.f;
    if (f3 > 0.f)
        x308_ = zeus::clamp(-1.f, std::fabs(std::sin(zeus::degToRad(f3 / (x2ec_ - 4.f) * 90.f))), 1.f);
    x190_curMinDistance = x308_ * (x198_maxDistance - x194_targetMinDistance) + x194_targetMinDistance;
    if (x308_ > 0.5f && mgr.GetPlayer().GetPlayerMovementState() == CPlayer::EPlayerMovementState::OnGround)
        x30c_ += dt * x308_;
    else
        x30c_ = 0.f;
    x30c_ = zeus::clamp(0.f, x30c_, 3.f);
}

void CBallCamera::UpdateTransform(const zeus::CVector3f& lookDir, const zeus::CVector3f& pos,
                                  float dt, CStateManager& mgr)
{
    zeus::CVector3f useLookDir = lookDir;
    if (x18d_31_)
        if (const CScriptCameraHint* hint = mgr.GetCameraManager()->GetCameraHint(mgr))
            useLookDir = hint->GetTransform().basis[1];
    zeus::CVector3f lookDirFlat = useLookDir;
    lookDirFlat.z = 0.f;
    if (!lookDirFlat.canBeNormalized())
    {
        SetTranslation(pos);
        return;
    }
    zeus::CVector3f curLookDir = x34_transform.basis[1];
    if (curLookDir.canBeNormalized())
    {
        curLookDir.normalize();
    }
    else
    {
        SetTransform(zeus::lookAt(pos, pos + lookDir));
        return;
    }
    float lookDirDot = zeus::clamp(-1.f, curLookDir.dot(lookDir), 1.f);
    if (std::fabs(lookDirDot) >= 1.f)
    {
        SetTransform(zeus::lookAt(pos, pos + lookDir));
    }
    else
    {
        float f31 = zeus::clamp(0.f, std::acos(lookDirDot) / (zeus::degToRad(60.f) * dt), 1.f);
        float x2b0 = dt * x1a4_curAnglePerSecond * f31;
        float f29 = std::fabs(zeus::clamp(-1.f, lookDir.dot(zeus::CVector3f::skUp), 1.f));
        float f28 = (1.f - f29) * zeus::degToRad(720.f) * dt;
        if (x36c_ == 1)
        {
            f28 = zeus::degToRad(240.f) * dt;
            if (x2b0 > f28)
                x2b0 = f28;
        }
        if (x2b0 > f28 && !mgr.GetPlayer().IsMorphBallTransitioning() && f29 > 1.f)
            x2b0 = f28;
        switch (x400_state)
        {
        case EBallCameraState::Two:
            if (x18c_25_)
                x2b0 = dt * x40c_chaseAnglePerSecond * f31;
            break;
        case EBallCameraState::Three:
            x2b0 = dt * x438_boostAnglePerSecond * f31;
            break;
        default:
            break;
        }
        if (x18d_26_ || mgr.GetCameraManager()->IsInterpolationCameraActive())
        {
            x18d_26_ = false;
            SetTransform(zeus::CQuaternion::lookAt(curLookDir, lookDir, 2.f * M_PIF).toTransform() *
                         x34_transform.getRotation());
        }
        else
        {
            SetTransform(zeus::CQuaternion::lookAt(curLookDir, lookDir, x2b0).toTransform() *
                         x34_transform.getRotation());
        }
    }
    SetTranslation(pos);
}

zeus::CVector3f CBallCamera::ConstrainYawAngle(const CPlayer& player, float angleVel, float maxAngle, float dt,
                                               CStateManager& mgr) const
{
    zeus::CVector3f playerToCamFlat = GetTranslation() - player.GetTranslation();
    playerToCamFlat.z = 0.f;
    zeus::CVector3f lookDir = player.GetTransform().basis[1];
    if (player.GetMorphballTransitionState() == CPlayer::EPlayerMorphBallState::Morphed)
    {
        lookDir = player.GetMoveDir();
        TCastToConstPtr<CScriptDoor> door = mgr.GetObjectById(x3dc_tooCloseActorId);
        if ((!door || !door->x2a8_26_) &&
            (x400_state == EBallCameraState::Three || x400_state == EBallCameraState::Two))
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
    return zeus::CQuaternion::lookAt(playerToCamFlat, -lookDir, angleVel * dt *
        zeus::clamp(0.f, std::acos(angleProj) / maxAngle, 1.f)).transform(playerToCamFlat);
}

void CBallCamera::CheckFailsafe(float dt, CStateManager& mgr)
{
    zeus::CVector3f ballPos = mgr.GetPlayer().GetBallPosition();
    x18d_24_ = x18c_31_;
    zeus::CVector3f camToBall = ballPos - GetTranslation();
    float camToBallMag = camToBall.magnitude();
    camToBall.normalize();
    rstl::reserved_vector<TUniqueId, 1024> nearList;
    mgr.BuildNearList(nearList, GetTranslation(), camToBall, camToBallMag, BallCameraFilter, nullptr);
    CRayCastResult result =
        mgr.RayWorldIntersection(x368_, GetTranslation(), camToBall, camToBallMag, BallCameraFilter, nearList);
    if (result.IsValid())
    {
        x350_ = result.GetMaterial();
        if (!mgr.RayCollideWorld(GetTranslation(), ballPos, nearList, BallCameraFilter, &mgr.GetPlayer()) &&
            !mgr.RayCollideWorld(GetTranslation(), mgr.GetPlayer().GetTranslation(), nearList,
                                 BallCameraFilter, &mgr.GetPlayer()))
        {
            x18c_31_ = false;
            if (x18d_24_)
            {
                x35c_ = ballPos;
                if (ShouldResetSpline(mgr) && !x18e_25_ && x350_.HasMaterial(EMaterialTypes::Floor) &&
                    mgr.RayCollideWorld(ballPos, ballPos + zeus::CVector3f(0.f, 0.f, -2.5f),
                                        nearList, BallCameraFilter, nullptr))
                    ResetSpline(mgr);
            }
        }
    }
    else
    {
        x18c_31_ = true;
        x350_ = CMaterialList(EMaterialTypes::Unknown);
    }

    if (!x18c_31_)
    {
        x34c_ += dt;
        if (ShouldResetSpline(mgr) && !x18e_25_ && x350_.HasMaterial(EMaterialTypes::Pillar))
            BuildSpline(mgr);
    }
    else
    {
        x34c_ = 0.f;
    }

    x358_ = zeus::clamp(0.f, x34c_ * 0.5f, 1.f);

    x3e4_ = x18c_27_ && (x34c_ > 2.f || (x3dc_tooCloseActorId != kInvalidUniqueId && x34c_ > 1.f)) &&
            !x18c_31_ && x36c_ == 0;

    bool doFailsafe = x3e4_;
    if ((GetTranslation() - ballPos).magnitude() < 0.3f + g_tweakPlayer->GetPlayerBallHalfExtent())
        doFailsafe = true;

    if (x18e_27_nearbyDoorClosed)
    {
        x18e_27_nearbyDoorClosed = false;
        if (result.IsValid())
            doFailsafe = true;
    }

    if (x18e_28_nearbyDoorClosing)
    {
        x18e_28_nearbyDoorClosing = false;
        if (IsBallNearDoor(GetTranslation(), mgr))
            doFailsafe = true;
    }

    if (doFailsafe)
        ActivateFailsafe(dt, mgr);
}

void CBallCamera::UpdateObjectTooCloseId(CStateManager& mgr)
{
    x3e0_tooCloseActorDist = 1000000.f;
    x3dc_tooCloseActorId = kInvalidUniqueId;
    zeus::CVector3f ballPos = mgr.GetPlayer().GetBallPosition();
    for (CEntity* ent : mgr.GetPlatformAndDoorObjectList())
    {
        if (TCastToPtr<CScriptDoor> door = ent)
        {
            if (mgr.GetPlayer().GetAreaIdAlways() == door->GetAreaIdAlways())
            {
                door->GetBoundingBox();
                float minMag = std::min((door->GetTranslation() - GetTranslation()).magnitude(),
                                        (door->GetTranslation() - ballPos).magnitude());
                if (minMag < 30.f && minMag < x3e0_tooCloseActorDist)
                {
                    x3dc_tooCloseActorId = door->GetUniqueId();
                    x3e0_tooCloseActorDist = minMag;
                }
            }
        }
    }
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
    if (TCastToPtr<CPathCamera> cam = mgr.ObjectById(mgr.GetCameraManager()->GetPathCameraId()))
    {
        TeleportCamera(cam->GetTransform(), mgr);
        x18d_26_ = true;
    }
}

zeus::CVector3f CBallCamera::GetFixedLookTarget(const zeus::CVector3f& pos, CStateManager& mgr) const
{
    const CScriptCameraHint* hint = mgr.GetCameraManager()->GetCameraHint(mgr);
    if (!hint)
        return pos;
    zeus::CVector3f lookDir = hint->GetTransform().basis[1];
    zeus::CVector3f lookDirFlat = lookDir;
    lookDirFlat.z = 0.f;
    if (lookDir.canBeNormalized() && lookDirFlat.canBeNormalized())
    {
        lookDir.normalize();
        lookDirFlat.normalize();
    }
    else
    {
        lookDir = zeus::CVector3f::skForward;
        lookDirFlat = zeus::CVector3f::skForward;
    }

    zeus::CVector3f posFlat = pos;
    posFlat.z = 0.f;
    if (pos.canBeNormalized() && posFlat.canBeNormalized())
        posFlat.normalize();
    else
        posFlat = lookDirFlat;

    float f31 = std::acos(zeus::clamp(-1.f, pos.dot(posFlat), 1.f));
    if (x18c_29_)
    {
        float f1 = std::acos(zeus::clamp(-1.f, lookDir.dot(lookDirFlat), 1.f));
        f31 = f1 + zeus::clamp(-x1ac_, f31 - f1, x1ac_);
    }

    if (pos.z >= 0.f)
        f31 = -f31;

    float f4 = std::acos(zeus::clamp(-1.f, posFlat.dot(lookDirFlat), 1.f));
    if (x18c_30_)
        f4 = zeus::clamp(-x1b0_, f4, x1b0_);

    if (posFlat.x * lookDirFlat.y - lookDirFlat.x * posFlat.y >= 0.f)
        f4 = -f4;

    zeus::CQuaternion quat;
    quat.rotateZ(f4);
    zeus::CVector3f x6c = quat.transform(lookDirFlat);
    zeus::CVector3f x78(x6c.y, -x6c.x, 0.f);
    x78.normalize();
    return zeus::CQuaternion::fromAxisAngle(x78, -f31).transform(x6c);
}

void CBallCamera::UpdateUsingFixedCameras(float dt, CStateManager& mgr)
{
    if (const CScriptCameraHint* hint = mgr.GetCameraManager()->GetCameraHint(mgr))
    {
        switch (x188_behaviour)
        {
        case EBallCameraBehaviour::Four:
        {
            zeus::CVector3f hintToPos = x1d8_ - hint->GetTranslation();
            if (hintToPos.canBeNormalized())
            {
                hintToPos = GetFixedLookTarget(hintToPos.normalized(), mgr);
                if ((hint->GetHint().GetOverrideFlags() & 0x40) != 0)
                    x18d_26_ = true;
                UpdateTransform(hintToPos, hint->GetTranslation(), dt, mgr);
            }
            break;
        }
        case EBallCameraBehaviour::Five:
            SetTransform(hint->GetTransform());
            break;
        default:
            break;
        }
        TeleportCamera(GetTranslation(), mgr);
    }
}

zeus::CVector3f CBallCamera::ComputeVelocity(const zeus::CVector3f& curVel, const zeus::CVector3f& posDelta) const
{
    zeus::CVector3f ret = posDelta;
    if (x470_ > 0.f && ret.canBeNormalized() && !x18d_28_)
    {
        float mag = ret.magnitude();
        mag = zeus::clamp(-x474_, mag, x474_);
        ret = ret.normalized() * mag;
    }
    return ret;
}

zeus::CVector3f CBallCamera::TweenVelocity(const zeus::CVector3f& curVel, const zeus::CVector3f& newVel,
                                           float rate, float dt)
{
    zeus::CVector3f velDelta = newVel - curVel;
    if (velDelta.canBeNormalized())
    {
        float t = zeus::clamp(-1.f, velDelta.magnitude() / (rate * dt), 1.f);
        return velDelta.normalized() * rate * dt * t + curVel;
    }
    return newVel;
}

zeus::CVector3f CBallCamera::MoveCollisionActor(const zeus::CVector3f& pos, float dt, CStateManager& mgr)
{
    if (TCastToPtr<CPhysicsActor> act = mgr.ObjectById(x46c_collisionActorId))
    {
        zeus::CVector3f posDelta = pos - act->GetTranslation();
        if (!posDelta.canBeNormalized() || posDelta.magnitude() < 0.01f)
        {
            act->Stop();
            return act->GetTranslation();
        }
        zeus::CVector3f oldTranslation = act->GetTranslation();
        zeus::CVector3f oldVel = act->GetVelocity();
        zeus::CVector3f newVel = ComputeVelocity(oldVel, posDelta * (1.f / dt));
        act->SetVelocityWR(newVel);
        act->SetMovable(true);
        act->AddMaterial(EMaterialTypes::Solid, mgr);
        CGameCollision::Move(mgr, *act, dt, nullptr);
        zeus::CVector3f posDelta2 = act->GetTranslation() - pos;
        if (posDelta2.canBeNormalized() && posDelta2.magnitude() > 0.1f)
        {
            act->SetTranslation(oldTranslation);
            act->SetVelocityWR(TweenVelocity(oldVel, newVel, 50.f, dt));
            CGameCollision::Move(mgr, *act, dt, nullptr);
            posDelta2 = act->GetTranslation() - pos;
            if (posDelta2.magnitude() > 0.1f)
                x478_ += 1;
            else
                x478_ = 0;
        }
        else
        {
            act->Stop();
            x478_ = 0;
        }
        act->SetMovable(false);
        act->RemoveMaterial(EMaterialTypes::Solid, mgr);
        return act->GetTranslation();
    }
    return pos;
}

void CBallCamera::UpdateUsingFreeLook(float dt, CStateManager& mgr)
{
    if (x400_state == EBallCameraState::Four || x400_state == EBallCameraState::Five)
    {
        x36c_ = 0;
        return;
    }

    if (x36c_ == 1 && x188_behaviour <= EBallCameraBehaviour::Eight &&
        x188_behaviour >= EBallCameraBehaviour::Four)
    {
        x36c_ = 0;
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
    zeus::CVector3f desiredPos = FindDesiredPosition(distance, elevation, knotToBall, mgr);

    if (x370_24_)
        x37c_camSpline.SetKnotPosition(3, desiredPos);

    x374_ -= dt;

    float f26 = 1.f - zeus::clamp(0.f, x374_ / x378_, 1.f);
    if (x36c_ == 1)
    {
        CMaterialList intersectMat;
        if (!SplineIntersectTest(intersectMat, mgr))
        {
            x37c_camSpline.SetKnotPosition(3, knot3);
            if (intersectMat.HasMaterial(EMaterialTypes::Floor))
            {
                x36c_ = 0;
                return;
            }
        }
    }

    if (x374_ <= 0.f || (f26 > 0.75f && x18c_31_))
    {
        if (x36c_ == 2 && !x18c_31_)
        {
            CMaterialList intersectMat;
            if (!SplineIntersectTest(intersectMat, mgr))
            {
                x36c_ = 0;
            }
            else
            {
                zeus::CVector3f oldKnot2 = x37c_camSpline.GetKnotPosition(2);
                zeus::CVector3f oldKnot1 = x37c_camSpline.GetKnotPosition(1);
                BuildSpline(mgr);
                x37c_camSpline.SetKnotPosition(3, x37c_camSpline.GetKnotPosition(1));
                x37c_camSpline.SetKnotPosition(2, x37c_camSpline.GetKnotPosition(0));
                x37c_camSpline.SetKnotPosition(1, oldKnot2);
                x37c_camSpline.SetKnotPosition(0, oldKnot1);
                x37c_camSpline.UpdateSplineLength();
                x374_ = x378_ - x378_ * (x37c_camSpline.GetKnotT(2) / x37c_camSpline.x44_length);
                x374_ -= dt;
                f26 = zeus::clamp(0.f, x374_ / x378_, 1.f);
            }
        }
        else
        {
            x36c_ = 0;
        }
    }

    x37c_camSpline.UpdateSplineLength();
    zeus::CVector3f pos = x37c_camSpline.GetInterpolatedSplinePointByLength(f26 * x37c_camSpline.x44_length).origin;
    if (TCastToPtr<CPhysicsActor> act = mgr.ObjectById(x46c_collisionActorId))
    {
        CMaterialFilter filter = act->GetMaterialFilter();
        CMaterialFilter tmpFilter = filter;
        tmpFilter.IncludeList().Add(EMaterialTypes::Wall);
        tmpFilter.ExcludeList().Add(x3c8_);
        act->SetMaterialFilter(tmpFilter);
        MoveCollisionActor(pos, dt, mgr);
        act->SetMaterialFilter(filter);
    }

    zeus::CVector3f lookDir = x1d8_ - desiredPos;
    if (x18d_26_)
        lookDir = ballPos - desiredPos;

    if (lookDir.canBeNormalized())
    {
        lookDir.normalize();
        UpdateTransform(lookDir, desiredPos, dt, mgr);
    }

    TeleportCamera(desiredPos, mgr);

    if (x3d0_24_ && x374_ / x378_ < 0.5f)
        x36c_ = 0;
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
    if (TCastToPtr<CScriptSpindleCamera> cam = mgr.ObjectById(mgr.GetCameraManager()->GetSpindleCameraId()))
    {
        TeleportCamera(cam->GetTransform(), mgr);
        x18d_26_ = true;
    }
}

zeus::CVector3f CBallCamera::ClampElevationToWater(zeus::CVector3f& pos, CStateManager& mgr) const
{
    zeus::CVector3f ret = pos;
    if (TCastToConstPtr<CScriptWater> water = mgr.GetObjectById(mgr.GetPlayer().GetFluidId()))
    {
        float waterZ = water->GetTriggerBoundsWR().max.z;
        if (pos.z >= waterZ && pos.z - waterZ <= 0.25f)
            ret.z = 0.25f + waterZ;
        else if (pos.z < waterZ && pos.z - waterZ >= -0.12f)
            ret.z = waterZ - 0.12f;
    }
    return ret;
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

bool CBallCamera::IsBallNearDoor(const zeus::CVector3f& pos, CStateManager& mgr)
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

void CBallCamera::TeleportColliders(std::vector<CCameraCollider>& colliderList, const zeus::CVector3f& pos)
{
    for (CCameraCollider& collider : colliderList)
    {
        collider.x2c_ = pos;
        collider.x14_ = pos;
        collider.x20_ = pos;
    }
}

void CBallCamera::TeleportCamera(const zeus::CVector3f& pos, CStateManager& mgr)
{
    x294_ = pos;
    TeleportColliders(x264_smallColliders, pos);
    TeleportColliders(x274_mediumColliders, pos);
    TeleportColliders(x284_largeColliders, pos);
    if (TCastToPtr<CCollisionActor> act = mgr.ObjectById(x46c_collisionActorId))
        act->SetTranslation(pos);
}

void CBallCamera::TeleportCamera(const zeus::CTransform& xf, CStateManager& mgr)
{
    SetTransform(xf);
    TeleportCamera(xf.origin, mgr);
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
