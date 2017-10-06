#include "CCameraManager.hpp"
#include "CCameraShakeData.hpp"
#include "CFirstPersonCamera.hpp"
#include "CStateManager.hpp"
#include "World/CScriptWater.hpp"
#include "World/CPlayer.hpp"
#include "GameGlobalObjects.hpp"
#include "TCastTo.hpp"
#include "CCinematicCamera.hpp"
#include "CBallCamera.hpp"
#include "CInterpolationCamera.hpp"
#include "World/CScriptCameraHint.hpp"
#include "CPathCamera.hpp"
#include "World/CScriptSpindleCamera.hpp"

namespace urde
{
float CCameraManager::sAspect = 1.42f;
float CCameraManager::sFarPlane = 750.f;
float CCameraManager::sNearPlane = 0.2f;
float CCameraManager::sFirstPersonFOV = 55.f;
float CCameraManager::sThirdPersonFOV = 60.f;

CCameraManager::CCameraManager(TUniqueId curCameraId) : x0_curCameraId(curCameraId)
{
    CSfxManager::AddListener(CSfxManager::ESfxChannels::Game, zeus::CVector3f::skZero, zeus::CVector3f::skZero,
                             {1.f, 0.f, 0.f}, {0.f, 0.f, 1.f}, 50.f, 50.f, 1000.f, 1, 1.f);
    sAspect = float(g_Viewport.x8_width / g_Viewport.xc_height);
    sFirstPersonFOV = g_tweakGame->GetFirstPersonFOV();
}

bool CCameraManager::IsInFirstPersonCamera() const
{
    return x7c_fpCamera->GetUniqueId() == x0_curCameraId;
}

zeus::CVector3f CCameraManager::GetGlobalCameraTranslation(const CStateManager& stateMgr) const
{
    TCastToConstPtr<CGameCamera> camera(GetCurrentCamera(stateMgr));
    return camera->GetTransform() * x30_shakeOffset;
}

zeus::CTransform CCameraManager::GetCurrentCameraTransform(const CStateManager& stateMgr) const
{
    TCastToConstPtr<CGameCamera> camera(GetCurrentCamera(stateMgr));
    return camera->GetTransform() * zeus::CTransform::Translate(x30_shakeOffset);
}

void CCameraManager::RemoveCameraShaker(int id)
{
    for (auto it = x14_shakers.begin(); it != x14_shakers.end(); ++it)
        if (it->xbc_shakerId == id)
        {
            x14_shakers.erase(it);
            break;
        }
}

int CCameraManager::AddCameraShaker(const CCameraShakeData& data, bool sfx)
{
    x14_shakers.emplace_back(data);
    x14_shakers.back().xbc_shakerId = ++x2c_lastShakeId;
    if (!xa0_24_pendingRumble)
    {
        xa0_24_pendingRumble = true;
        x90_rumbleCooldown = 0.5f;
    }
    if (sfx && data.x0_duration > 0.f)
    {
        float vol =zeus::clamp(100.f, std::max(data.GetSomething(), data.GetSomething2()) * 9.f + 100.f, 127.f);
        CSfxHandle sfxHandle;
        if (data.xc0_flags & 0x1)
            sfxHandle = CSfxManager::AddEmitter(1133, data.xc4_sfxPos, zeus::CVector3f::skZero,
                                                vol / 127.f, false, false, 0x7f, kInvalidAreaId);
        else
            sfxHandle = CSfxManager::SfxStart(1133, vol / 127.f, 0.f, false, 0x7f, false, kInvalidAreaId);
        sfxHandle->SetTimeRemaining(data.x0_duration);
    }
    return x2c_lastShakeId;
}

void CCameraManager::AddCinemaCamera(TUniqueId id, CStateManager& stateMgr) { x4_cineCameras.push_back(id); }

void CCameraManager::SetInsideFluid(bool val, TUniqueId fluidId)
{
    if (val)
    {
        ++x74_fluidCounter;
        x78_fluidId = fluidId;
    }
    else
        --x74_fluidCounter;
}

void CCameraManager::Update(float dt, CStateManager& stateMgr)
{
    UpdateCameraHints(dt, stateMgr);
    ThinkCameras(dt, stateMgr);
    UpdateListener(stateMgr);
    UpdateRumble(dt, stateMgr);
    UpdateFog(dt, stateMgr);
}

CGameCamera* CCameraManager::GetCurrentCamera(CStateManager& stateMgr) const
{
    CObjectList* camList = stateMgr.ObjectListById(EGameObjectList::GameCamera);
    return static_cast<CGameCamera*>(camList->GetObjectById(GetCurrentCameraId()));
}

const CGameCamera* CCameraManager::GetCurrentCamera(const CStateManager& stateMgr) const
{
    const CObjectList* camList = stateMgr.GetObjectListById(EGameObjectList::GameCamera);
    return static_cast<const CGameCamera*>(camList->GetObjectById(GetCurrentCameraId()));
}

void CCameraManager::CreateStandardCameras(CStateManager& stateMgr)
{
    TUniqueId fpId = stateMgr.AllocateUniqueId();
    x7c_fpCamera = new CFirstPersonCamera(fpId, zeus::CTransform::Identity(),
                                          stateMgr.Player()->GetUniqueId(), g_tweakPlayer->GetOrbitCameraSpeed(),
                                          sFirstPersonFOV, sNearPlane, sFarPlane, sAspect);
    stateMgr.AddObject(x7c_fpCamera);
    stateMgr.Player()->SetCameraState(CPlayer::EPlayerCameraState::FirstPerson, stateMgr);
    SetCurrentCameraId(fpId, stateMgr);

    x80_ballCamera = new CBallCamera(stateMgr.AllocateUniqueId(), stateMgr.Player()->GetUniqueId(),
                                     zeus::CTransform::Identity(), sThirdPersonFOV,
                                     sNearPlane, sFarPlane, sAspect);
    stateMgr.AddObject(x80_ballCamera);

    x88_interpCamera = new CInterpolationCamera(stateMgr.AllocateUniqueId(), zeus::CTransform::Identity());
    stateMgr.AddObject(x88_interpCamera);
}

void CCameraManager::SkipCinematic(CStateManager& stateMgr)
{
    TUniqueId camId = GetCurrentCameraId();
    CCinematicCamera* ent = static_cast<CCinematicCamera*>(stateMgr.ObjectById(camId));
    while (ent)
    {
        ent->SetActive(false);
        ent->WasDeactivated(stateMgr);
        ent = TCastToPtr<CCinematicCamera>(GetCurrentCamera(stateMgr)).GetPtr();
    }
    stateMgr.GetPlayer().UpdateCinematicState(stateMgr);
    x7c_fpCamera->SkipCinematic();
}

float CCameraManager::sub80009148() const
{
    const zeus::CVector3f uVec = x7c_fpCamera->GetTransform().upVector();
    return 1.f - std::min(std::fabs(std::min(std::fabs(uVec.dot(zeus::CVector3f::skUp)), 1.f) /
                          std::cos(zeus::degToRad(30.f))), 1.f);
}

void CCameraManager::SetPathCamera(TUniqueId id, CStateManager& mgr)
{
    xa4_pathCamId = id;
    if (TCastToPtr<CPathCamera> cam = mgr.ObjectById(id))
    {
        cam->Reset(GetCurrentCameraTransform(mgr), mgr);
        x80_ballCamera->TeleportCamera(cam->GetTransform(), mgr);
    }
}

void CCameraManager::SetSpindleCamera(TUniqueId id, CStateManager& mgr)
{
    xa2_spindleCamId = id;
    if (TCastToPtr<CScriptSpindleCamera> cam = mgr.ObjectById(id))
    {
        cam->Reset(GetCurrentCameraTransform(mgr), mgr);
        x80_ballCamera->TeleportCamera(cam->GetTransform(), mgr);
    }
}

void CCameraManager::InterpolateToBallCamera(const zeus::CTransform& xf, TUniqueId camId,
                                             const zeus::CVector3f& lookDir, float f1, float f2, float f3,
                                             bool b1, CStateManager& mgr)
{
    if (!IsInFirstPersonCamera())
    {
        x88_interpCamera->SetInterpolation(xf, lookDir, f1, f2, f3, camId, b1, mgr);
        if (!ShouldBypassInterpolation())
            SetCurrentCameraId(x88_interpCamera->GetUniqueId(), mgr);
    }
}

void CCameraManager::RestoreHintlessCamera(CStateManager& mgr)
{
    TCastToPtr<CScriptCameraHint> hint = mgr.ObjectById(xa6_camHintId);
    zeus::CTransform ballCamXf = x80_ballCamera->GetTransform();
    xa6_camHintId = kInvalidUniqueId;
    xa8_hintPriority = 1000;
    if (hint)
    {
        zeus::CVector3f camToPlayerFlat = mgr.GetPlayer().GetTranslation() - ballCamXf.origin;
        camToPlayerFlat.z = 0.f;
        if (camToPlayerFlat.canBeNormalized())
            camToPlayerFlat.normalize();
        else
            camToPlayerFlat = mgr.GetPlayer().GetMoveDir();

        x80_ballCamera->ResetToTweaks(mgr);
        x80_ballCamera->UpdateLookAtPosition(0.f, mgr);
        if (!mgr.GetPlayer().IsMorphBallTransitioning() &&
            hint->GetHint().GetBehaviourType() != CBallCamera::EBallCameraBehaviour::Zero)
        {
            if ((hint->GetHint().GetOverrideFlags() & 0x1000) != 0)
            {
                x80_ballCamera->SetX474(hint->GetHint().GetX4C());
                x80_ballCamera->SetX470(hint->GetHint().GetX5C());
            }
            else
            {
                x80_ballCamera->TeleportCamera(x80_ballCamera->UpdateLookDirection(camToPlayerFlat, mgr), mgr);
                InterpolateToBallCamera(ballCamXf, x80_ballCamera->GetUniqueId(), x80_ballCamera->GetX1D8(),
                                        hint->GetHint().GetX5C(), hint->GetHint().GetX4C(), hint->GetHint().GetX50(),
                                        ((hint->GetHint().GetOverrideFlags() >> 11) & 0x1) != 0, mgr);
            }
        }
    }
}

void CCameraManager::SkipBallCameraCinematic(CStateManager& mgr)
{
    if (IsInCinematicCamera())
    {
        x80_ballCamera->TeleportCamera(GetLastCineCamera(mgr)->GetTransform(), mgr);
        x80_ballCamera->SetFovInterpolation(GetLastCineCamera(mgr)->GetFov(), x80_ballCamera->GetFov(), 1.f, 0.f);
        SkipCinematic(mgr);
        SetCurrentCameraId(x80_ballCamera->GetUniqueId(), mgr);
    }
}

void CCameraManager::ApplyCameraHint(const CScriptCameraHint& hint, CStateManager& mgr)
{
    if (x80_ballCamera->GetState() == CBallCamera::EBallCameraState::Four)
    {
        x80_ballCamera->SetState(CBallCamera::EBallCameraState::Zero, mgr);
        mgr.GetPlayer().SetCameraState(CPlayer::EPlayerCameraState::Ball, mgr);
    }

    TCastToPtr<CScriptCameraHint> oldHint = mgr.ObjectById(xa6_camHintId);
    xa6_camHintId = hint.GetUniqueId();
    xa8_hintPriority = hint.GetPriority();

    zeus::CTransform camXf = GetCurrentCameraTransform(mgr);
    x80_ballCamera->ApplyCameraHint(mgr);

    if ((hint.GetHint().GetOverrideFlags() & 0x20) != 0)
        x80_ballCamera->ResetPosition();

    switch (hint.GetHint().GetBehaviourType())
    {
    case CBallCamera::EBallCameraBehaviour::Six:
    case CBallCamera::EBallCameraBehaviour::Seven:
        SetPathCamera(hint.GetDelegatedCamera(), mgr);
        break;
    case CBallCamera::EBallCameraBehaviour::Eight:
        SetSpindleCamera(hint.GetDelegatedCamera(), mgr);
        break;
    default:
        SetPathCamera(kInvalidUniqueId, mgr);
        SetSpindleCamera(kInvalidUniqueId, mgr);
        break;
    }

    if ((hint.GetHint().GetOverrideFlags() & 0x2000) != 0)
        SkipBallCameraCinematic(mgr);

    x80_ballCamera->UpdateLookAtPosition(0.f, mgr);

    if ((hint.GetHint().GetOverrideFlags() & 0x20) == 0 &&
        (hint.GetHint().GetBehaviourType() != CBallCamera::EBallCameraBehaviour::Zero ||
            (oldHint && oldHint->GetHint().GetBehaviourType() != CBallCamera::EBallCameraBehaviour::Zero)))
    {
        InterpolateToBallCamera(camXf, x80_ballCamera->GetUniqueId(), x80_ballCamera->GetX1D8(),
                                hint.GetHint().GetX58(), hint.GetHint().GetX4C(), hint.GetHint().GetX50(),
                                ((hint.GetHint().GetOverrideFlags() >> 10) & 0x1) != 0, mgr);
    }
}

void CCameraManager::UpdateCameraHints(float, CStateManager& mgr)
{
    bool r27 = false;
    for (auto it = xac_cameraHints.begin() ; it != xac_cameraHints.end() ;)
    {
        if (!TCastToPtr<CScriptCameraHint>(mgr.ObjectById(it->second)))
        {
            r27 = true;
            it = xac_cameraHints.erase(it);
            continue;
        }
        ++it;
    }

    bool r26 = false;
    for (TUniqueId id : x2b0_inactiveCameraHints)
    {
        if (TCastToConstPtr<CScriptCameraHint> hint = mgr.GetObjectById(id))
        {
            if (hint->GetHelperCount() == 0 || hint->GetInactive())
            {
                for (auto it = xac_cameraHints.begin() ; it != xac_cameraHints.end() ; ++it)
                {
                    if (it->second == id)
                    {
                        xac_cameraHints.erase(it);
                        if (xa6_camHintId == id)
                        {
                            r26 = true;
                            SetPathCamera(kInvalidUniqueId, mgr);
                            SetSpindleCamera(kInvalidUniqueId, mgr);
                        }
                        break;
                    }
                }
            }
        }
    }
    x2b0_inactiveCameraHints.clear();

    bool r25 = false;
    for (TUniqueId id : x334_activeCameraHints)
    {
        if (TCastToConstPtr<CScriptCameraHint> hint = mgr.GetObjectById(id))
        {
            bool newActiveHint = false;
            for (auto it = xac_cameraHints.begin() ; it != xac_cameraHints.end() ; ++it)
            {
                if (it->second == id)
                {
                    newActiveHint = true;
                    break;
                }
            }

            if (!newActiveHint)
            {
                r25 = true;
                xac_cameraHints.emplace_back(hint->GetPriority(), id);
            }
        }
    }
    x334_activeCameraHints.clear();

    if (r26 || r25 || r27)
    {
        std::sort(xac_cameraHints.begin(), xac_cameraHints.end(),
                  [](const auto& a, const auto& b) { return a.first < b.first; });
        zeus::CTransform ballCamXf = x80_ballCamera->GetTransform();
        if ((r26 || r27) && xac_cameraHints.empty())
        {
            RestoreHintlessCamera(mgr);
            return;
        }
        bool r26b = false;
        CScriptCameraHint* foundHint = nullptr;
        for (auto& h : xac_cameraHints)
        {
            if (TCastToPtr<CScriptCameraHint> hint = mgr.ObjectById(h.second))
            {
                foundHint = hint.GetPtr();
                r26b = true;
                break;
            }
        }
        if (!r26b)
            RestoreHintlessCamera(mgr);

        bool r25b = false;
        if (foundHint && r26b)
        {
            if ((foundHint->GetHint().GetOverrideFlags() & 0x80) != 0 && xac_cameraHints.size() > 1)
            {
                zeus::CVector3f ballPos = mgr.GetPlayer().GetBallPosition();
                if ((foundHint->GetHint().GetOverrideFlags() & 0x100) != 0)
                {
                    zeus::CVector3f camToBall = ballPos - ballCamXf.origin;
                    if (camToBall.canBeNormalized())
                        camToBall.normalize();
                    else
                        camToBall = ballCamXf.basis[1];

                    for (auto it = xac_cameraHints.begin() + 1 ; it != xac_cameraHints.end() ; ++it)
                    {
                        if (TCastToPtr<CScriptCameraHint> hint = mgr.ObjectById(it->second))
                        {
                            if ((hint->GetHint().GetOverrideFlags() & 0x80) != 0 && hint->GetPriority() ==
                                                                                        foundHint->GetPriority() &&
                                hint->GetAreaIdAlways() == foundHint->GetAreaIdAlways())
                            {
                                zeus::CVector3f hintToBall = ballPos - foundHint->GetTranslation();
                                if (hintToBall.canBeNormalized())
                                    hintToBall.normalize();
                                else
                                    hintToBall = foundHint->GetTransform().basis[1];

                                float camHintDot = zeus::clamp(-1.f, camToBall.dot(hintToBall), 1.f);

                                zeus::CVector3f thisHintToBall = ballPos - hint->GetTranslation();
                                if (thisHintToBall.canBeNormalized())
                                    thisHintToBall.normalize();
                                else
                                    thisHintToBall = hint->GetTransform().basis[1];

                                float camThisHintDot = zeus::clamp(-1.f, camToBall.dot(thisHintToBall), 1.f);

                                if (camThisHintDot > camHintDot)
                                    foundHint = hint.GetPtr();
                            }
                            else
                            {
                                break;
                            }
                        }
                        else
                        {
                            break;
                        }
                    }
                }
                else
                {
                    if (TCastToConstPtr<CActor> act = mgr.GetObjectById(foundHint->GetFirstHelper()))
                    {
                        zeus::CVector3f ballPos = mgr.GetPlayer().GetBallPosition();
                        zeus::CVector3f f26 = act->GetTranslation() - ballPos;
                        zeus::CVector3f ballToHelper = f26;
                        if (ballToHelper.canBeNormalized())
                            ballToHelper.normalize();
                        else
                            ballToHelper = foundHint->GetTransform().basis[1];

                        for (auto it = xac_cameraHints.begin() + 1 ; it != xac_cameraHints.end() ; ++it)
                        {
                            if (TCastToPtr<CScriptCameraHint> hint = mgr.ObjectById(it->second))
                            {
                                if ((hint->GetHint().GetOverrideFlags() & 0x80) != 0 && hint->GetPriority() ==
                                                                                            foundHint->GetPriority() &&
                                    hint->GetAreaIdAlways() == foundHint->GetAreaIdAlways())
                                {
                                    zeus::CVector3f hintToHelper = act->GetTranslation() - foundHint->GetTranslation();
                                    if (hintToHelper.canBeNormalized())
                                        hintToHelper.normalize();
                                    else
                                        hintToHelper = foundHint->GetTransform().basis[1];

                                    float ballHintDot = zeus::clamp(-1.f, ballToHelper.dot(hintToHelper), 1.f);

                                    zeus::CVector3f thisBallToHelper = f26;
                                    if (thisBallToHelper.canBeNormalized())
                                        thisBallToHelper.normalize();
                                    else
                                        thisBallToHelper = hint->GetTransform().basis[1];

                                    zeus::CVector3f thisHintToHelper = act->GetTranslation() - hint->GetTranslation();
                                    if (thisHintToHelper.canBeNormalized())
                                        thisHintToHelper.normalize();
                                    else
                                        thisHintToHelper = hint->GetTransform().basis[1];

                                    float thisBallHintDot =
                                        zeus::clamp(-1.f, thisBallToHelper.dot(thisHintToHelper), 1.f);

                                    if (thisBallHintDot > ballHintDot)
                                        foundHint = hint.GetPtr();
                                }
                                else
                                {
                                    break;
                                }
                            }
                            else
                            {
                                break;
                            }
                        }
                    }
                }

                if (foundHint->GetUniqueId() != xa6_camHintId)
                    r25b = true;
            }
            else if (xa6_camHintId != foundHint->GetUniqueId())
            {
                if (foundHint->GetHint().GetBehaviourType() == CBallCamera::EBallCameraBehaviour::Three)
                {
                    if ((foundHint->GetHint().GetOverrideFlags() & 0x20) != 0)
                    {
                        x80_ballCamera->TeleportCamera(
                            zeus::lookAt(foundHint->GetTranslation(), x80_ballCamera->GetX1D8()), mgr);
                    }
                    DeleteCameraHint(foundHint->GetUniqueId(), mgr);
                    if ((foundHint->GetHint().GetOverrideFlags() & 0x2000) != 0)
                        SkipBallCameraCinematic(mgr);
                    r25b = false;
                }
                else
                {
                    r25b = true;
                }
            }

            if (r25b)
                ApplyCameraHint(*foundHint, mgr);
        }
    }
}

void CCameraManager::ThinkCameras(float dt, CStateManager& mgr)
{
    CGameCameraList gcList = mgr.GetCameraObjectList();

    for (CEntity* ent : gcList)
    {
        if (TCastToPtr<CGameCamera> gc = ent)
        {
            gc->Think(dt, mgr);
            gc->UpdatePerspective(dt);
        }
    }

    if (IsInCinematicCamera())
        return;

    TUniqueId camId = GetLastCameraId();
    if (const CGameCamera* cam = TCastToConstPtr<CGameCamera>(mgr.GetObjectById(camId)))
        x3bc_curFov = cam->GetFov();
}

void CCameraManager::UpdateFog(float dt, CStateManager& mgr)
{
    if (x98_fogDensitySpeed != 0.f)
    {
        x94_fogDensityFactor += dt * x98_fogDensitySpeed;
        if ((x98_fogDensitySpeed > 0.f) ? x94_fogDensityFactor > x9c_fogDensityFactorTarget :
                                          x94_fogDensityFactor < x9c_fogDensityFactorTarget)
        {
            x94_fogDensityFactor = x9c_fogDensityFactorTarget;
            x98_fogDensitySpeed = 0.f;
        }
    }

    if (x74_fluidCounter)
    {
        if (TCastToConstPtr<CScriptWater> water = mgr.GetObjectById(x78_fluidId))
        {
            zeus::CVector2f zRange(GetCurrentCamera(mgr)->GetNearClipDistance(),
                                   CalculateFogDensity(mgr, water.GetPtr()));
            x3c_fog.SetFogExplicit(ERglFogMode::PerspExp, water->GetFogColor(), zRange);
            if (mgr.GetPlayerState()->GetActiveVisor(mgr) == CPlayerState::EPlayerVisor::Thermal)
                mgr.GetCameraFilterPass(4).DisableFilter(0.f);
            else
                mgr.GetCameraFilterPass(4).SetFilter(EFilterType::Multiply, EFilterShape::Fullscreen,
                                                     0.f, water->GetFogColor(), {});
        }
        xa0_26_inWater = true;
    }
    else if (xa0_26_inWater)
    {
        mgr.GetCameraManager()->x3c_fog.DisableFog();
        mgr.GetCameraFilterPass(4).DisableFilter(0.f);
        xa0_26_inWater = false;
    }

    x3c_fog.Update(dt);
}

void CCameraManager::UpdateRumble(float dt, CStateManager& mgr)
{
    x30_shakeOffset = zeus::CVector3f::skZero;
    for (auto it = x14_shakers.begin() ; it != x14_shakers.end() ;)
    {
        CCameraShakeData& shaker = *it;
        shaker.Update(dt, mgr);
        if (shaker.x4_curTime >= shaker.x0_duration)
        {
            it = x14_shakers.erase(it);
            continue;
        }
        x30_shakeOffset += shaker.GetPoint();
        ++it;
    }

    if (!x14_shakers.empty() && !xa0_25_rumbling && xa0_24_pendingRumble)
    {
        mgr.GetRumbleManager().Rumble(mgr, ERumbleFxId::Six, 1.f, ERumblePriority::Two);
        xa0_25_rumbling = true;
    }

    if (x90_rumbleCooldown > 0.f)
    {
        x90_rumbleCooldown -= dt;
    }
    else if (xa0_25_rumbling)
    {
        xa0_24_pendingRumble = false;
        xa0_25_rumbling = false;
    }

    if (mgr.GetPlayer().GetCameraState() != CPlayer::EPlayerCameraState::FirstPerson &&
        !IsInCinematicCamera())
        x30_shakeOffset = zeus::CVector3f::skZero;
}

void CCameraManager::UpdateListener(CStateManager& mgr)
{
    const zeus::CTransform xf = GetCurrentCameraTransform(mgr);
    CSfxManager::UpdateListener(xf.origin, zeus::CVector3f::skZero, xf.frontVector(), xf.upVector(), 1.f);
}

float CCameraManager::CalculateFogDensity(CStateManager& mgr, const CScriptWater* water)
{
    float distanceFactor = 1.f - water->GetFluidPlane().GetAlpha();
    float distance = 0;
    if (mgr.GetPlayerState()->HasPowerUp(CPlayerState::EItemType::GravitySuit))
        distance = g_tweakGame->x5c_gravityWaterFogDistanceRange * distanceFactor + g_tweakGame->x58_gravityWaterFogDistanceBase;
    else
        distance = g_tweakGame->x54_waterFogDistanceRange * distanceFactor + g_tweakGame->x50_waterFogDistanceBase;

    return distance * x94_fogDensityFactor;
}

void CCameraManager::ResetCameras(CStateManager& mgr)
{
    zeus::CTransform xf = mgr.GetPlayer().CreateTransformFromMovementDirection();
    xf.origin = mgr.GetPlayer().GetEyePosition();

    for (CEntity* ent : mgr.GetCameraObjectList())
    {
        TCastToPtr<CGameCamera> camObj(ent);
        camObj->Reset(xf, mgr);
    }
}

void CCameraManager::SetSpecialCameras(CFirstPersonCamera& fp, CBallCamera& ball)
{
    x7c_fpCamera = &fp;
    x80_ballCamera = &ball;
}

void CCameraManager::ProcessInput(const CFinalInput& input, CStateManager& stateMgr)
{
    for (CEntity* ent : stateMgr.GetCameraObjectList())
    {
        if (ent == nullptr)
            continue;
        auto& cam = static_cast<CGameCamera&>(*ent);
        if (input.ControllerIdx() != cam.x16c_controllerIdx)
            continue;
        cam.ProcessInput(input, stateMgr);
    }
}

void CCameraManager::RenderCameras(const CStateManager& mgr)
{
    for (CEntity* cam : mgr.GetCameraObjectList())
        static_cast<CGameCamera*>(cam)->Render(mgr);
}

void CCameraManager::SetupBallCamera(CStateManager& mgr)
{

}

void CCameraManager::SetPlayerCamera(CStateManager& mgr, TUniqueId newCamId)
{

}

float CCameraManager::GetCameraBobMagnitude() const
{
    return 1.f - zeus::clamp(-1.f, zeus::clamp(-1.f,
        x7c_fpCamera->GetTransform().basis[1].dot(zeus::CVector3f::skUp), 1.f) /
        std::cos(2.f * M_PIF / 12.f), 1.f);
}

bool CCameraManager::HasBallCameraInitialPositionHint(CStateManager& mgr) const
{
    return false;
}

void CCameraManager::RemoveCinemaCamera(TUniqueId uid, CStateManager& mgr)
{
    x4_cineCameras.erase(std::remove(x4_cineCameras.begin(), x4_cineCameras.end(), uid));
}

void CCameraManager::DeleteCameraHint(TUniqueId id, CStateManager& mgr)
{
    if (TCastToPtr<CScriptCameraHint> hint = mgr.ObjectById(id))
    {
        auto search = std::find_if(x2b0_inactiveCameraHints.begin(), x2b0_inactiveCameraHints.end(),
                                   [id](TUniqueId tid) { return tid == id; });
        if (search == x2b0_inactiveCameraHints.end())
        {
            hint->ClearIdList();
            hint->SetInactive(true);
            if (x2b0_inactiveCameraHints.size() != 64)
                x2b0_inactiveCameraHints.push_back(id);
        }
    }
}

void CCameraManager::AddInactiveCameraHint(TUniqueId id, CStateManager& mgr)
{
    if (TCastToPtr<CScriptCameraHint> hint = mgr.ObjectById(id))
    {
        auto search = std::find_if(x2b0_inactiveCameraHints.begin(), x2b0_inactiveCameraHints.end(),
                                   [id](TUniqueId tid) { return tid == id; });
        if (search == x2b0_inactiveCameraHints.end() &&
            x2b0_inactiveCameraHints.size() != 64)
            x2b0_inactiveCameraHints.push_back(id);
    }
}

void CCameraManager::AddActiveCameraHint(TUniqueId id, CStateManager& mgr)
{
    if (TCastToPtr<CScriptCameraHint> hint = mgr.ObjectById(id))
    {
        auto search = std::find_if(x334_activeCameraHints.begin(), x334_activeCameraHints.end(),
                                   [id](TUniqueId tid) { return tid == id; });
        if (search == x334_activeCameraHints.end() && xac_cameraHints.size() != 64 &&
            x334_activeCameraHints.size() != 64)
            x334_activeCameraHints.push_back(id);
    }
}

TUniqueId CCameraManager::GetLastCineCameraId() const
{
    if (x4_cineCameras.empty())
        return kInvalidUniqueId;
    return x4_cineCameras.back();
}

const CCinematicCamera* CCameraManager::GetLastCineCamera(CStateManager& mgr) const
{
    return static_cast<const CCinematicCamera*>(mgr.GetObjectById(GetLastCineCameraId()));
}

const CScriptCameraHint* CCameraManager::GetCameraHint(CStateManager& mgr) const
{
    return TCastToConstPtr<CScriptCameraHint>(mgr.GetObjectById(xa6_camHintId)).GetPtr();
}

bool CCameraManager::HasCameraHint(CStateManager& mgr) const
{
    if (xac_cameraHints.empty() || xa6_camHintId == kInvalidUniqueId)
        return false;
    return mgr.GetObjectById(xa6_camHintId) != nullptr;
}
}
