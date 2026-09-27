#include "MetroidPrime/Player/CGrappleArm.hpp"

#include "Kyoto/Animation/CSkinnedModel.hpp"
#include "Kyoto/Animation/CVertexMorphEffect.hpp"
#include "Kyoto/Audio/CSfxManager.hpp"
#include "Kyoto/CDependencyGroup.hpp"
#include "Kyoto/CSimplePool.hpp"
#include "Kyoto/Graphics/CGraphics.hpp"
#include "Kyoto/Graphics/CModelFlags.hpp"
#include "Kyoto/Math/CTransform4f.hpp"
#include "Kyoto/Particles/CElementGen.hpp"
#include "Kyoto/Particles/CParticleSwoosh.hpp"
#include "MetaRender/CCubeRenderer.hpp"

#include "Kyoto/Audio/CAudioSys.hpp"
#include "Kyoto/Math/CMath.hpp"
#include "MetroidPrime/CAnimData.hpp"
#include "MetroidPrime/CAnimPlaybackParms.hpp"
#include "MetroidPrime/CAnimRes.hpp"
#include "MetroidPrime/CRainSplashGenerator.hpp"
#include "MetroidPrime/CRumbleManager.hpp"
#include "MetroidPrime/Cameras/CCameraManager.hpp"
#include "MetroidPrime/Cameras/CGameCamera.hpp"
#include "MetroidPrime/Player/CPlayer.hpp"
#include "MetroidPrime/Player/CPlayerState.hpp"
#include "MetroidPrime/TCastTo.hpp"
#include "MetroidPrime/Tweaks/CTweakGunRes.hpp"
#include "MetroidPrime/Tweaks/CTweakPlayer.hpp"
#include "MetroidPrime/Weapons/GunController/CGunController.hpp"
#include "MetroidPrime/Weapons/WeaponCommon.hpp"

static const char* const kGrappleGear = "GrappleGear";
static const char* const kGrappleNoz1 = "GrapNoz1";
static const char* const kGrappleNoz2 = "GrapNoz2";
static const char* const kGearLocator = "grapLocator_SDK";
static const char* const kNoz1Locator = "gNozLoc1_SDK";
static const char* const kNoz2Locator = "gNozLoc1_SDK";
static const char* const kBeamNozzle = "LGBeam";

static const char* const kSuitDependencyNames[] = {
    "PowerSuit_DGRP",  "GravitySuit_DGRP", "VariaSuit_DGRP",   "PhazonSuit_DGRP",
    "FusionSuit_DGRP", "FusionSuitG_DGRP", "FusionSuitV_DGRP", "FusionSuitP_DGRP"};

static const CPlayerState::EPlayerSuit suitIndices[] = {
    CPlayerState::kPS_Power,       CPlayerState::kPS_Gravity,     CPlayerState::kPS_Varia,
    CPlayerState::kPS_Phazon,      CPlayerState::kPS_FusionPower, CPlayerState::kPS_FusionGravity,
    CPlayerState::kPS_FusionVaria, CPlayerState::kPS_FusionPhazon};

float CGrappleArm::kPhaseDelta = 0.875f;
float CGrappleArm::kAmplitudeX = 0.25f;
float CGrappleArm::kAmplitudeZ = 0.125f;
float CGrappleArm::kMetresPerSecond = 5.f;

CGrappleArm::CGrappleArm(const CVector3f& scale)
: mGrappleArmModel(
      CAnimRes(gpTweakGunRes->mGrappleArm, CAnimRes::kDefaultCharIdx, scale, 41, false))
