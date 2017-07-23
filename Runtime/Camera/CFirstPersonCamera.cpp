#include "CFirstPersonCamera.hpp"
#include "GameGlobalObjects.hpp"
#include "Character/CCharLayoutInfo.hpp"
#include "CStateManager.hpp"
#include "World/CPlayer.hpp"
#include "World/CPlayerCameraBob.hpp"
#include "World/CScriptGrapplePoint.hpp"
#include "Particle/CGenDescription.hpp"
#include "TCastTo.hpp"
#include <math.h>

namespace urde
{

CFirstPersonCamera::CFirstPersonCamera(TUniqueId uid, const zeus::CTransform& xf, TUniqueId watchedObj, float f1,
                                       float fov, float nearz, float farz, float aspect)
: CGameCamera(uid, true, "First Person Camera", CEntityInfo(kInvalidAreaId, CEntity::NullConnectionList), xf, fov,
              nearz, farz, aspect, watchedObj, false, 0)
{
}

void CFirstPersonCamera::Accept(IVisitor& visitor)
{
    visitor.Visit(this);
}

void CFirstPersonCamera::PreThink(float, CStateManager&) {}

void CFirstPersonCamera::Think(float, CStateManager&)
{

}

void CFirstPersonCamera::ProcessInput(const CFinalInput&, CStateManager& mgr) {}

void CFirstPersonCamera::Reset(const zeus::CTransform&, CStateManager& mgr) {}

void CFirstPersonCamera::SkipCinematic()
{
    x1c8_ = zeus::CVector3f::skZero;
    x1d4_ = 0.f;
}

void CFirstPersonCamera::CalculateGunFollowOrientationAndTransform(zeus::CTransform& gunXf, zeus::CQuaternion& gunQ,
                                                                   float dt, zeus::CVector3f& rVec)
{
    zeus::CVector3f gunFrontVec = x190_gunFollowXf.frontVector();
    gunFrontVec.z = 0.f;

    if (gunFrontVec.canBeNormalized())
        gunFrontVec.normalize();

    zeus::CVector3f rVecNoZ = rVec;
    rVecNoZ.z = 0.f;
    if (rVecNoZ.canBeNormalized())
        rVecNoZ.normalize();

    gunXf = zeus::CQuaternion::lookAt(rVecNoZ, gunFrontVec, zeus::CRelAngle::FromDegrees(360.f)).toTransform() *
            gunXf.getRotation();

    zeus::CVector3f newgunFront = gunXf.frontVector();

    if (newgunFront.canBeNormalized())
        newgunFront.normalize();

    float angle = newgunFront.dot(rVec);
    if (std::fabs(angle) > 0.f)
        angle = (angle > -0.f ? -1.f : 1.f);
    gunQ = zeus::CQuaternion::lookAt(rVec, newgunFront, zeus::clamp(0.f, std::acos(angle) / dt, 1.f));
}

void CFirstPersonCamera::UpdateTransform(CStateManager& mgr, float dt)
{
    TCastToPtr<CPlayer> player(mgr.ObjectById(GetWatchedObject()));
    if (!player)
    {
        x34_transform = zeus::CTransform::Identity();
        xe4_27_ = true;
        xe4_28_ = true;
        xe4_29_ = true;
        return;
    }

    zeus::CTransform playerXf = player->GetTransform();
    zeus::CVector3f rVec =
        playerXf.rotate({0.f, std::min(std::fabs(std::cos(x1c0_)), 1.0f), std::min(std::fabs(std::sin(x1c0_)), 1.0f)});
    if (player->x3dc_inFreeLook)
    {
        float angle = player->x3ec_freeLookPitchAngle;
        if (std::fabs(player->x3ec_freeLookPitchAngle) > (g_tweakPlayer->GetX124() - std::fabs(x1c0_)))
            angle = (player->x3ec_freeLookPitchAngle > -0.f ? -1.f : 1.f);
        zeus::CVector3f vec;
        vec.z = std::sin(angle);
        vec.y = std::cos(-player->x3e4_) * std::cos(angle);
        vec.x = std::sin(-player->x3e4_) * std::cos(angle);
        if (g_tweakPlayer->GetX228_24() && !zeus::close_enough(vec, zeus::CVector3f::skZero))
            vec.normalize();

        rVec = zeus::CQuaternion::lookAt({0.f, 1.f, 0.f}, rVec, zeus::CRelAngle::FromDegrees(360.f)).transform(vec);
    }

    zeus::CVector3f eyePos = player->GetEyePosition();
    if (x1d4_ > 0.f)
    {
        eyePos += zeus::clamp(0.f, 0.5f * x1d4_, 1.f) * x1c8_;
        player->GetCameraBob()->ResetCameraBobTime();
        player->GetCameraBob()->SetCameraBobTransform(zeus::CTransform::Identity());
    }

    if (player->GetOrbitState() == CPlayer::EPlayerOrbitState::Four ||
        player->GetOrbitState() == CPlayer::EPlayerOrbitState::One)
    {
        const CActor* act = TCastToConstPtr<CActor>(mgr.GetObjectById(player->x310_orbitTargetId));
        if (act && act->GetMaterialList().Intersection(CMaterialList(EMaterialTypes::Lava)) != 0)
        {
            zeus::CVector3f v = player->x314_orbitPoint.y - eyePos;
            if (v.canBeNormalized())
                v.normalize();

            rVec = v;
        }
    }
    else if (player->GetOrbitState() == CPlayer::EPlayerOrbitState::Zero &&
             player->GetMorphballTransitionState() == CPlayer::EPlayerMorphBallState::Unmorphed &&
             player->x3dc_inFreeLook && x1c4_pitchId == kInvalidUniqueId)
    {
        if (player->x294_jumpCameraPitchTimer > 0.f)
        {
            float angle = zeus::clamp(0.f, (player->x294_jumpCameraPitchTimer - g_tweakPlayer->GetX288()) / g_tweakPlayer->GetX28c(), 1.f) *
                          g_tweakPlayer->GetX290();
            angle += x1c0_;
            rVec.x = 0.f;
            rVec.y = std::cos(angle);
            rVec.z = -std::sin(angle);

            rVec = playerXf.rotate(rVec);
        }
        else if (player->x29c_spaceJumpCameraPitchTimer > 0.f)
        {
            float angle = zeus::clamp(0.f, (player->x29c_spaceJumpCameraPitchTimer - g_tweakPlayer->GetX294()) / g_tweakPlayer->GetX298(), 1.f) *
                          g_tweakPlayer->GetX29C();
            rVec.x = 0.f;
            rVec.y = std::cos(angle);
            rVec.z = -std::sin(angle);

            rVec = playerXf.rotate(rVec);
        }
    }

    if (rVec.canBeNormalized())
        rVec.normalize();

    zeus::CTransform gunXf = x190_gunFollowXf;
    zeus::CQuaternion qGun = zeus::CQuaternion::skNoRotation;

    if (player->x3dc_inFreeLook)
    {
        if (player->GetOrbitState() == CPlayer::EPlayerOrbitState::Four ||
            player->GetOrbitState() == CPlayer::EPlayerOrbitState::One)
        {
            zeus::CVector3f gunFrontVec = gunXf.frontVector();

            if (gunFrontVec.canBeNormalized())
                gunFrontVec.normalize();

            float scaledDt = (dt * g_tweakPlayer->GetX184());
            float angle = gunFrontVec.dot(rVec);
            if (std::fabs(angle) > 1.f)
                angle = (angle > -0.f ? -1.f : 1.f);
            float clampedAngle = zeus::clamp(0.f, std::acos(angle) / scaledDt, 1.f);
            if (angle > 0.999f && x18c_ && !player->x374_)
                qGun = zeus::CQuaternion::lookAt(rVec, gunFrontVec, zeus::CRelAngle::FromDegrees(360.f));
            else
                qGun = zeus::CQuaternion::lookAt(rVec, gunFrontVec, scaledDt * clampedAngle);

            const CScriptGrapplePoint* gPoint =
                TCastToConstPtr<CScriptGrapplePoint>(mgr.GetObjectById(player->x310_orbitTargetId));
            if (gPoint && player->x29c_spaceJumpCameraPitchTimer > 0.f)
            {
                gunFrontVec = x190_gunFollowXf.frontVector();
                if (gunFrontVec.canBeNormalized())
                    gunFrontVec.normalize();

                zeus::CVector3f rVecCpy = rVec;
                if (rVecCpy.canBeNormalized())
                    rVecCpy.normalize();

                gunXf =
                    zeus::CQuaternion::lookAt(rVecCpy, gunFrontVec, zeus::CRelAngle::FromDegrees(360.f)).toTransform() *
                    x190_gunFollowXf.getRotation();

                gunFrontVec = gunXf.frontVector();
                if (gunFrontVec.canBeNormalized())
                    gunFrontVec.normalize();

                /* BUG: This is exactly what the runtime is doing, should we restore the intended behavior? */
                float angle = gunFrontVec.dot(rVec);
                float sdt = dt * g_tweakPlayer->GetX2B0();

                if (std::fabs(angle) > 1.0f)
                    angle = (angle > -0.f ? -1.f : 1.f);

                angle = zeus::clamp(0.f, std::acos(angle) / sdt, 1.f);
                qGun = zeus::CQuaternion::lookAt(rVec, gunFrontVec, zeus::CRelAngle::FromDegrees(360.f));
            }
        }
        else if (player->GetOrbitState() == CPlayer::EPlayerOrbitState::Two ||
                 player->GetOrbitState() == CPlayer::EPlayerOrbitState::Three)
        {
            dt *= g_tweakPlayer->GetX184();
            CalculateGunFollowOrientationAndTransform(gunXf, qGun, dt, rVec);
        }
        else if (player->GetOrbitState() == CPlayer::EPlayerOrbitState::Five)
        {
            dt *= g_tweakPlayer->GetX2B0();
            CalculateGunFollowOrientationAndTransform(gunXf, qGun, dt, rVec);
        }
        else
        {
            dt *= g_tweakPlayer->GetX280();
            CalculateGunFollowOrientationAndTransform(gunXf, qGun, dt, rVec);
        }
    }
    else
    {
        zeus::CVector3f gunFront = x190_gunFollowXf.frontVector();
        if (gunFront.canBeNormalized())
            gunFront.normalize();

        zeus::CVector3f rVecCpy = rVec;
        if (rVecCpy.canBeNormalized())
            rVecCpy.normalize();

        gunXf = zeus::CQuaternion::lookAt(rVecCpy, gunFront, zeus::CRelAngle::FromDegrees(360.f)).toTransform() *
                x190_gunFollowXf.getRotation();
        gunFront = gunXf.frontVector();
        if (gunFront.canBeNormalized())
            gunFront.normalize();

        float angle = gunFront.dot(rVec);
        if (std::fabs(angle) > 1.f)
            angle = (angle > -0.f ? -1.f : 1.f);
        float sdt = dt * g_tweakPlayer->GetX138();
        qGun = zeus::CQuaternion::lookAt(
            rVec, gunFront, sdt * zeus::clamp(0.f, g_tweakPlayer->GetX14C() * (std::acos(angle) / sdt), 1.f));
    }
    zeus::CTransform bobXf = player->GetCameraBob()->GetCameraBobTransformation();

    if (player->GetMorphballTransitionState() == CPlayer::EPlayerMorphBallState::Morphed ||
        player->GetOrbitState() == CPlayer::EPlayerOrbitState::Five ||
        player->GetGrappleState() == CPlayer::EGrappleState::None ||
        mgr.GetGameState() == CStateManager::EGameState::SoftPaused ||
        mgr.GetCameraManager()->IsInCinematicCamera())
    {
        bobXf = zeus::CTransform::Identity();
        player->GetCameraBob()->SetCameraBobTransform(bobXf);
    }

    x190_gunFollowXf = qGun.toTransform() * gunXf;
    x34_transform = x190_gunFollowXf * bobXf.getRotation();

    xe4_27_ = true;
    xe4_28_ = true;
    xe4_28_ = true;
    xe4_29_ = true;

    CActor::SetTranslation(x190_gunFollowXf.origin + player->GetTransform().rotate(bobXf.origin));
    x190_gunFollowXf.orthonormalize();
}

void CFirstPersonCamera::UpdateElevation(CStateManager&) {}
}