, mGrappleGearModel(CStaticRes(NWeaponTypes::get_asset_id_from_name(kGrappleGear), scale))
, mGrapNoz1Model(CStaticRes(NWeaponTypes::get_asset_id_from_name(kGrappleNoz1), scale))
, mGrapNoz2Model(CStaticRes(NWeaponTypes::get_asset_id_from_name(kGrappleNoz2), scale))
, mGrappleArm(gpSimplePool->GetObj(SObjectTag('ANCS', gpTweakGunRes->mGrappleArm)))
, mXf(CTransform4f::Identity())
, mGrapLocatorXf(CTransform4f::Identity())
, mGrapNozLoc1Xf(CTransform4f::Identity())
, mGrapNozLoc2Xf(CTransform4f::Identity())
, mAuxXf(CTransform4f::Identity())
, mGrapplePointPos(CVector3f::Zero())
, mScale(scale)
, mAnimState(kAS_Done)
, mBeamT(0.f)
, mBeamDist(0.f)
, mAnglePhase(0.f)
, mXAmplitude(0.f)
, mZAmplitude(0.f)
, mAnimSfx(-1, CSfxHandle())
, mGrappleSegmentDesc(
      gpSimplePool->GetObj(SObjectTag('PART', gpTweakGunRes->mGrappleSegment)))
, mGrappleClawDesc(gpSimplePool->GetObj(SObjectTag('PART', gpTweakGunRes->mGrappleClaw)))
, mGrappleHitDesc(gpSimplePool->GetObj(SObjectTag('PART', gpTweakGunRes->mGrappleHit)))
, mGrappleMuzzleDesc(gpSimplePool->GetObj(SObjectTag('PART', gpTweakGunRes->mGrappleMuzzle)))
, mGrappleSwooshDesc(gpSimplePool->GetObj(SObjectTag('SWHC', gpTweakGunRes->mGrappleSwoosh)))
, mGrappleSegmentGen(rs_new CElementGen(mGrappleSegmentDesc))
, mGrappleClawGen(rs_new CElementGen(mGrappleClawDesc))
, mGrappleHitGen(rs_new CElementGen(mGrappleHitDesc))
, mGrappleMuzzleGen(rs_new CElementGen(mGrappleMuzzleDesc))
, mGrappleSwooshGen(rs_new CParticleSwoosh(mGrappleSwooshDesc, 0))
, mRainSplashGenerator(rs_new CRainSplashGenerator(scale, 20, 2, 0.f, 0.125f))
, mLoadedSuit(CPlayerState::kPS_Invalid)
, mPitchBend(8192)
, mRumbleHandle(-1)
, mActive(false)
, mBeamActive(false)
, mGrappleHit(false)
, mArmMoving(false)
, mIsGrappling(false)
, mSuitLoading(false) {
  mGrappleArmModel->SetSortThermal(true);
  mGrappleGearModel.SetSortThermal(true);
  mGrapNoz1Model.SetSortThermal(true);
  mGrapNoz2Model.SetSortThermal(true);

  kPhaseDelta = gpTweakPlayer->GetGrappleBeamAnglePhaseDelta();
  kAmplitudeX = gpTweakPlayer->GetGrappleBeamXWaveAmplitude();
  kAmplitudeZ = gpTweakPlayer->GetGrappleBeamZWaveAmplitude();
  kMetresPerSecond = gpTweakPlayer->GetGrappleBeamSpeed();

  mGrappleMuzzleGen->SetParticleEmission(false);
  mGrappleSegmentGen->SetParticleEmission(false);
  CParticleSwoosh* swoosh = mGrappleSwooshGen.get();
  int count = 0;
  while (count < swoosh->GetSwooshCount() - 1) {
    mGrappleSwooshGen->SetWarmUp();
    mGrappleSwooshGen->Update(0.f);
    count++;
  }

  BuildSuitDependencyList();
  LoadAnimations();
}

CGrappleArm::~CGrappleArm() {}

void CGrappleArm::TouchModel(const CStateManager& mgr) const {
  if (CanRender()) {
    mGrappleArmModel->Touch(mgr, 0);
    if (mGrappleArmSkeletonModel) {
      mGrappleArmSkeletonModel->Touch(mgr, 0);
    }

    if (mgr.GetPlayerState()->HasPowerUp(CPlayerState::kIT_GrappleBeam)) {
      mGrappleGearModel.Touch(mgr, 0);
      mGrapNoz1Model.Touch(mgr, 0);
      mGrapNoz2Model.Touch(mgr, 0);
    }
  }
}

void CGrappleArm::PreRender(CStateManager& mgr, const CFrustumPlanes& frustum,
                            const CVector3f& camPos) {
  if (CanRender()) {
    mGrappleArmModel->AnimationData()->PreRender();
    if (mGrappleArmSkeletonModel) {
      mGrappleArmSkeletonModel->AnimationData()->PreRender();
    }
  }
}

void CGrappleArm::Render(const CStateManager& mgr, const CVector3f& pos, const CModelFlags& flags,
                         const CActorLights* lights) const {
  if (CanRender()) {
    const CTransform4f xf = CTransform4f::Translate(pos) * mXf * mAuxXf;
    if (mGrappleArmSkeletonModel) {
      RenderXRayModel(mgr, xf, flags);
    }
    const bool xray = mgr.GetPlayerState()->GetCurrentVisor() == CPlayerState::kPV_XRay;
    const CModelFlags useFlags = xray ? CModelFlags::AlphaBlended(0.25f) : flags;
    const CActorLights* useLights = xray ? nullptr : lights;
    if (mRainSplashGenerator.get() && mRainSplashGenerator->IsRaining()) {
      CSkinnedModel::SetPointGeneratorFunc(mRainSplashGenerator.get(), PointGenerator);
    }
    mGrappleArmModel->Render(mgr, xf, useLights, useFlags);
    if (mRainSplashGenerator.get() && mRainSplashGenerator->IsRaining()) {
      CSkinnedModel::ClearPointGeneratorFunc();
      mRainSplashGenerator->Draw(xf);
    }
    if (mgr.GetPlayerState()->HasPowerUp(CPlayerState::kIT_GrappleBeam)) {
      mGrappleGearModel.Render(mgr, xf * mGrapLocatorXf, useLights, useFlags);
      mGrapNoz1Model.Render(mgr, xf * mGrapNozLoc1Xf, useLights, useFlags);
      mGrapNoz2Model.Render(mgr, xf * mGrapNozLoc2Xf, useLights, useFlags);
    }
  }
}

void CGrappleArm::RenderGrappleBeam(const CStateManager& mgr, const CVector3f& pos) const {
  if (CanRender()) {
    const CTransform4f xf = CTransform4f::Translate(pos) * mXf;
    if (mBeamActive) {
      if (mGrappleHit) {
        mGrappleHitGen->Render();
      }
      mGrappleClawGen->Render();
      mGrappleSwooshGen->Render();
      mGrappleSegmentGen->Render();
      const CTransform4f viewMatrix = CGraphics::GetViewMatrix();
      CGraphics::SetViewPointMatrix(xf.GetInverse() * viewMatrix);
      gpRender->SetModelMatrix(CTransform4f::Identity());
      mGrappleMuzzleGen->Render();
      CGraphics::SetViewPointMatrix(viewMatrix);
    }
  }
}

void CGrappleArm::UpdateSwingAction(float swingT, float dt, CStateManager& mgr) {
  if (mSuitLoading) {
    return;
  }
  if (mAnimState == kAS_FireGrapple) {
    DoUserAnimEvents(mgr);
  }
  const CTransform4f beamLoc =
      mGrappleArmModel->GetScaledLocatorTransform(rstl::string_l(kBeamNozzle));
  const bool connected = UpdateGrappleBeam(dt, beamLoc, mgr);
  if ((swingT > 0.175f && swingT < 0.3f) || (swingT > 0.7f && swingT < 0.9f)) {
    if (!CSfxManager::IsPlaying(mSwooshSfx)) {
      mSwooshSfx = NWeaponTypes::play_sfx(0x5f8, false, false, 0x36);
      if (mRumbleHandle != -1) {
        mgr.GetRumbleManager()->StopRumble(mRumbleHandle);
      }
      mRumbleHandle =
          mgr.GetRumbleManager()->Rumble(mgr, kRFX_PlayerGrappleSwoosh, 1.f, kRP_Three);
    }
  }
  const CAnimData& animData = *mGrappleArmModel->GetAnimationData();
  if (!animData.IsAnimTimeRemaining(dt, rstl::string_l("Whole Body"))) {
    switch (mAnimState) {
    case kAS_IntoGrapple:
    case kAS_Seven:
      SetAnimState(kAS_IntoGrappleIdle);
      break;
    case kAS_FireGrapple:
      if (connected) {
        SetAnimState(kAS_ConnectGrapple);
        mGrappleHit = true;
        mGrappleHitGen->SetParticleEmission(true);
        GrappleBeamConnected();
        if (mRumbleHandle != -1) {
          mgr.GetRumbleManager()->StopRumble(mRumbleHandle);
        }
      }
      break;
    case kAS_ConnectGrapple:
      if (mXAmplitude == 0.f) {
        SetAnimState(kAS_Connected);
      }
      break;
    case kAS_OutOfGrapple:
      if (mRumbleHandle != -1) {
        mgr.GetRumbleManager()->StopRumble(mRumbleHandle);
      }
      SetAnimState(kAS_Done);
      mActive = false;
      break;
    default:
      break;
    }
  }
  if (mBeamActive) {
    mGrappleMuzzleGen->SetTranslation(beamLoc.GetTranslation());
    mGrappleMuzzleGen->Update(dt);
    if (mGrappleHit) {
      mGrappleHit = !mGrappleHitGen->IsSystemDeletable();
      mGrappleHitGen->SetTranslation(mGrapplePointPos);
      mGrappleHitGen->Update(dt);
    }
  }
}

void CGrappleArm::UpdateArmMovement(float dt, CStateManager& mgr) {
  DoUserAnimEvents(mgr);
  switch (mGunController->Update(dt, mgr)) {
  case 1:
    ResetAuxParams(false);
    break;
  default:
    break;
  }
}

void CGrappleArm::Update(float swingT, float dt, CStateManager& mgr) {
  if (!CanRender()) {
    if (mSuitLoading) {
      LoadSuitPoll();
    }
    return;
  }
  if (mgr.GetPlayerState()->GetCurrentVisor() == CPlayerState::kPV_XRay) {
    if (!mGrappleArmSkeletonModel) {
      BuildXRayModel();
    }
  } else if (mGrappleArmSkeletonModel) {
    mGrappleArmSkeletonModel = rstl::optional_object< CModelData >();
  }
  float speed = 1.f;
  if (!mArmMoving) {
    speed = mgr.GetPlayer()->GetPlayerMovementState() != NPlayer::kMS_OnGround &&
                    mAnimState != kAS_OutOfGrapple
                ? 4.f
                : 1.f;
  }
  mGrappleArmModel->AdvanceAnimation(dt * speed, mgr, kInvalidAreaId, true);
  if (mgr.GetPlayerState()->HasPowerUp(CPlayerState::kIT_GrappleBeam)) {
    mGrapLocatorXf =
        mGrappleArmModel->GetScaledLocatorTransformDynamic(rstl::string_l(kGearLocator), nullptr);
    mGrapNozLoc1Xf =
        mGrappleArmModel->GetScaledLocatorTransform(rstl::string_l(kNoz1Locator));
    mGrapNozLoc2Xf =
        mGrappleArmModel->GetScaledLocatorTransform(rstl::string_l(kNoz2Locator));
  }
  if (mArmMoving) {
    UpdateArmMovement(dt, mgr);
  } else {
    UpdateSwingAction(swingT, dt, mgr);
  }
  if (mRainSplashGenerator.get()) {
    mRainSplashGenerator->Update(dt, mgr);
  }
}

const bool CGrappleArm::UpdateGrappleBeam(float dt, const CTransform4f& beamLoc,
                                          CStateManager& mgr) {
  bool connected = false;
  const TUniqueId targetId = mgr.GetPlayer()->GetOrbitTargetId();
  const CActor* actor = TCastToConstPtr< CActor >(mgr.GetObjectById(targetId));
  mGrapplePointPos = actor ? actor->GetTranslation() : mXf.GetTranslation();
  const CVector3f beamGunPos = (mXf * beamLoc).GetTranslation();
  const CVector3f beamAirPos = CVector3f::Lerp(beamGunPos, mGrapplePointPos, mBeamT);
  switch (mAnimState) {
  case kAS_Five:
    break;
  default:
    break;
  case kAS_FireGrapple:
  case kAS_Three: {
    const float distance = (mGrapplePointPos - beamGunPos).Magnitude();
    mBeamT = distance > 0.f ? mBeamDist / distance : 1.f;
    const float speed =
        mgr.GetPlayer()->GetPlayerMovementState() != NPlayer::kMS_OnGround ? 2.f : 1.f;
    mBeamDist += speed * (dt * kMetresPerSecond);
    if (!(mBeamT < 1.f)) {
      mBeamT = 1.f;
      connected = true;
    }
    break;
  }
  case kAS_ConnectGrapple: {
    const float delta = 4.f * dt;
    mXAmplitude -= delta;
    mZAmplitude -= delta;
    if (mXAmplitude < 0.f) {
      mXAmplitude = 0.f;
    }
    if (mZAmplitude < 0.f) {
      mZAmplitude = 0.f;
    }
    break;
  }
  }
  if (mBeamActive) {
    mAnglePhase += kPhaseDelta;
    UpdateGrappleBeamFX(beamGunPos, beamAirPos, mgr);
    mGrappleClawGen->Update(dt);
    mGrappleSegmentGen->Update(dt);
  }
  return connected;
}

void CGrappleArm::UpdateGrappleBeamFX(const CVector3f& beamGunPos, const CVector3f& beamAirPos,
                                      CStateManager& mgr) {
  mGrappleClawGen->SetTranslation(beamAirPos);
  mGrappleSegmentGen->SetParticleEmission(true);
  CVector3f segmentDelta = beamAirPos - beamGunPos;
  const int segmentCount = static_cast< int >(2.f * segmentDelta.Magnitude() + 1.f);
  CVector3f swooshDelta = segmentDelta;
  const float invCount = 1.f / float(segmentCount);
  segmentDelta *= invCount;
  swooshDelta *= 0.02f;
  CVector3f segmentPos = beamGunPos;
  const CTransform4f& rotation = mXf.GetRotation();
  for (int i = 0; i < segmentCount; ++i) {
    const float index = float(i);
    const float x = CMath::FastCosR(index + mAnglePhase) * mXAmplitude;
    const float z = CMath::FastSinR(index) * mZAmplitude;
    mGrappleSegmentGen->SetTranslation(
        segmentPos + (i > 0 ? rotation * CVector3f(x, 0.f, z) : CVector3f::Zero()));
    mGrappleSegmentGen->ForceParticleCreation(1);
    segmentPos += segmentDelta;
  }
  mGrappleSegmentGen->SetParticleEmission(false);
  CParticleSwoosh& swoosh = *mGrappleSwooshGen;
  CVector3f swooshPos = beamGunPos;
  float previousRotation = swoosh.GetSwooshes()[swoosh.GetSwooshCount() - 1].mInitialRot;
  for (int i = 0; i < swoosh.GetSwooshCount(); ++i) {
    const float index = float(i);
    const float x = CMath::FastCosR(index + mAnglePhase) * mXAmplitude;
    const float z = CMath::FastSinR(index) * mZAmplitude;
    const CVector3f offset = i > 0 ? rotation * CVector3f(x, 0.f, z) : CVector3f::Zero();
    CParticleSwoosh::SSwooshData& segment = swoosh.Swooshes()[i];
    segment.mTranslation = swooshPos + offset;
    swooshPos += swooshDelta;
    const float rotation = segment.mInitialRot;
    segment.mInitialRot = previousRotation;
    previousRotation = rotation;
  }
}

void CGrappleArm::Activate(bool active) {
  SetAnimState(active ? kAS_IntoGrapple : kAS_OutOfGrapple);
}

void CGrappleArm::SetAnimState(EArmState state) {
  if (mAnimState == state) {
    return;
  }
  CAnimData& animData = *mGrappleArmModel->AnimationData();
  animData.EnableLooping(false);
  mIsGrappling = true;
  switch (state) {
  case kAS_IntoGrapple: {
    ResetAuxParams(true);
    const CAnimPlaybackParms parms(0, -1, 1.f, true);
    animData.SetAnimation(parms, false);
    mBeamActive = false;
    mActive = true;
    break;
  }
  case kAS_IntoGrappleIdle: {
    animData.EnableLooping(true);
    const CAnimPlaybackParms parms(1, -1, 1.f, true);
    animData.SetAnimation(parms, false);
    break;
  }
  case kAS_FireGrapple: {
    const CAnimPlaybackParms parms(2, -1, 1.f, true);
    animData.SetAnimation(parms, false);
    break;
  }
  case kAS_ConnectGrapple: {
    const CAnimPlaybackParms parms(3, -1, 1.f, true);
    animData.SetAnimation(parms, false);
    break;
  }
  case kAS_Connected: {
    const CAnimPlaybackParms parms(3, -1, 1.f, true);
    animData.SetAnimation(parms, false);
    break;
  }
  case kAS_OutOfGrapple: {
    const CAnimPlaybackParms parms(4, -1, 1.f, true);
    animData.SetAnimation(parms, false);
    DisconnectGrappleBeam();
    break;
  }
  case kAS_Done:
    mIsGrappling = false;
    break;
  default:
    break;
  }
  mAnimState = state;
}

void CGrappleArm::DoUserAnimEvents(CStateManager& mgr) {
  int aid = mgr.GetPlayer()->GetCurrentAreaId().Value();
  const CGameCamera& camera = mgr.GetCameraManager()->GetCurrentCamera(mgr);
  const CVector3f origin = mXf.GetTranslation();
  const CVector3f posToCam = camera.GetTranslation() - origin;
  int soundCount = 0;
  const CSoundPOINode* soundNodes =
      mGrappleArmModel->AnimationData()->GetSoundPOIList(soundCount);
  if (soundCount > 0) {
    for (int i = 0; i < soundCount; ++i) {
      const CSoundPOINode& soundNode = soundNodes[i];
      const int charIdx = soundNode.GetCharacterIndex();
      if (soundNode.GetPoiType() == kPT_Sound &&
          (charIdx == -1 || charIdx == mGrappleArmModel->AnimationData()->GetCharacterIndex())) {
        NWeaponTypes::do_sound_event(mAnimSfx, mPitchBend, false, soundNode.GetSoundId(),
                                     soundNode.GetWeight(), soundNode.GetFlags(),
                                     soundNode.GetFallOff(), soundNode.GetMaxDistance(), 0x14,
                                     CAudioSys::kMaxVolume, posToCam, origin, aid, mgr);
      }
    }
  }
  int intCount = 0;
  const CInt32POINode* intNodes = mGrappleArmModel->AnimationData()->GetInt32POIList(intCount);
  if (intCount > 0) {
    for (int i = 0; i < intCount; ++i) {
      const CInt32POINode& intNode = intNodes[i];
      switch (intNode.GetPoiType()) {
      case kPT_UserEvent:
        DoUserAnimEvent(mgr, intNode, static_cast< EUserEventType >(intNode.GetValue()));
        break;
      case kPT_SoundInt32:
        if (intNode.GetCharacterIndex() == -1 ||
            intNode.GetCharacterIndex() ==
                mGrappleArmModel->AnimationData()->GetCharacterIndex()) {
          NWeaponTypes::do_sound_event(mAnimSfx, mPitchBend, false, intNode.GetValue(),
                                       intNode.GetWeight(), intNode.GetFlags(), 0.1f, 150.f, 0x14,
                                       CAudioSys::kMaxVolume, posToCam, origin, aid, mgr);
        }
        break;
      default:
        break;
      }
    }
  }
}

void CGrappleArm::DoUserAnimEvent(CStateManager& mgr, const CInt32POINode& node,
                                  EUserEventType type) {
  switch (type) {
  case kUE_Projectile:
    if (mArmMoving) {
      return;
    }
    mBeamActive = true;
    mGrappleHitGen = rs_new CElementGen(mGrappleHitDesc);
    mGrappleMuzzleGen = rs_new CElementGen(mGrappleMuzzleDesc);
    mBeamT = 0.f;
    mBeamDist = 0.f;
    mAnglePhase = 0.f;
    mXAmplitude = kAmplitudeX;
    mZAmplitude = kAmplitudeZ;
    mGrappleHitGen->SetParticleEmission(false);
    mGrappleClawGen->SetParticleEmission(true);
    NWeaponTypes::play_sfx(0x5f6, false, false, 0x36);
    mgr.GetRumbleManager()->Rumble(mgr, kRFX_PlayerGrappleFire, 1.f, kRP_Three);
    break;
  case kUE_Delete:
  case kUE_DamageOn:
    break;
  default:
    break;
  }
}

void CGrappleArm::GrappleBeamConnected() {
  if (!mGrappleLoopSfx) {
    mGrappleLoopSfx = NWeaponTypes::play_sfx(0x5f7, false, true, 0x36);
  }
}

void CGrappleArm::GrappleBeamDisconnected() {
  if (mGrappleLoopSfx) {
    CSfxManager::SfxStop(mGrappleLoopSfx);
    mGrappleLoopSfx.Clear();
  }
}

void CGrappleArm::LoadAnimations() {
  NWeaponTypes::get_token_vector(*mGrappleArmModel->GetAnimationData(), 0, 42, mAnims, true);
  mGrappleArmModel = rstl::optional_object< CModelData >();
}

void CGrappleArm::EnterFreeLook(int gunId, int setId, CStateManager& mgr) {
  if (mSuitLoading) {
    return;
  }
  mActive = true;
  mArmMoving = true;
  mAnimState = kAS_GunControllerAnimation;
  mGunController->EnterFreeLook(mgr, gunId, setId);
}

void CGrappleArm::EnterFidget(CStateManager& mgr, int a, int b, int c) {
  if (mSuitLoading) {
    return;
  }

  SetActive(true);
  SetIsArmMoving(true);
  mAnimState = kAS_GunControllerAnimation;
  mGunController->EnterFidget(mgr, a, b, c);
}

void CGrappleArm::EnterComboFire(int a, CStateManager& mgr) {
  if (mSuitLoading) {
    return;
  }

  SetActive(true);
  SetIsArmMoving(true);
  mAnimState = kAS_GunControllerAnimation;
  mGunController->EnterComboFire(mgr, a);
}

void CGrappleArm::ReturnToDefault(CStateManager& mgr, float dt, bool setState) {
  if (IsSuitLoading()) {
    return;
  }

  mGunController->ReturnToDefault(mgr, dt, setState);
}

void CGrappleArm::EnterIdle(CStateManager& mgr) {
  if (IsSuitLoading()) {
    return;
  }
  mGunController->EnterIdle(mgr);
}

void CGrappleArm::EnterStruck(CStateManager& mgr, float angle, bool bigStrike, bool notInFreeLook) {
  if (IsSuitLoading()) {
    return;
  }

  if (IsGrappling()) {
    DisconnectGrappleBeam();
    mIsGrappling = false;
  }

  if (!mArmMoving) {
    mActive = true;
    mArmMoving = true;
    mAnimState = kAS_GunControllerAnimation;
  }

  mGunController->EnterStruck(mgr, angle, bigStrike, notInFreeLook);
}

void CGrappleArm::ResetAuxParams(bool resetGunController) {
  mActive = false;
  mArmMoving = false;
  mAnimState = kAS_Done;
  mAuxXf = CTransform4f::Identity();
  if (resetGunController) {
    mGunController->Reset();
  }
}

void CGrappleArm::AsyncLoadSuit(CStateManager& mgr) {

  const int suit = NWeaponTypes::get_current_suit(mgr);
  if (suit == mLoadedSuit) {
    return;
  }
  mGrappleArmModel = rstl::optional_object< CModelData >();
  mGunController = nullptr;
  mSuitLoading = true;
  if (mLoadedSuit != CPlayerState::kPS_Invalid) {
    NWeaponTypes::unlock_tokens(mSuitDeps[mLoadedSuit]);
    mSuitDeps[mLoadedSuit] = rstl::vector< CToken >();
  }
  if (suit < CPlayerState::kPS_Power || suit > CPlayerState::kPS_FusionPhazon) {
    mLoadedSuit = CPlayerState::kPS_Power;
  } else {
    mLoadedSuit = suitIndices[suit];
  }
  NWeaponTypes::lock_tokens(mSuitDeps[mLoadedSuit]);
}

void CGrappleArm::LoadSuitPoll() {
  if (NWeaponTypes::are_tokens_ready(mSuitDeps[mLoadedSuit])) {
    mGrappleArmModel =
        CModelData(CAnimRes(gpTweakGunRes->mGrappleArm, mLoadedSuit, mScale, 41, false));
    mGrappleArmModel->SetSortThermal(true);
    mGunController = rs_new CGunController(*mGrappleArmModel);
    mSuitLoading = false;
  }
}

void CGrappleArm::BuildXRayModel() {
  mGrappleArmSkeletonModel = CModelData(CAnimRes(
      gpTweakGunRes->mGrappleArm, 8, mScale,
      mGunController.get() == nullptr ? 41 : mGunController->GetCurAnimId(), false));
  mGrappleArmSkeletonModel->SetSortThermal(true);
}

void CGrappleArm::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId sender, CStateManager& mgr) {
  switch (msg) {
  case kSM_Registered:
    AsyncLoadSuit(mgr);
    break;
  default:
    break;
  }
}

void CGrappleArm::FillTokenVector(const rstl::vector< SObjectTag >& tags,
                                  rstl::vector< CToken >& objects) {
  objects.reserve(tags.size());
  for (AUTO(it, tags.begin()); it != tags.end(); ++it) {
    const CToken token = gpSimplePool->GetObj(*it);
    objects.push_back(token);
  }
}

void CGrappleArm::BuildSuitDependencyList() {

  mGrappleArm.Lock();
  for (int i = 0; i < 8; ++i) {
    TLockedToken< CDependencyGroup > dep = gpSimplePool->GetObj(kSuitDependencyNames[i]);
    const rstl::vector< SObjectTag >& tags = dep->GetObjectTagVector();
    mSuitDeps.push_back(rstl::vector< CToken >());
    FillTokenVector(tags, mSuitDeps[i]);
  }
}

void CGrappleArm::RenderXRayModel(const CStateManager& mgr, const CTransform4f& xf,
                                  const CModelFlags& flags) const {
  const CVector3f& scale = CVector3f(mGrappleArmModel->GetScale());
  CTransform4f modelMatrix(xf);
  modelMatrix *= CTransform4f::Scale(scale.GetX(), scale.GetY(), scale.GetZ());
  gpRender->SetModelMatrix(modelMatrix);
  CGraphics::DisableAllLights();
  gpRender->SetAmbientColor(CColor::White());
  mGrappleArmModel->GetAnimationData()->Render(
      **mGrappleArmSkeletonModel->GetAnimationData()->GetModelData(), flags,
      rstl::optional_object< CVertexMorphEffect >(), nullptr);
  gpRender->SetAmbientColor(CColor::White());
  CGraphics::DisableAllLights();
}

void CGrappleArm::PointGenerator(void* context, const CVector3f* vertices, const CVector3f* normals,
                                 int count) {
  if (context) {
    static_cast< CRainSplashGenerator* >(context)->GeneratePoints(vertices, normals, count);
  }
}

void CGrappleArm::DisconnectGrappleBeam() {
  mGrappleClawGen->SetParticleEmission(false);
  mBeamActive = false;
  GrappleBeamDisconnected();
}
