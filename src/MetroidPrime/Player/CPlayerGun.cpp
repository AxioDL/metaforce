#include "MetroidPrime/Player/CPlayerGun.hpp"

#include "Kyoto/Math/CTransform4f.hpp"
#include "Kyoto/Math/CVector3f.hpp"
#include "MetroidPrime/CAnimData.hpp"
#include "MetroidPrime/CAnimRes.hpp"
#include "MetroidPrime/CControlMapper.hpp"
#include "MetroidPrime/CFluidPlaneCPU.hpp"
#include "MetroidPrime/CGameLight.hpp"
#include "MetroidPrime/CRainSplashGenerator.hpp"
#include "MetroidPrime/CRumbleManager.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/CWorld.hpp"
#include "MetroidPrime/CWorldShadow.hpp"
#include "MetroidPrime/Cameras/CCameraManager.hpp"
#include "MetroidPrime/Cameras/CGameCamera.hpp"
#include "MetroidPrime/Enemies/CMetroid.hpp"
#include "MetroidPrime/Enemies/CMetroidBeta.hpp"
#include "MetroidPrime/Enemies/CPatterned.hpp"
#include "MetroidPrime/HUD/CSamusHud.hpp"
#include "MetroidPrime/Player/CGrappleArm.hpp"
#include "MetroidPrime/Player/CPlayer.hpp"
#include "MetroidPrime/SFX/PhazonGun.h"
#include "MetroidPrime/SFX/Weapons.h"
#include "MetroidPrime/ScriptObjects/CScriptPlatform.hpp"
#include "MetroidPrime/ScriptObjects/CScriptWater.hpp"
#include "MetroidPrime/TCastTo.hpp"
#include "MetroidPrime/TGameTypes.hpp"
#include "MetroidPrime/Tweaks/CTweakGunRes.hpp"
#include "MetroidPrime/Tweaks/CTweakPlayer.hpp"
#include "MetroidPrime/Tweaks/CTweakPlayerGun.hpp"
#include "MetroidPrime/Weapons/CAuxWeapon.hpp"
#include "MetroidPrime/Weapons/CBomb.hpp"
#include "MetroidPrime/Weapons/CEnergyProjectile.hpp"
#include "MetroidPrime/Weapons/CIceBeam.hpp"
#include "MetroidPrime/Weapons/CPhazonBeam.hpp"
#include "MetroidPrime/Weapons/CPlasmaBeam.hpp"
#include "MetroidPrime/Weapons/CPowerBeam.hpp"
#include "MetroidPrime/Weapons/CPowerBomb.hpp"
#include "MetroidPrime/Weapons/CWaveBeam.hpp"
#include "MetroidPrime/Weapons/GunController/CGunMotion.hpp"
#include "MetroidPrime/Weapons/WeaponTypes.hpp"

#include "Kyoto/Animation/CPrimitive.hpp"
#include "Kyoto/Audio/CAudioSys.hpp"
#include "Kyoto/Audio/CSfxManager.hpp"
#include "Kyoto/Basics/CCast.hpp"
#include "Kyoto/Graphics/CGX.hpp"
#include "Kyoto/Graphics/CModelFlags.hpp"
#include "Kyoto/Math/CAbsAngle.hpp"
#include "Kyoto/Math/CMath.hpp"
#include "Kyoto/Math/CRelAngle.hpp"
#include "Kyoto/Particles/CElementGen.hpp"
#include "Kyoto/Particles/CGenDescription.hpp"
#include "Kyoto/SObjectTag.hpp"
#include "Kyoto/Text/CStringTable.hpp"

#include "Collision/CMaterialFilter.hpp"
#include "Collision/CMaterialList.hpp"

#include "MetaRender/CCubeRenderer.hpp"

#include "Weapons/IWeaponRenderer.hpp"

#include "rstl/set.hpp"

#include "dolphin/gx/GXFrameBuffer.h"
#include "dolphin/gx/GXManage.h"

#include "math.h"

static const char* const kGunLocator = "GBSE_SDK";
const float CPlayerGun::kGunScale = 2.f;

static float kVerticalAngleTable[3] = {-30.f, 0.f, 30.f};
static float kHorizontalAngleTable[3] = {30.f, 30.f, 30.f};
static float kVerticalVarianceTable[3] = {30.f, 30.f, 30.f};

static const float chargeShakeTbl[3] = {
    -0.001f,
    0.f,
    0.001f,
};

const uint CPlayerGun::mHandAnimId[4] = {
    0,
    1,
    2,
    1,
};

static const CPlayerState::EItemType mBeamArr[4] = {
    CPlayerState::kIT_PowerBeam,
    CPlayerState::kIT_IceBeam,
    CPlayerState::kIT_WaveBeam,
    CPlayerState::kIT_PlasmaBeam,
};

static const CPlayerState::EItemType mBeamComboArr[4] = {
    CPlayerState::kIT_SuperMissile,
    CPlayerState::kIT_IceSpreader,
    CPlayerState::kIT_Wavebuster,
    CPlayerState::kIT_Flamethrower,
};

static const ControlMapper::ECommands mBeamCtrlCmd[4] = {
    ControlMapper::kC_PowerBeam,
    ControlMapper::kC_IceBeam,
    ControlMapper::kC_WaveBeam,
    ControlMapper::kC_PlasmaBeam,
};

static const ushort mToMissileSound[4] = {
    SFXsam_b_misswitch_00,
    SFXsam_b_misswitch_10,
    SFXsam_b_misswitch_20,
    SFXsam_b_misswitch_30,
};

static const ushort mFromMissileSound[4] = {
    SFXsam_b_misswitch_01,
    SFXsam_b_misswitch_11,
    SFXsam_b_misswitch_21,
    SFXsam_b_misswitch_31,
};

float CPlayerGun::kTractorBeamFactor = 0.25f / CPlayerState::GetMissileComboChargeFactor();
CVector3f CPlayerGun::kScaleVector(2.f, 2.f, 2.f);
float CPlayerGun::CMotionState::gGunExtendDistance = 0.125f;

static CColor kArmColor = CColor(0.75f, 0.5f, 0.f, 1.f);
static CMaterialFilter sAimFilter = CMaterialFilter::MakeIncludeExclude(
    CMaterialList(kMT_Solid), CMaterialList(kMT_ProjectilePassthrough));
static const float kChargeSpeed = 1.f / CPlayerState::GetMissileComboChargeFactor();
static const float kChargeStart = 0.025f / CPlayerState::GetMissileComboChargeFactor();
static const float kChargeFxStart = 1.f / CPlayerState::GetMissileComboChargeFactor();

static const CPlayerState::EItemType skItemArr[2] = {
    CPlayerState::kIT_Invalid,
    CPlayerState::kIT_Missiles,
};

static const CPlayerState::EBeamId mCurrentBeamId[5] = {
    CPlayerState::kBI_Power,  CPlayerState::kBI_Ice,   CPlayerState::kBI_Wave,
    CPlayerState::kBI_Plasma, CPlayerState::kBI_Power,
};

static const CModelFlags kThermalFlags[4] = {
    CModelFlags::Normal(),
    CModelFlags::AlphaBlended(CColor(u8(0), u8(0), u8(0), u8(128))),
    CModelFlags::Normal(),
    CModelFlags::Normal(),
};

static const CModelFlags kHandThermalFlag = CModelFlags::Additive(CColor::White());
static const CModelFlags kHandHoloFlag = CModelFlags::ColorModulate(kArmColor);

static const ushort mItemEmptySound[2] = {
    CSfxManager::kInternalInvalidSfxId,
    SFXsam_a_mislemp_00,
};

static const ushort mIntoBeamSound[4] = {
    CSfxManager::kInternalInvalidSfxId,
    SFXsam_b_iceswitch_00,
    SFXsam_b_wavswitch_00,
    SFXsam_b_plaswitch_00,
};

static const ushort mFromBeamSound[4] = {
    CSfxManager::kInternalInvalidSfxId,
    SFXsam_b_iceswitch_01,
    SFXsam_b_wavswitch_01,
    SFXsam_b_plaswitch_01,
};

static ushort sBeamChargeUpSound[4] = {
    SFXsam_a_cbmcharge_lp_00,
    SFXsam_a_icecharge_lp_00,
    SFXsam_a_wavcharge_lp_00,
    SFXsam_a_placharge_lp_00,
};

static const float kDepthFar = 1.f;
static const float kDepthWorld = 1.f / 8.f;
static const float kDepthGun = 1.f / 32.f;

extern void DrawClipCube(const CAABox& aabb);

CPlayerGun::CPlayerGun(TUniqueId playerId)
: mLights(8, CVector3f::Zero(), 4, 4, CActorLights::kDefaultPositionUpdateThreshold, false, false,
            false)
, mLastFireButtonStates(0)
, mPressedFireButtonStates(0)
, mFireButtonStates(0)
, mStateFlags(0x1)
, mFidgetAnimBits(0)
, mRemainingMissiles(0)
, x304_(0)
, mBombCount(3)
, mRapidFireShots(0)
, mEquippedBeamId(CPlayerState::kBI_Power)
, mNextBeamId(CPlayerState::kBI_Power)
, mComboAmmoIdx(0)
, mMissileMode(EMissileMode(mComboAmmoIdx))
, mCurrentAuxBeam(mEquippedBeamId)
, mIdleState(kIS_Four)
, mAnimSfxPitch(0x2000)
, mChargePhase(kCP_NotCharging)
, mChargeState(kCS_Normal)
, x334_(0)
, mNextState(kNS_StatusQuo)
, mPhazonBeamState(kPBS_Inactive)
, mChargeBeamFactor(0.f)
, mComboXferTimer(0.f)
, mChargeCooldownTimer(0.f)
, mShakeX(0.f)
, mShakeZ(0.f)
, mBombFuseTime(gpTweakPlayerGun->GetBombFuseTime())
, mBombDropDelayTime(gpTweakPlayerGun->GetBombDropDelayTime())
, mBombTime(0.f)
, x360_(0.f)
, mGunStrikeCoolTimer(0.f)
, mIdleWanderDelayTimer(0.f)
, x36c_(1.f)
, mGunMotionSpeedMult(1.f)
, x374_(0.f)
, mShotSmokeStartTimer(0.f)
, mRapidFireShotsDecayTimer(0.f)
, mShotSmokeTimer(0.f)
, mGunStrikeDelayTimer(0.f)
, mEnterFreeLookDelayTimer(0.f)
, mMuzzleEffectVisTimer(0.f)
, mCooldown(0.f)
, mDamageTimer(0.f)
, mDamageAmt(0.f)
, mPhazonMorphT(0.f)
, mMissileExitTimer(0.f)
, mDamageLocation(CVector3f::Zero())
, mXf(CTransform4f::Identity())
, mBeamLocalXf(CTransform4f::Identity())
, mElbowWorldXf(CTransform4f::Identity())
, mAssistAimXf(CTransform4f::Identity())
, mGunWorldXf(CTransform4f::Identity())
, mGunLocalXf(CTransform4f::Identity())
, mElbowLocalXf(CTransform4f::Identity())
, mPlayerId(playerId)
, mPowerBomb(kInvalidUniqueId)
, mLightId(kInvalidUniqueId)
, mCamBob(CPlayerCameraBob::kCBT_One,
              CVector2f(CPlayerCameraBob::kCameraBobExtentX, CPlayerCameraBob::kCameraBobExtentY),
              CPlayerCameraBob::kCameraBobPeriod)
, x658_(1)
, x65c_(0.f)
, x660_(0.f)
, x664_(0.f)
, mAimVerticalSpeed(gpTweakPlayerGun->GetAimVerticalSpeed())
, mAimHorizontalSpeed(gpTweakPlayerGun->GetAimHorizontalSpeed())
, mAnimSfx(static_cast< TSfxId >(0xffff), CSfxHandle())
, mMorph(gpTweakPlayerGun->GetGunTransformTime(), gpTweakPlayerGun->GetHoloHoldTime())
, mMotionState()
, mHologramClipCube(CVector3f(-0.29329199f, 0.f, -0.2481945f),
                        CVector3f(0.29329199f, 1.292392f, 0.2481945f))
, mRightHandModel(CAnimRes(gpTweakGunRes->mRightHand, CAnimRes::kDefaultCharIdx,
                               CVector3f(3.f, 3.f, 3.f), 0, true))
, mCurrentBeam(nullptr)
, mOutgoingBeam(nullptr)
, mLoadingBeam(nullptr)
, mNextBeam(nullptr)
, mGunMotion(rs_new CGunMotion(gpTweakGunRes->mGunMotion, kScaleVector))
, mGrappleArm(rs_new CGrappleArm(kScaleVector))
, mAuxWeapon(rs_new CAuxWeapon(playerId))
, mRainSplashGenerator(rs_new CRainSplashGenerator(kScaleVector, 20, 2, 0.f, 0.125f))
, mPowerBeam(rs_new CPowerBeam(gpTweakGunRes->mPowerBeam, kWT_Power, playerId, kMT_Player,
                                   kScaleVector))
, mIceBeam(
      rs_new CIceBeam(gpTweakGunRes->mIceBeam, kWT_Ice, playerId, kMT_Player, kScaleVector))
, mWaveBeam(
      rs_new CWaveBeam(gpTweakGunRes->mWaveBeam, kWT_Wave, playerId, kMT_Player, kScaleVector))
, mPlasmaBeam(rs_new CPlasmaBeam(gpTweakGunRes->mPlasmaBeam, kWT_Plasma, playerId,
                                     kMT_Player, kScaleVector))
, mPhazonBeam(rs_new CPhazonBeam(gpTweakGunRes->mPhazonBeam, kWT_Phazon, playerId,
                                     kMT_Player, kScaleVector))
, mSelectableBeams(nullptr)
, mHoloTransitionGen(rs_new CElementGen(
      gpSimplePool->GetObj(SObjectTag('PART', gpTweakGunRes->mHoloTransition))))
, mShadow(rs_new CWorldShadow(32, 32, true))
, mChargeRumbleHandle(-1)
, mCoolingCharge(false)
, mChargeEffectVisible(false)
, mComboFiring(false)
, mChargeAnimStarted(false)
, mReadyForShot(false)
, mLockedOn(false)
, mRequestReturnToDefault(false)
, mInRestPose(true)
, mNotFidgeting(true)
, x833_25_(false)
, x833_26_(false)
, x833_27_(false)
, mPhazonBeamActive(false)
, mPointBlankWorldSurface(false)
, mCanShowAuxMuzzleEffect(true)
, mInFreeLook(false)
, mCharging(false)
, mGunMotionFidgeting(false)
, mAnimPlaying(false)
, mUnderwater(false)
, mRequestImmediateRecharge(false)
, mFrozen(false)
, mInBigStrike(false)
, mGunMotionInFidgetBasePosition(false)
, mCanFirePhazon(false)
, mInPhazonBeam(false)
, mPhazonBeamMorphing(false)
, mIntoPhazonBeam(false)
, mBombReady(false)
, mPowerBombReady(false)
, mInPhazonPool(false)
, mActorAttached(false) {

  mRightHandModel.SetSortThermal(true);
  kVerticalAngleTable[2] = gpTweakPlayerGun->GetUpLookAngle();
  kVerticalAngleTable[0] = gpTweakPlayerGun->GetDownLookAngle();
  kHorizontalAngleTable[1] = gpTweakPlayerGun->GetHorizontalSpread();
  kHorizontalAngleTable[2] = gpTweakPlayerGun->GetHighHorizontalSpread();
  kHorizontalAngleTable[0] = gpTweakPlayerGun->GetLowHorizontalSpread();
  kVerticalVarianceTable[1] = gpTweakPlayerGun->GetVerticalSpread();
  kVerticalVarianceTable[2] = gpTweakPlayerGun->GetHighVerticalSpread();
  kVerticalVarianceTable[0] = gpTweakPlayerGun->GetLowVerticalSpread();
  CMotionState::SetExtendDistance(gpTweakPlayerGun->GetGunExtendDistance());

  InitBeamData();
  InitBombData();
  InitMuzzleData();
  InitCTData();
  LoadHandAnimTokens();
  mCamBob.SetPlayerVelocity(CVector3f::Zero());
  mCamBob.SetBobMagnitude(0.f);
  mCamBob.SetBobTimeScale(0.f);
}

CPlayerGun::~CPlayerGun() {}

void CPlayerGun::AddToRenderer(const CFrustumPlanes& frustum, const CStateManager&) const {
  rstl::optional_object< CModelData >& model = mCurrentBeam->SolidModelData();
  if (model) {
    model->RenderParticles(frustum);
  }
}

void CPlayerGun::PreRender(CStateManager& mgr, const CFrustumPlanes& frustum,
                           const CVector3f& camPos) {
  const CPlayerState& playerState = *mgr.GetPlayerState();
  if (playerState.GetCurrentVisor() == CPlayerState::kPV_Scan) {
    return;
  }

  CPlayerState::EPlayerVisor activeVisor = playerState.GetActiveVisor(mgr);
  switch (activeVisor) {
  case CPlayerState::kPV_Thermal: {
    float rgb =
        CMath::Clamp(0.6f, 0.5f * mShotSmokeTimer + 0.6f - mShotSmokeStartTimer, 1.f);
    mLights.BuildConstantAmbientLighting(CColor(rgb, rgb, rgb, 1.f));
    break;
  }
  case CPlayerState::kPV_Combat: {
    CTransform4f offsetXf = CTransform4f::Translate(camPos) * GetGunMotionTransform();
    CAABox aabb = mCurrentBeam->GetBounds(offsetXf);
    if (mgr.GetNextAreaId() != kInvalidAreaId) {
      mLights.SetFindShadowLight(true);
      mLights.SetShadowDynamicRangeThreshold(0.25f);
      mLights.BuildAreaLightList(mgr, mgr.GetWorld()->GetAreaAlways(mgr.GetNextAreaId()), aabb);
    }
    mLights.BuildDynamicLightList(mgr, aabb);
    if (mLights.HasShadowLight()) {
      if (mCurrentBeam->IsLoaded()) {
        mShadow->BuildLightShadowTexture(mgr, mgr.GetNextAreaId(),
                                             mLights.GetShadowLightIndex(), aabb, true, false);
      }
    } else {
      mShadow->ResetBlur();
    }
    break;
  }
  default:
    break;
  }

  if (mGrappleArm->GetActive()) {
    mGrappleArm->PreRender(mgr, frustum, camPos);
  }

  if (mMorph.GetGunState() != CGunMorph::kGS_OutWipeDone ||
      activeVisor == CPlayerState::kPV_XRay) {
    mRightHandModel.AnimationData()->PreRender();
  }

  if (mPhazonBeamActive) {
    gpRender->AllocatePhazonSuitMaskTexture();
  }
}

static void CopyScreenTex() {
  GXSetTexCopySrc(0x140, 0xe0, 0x140, 0xe0);
  GXSetTexCopyDst(0x140, 0xe0, GX_TF_RGBA8, false);
  GXCopyTex(CGraphics::GetDolphinSpareBuffer(), false);
  GXPixModeSync();
}

void DrawScreenTex(float z) {
  const CTransform4f backupViewMtx(CGraphics::GetViewMatrix());
  const CGraphics::CProjectionState backupProjectionState(CGraphics::GetProjectionState());
  int left, top, width, height;
  CGraphics::GetViewport(left, top, width, height);
  CGraphics::SetOrtho(CCast::ToReal32(left), left + width, top + height, top, -1.f, 1.f);
  CGraphics::SetViewPointMatrix(CTransform4f::Identity());
  gpRender->SetModelMatrix(CTransform4f::Identity());
  gpRender->SetBlendMode_AlphaBlended();
  CGraphics::SetDepthWriteMode(true, kE_GEqual, true);

  CGraphics::LoadDolphinSpareTexture(0x140, 0xe0, GX_TF_RGBA8, NULL,
                                     CGraphics::kSpareBufferTexMapID);

  const GXVtxDescList vtxDesc[3] = {
      {GX_VA_POS, GX_DIRECT},
      {GX_VA_TEX0, GX_DIRECT},
      {GX_VA_NULL, GX_NONE},
  };
  CGX::SetVtxDescv(vtxDesc);
  CGX::SetNumChans(0);
  CGX::SetNumTexGens(1);
  CGX::SetNumTevStages(1);
  CGX::SetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, CGraphics::kSpareBufferTexMapID, GX_COLOR_NULL);
  CGX::SetChanCtrl(CGX::Channel0, false, GX_SRC_REG, GX_SRC_REG, GX_LIGHT_NULL, GX_DF_NONE,
                   GX_AF_NONE);

  const float screenWidth = 640.f;
  const float& screenRight = screenWidth;
  CGX::Begin(GX_TRIANGLESTRIP, GX_VTXFMT0, 4);
  RSPosition3f32(screenWidth / 2.f, z, 0.f);
  GXTexCoord2f32(0.f, 1.f);
  RSPosition3f32(screenRight, z, 0.f);
  GXTexCoord2f32(1.f, 1.f);
  RSPosition3f32(screenWidth / 2.f, z, 224.f);
  GXTexCoord2f32(0.f, 0.f);
  RSPosition3f32(screenRight, z, 224.f);
  GXTexCoord2f32(1.f, 0.f);
  CGX::End();

  CGraphics::SetDepthWriteMode(true, kE_LEqual, true);
  CGraphics::SetViewPointMatrix(backupViewMtx);
  CGraphics::SetProjectionState(backupProjectionState);
}

void CPlayerGun::TouchModel(const CStateManager& mgr) const {
  if (mgr.GetPlayer()->GetMorphballTransitionState() != CPlayer::kMS_Morphed) {
    mGunMotion->GetModelData().Touch(mgr, 0);

    switch (mPhazonBeamState) {
    case kPBS_Entering:
      if (mPhazonBeam.get()) {
        mPhazonBeam->Touch(mgr);
      }
      break;
    case kPBS_Exiting:
      if (mNextBeam != nullptr) {
        mNextBeam->Touch(mgr);
      }
      break;
    default:
      if (!mPhazonBeamActive) {
        mCurrentBeam->Touch(mgr);
      } else {
        mPhazonBeam->Touch(mgr);
      }
      break;
    }

    mCurrentBeam->TouchHolo(mgr);
    mGrappleArm->TouchModel(mgr);
    mRightHandModel.Touch(mgr, 0);
  }

  if (mLoadingBeam != nullptr) {
    mLoadingBeam->Touch(mgr);
    mLoadingBeam->TouchHolo(mgr);
  }
}

CVector3f CPlayerGun::ConvertToScreenSpace(const CVector3f& pos, const CGameCamera& cam) const {
  CVector3f viewPos = cam.GetTransform().TransposeRotate(
      CVector3f(pos.GetX() - cam.GetTransform().Get03(), pos.GetY() - cam.GetTransform().Get13(),
                pos.GetZ() - cam.GetTransform().Get23()));
  CVector3f screenPos(viewPos);

  if (screenPos.IsNonZero()) {
    return CGraphics::GetPerspectiveProjectionMatrix().MultiplyOneOverW(screenPos);
  }

  return CVector3f(-1.f, -1.f, 1.f);
}

inline void CPlayerGun::DrawArm(const CStateManager& mgr, const CVector3f& pos,
                                const CModelFlags& flags) const {
  if (!mGrappleArm->GetActive()) {
    return;
  }
  if (mGrappleArm->GetAnimState() == CGrappleArm::kAS_Done) {
    return;
  }

  const float dot = CVector3f::Dot(mGrappleArm->GetTransform().GetForward(),
                                   mgr.GetPlayer()->GetTransform().GetForward());
  if (mgr.GetPlayer()->GetGrappleState() != CPlayer::kGS_None ||
      (mgr.GetPlayer()->GetGrappleState() == CPlayer::kGS_None && dot > 0.1f)) {
    mGrappleArm->Render(
        mgr, pos, mGrappleArm->IsArmMoving() ? flags : CModelFlags::Normal(), &mLights);
  }
}

void CPlayerGun::Render(const CStateManager& mgr, const CVector3f& pos,
                        const CModelFlags& flags) const {
  const CGraphics::CProjectionState projState = CGraphics::GetProjectionState();
  const CPlayerState::EPlayerVisor visor = mgr.GetPlayerState()->GetCurrentVisor();
  const bool thermalVisor = visor == CPlayerState::kPV_Thermal;
  const CModelFlags& beamFlags =
      thermalVisor ? kThermalFlags[mEquippedBeamId]
      : mPhazonBeamMorphing
          ? static_cast< const CModelFlags& >(CModelFlags::ColorModulate(
                CColor(CColor::Lerp(0xffffffff, 0x000000ff, mPhazonMorphT))))
          : flags;

  const CGameCamera& cam = mgr.GetCameraManager()->GetCurrentCamera(mgr);
  CGraphics::SetDepthRange(kDepthGun, kDepthWorld);
  CTransform4f offsetWorldXf(CTransform4f::Translate(pos) * GetGunMotionTransform());
  CTransform4f elbowOffsetXf(offsetWorldXf * mElbowLocalXf);
  if (mChargePhase != kCP_NotCharging && !IsWeaponStateSet(0x10)) {
    offsetWorldXf.AddTranslation(CVector3f(mShakeX, 0.f, mShakeZ));
  }

  const CGunMorph::EGunState gunState = mMorph.GetGunState();
  CTransform4f oldViewMtx(CGraphics::GetViewMatrix());
  CGraphics::SetViewPointMatrix(offsetWorldXf.GetInverse() * oldViewMtx);
  gpRender->SetModelMatrix(CTransform4f::Identity());
  if (mChargePhase >= kCP_FxGrown && mChargePhase < kCP_ComboXfer) {
    mAuxMuzzleGenerators[mCurrentAuxBeam]->Render();
  }

  if (mChargeEffectVisible &&
      (mMuzzleEffectVisTimer > 0.f || mChargePhase > kCP_AnimAndSfx)) {
    mCurrentBeam->DrawMuzzleFx(mgr);
  }

  if (gunState == CGunMorph::kGS_InWipe || gunState == CGunMorph::kGS_OutWipe) {
    mHoloTransitionGen->Render();
  }

  CGraphics::SetViewPointMatrix(oldViewMtx);
  if (IsWeaponStateSet(0x10)) {
    mAuxWeapon->RenderMuzzleFx();
  }
  mCurrentBeam->PreRenderGunFx(mgr, offsetWorldXf);
  const bool drawSuitArm = !mGrappleArm->IsGrappling() &&
                           mgr.GetPlayer()->GetGunHolsterState() == CPlayer::kGH_Drawn;
  mGunMotion->Draw(mgr, offsetWorldXf);

  switch (gunState) {
  case CGunMorph::kGS_OutWipeDone:
    if (mLights.HasShadowLight()) {
      mShadow->EnableModelProjectedShadow(offsetWorldXf, mLights.GetShadowLightArrIndex(),
                                              2.15f);
    }
    if (visor == CPlayerState::kPV_XRay) {
      CTransform4f handXf(elbowOffsetXf * CTransform4f::Translate(0.f, -0.2f, 0.02f));
      mRightHandModel.Render(mgr, handXf, &mLights,
                                 thermalVisor ? kHandThermalFlag : kHandHoloFlag);
    }
    DrawArm(mgr, pos, flags);
    mCurrentBeam->Draw(drawSuitArm, mgr, offsetWorldXf, beamFlags, &mLights);
    mShadow->DisableModelProjectedShadow();
    break;
  case CGunMorph::kGS_InWipeDone:
  case CGunMorph::kGS_InWipe:
  case CGunMorph::kGS_OutWipe: {
    CTransform4f handXf(elbowOffsetXf * CTransform4f::Translate(0.f, -0.2f, 0.02f));
    if (gunState != CGunMorph::kGS_InWipeDone) {
      CTransform4f morphXf(elbowOffsetXf *
                           CTransform4f::Translate(0.f, mMorph.GetYLerp(), 0.f));
      CopyScreenTex();
      mRightHandModel.Render(mgr, handXf, &mLights,
                                 thermalVisor ? kHandThermalFlag : kHandHoloFlag);
      mCurrentBeam->DrawHologram(mgr, offsetWorldXf, CModelFlags::Normal());
      DrawScreenTex(ConvertToScreenSpace(morphXf.GetTranslation(), cam).GetZ());
      if (mLights.HasShadowLight()) {
        mShadow->EnableModelProjectedShadow(offsetWorldXf, mLights.GetShadowLightArrIndex(),
                                                2.15f);
      }
      gpRender->SetModelMatrix(morphXf);
      DrawClipCube(mHologramClipCube);
      mCurrentBeam->Draw(drawSuitArm, mgr, offsetWorldXf, beamFlags, &mLights);
      DrawArm(mgr, pos, flags);
      mShadow->DisableModelProjectedShadow();
    } else {
      mRightHandModel.Render(mgr, handXf, &mLights,
                                 thermalVisor ? kHandThermalFlag : kHandHoloFlag);
      mCurrentBeam->DrawHologram(mgr, offsetWorldXf, CModelFlags::Normal());
      if (mLights.HasShadowLight()) {
        mShadow->EnableModelProjectedShadow(offsetWorldXf, mLights.GetShadowLightArrIndex(),
                                                2.15f);
      }
      DrawArm(mgr, pos, flags);
      mShadow->DisableModelProjectedShadow();
    }
    break;
  }
  }

  CTransform4f oldViewMtx2(CGraphics::GetViewMatrix());
  CGraphics::SetViewPointMatrix(offsetWorldXf.GetInverse() * oldViewMtx2);
  gpRender->SetModelMatrix(CTransform4f::Identity());
  mCurrentBeam->PostRenderGunFx(mgr, offsetWorldXf);
  if (mComboFiring && mComboXferGen.get()) {
    mComboXferGen->Render();
  }
  CGraphics::SetViewPointMatrix(oldViewMtx2);

  RenderEnergyDrainEffects(mgr);

  CGraphics::SetDepthRange(kDepthWorld, kDepthFar);
  CGraphics::SetProjectionState(projState);
}

void CPlayerGun::GetLctrWithShake(CTransform4f& xfOut, const CModelData& modelData,
                                  const rstl::string& locatorName, bool shake, bool dynamic) {
  xfOut = dynamic ? modelData.GetScaledLocatorTransformDynamic(locatorName, NULL)
                  : modelData.GetScaledLocatorTransform(locatorName);

  if (mCharging && shake) {
    xfOut.AddTranslation(CVector3f(mShakeX, 0.f, mShakeZ));
  }
}

void CPlayerGun::PlayAnim(NWeaponTypes::EGunAnimType type, bool loop) {
  if (mNextState != kNS_ChangeWeapon)
    mCurrentBeam->PlayAnim(type, loop);

  ushort sfx = CSfxManager::kInternalInvalidSfxId;
  switch (type) {
  case NWeaponTypes::kGAT_FromMissile:
    DisableWeaponState(0x4);
    sfx = mFromMissileSound[mEquippedBeamId];
    break;
  case NWeaponTypes::kGAT_MissileReload:
    sfx = SFXsam_a_mislload_00;
    break;
  case NWeaponTypes::kGAT_FromBeam:
    sfx = mFromBeamSound[mEquippedBeamId];
    break;
  case NWeaponTypes::kGAT_ToMissile:
    DisableWeaponState(0x1);
    sfx = mToMissileSound[mEquippedBeamId];
    break;
  default:
    break;
  }

  if (sfx != CSfxManager::kInternalInvalidSfxId)
    NWeaponTypes::play_sfx(sfx, mUnderwater, false, 0x4a);
}

inline bool just_thawed(bool frozen, bool playerFrozen) {
  return (frozen ^ playerFrozen) & frozen;
}

inline bool just_froze(bool frozen, bool playerFrozen) {
  return (frozen ^ playerFrozen) & playerFrozen;
}

void CPlayerGun::Update(float grappleSwingT, float cameraBobT, float dt, CStateManager& mgr) {
  CPlayer& player = *mgr.Player();
  const CGunMorph::EGunState gunState = mMorph.GetGunState();
  CPlayerState& playerState = *mgr.PlayerState();
  const bool isUnmorphed = player.GetMorphballTransitionState() == CPlayer::kMS_Unmorphed;

  const bool justFroze = isUnmorphed ? just_froze(mFrozen, player.GetFrozenState()) : false;
  const bool justThawed =
      isUnmorphed ? just_thawed(mFrozen, player.GetFrozenState()) : false;
  mFrozen = isUnmorphed ? player.GetFrozenState() : false;
  const float advDt = mFrozen ? 0.f : dt;

  const bool inMorph = gunState != CGunMorph::kGS_OutWipeDone;
  if (mgr.GetPlayerState()->GetCurrentVisor() == CPlayerState::kPV_XRay || inMorph) {
    mRightHandModel.AdvanceAnimation(advDt, mgr, kInvalidAreaId, true);
  }
  if (inMorph && mLoadingBeam != NULL && mLoadingBeam != mCurrentBeam) {
    mAuxWeapon->LoadIdle();
    mLoadingBeam->Update(advDt, mgr);
  }
  if (!mAuxWeapon->IsLoaded()) {
    mAuxWeapon->LoadIdle();
  }

  if (justFroze) {
    mCurrentBeam->EnableSecondaryFx(CGunWeapon::kSFT_None);
    mCurrentBeam->EnableFrozenEffect(kFFT_Frozen);
  } else if (justThawed) {
    mCurrentBeam->EnableFrozenEffect(kFFT_Thawed);
  }

  if (justFroze || justThawed) {
    mFireButtonStates = 0;
    mLastFireButtonStates = 0;
    CancelFiring(mgr);
  }

  mCurrentBeam->Update(advDt, mgr);
  mGunMotion->Update(advDt * mGunMotionSpeedMult, mgr);
  mGrappleArm->Update(grappleSwingT, advDt, mgr);

  if (mNextState != kNS_StatusQuo) {
    const CAnimData& animData = *mCurrentBeam->GetSolidModelData().GetAnimationData();
    if (gunState == CGunMorph::kGS_InWipeDone) {
      if (mNextState == kNS_ChangeWeapon) {
        ChangeWeapon(playerState, mgr);
        mNextState = kNS_StatusQuo;
      }
    } else if (!animData.IsAnimTimeRemaining(0.001f, rstl::string_l("Whole Body")) ||
               mRequestReturnToDefault) {
      bool statusQuo = true;
      switch (mNextState) {
      case kNS_EnterMissile:
        ResetToMissile();
        break;
      case kNS_ExitMissile:
        ResetToBeam();
        mCooldown = mCurrentBeam->GetWeaponInfo().mCoolDown;
        break;
      case kNS_MissileReload:
        PlayAnim(NWeaponTypes::kGAT_MissileReload, false);
        mNextState = kNS_MissileShotDone;
        statusQuo = false;
        break;
      case kNS_MissileShotDone:
        EnableWeaponState(0x4);
        break;
      case kNS_ChangeWeapon:
        ChangeWeapon(playerState, mgr);
        break;
      case kNS_SetupBeam:
        mCooldown = mCurrentBeam->GetWeaponInfo().mCoolDown;
        DisableWeaponState(0x8);
        ResetToBeam();
        break;
      case kNS_EnterPhazonBeam:
        if (mPhazonBeam->IsLoaded()) {
          break;
        }
        mCurrentBeam->mDrawHologram = true;
        mPhazonBeam->Load(mgr, false);
        mPhazonBeamState = kPBS_Entering;
        break;
      case kNS_ExitPhazonBeam:
        if (mNextBeam->IsLoaded()) {
          break;
        }
        mCurrentBeam->mDrawHologram = true;
        mNextBeam->Load(mgr, false);
        mPhazonBeamState = kPBS_Exiting;
        break;
      default:
        break;
      }

      if (statusQuo) {
        mNextState = kNS_StatusQuo;
      }
    }
  }

  if (mRapidFireShotsDecayTimer < 0.2f) {
    mRapidFireShotsDecayTimer += advDt;
  } else {
    mRapidFireShotsDecayTimer = 0.f;
    if (mRapidFireShots > 0) {
      mRapidFireShots -= 1;
    }
  }

  if (mChargePhase != kCP_NotCharging && !player.GetFrozenState()) {
    mShakeX =
        chargeShakeTbl[static_cast< int >(mgr.Random()->Next()) % 3] * mChargeBeamFactor;
    mShakeZ =
        chargeShakeTbl[static_cast< int >(mgr.Random()->Next()) % 3] * mChargeBeamFactor;
  }

  if (!mCurrentBeam->IsLoaded()) {
    return;
  }

  CModelData& beamModel = *mCurrentBeam->SolidModelData();
  const CModelData& motionModel = mGunMotion->GetModelData();
  GetLctrWithShake(mGunLocalXf, motionModel, rstl::string_l(kGunLocator), true, true);
  GetLctrWithShake(mBeamLocalXf, beamModel, rstl::string_l(CGunWeapon::skMuzzleLocator), false,
                   true);
  GetLctrWithShake(mElbowLocalXf, beamModel, rstl::string_l("elbow"), false, false);
  mGunWorldXf = mXf * mGunLocalXf * mCamBob.GetCameraBobTransformation();

  if (mGrappleArm->GetActive() && !mGrappleArm->IsGrappling()) {
    UpdateLeftArmTransform(beamModel, mgr);
  }

  mMotionState.Update((mPressedFireButtonStates & 1) != 0 && mReadyForShot &&
                              mChargePhase < kCP_AnimAndSfx && !player.IsInFreeLook(),
                          advDt, mGunWorldXf, mgr);

  beamModel.AdvanceParticles(GetGunMotionTransform(), advDt, mgr);
  mCurrentBeam->UpdateGunFx(mShotSmokeTimer > 2.f && mShotSmokeStartTimer > 0.15f, dt,
                                mgr, mElbowLocalXf);

  CTransform4f beamWorldXf = GetGunMotionTransform() * mBeamLocalXf;

  if (player.GetMorphballTransitionState() == CPlayer::kMS_Unmorphed &&
      !mgr.GetCameraManager()->IsInCinematicCamera()) {
    TEntityList nearList;
    mgr.BuildNearList(nearList,
                      mCurrentBeam->GetBounds().GetTransformedAABox(GetGunMotionTransform()),
                      sAimFilter, &player);
    TUniqueId bestId = kInvalidUniqueId;
    const CVector3f dir = mGunWorldXf.GetForward().AsNormalized();
    const CVector3f offset = -(dir * 0.5f);
    const CVector3f pos = mGunWorldXf.GetTranslation() + offset;
    const CRayCastResult result = mgr.RayWorldIntersection(
        bestId, pos, dir, 3.5f,
        CMaterialFilter::MakeIncludeExclude(CMaterialList(kMT_Solid),
                                            CMaterialList(kMT_ProjectilePassthrough)),
        nearList);
    mPointBlankWorldSurface = result.IsValid();
    if (result.IsValid()) {
      mElbowWorldXf = GetGunMotionTransform() * mElbowLocalXf;
      mElbowWorldXf.AddTranslation(offset);
      beamWorldXf.SetTranslation(result.GetPoint());
    }
  } else {
    mPointBlankWorldSurface = false;
  }

  CTransform4f beamTargetXf = mPointBlankWorldSurface ? mElbowWorldXf : beamWorldXf;

  const CVector3f camTrans = mgr.GetCameraManager()->GetGlobalCameraTranslation(mgr);
  beamWorldXf.AddTranslation(camTrans);
  beamTargetXf.AddTranslation(camTrans);

  if (mChargeEffectVisible) {
    const bool emitting = mCanShowAuxMuzzleEffect ? mComboXferTimer < 1.f : false;
    const float scaleFactor =
        (emitting && mComboFiring) ? (1.f - mComboXferTimer) * 2.f : 2.f;
    const CVector3f scale(scaleFactor, scaleFactor, scaleFactor);
    mCurrentBeam->UpdateMuzzleFx(advDt, scale, mBeamLocalXf.GetTranslation(), emitting);
    mAuxMuzzleGenerators[mCurrentAuxBeam]->SetGlobalOrientAndTrans(mBeamLocalXf);
    mAuxMuzzleGenerators[mCurrentAuxBeam]->SetGlobalScale(scale);
    mAuxMuzzleGenerators[mCurrentAuxBeam]->SetParticleEmission(emitting);
    mAuxMuzzleGenerators[mCurrentAuxBeam]->Update(advDt);
  }

  if (mRainSplashGenerator.get()) {
    mRainSplashGenerator->Update(advDt, mgr);
  }

  UpdateGunLight(beamWorldXf, mgr);
  ProcessGunMorph(advDt, mgr);
  if (mPhazonBeamMorphing) {
    ProcessPhazonGunMorph(advDt, mgr);
  }

  if (mComboFiring && !mComboXferGen.null()) {
    mComboXferGen->SetGlobalTranslation(mBeamLocalXf.GetTranslation());
    mComboXferGen->SetGlobalOrientation(mBeamLocalXf.GetRotation());
    mComboXferGen->Update(advDt);
    mComboXferTimer += advDt * 4.f;
  }

  if (mBombTime > 0.f) {
    mBombTime -= advDt;
    if (mBombTime <= 0.f) {
      mBombCount = 3;
    }
  }

  if (playerState.ItemEnabled(CPlayerState::kIT_ChargeBeam) &&
      mChargePhase != kCP_NotCharging) {
    UpdateChargeState(advDt, mgr);
  } else {
    mChargeBeamFactor -= advDt;
    if (mChargeBeamFactor < 0.f) {
      mChargeBeamFactor = 0.f;
    }
  }

  UpdateAuxWeapons(advDt, beamTargetXf, mgr);
  DoUserAnimEvents(advDt, mgr);

  if (player.GetOrbitState() == CPlayer::kOS_OrbitObject && GetTargetId(mgr) != kInvalidUniqueId) {
    if (!mLockedOn && !mComboFiring && !IsWeaponStateSet(0x10)) {
      mLockedOn = true;
      mMotionState.SetState(CMotionState::kMS_LockOn);
      ReturnArmAndGunToDefault(mgr, true);
    }
  } else {
    CancelLockOn();
  }

  UpdateWeaponFire(advDt, playerState, mgr);
  UpdateGunIdle(mGunStrikeCoolTimer > 0.f, cameraBobT, advDt, mgr);

  if ((mLastFireButtonStates & 0x1) != 0) {
    mShotSmokeStartTimer = 0.f;
  } else if (mShotSmokeStartTimer < 2.f) {
    mShotSmokeStartTimer += advDt;
    if (mShotSmokeStartTimer > 1.f) {
      mRapidFireShots = 0;
      mShotSmokeTimer = 0.f;
    }
  }

  if (mMuzzleEffectVisTimer > 0.f) {
    mMuzzleEffectVisTimer -= advDt;
  }

  if (mRapidFireShots > 5 && mShotSmokeTimer < 2.f) {
    mShotSmokeTimer += advDt;
  }

  if (mGunStrikeDelayTimer > 0.f) {
    mGunStrikeDelayTimer -= advDt;
  }

  if (mGunStrikeCoolTimer > 0.f) {
    mFireButtonStates = 0;
    mGunStrikeCoolTimer -= advDt;
  }

  if (isUnmorphed && IsWeaponStateSet(0x4)) {
    mMissileExitTimer -= advDt;
    if (mMissileExitTimer < 0.f) {
      mMissileExitTimer = 0.f;
      ExitMissile();
    }
  }
}

void CPlayerGun::ProcessInput(const CFinalInput& input, CStateManager& mgr) {
  const CPlayer* player = mgr.GetPlayer();
  const CPlayerState* cPlayerState = mgr.GetPlayerState();
  CPlayerState* playerState = const_cast< CPlayerState* >(cPlayerState);
  bool damageNotMorphed = false;
  if (mInBigStrike && player->GetMorphballTransitionState() != CPlayer::kMS_Morphed) {
    damageNotMorphed = true;
  }

  if (mCoolingCharge || damageNotMorphed || IsWeaponStateSet(0x8)) {
#if VERSION >= VERSION_GM8P_00
    if (IsWeaponStateSet(0x8)) {
      mFireButtonStates = 0;
    }
#endif
    return;
  }

  if (cPlayerState->HasPowerUp(CPlayerState::kIT_ChargeBeam)) {
    if (!cPlayerState->ItemEnabled(CPlayerState::kIT_ChargeBeam)) {
      playerState->EnableItem(CPlayerState::kIT_ChargeBeam);
    }
  } else if (cPlayerState->ItemEnabled(CPlayerState::kIT_ChargeBeam)) {
    playerState->DisableItem(CPlayerState::kIT_ChargeBeam);
    ResetCharge(mgr, false);
  }

  switch (player->GetMorphballTransitionState()) {
  case CPlayer::kMS_Morphing:
  case CPlayer::kMS_Unmorphing:
    mFireButtonStates = 0;
    break;
  case CPlayer::kMS_Unmorphed:
    if (!IsWeaponStateSet(0x10)) {
      HandleWeaponChange(input, mgr);
    }
    // fallthrough
  case CPlayer::kMS_Morphed:
    mFireButtonStates =
        ControlMapper::GetDigitalInput(ControlMapper::kC_FireOrBomb, input) ? 1 : 0;
    mFireButtonStates |=
        ControlMapper::GetDigitalInput(ControlMapper::kC_MissileOrPowerBomb, input) ? 2 : 0;
    break;
  }
}

void CPlayerGun::ProcessChargeState(int releasedStates, int pressedStates, CStateManager& mgr,
                                    float dt) {
  if ((releasedStates & 0x1) != 0) {
    ResetCharged(dt, mgr);
    return;
  }
  if ((pressedStates & 0x1) != 0) {
    if (mChargePhase == kCP_NotCharging && (pressedStates & 0x1) != 0 &&
        mChargeCooldownTimer == 0.f && mReadyForShot == 1) {
      UpdateNormalShotCycle(dt, mgr);
      mChargePhase = kCP_ChargeRequested;
    }
  } else {
    const CPlayerState* state = mgr.GetPlayerState();
    if (state->HasPowerUp(CPlayerState::kIT_Missiles) && (pressedStates & 0x2) != 0) {
      if (mChargePhase >= kCP_FxGrown) {
        if (state->HasPowerUp(mBeamComboArr[size_t(mEquippedBeamId)]))
          ActivateCombo(mgr);
      } else if (mChargePhase == kCP_NotCharging) {
        FireSecondary(dt, mgr);
      }
    }
  }
}

void CPlayerGun::ResetNormal(CStateManager& mgr) {
  Reset(mgr, false);
  mReadyForShot = false;
}

void CPlayerGun::ResetCharged(float dt, CStateManager& mgr) {
  if (mComboFiring == true) {
    return;
  }

  if (mChargePhase >= kCP_FxGrowing) {
    mCanShowAuxMuzzleEffect = false;
    UpdateNormalShotCycle(dt, mgr);
    mCoolingCharge = true;
    CancelCharge(mgr, true);
  } else if (mChargePhase != kCP_NotCharging) {
    mCurrentAuxBeam = mEquippedBeamId;
    mCanShowAuxMuzzleEffect = true;
    mChargePhase = kCP_ChargeDone;
  }
  StopChargeSound(mgr);
}

void CPlayerGun::ProcessNormalState(int releasedStates, int pressedStates, CStateManager& mgr,
                                    float dt) {
  if ((releasedStates & 0x1) != 0) {
    ResetNormal(mgr);
    return;
  }

  if ((pressedStates & 0x1) != 0 && mChargeCooldownTimer == 0.f &&
      mReadyForShot == true) {
    UpdateNormalShotCycle(dt, mgr);
    return;
  }
  if ((pressedStates & 0x2) != 0) {
    FireSecondary(dt, mgr);
  }
}

bool CPlayerGun::ExitMissile() {
  if (!IsWeaponStateSet(0x1)) {
    if (!IsWeaponStateSet(0x10) && mNextState != kNS_ExitMissile) {
      mNextState = kNS_ExitMissile;
      PlayAnim(NWeaponTypes::kGAT_FromMissile, false);
    }
    return false;
  }
  return true;
}

void CPlayerGun::UpdateNormalShotCycle(float dt, CStateManager& mgr) {
  if (!ExitMissile() || mgr.GetCameraManager()->IsInCinematicCamera()) {
    return;
  }
  bool showChargeFx = mPhazonBeamActive;
  if (!mPhazonBeamActive && mEquippedBeamId == CPlayerState::kBI_Plasma) {
    showChargeFx = true;
  }
  const uchar hideChargeEffect = showChargeFx && mChargePhase == kCP_NotCharging;
  mChargeEffectVisible = !hideChargeEffect;
  mRapidFireShots += 1;

  const uint targetHoming =
      mCurrentBeam->GetVelocityInfo().GetTargetHoming(int(mChargeState));

  CTransform4f xf(mPointBlankWorldSurface ? mElbowWorldXf
                                                 : GetGunMotionTransform() * mBeamLocalXf);
  if (!mPointBlankWorldSurface && mGunStrikeCoolTimer <= 0.f) {
    const CVector3f pos = xf.GetTranslation();
    xf = mAssistAimXf;
    xf.SetTranslation(pos);
  }

  xf.AddTranslation(mgr.GetCameraManager()->GetGlobalCameraTranslation(mgr));
  mMuzzleEffectVisTimer = 0.0625f;

  mCurrentBeam->Fire(
      mUnderwater, dt, CPlayerState::EChargeStage(mChargeState), xf, mgr,
      static_cast< const TUniqueId& >(targetHoming ? GetTargetId(mgr) : kInvalidUniqueId),
      mChargeBeamFactor, mChargeBeamFactor);

  mgr.InformListeners(GetGunMotionTransform().GetTranslation(), kLNT_PlayerFire);
}

void CPlayerGun::FireSecondary(float dt, CStateManager& mgr) {
  if (mgr.GetCameraManager()->IsInCinematicCamera()) {
    return;
  }

  CPlayerState* playerState = mgr.PlayerState();
  if (mInPhazonBeam || int(mComboAmmoIdx) == 0 ||
      playerState->HasPowerUp(skItemArr[mComboAmmoIdx]) != true || !IsWeaponStateSet(0x4)) {
    NWeaponTypes::play_sfx(SFXsam_b_malfxn_00, mUnderwater, false, 0x4a);
    return;
  }

  bool fired = false;
  switch (mComboAmmoIdx) {
  case 1: {
    mRemainingMissiles = playerState->GetItemAmount(CPlayerState::kIT_Missiles);
    if (mgr.GetWeaponIdCount(mPlayerId, kWT_Missile) < 3 && int(mRemainingMissiles) != 0) {
      playerState->DecrPickUp(CPlayerState::kIT_Missiles,
                              mComboFiring ? playerState->GetMissileCostForAltAttack() : 1);
      fired = true;
    }

    if (int(mRemainingMissiles) > 5) {
      mRemainingMissiles = 5;
    } else {
      mRemainingMissiles -= 1;
    }
    break;
  }
  default:
    break;
  }

  if (fired) {
    TUniqueId targetId = GetTargetId(mgr);
    if (mComboFiring && targetId == kInvalidUniqueId &&
        mEquippedBeamId == CPlayerState::kBI_Wave) {
      targetId = mgr.GetPlayer()->GetAimTargetId();
    }

    CTransform4f xf(mPointBlankWorldSurface ? mElbowWorldXf
                                                   : GetGunMotionTransform() * mBeamLocalXf);
    if (!mPointBlankWorldSurface && mGunStrikeCoolTimer <= 0.f) {
      const CVector3f pos = xf.GetTranslation();
      xf = mAssistAimXf;
      xf.SetTranslation(pos);
    }

    xf.AddTranslation(mgr.GetCameraManager()->GetGlobalCameraTranslation(mgr));
    mAuxWeapon->Fire(dt, mUnderwater, CPlayerState::EBeamId(mEquippedBeamId),
                         CPlayerState::EChargeStage(mChargeState), xf, mgr,
                         mCurrentBeam->GetType(), targetId);

    mgr.InformListeners(GetGunMotionTransform().GetTranslation(), kLNT_PlayerFire);

    mMissileExitTimer = 7.f;
    if (mComboFiring != true) {
      PlayAnim(NWeaponTypes::kGAT_MissileShoot, false);
      mNextState = int(mRemainingMissiles) > 0 ? kNS_MissileReload : kNS_MissileShotDone;
      DisableWeaponState(0x4);
    }
  } else {
    NWeaponTypes::play_sfx(mItemEmptySound[mComboAmmoIdx], mUnderwater, false, 0x4a);
  }
}

void CPlayerGun::DropBomb(CPlayerGun::EBWeapon weapon, CStateManager& mgr) {
  const float ballHalfExtent = gpTweakPlayer->GetPlayerBallHalfExtent();

  switch (weapon) {
  case kBW_Bomb: {
    if (mChargePhase != kCP_NotCharging) {
      mChargePhase = kCP_ChargeDone;
      break;
    }

    if (mBombCount <= 0) {
      break;
    }

    CBomb* const bomb = rs_new CBomb(mBombEffects[weapon][0], mBombEffects[weapon][1],
                                     mgr.AllocateUniqueId(), mgr.GetPlayer()->GetCurrentAreaId(),
                                     mPlayerId, mBombFuseTime,
                                     CTransform4f::Translate(mgr.GetPlayer()->GetTranslation() +
                                                             CVector3f(0.f, 0.f, ballHalfExtent)),
                                     gpTweakPlayerGun->GetBombInfo());
    mgr.AddObject(*bomb);

    if (mBombCount == 3) {
      mBombTime = mBombDropDelayTime;
    }

    --mBombCount;

    const TUniqueId platformId = mgr.GetPlayer()->GetRidingPlatformId();
    if (CEntity* ent = mgr.ObjectById(platformId)) {
      if (CScriptPlatform* plat = TCastToPtr< CScriptPlatform >(ent)) {
        plat->AddSlave(bomb->GetUniqueId(), mgr);
      }
    }
    break;
  }
  case kBW_PowerBomb:
    mgr.PlayerState()->DecrPickUp(CPlayerState::kIT_PowerBombs, 1);
    mPowerBomb = DropPowerBomb(mgr);
    break;
  default:
    break;
  }
}

void CPlayerGun::ActivateCombo(CStateManager& mgr) {
  if (mComboFiring == true) {
    return;
  }

  CPlayerState& playerState = *mgr.PlayerState();
  const int altCost = playerState.GetMissileCostForAltAttack();
  if (playerState.GetItemAmount(skItemArr[mComboAmmoIdx]) >= altCost) {
    bool canFire = true;
    if (mEquippedBeamId == CPlayerState::kBI_Plasma) {
      canFire = !mUnderwater;
    }

    if (canFire) {
      mComboFiring = true;

      TCachedToken< CGenDescription >& cachedXferEffect = mCurrentBeam->mXferEffect;
      if (cachedXferEffect.TryCache()) {
        mComboXferGen = rs_new CElementGen(cachedXferEffect);
        mComboXferGen->SetGlobalScale(kScaleVector);
      }

      mCurrentBeam->mEnableCharge = true;
      StopChargeSound(mgr);
      NWeaponTypes::play_sfx(SFXsam_a_combochg_00, mUnderwater, false, 0x4a);
      mChargePhase = kCP_ComboXfer;
    }
  } else {
    NWeaponTypes::play_sfx(SFXsam_b_malfxn_00, mUnderwater, false, 0x4a);
  }
}

void CPlayerGun::EnableChargeFx(CPlayerState::EChargeStage, CStateManager& mgr) {
  mCurrentBeam->ActivateCharge(true, false);
  SetGunLightActive(true, mgr);
  mCurrentBeam->EnableSecondaryFx(CGunWeapon::kSFT_Charge);
  StopContinuousBeam(mgr, false);

  switch (mEquippedBeamId) {
  case CPlayerState::kBI_Power:
  case CPlayerState::kBI_Plasma:
    mChargeEffectVisible = true;
    break;
  default:
    break;
  }

  EnableWeaponState(0x7);
  mComboAmmoIdx = 1;
  mNextState = kNS_StatusQuo;
  mCanShowAuxMuzzleEffect = true;

  mAuxMuzzleGenerators[mCurrentAuxBeam] =
      rs_new CElementGen(mAuxMuzzleEffects[mCurrentAuxBeam]);
  mAuxMuzzleGenerators[mCurrentAuxBeam]->SetParticleEmission(true);
}

void CPlayerGun::UpdateChargeState(float dt, CStateManager& mgr) {
  switch (mChargePhase) {
  case kCP_ChargeRequested:
    mChargeBeamFactor = 0.f;
    mChargeState = kCS_Normal;
    mChargeAnimStarted = false;
    mCharging = true;
    mChargePhase = kCP_AnimAndSfx;
    break;
  case kCP_AnimAndSfx:
    if (mChargeAnimStarted != true) {
      if (mChargeBeamFactor > kChargeStart && mChargeEffectVisible) {
        mChargeEffectVisible = false;
      }
      if (mChargeBeamFactor > kTractorBeamFactor) {
        PlayAnim(NWeaponTypes::kGAT_ChargeUp, false);
        if (!mChargeSfx) {
          mChargeSfx = NWeaponTypes::play_sfx(sBeamChargeUpSound[mEquippedBeamId],
                                                  mUnderwater, true, 0x4a);
        }
        if (mChargeRumbleHandle == -1) {
          mChargeRumbleHandle =
              mgr.GetRumbleManager()->Rumble(mgr, kRFX_PlayerGunCharge, 1.f, kRP_Three);
        }
        mChargeAnimStarted = true;
      }
    } else if (mChargeBeamFactor >= kChargeFxStart && !IsWeaponStateSet(0x8)) {
      mChargeEffectVisible = true;
      mChargeAnimStarted = false;
      mChargePhase = kCP_FxGrowing;
      mChargeState = kCS_Charged;
      EnableChargeFx(CPlayerState::kCS_Charged, mgr);
      PlayAnim(NWeaponTypes::kGAT_ChargeLoop, true);
    }
    break;
  case kCP_FxGrowing:
    if (mChargeBeamFactor >= 1.f) {
      mChargePhase = kCP_FxGrown;
    }
    break;
  case kCP_ComboXfer:
    if (mComboXferTimer >= 1.f) {
      mChargePhase = kCP_ComboXferDone;
      mChargeEffectVisible = false;
    }
    break;
  case kCP_ComboXferDone:
    mChargePhase = kCP_ComboFire;
    mChargeCooldownTimer = 0.f;
    break;
  case kCP_ComboFire:
    mGrappleArm->EnterComboFire(int(mEquippedBeamId), mgr);
    mGunMotion->PlayPasAnim(SamusGun::kAS_ComboFire, mgr, 0.f, false);
    mCurrentBeam->PlayPasAnim(SamusGun::kAS_ComboFire, mgr, 0.f);
    mInFreeLook = false;
    mChargePhase = kCP_ComboFireDone;
    break;
  case kCP_ChargeCooldown:
    if (!IsWeaponStateSet(0x10)) {
      mChargeCooldownTimer += dt;
      if (mChargeCooldownTimer >= 0.3f && mCurrentBeam->IsChargeAnimOver()) {
        mChargePhase = kCP_ChargeDone;
      }
    } else {
      mCoolingCharge = false;
    }
    break;
  case kCP_ChargeDone:
    ResetCharge(mgr, false);
    Reset(mgr, false);
    break;
  default:
    break;
  }

  if (mChargeSfx) {
    CSfxManager::PitchBend(mChargeSfx, mUnderwater ? 0 : 0x2000);
  }

  if (mChargePhase > kCP_NotCharging && mChargePhase < kCP_FxGrown) {
    mChargeBeamFactor += kChargeSpeed * dt;
    if (mChargeBeamFactor > 1.f) {
      mChargeBeamFactor = 1.f;
    }
  }
}

void CPlayerGun::Reset(CStateManager& mgr, bool b1) {
  mCurrentBeam->Reset(mgr);
  mChargeEffectVisible = false;
  mCoolingCharge = false;
  x833_26_ = false;
  mChargeCooldownTimer = 0.f;
  SetGunLightActive(false, mgr);
  if (!IsWeaponStateSet(0x10)) {
    if (!b1 && !IsWeaponStateSet(0x2)) {
      ResetToBeam();
    }
  } else {
    DisableWeaponState(0x7);
  }
}

void CPlayerGun::ResetCharge(CStateManager& mgr, bool resetBeam) {
  if (mChargePhase != kCP_NotCharging) {
#if VERSION >= VERSION_GM8P_00
    mCurrentBeam->ActivateCharge(false, false);
    SetGunLightActive(false, mgr);
#endif
    StopChargeSound(mgr);
  }

  if (!IsWeaponStateSet(0x8) && !IsWeaponStateSet(0x10)) {
    bool doResetBeam =
        mgr.GetPlayer()->GetMorphballTransitionState() == CPlayer::kMS_Morphed || resetBeam;
    if (mChargeAnimStarted || doResetBeam)
      PlayAnim(NWeaponTypes::kGAT_BasePosition, false);
    if (doResetBeam)
      mCurrentBeam->EnableSecondaryFx(CGunWeapon::kSFT_None);
    if (!IsWeaponStateSet(0x2) || mChargeState != kCS_Normal) {
      ResetToBeam();
    }
  }

  mChargePhase = kCP_NotCharging;
  mChargeState = kCS_Normal;
  mCurrentAuxBeam = mEquippedBeamId;
  mCanShowAuxMuzzleEffect = true;
  mChargeAnimStarted = false;
  mComboFiring = false;
  mComboXferTimer = 0.f;
}

#define SFXwpn_morph_out_wipe 1774
#define SFXwpn_morph_in_wipe_done 1775

void CPlayerGun::ResetBeamParams(CStateManager& mgr, const CPlayerState& playerState,
                                 bool playSelectionSfx) {
  StopContinuousBeam(mgr, true);
  if (playerState.ItemEnabled(CPlayerState::kIT_ChargeBeam)) {
    ResetCharge(mgr, false);
  }
  const CAnimPlaybackParms parms(mHandAnimId[size_t(mNextBeamId)], -1, 1.f, true);
  mRightHandModel.AnimationData()->SetAnimation(parms, false);
  Reset(mgr, false);
  if (playSelectionSfx) {
    CSfxManager::SfxStart(SFXwpn_morph_out_wipe, 0x7f, 0x40, true, CSfxManager::kMaxPriority, false,
                          CSfxManager::kAllAreas);
  }
  mLastFireButtonStates &= ~0x1;
  mCurrentAuxBeam = mEquippedBeamId;
  mCanShowAuxMuzzleEffect = true;
}

void CPlayerGun::ChangeWeapon(const CPlayerState& playerState, CStateManager& mgr) {
  if (mOutgoingBeam != nullptr && mOutgoingBeam != mCurrentBeam)
    mOutgoingBeam->Unload(mgr);

  mLoadingBeam = mSelectableBeams[mNextBeamId];
  if (mLoadingBeam != nullptr && mLoadingBeam != mCurrentBeam) {
    mLoadingBeam->Load(mgr, false);
    mAuxWeapon->Load(mNextBeamId, mgr);
  }

  mCurrentBeam->EnableFx(false);
  mRequestImmediateRecharge = mChargePhase != kCP_NotCharging;
  ResetBeamParams(mgr, playerState, true);
  mMorph.StartWipe(CGunMorph::kD_In);
}

void CPlayerGun::StartPhazonBeamTransition(bool active, CStateManager& mgr,
                                           CPlayerState& playerState) {
  if (mPhazonBeamActive == active) {
    return;
  }

  mSelectableBeams[mEquippedBeamId]->Unload(mgr);
  mSelectableBeams[mEquippedBeamId] = active ? mPhazonBeam.get() : mNextBeam;
  ResetBeamParams(mgr, playerState, false);
  mCurrentBeam = mSelectableBeams[mEquippedBeamId];
  mPhazonBeamActive = active;
  SetPhazonBeamFeedback(active);
  mCurrentBeam->mRainSplashGenerator = mRainSplashGenerator.get();
  mCurrentBeam->EnableFx(true);
  mCurrentBeam->mDrawHologram = false;
  PlayAnim(NWeaponTypes::kGAT_ToBeam, false);
  if (mInFreeLook) {
    EnterFreeLook(mgr);
  } else if (mRequestReturnToDefault) {
    ReturnArmAndGunToDefault(mgr, false);
  }
  mRequestReturnToDefault = false;
}

void CPlayerGun::HandleWeaponChange(const CFinalInput& input, CStateManager& mgr) {
  x833_25_ = false;
  if (ControlMapper::GetPressInput(ControlMapper::kC_Morph, input)) {
    StopContinuousBeam(mgr, true);
  }
  if (!IsWeaponStateSet(0x8)) {
    if (!mInPhazonBeam) {
      HandleBeamChange(input, mgr);
    } else {
      HandlePhazonBeamChange(mgr);
    }
  }
}

void CPlayerGun::HandleBeamChange(const CFinalInput& input, CStateManager& mgr) {
  const CPlayerState& playerState = *mgr.GetPlayerState();
  float maxInput = 0.f;
  int beam = -1;

  for (int i = 0; i < 4; ++i) {
    if (playerState.HasPowerUp(mBeamArr[i])) {
      const float inputVal = ControlMapper::GetAnalogInput(mBeamCtrlCmd[i], input);
      if (inputVal > 0.65f && inputVal > maxInput) {
        maxInput = inputVal;
        beam = i;
      }
    }
  }

  if (beam > -1) {
    x833_25_ = true;
    if (mEquippedBeamId != beam && playerState.HasPowerUp(mBeamArr[beam])) {
      mNextBeamId = static_cast< CPlayerState::EBeamId >(beam);

      uint stateFlags = 0;
      if (IsWeaponStateSet(0x10)) {
        stateFlags = 0x10;
      }
      SetStateFlags(0);
      EnableWeaponState(stateFlags | 0x8);
      PlayAnim(NWeaponTypes::kGAT_FromBeam, false);

      if (mInFreeLook || mAuxWeapon->IsComboFxActive(mgr) || mComboFiring) {
        mRequestReturnToDefault = true;
        mGrappleArm->EnterIdle(mgr);
      }

      mCurrentBeam->EnableSecondaryFx(CGunWeapon::kSFT_None);
      mNextState = kNS_ChangeWeapon;
      mInvalidSfx.Clear();
    } else if (playerState.HasPowerUp(mBeamArr[beam])) {
      if (ExitMissile()) {
        if (!CSfxManager::IsPlaying(mInvalidSfx)) {
          mInvalidSfx =
              NWeaponTypes::play_sfx(SFXsam_a_mislemp_00, mUnderwater, false, 0x4a);
        }
      } else {
        mInvalidSfx.Clear();
      }
    }
  }
}

void CPlayerGun::SetPhazonBeamMorph(bool intoPhazonBeam) {
  mPhazonMorphT = intoPhazonBeam ? 0.f : 1.f;
  const bool into = intoPhazonBeam;
  const int morphing = true;
  mIntoPhazonBeam = into;
  mPhazonBeamMorphing = morphing;
}

void CPlayerGun::HandlePhazonBeamChange(CStateManager& mgr) {
  bool inMorph = false;
  switch (mPhazonBeamState) {
  case kPBS_Inactive:
    SetPhazonBeamMorph(true);
    mNextState = kNS_EnterPhazonBeam;
    inMorph = true;
    break;
  case kPBS_Active:
    if (!mCanFirePhazon) {
      SetPhazonBeamMorph(true);
      mNextState = kNS_ExitPhazonBeam;
      inMorph = true;
      CPhazonBeam* beam = mPhazonBeam.get();
      if (beam) {
        beam->mClipWipeActive = false;
        beam->mVeinsAlphaActive = true;
      }
    }
    break;
  default:
    break;
  }

  if (inMorph) {
    ResetBeamParams(mgr, *mgr.GetPlayerState(), true);
    SetStateFlags(0);
    EnableWeaponState(0x8);
    PlayAnim(NWeaponTypes::kGAT_FromBeam, false);
    if (mInFreeLook) {
      mRequestReturnToDefault = true;
      mGrappleArm->EnterIdle(mgr);
    }
    CancelCharge(mgr, false);
  }
}

void CPlayerGun::InitBeamData() {
  mSelectableBeams[0] = mPowerBeam.get();
  mSelectableBeams[1] = mIceBeam.get();
  mSelectableBeams[2] = mWaveBeam.get();
  mSelectableBeams[3] = mPlasmaBeam.get();
  mCurrentBeam = mSelectableBeams[0];
  mNextBeam = mCurrentBeam;
  mHoloTransitionGen->SetParticleEmission(true);
}

void CPlayerGun::InitBombData() {
  for (int i = 0; i < 2; ++i)
    mBombEffects.push_back(rstl::reserved_vector< TLockedToken< CGenDescription >, 2 >());

  TToken< CGenDescription > obj1 =
      gpSimplePool->GetObj(SObjectTag('PART', gpTweakGunRes->mBombSet));
  TToken< CGenDescription > obj2 =
      gpSimplePool->GetObj(SObjectTag('PART', gpTweakGunRes->mBombExplode));
  TToken< CGenDescription > obj3 =
      gpSimplePool->GetObj(SObjectTag('PART', gpTweakGunRes->mPowerBombExplode));

  mBombEffects[0].push_back(obj1);
  mBombEffects[0].push_back(obj2);
  mBombEffects[1].push_back(obj3);
  mBombEffects[1].push_back(obj3);
}

void CPlayerGun::InitMuzzleData() {
  for (int i = 0; i < 5; ++i) {
    mAuxMuzzleEffects.push_back(gpSimplePool->GetObj(SObjectTag(
        'PART', gpTweakGunRes->GetAuxMuzzleResId(static_cast< CPlayerState::EBeamId >(i)))));
    CElementGen* gen = rs_new CElementGen(mAuxMuzzleEffects[i]);
    gen->SetParticleEmission(false);
    mAuxMuzzleGenerators.push_back(gen);
  }
}

void CPlayerGun::InitCTData() { mComboXferGen = rstl::auto_ptr< CElementGen >(); }

float CPlayerGun::GetBeamVelocity() const {
  if (mCurrentBeam->IsLoaded())
    return mCurrentBeam->GetVelocityInfo().GetVelocity(int(mChargeState)).GetY();
  return 10.f;
}

TUniqueId CPlayerGun::GetTargetId(CStateManager& mgr) {
  TUniqueId ret = mgr.GetPlayer()->GetOrbitTargetId();
  if (mComboFiring && ret == kInvalidUniqueId && mEquippedBeamId == CPlayerState::kBI_Wave)
    ret = mgr.GetPlayer()->GetOrbitNextTargetId();

  if (ret != kInvalidUniqueId) {
    CActor* act = TCastToPtr< CActor >(const_cast< CEntity* >(mgr.GetObjectById(ret)));
    if (act != nullptr) {
      const uchar hasTarget = act->GetMaterialList().HasMaterial(kMT_Target) ? 1 : 0;
      if (hasTarget != 1) {
        ret = kInvalidUniqueId;
      }
    }
  }

  return ret;
}

CPlayerGun::CGunMorph::CGunMorph(float gunTransformTime, float holoHoldTime)
: mYLerp(0.f)
, mGunTransformTime(CMath::FastFSel(-gunTransformTime, 1.f, gunTransformTime))
, mRemTime(0.f)
, mSpeed(0.1f)
, mHoloHoldTime(fabs(holoHoldTime))
, mRemHoldTime(2.f)
, mTransitionFactor(1.f)
, mDir(kD_Done)
, mGunState(kGS_OutWipeDone)
, mMorphing(false)
, mWeaponChanged(false) {}

void CPlayerGun::CGunMorph::StartWipe(CPlayerGun::CGunMorph::EDir dir) {
  mRemHoldTime = mHoloHoldTime;
  if (dir == kD_In && mGunState == kGS_InWipeDone)
    return;

  if (mDir != dir && mGunState != kGS_OutWipe) {
    mRemTime = mGunTransformTime;
    mSpeed = 1.f / mGunTransformTime;
  } else if (mGunState != kGS_InWipe) {
    mRemTime = mGunTransformTime - mRemTime;
  }

  mDir = dir;
  mGunState = mDir == kD_In ? kGS_InWipe : kGS_OutWipe;
  mMorphing = true;
}

CPlayerGun::CGunMorph::EMorphEvent CPlayerGun::CGunMorph::Update(float inY, float outY, float dt) {
  EMorphEvent ret = kME_None;

  switch (mGunState) {
  case kGS_InWipeDone:
    mRemHoldTime -= dt;
    if (mRemHoldTime <= 0.f && mWeaponChanged) {
      StartWipe(kD_Out);
      mWeaponChanged = false;
      mRemHoldTime = 0.f;
      ret = kME_InWipeDone;
    }
    // explicitly no break

  case kGS_OutWipeDone:
  case kGS_InWipe:
  case kGS_OutWipe:
  default:
    if (mMorphing) {
      float omt = mRemTime * mSpeed;
      float t = 1.f - omt;
      if (mDir == kD_In) {
        mYLerp = inY * t + outY * omt;
        mTransitionFactor = omt;
      } else {
        mYLerp = outY * t + inY * omt;
        mTransitionFactor = t;
      }

      if (mRemTime <= 0.f) {
        mMorphing = false;
        mRemTime = 0.f;
        if (mDir == kD_In) {
          mGunState = kGS_InWipeDone;
          mTransitionFactor = 0.f;
        } else {
          mTransitionFactor = 1.f;
          mGunState = kGS_OutWipeDone;
          mDir = kD_Done;
          ret = kME_OutWipeDone;
        }
      } else {
        mRemTime -= dt;
      }
    }
  }

  return ret;
}

void CPlayerGun::UpdateWeaponFire(float dt, CPlayerState& playerState, CStateManager& mgr) {
  uint oldFiring = mLastFireButtonStates;
  uint fireButtonStates = mFireButtonStates;
  mLastFireButtonStates = fireButtonStates;
  uint releasedStates = oldFiring & (oldFiring ^ fireButtonStates);
  uint pressedStates = fireButtonStates & (oldFiring ^ fireButtonStates);
  mPressedFireButtonStates = pressedStates;
  const CPlayer::EPlayerMorphBallState morphState = mgr.GetPlayer()->GetMorphballTransitionState();
  bool chargeRequested = mChargePhase != kCP_NotCharging;

  mReadyForShot = false;
  CPlayer& player = *mgr.Player();

  if (!mCoolingCharge && !mInBigStrike) {
    float coolDown = mCurrentBeam->GetWeaponInfo().mCoolDown;
    if ((pressedStates & 0x1) == 0) {
      if (mCooldown >= coolDown) {
        mCooldown = coolDown;
        if (morphState == CPlayer::kMS_Unmorphed &&
            playerState.ItemEnabled(CPlayerState::kIT_ChargeBeam) &&
            player.GetGunHolsterState() == CPlayer::kGH_Drawn &&
            player.GetGrappleState() == CPlayer::kGS_None &&
            mgr.GetPlayerState()->GetTransitioningVisor() != CPlayerState::kPV_Scan &&
            mgr.GetPlayerState()->GetCurrentVisor() != CPlayerState::kPV_Scan &&
            (mLastFireButtonStates & 0x1) != 0 && !chargeRequested) {
          mReadyForShot = true;
          pressedStates |= 0x1;
          mCooldown = 0.f;
        }
      }
    } else if (mCooldown >= coolDown) {
      mReadyForShot = true;
      mCooldown = 0.f;
    }
    mCooldown += dt;
  }

  if (mRequestImmediateRecharge)
    mRequestImmediateRecharge = (mLastFireButtonStates & 0x1) != 0;

  if (morphState == CPlayer::kMS_Morphed) {
    mBombReady = false;
    mPowerBombReady = false;
    if (!mActorAttached) {
      mBombReady = true;
      if (mPowerBomb != kInvalidUniqueId &&
          !mgr.CanCreateProjectile(mPlayerId, kWT_PowerBomb, 1)) {
        const CPowerBomb* pb = static_cast< const CPowerBomb* >(mgr.GetObjectById(mPowerBomb));
        if (pb != NULL && !(pb->GetCurTime() > CPowerBomb::EndingTime())) {
          mBombReady = false;
        } else {
          mPowerBomb = kInvalidUniqueId;
        }
      }
      if (((pressedStates & 0x1) != 0 || chargeRequested) &&
          playerState.HasPowerUp(CPlayerState::kIT_MorphBallBombs)) {
        if (mBombReady)
          DropBomb(kBW_Bomb, mgr);
      } else {
        mPowerBombReady = playerState.HasPowerUp(CPlayerState::kIT_PowerBombs) &&
                                 playerState.GetItemAmount(CPlayerState::kIT_PowerBombs) > 0 &&
                                 mgr.CanCreateProjectile(mPlayerId, kWT_PowerBomb, 1) &&
                                 mgr.CanCreateProjectile(mPlayerId, kWT_Bomb, 1);
        if ((pressedStates & 0x2) != 0 && mPowerBombReady)
          DropBomb(kBW_PowerBomb, mgr);
      }
    }
  } else {
    if (IsWeaponStateSet(0x8) || morphState != CPlayer::kMS_Unmorphed) {
      return;
    }
    if ((pressedStates & 0x2) != 0 && static_cast< int >(mComboAmmoIdx) == 0 &&
        !IsWeaponStateSet(0x2) && mChargePhase == kCP_NotCharging) {
      int missileCount = playerState.GetItemAmount(CPlayerState::kIT_Missiles);
      if (mNextState != kNS_EnterMissile && mNextState != kNS_ExitMissile) {
        if (playerState.HasPowerUp(CPlayerState::kIT_Missiles) && missileCount > 0) {
          mRemainingMissiles = missileCount;
          if (static_cast< int >(mRemainingMissiles) > 5)
            mRemainingMissiles = 5;
          if (!mInPhazonBeam) {
            ResetToMissile();
          }
          FireSecondary(dt, mgr);
        } else {
          if (!CSfxManager::IsPlaying(mInvalidSfx)) {
            mInvalidSfx =
                NWeaponTypes::play_sfx(SFXsam_b_malfxn_00, mUnderwater, false, 0x4a);
          } else {
            mInvalidSfx.Clear();
          }
        }
      }
    } else {
      if (mFidget.GetState() == CFidget::kS_NoFidget) {
        if (IsWeaponStateSet(0x10) && mAuxWeapon->IsComboFxActive(mgr)) {
          bool stopCombo = false;
          if (mEquippedBeamId == CPlayerState::kBI_Wave && mPointBlankWorldSurface) {
            stopCombo = true;
          }
          if (GetFiring() == 0 || stopCombo) {
            StopContinuousBeam(mgr, IsWeaponStateSet(0x8));
          }
        } else {
          if (playerState.ItemEnabled(CPlayerState::kIT_ChargeBeam) &&
              mPhazonBeamState == kPBS_Inactive)
            ProcessChargeState(releasedStates, pressedStates, mgr, dt);
          else
            ProcessNormalState(releasedStates, pressedStates, mgr, dt);
        }
      }
    }
  }
}

void CPlayerGun::ResetIdle(CStateManager& mgr) {
  CFidget::EState fidgetState = mFidget.GetState();
  mGunMotionSpeedMult = 1.f;
  mCamBob.SetState(CPlayerCameraBob::kCBS_GunFireNoBob, mgr);
  if (fidgetState != CFidget::kS_NoFidget) {
    if (fidgetState == CFidget::kS_Loading) {
      UnLoadFidget();
    }
    ReturnArmAndGunToDefault(mgr, true);
  }
  mFidget.ResetAll();
  ReturnToRestPose();
  if (mIdleState != kIS_NotIdle)
    mIdleState = kIS_NotIdle;
  if (!mGrappleArm->GetActive())
    mAnimPlaying = false;
}

void CPlayerGun::UpdateGunIdle(bool inStrikeCooldown, float camBobT, float dt, CStateManager& mgr) {
  CPlayer& player = *mgr.Player();

  if (player.IsInFreeLook() && !mLockedOn && !mGrappleArm->IsGrappling() &&
      mFidget.GetState() != CFidget::kS_HolsterBeam &&
      player.GetGunHolsterState() == CPlayer::kGH_Drawn && !mInBigStrike) {
    if (!IsWeaponStateSet(0x8)) {
      if (mInFreeLook != true && !mAnimPlaying) {
        if (mEnterFreeLookDelayTimer < 0.25f)
          mEnterFreeLookDelayTimer += dt;
        if (mEnterFreeLookDelayTimer >= 0.25f && !mGrappleArm->IsSuitLoading()) {
          EnterFreeLook(mgr);
          mInFreeLook = true;
        }
      } else {
        mEnterFreeLookDelayTimer = 0.f;
        if (mAnimPlaying)
          ResetIdle(mgr);
      }
    }
  } else {
    if (mInFreeLook) {
      if (!IsWeaponStateSet(0x10)) {
        mGunMotion->ReturnToDefault(mgr, mInBigStrike);
        mGrappleArm->ReturnToDefault(mgr, 0.f, false);
      }
      mInFreeLook = false;
    }
    mEnterFreeLookDelayTimer = 0.f;
    const CPlayerState& playerState = *mgr.GetPlayerState();
    const bool moving = camBobT > 0.01f;
    const bool firing = (mFireButtonStates & 0x3) ? true : false;
    mNotFidgeting = player.GetMorphballTransitionState() == CPlayer::kMS_Morphed ||
                           player.GetSurfaceRestraint() == CPlayer::kSR_Water ||
                           playerState.GetCurrentVisor() == CPlayerState::kPV_Scan || firing ||
                           mChargePhase != kCP_NotCharging || mLockedOn ||
                           IsWeaponStateSet(0x8) || mGunStrikeCoolTimer > 0.f ||
                           player.GetPlayerMovementState() != NPlayer::kMS_OnGround ||
                           player.IsInFreeLook() || player.GetFreeLookStickState() ||
                           player.GetOrbitState() != CPlayer::kOS_NoOrbit ||
                           CMath::AbsF(player.GetAngularVelocityOR().GetAngle()) > 0.1f || moving ||
                           mgr.GetCameraManager()->IsInCinematicCamera() ||
                           player.GetGunHolsterState() != CPlayer::kGH_Drawn ||
                           player.GetGrappleState() != CPlayer::kGS_None || mInBigStrike ||
                           mInPhazonBeam;
    if (mNotFidgeting) {
      if (!mInBigStrike) {
        bool doWander = moving && !firing;
        if (doWander) {
          mGunMotionSpeedMult = 1.f;
          x374_ = 0.f;
          if (mGunStrikeCoolTimer <= 0.f && mIdleWanderDelayTimer <= 0.f) {
            mIdleWanderDelayTimer = 8.f;
            mGunMotion->PlayPasAnim(SamusGun::kAS_Wander, mgr, 0.f, false);
            mIdleState = kIS_Wander;
            mCamBob.SetState(CPlayerCameraBob::kCBS_Walk, mgr);
          }
          mIdleWanderDelayTimer -= dt;
          x360_ += dt;
        }
        if (!doWander || mAnimPlaying)
          ResetIdle(mgr);
      } else if (mDamageTimer > 0.f) {
        mDamageTimer -= dt;
      } else if (mGunMotionInFidgetBasePosition != true) {
        mDamageTimer = 0.f;
        mGunMotionInFidgetBasePosition = true;
        mGunMotion->BasePosition(true);
      } else if (!mGunMotion->GetModelData().GetAnimationData()->IsAnimTimeRemaining(
                     0.001f, rstl::string_l("Whole Body"))) {
        mInBigStrike = false;
        mGunMotionInFidgetBasePosition = false;
      }
    } else {
      switch (mFidget.Update(mLastFireButtonStates, camBobT > 0.01f, inStrikeCooldown, dt,
                                 mgr)) {
      case CFidget::kS_NoFidget:
        if (mIdleState != kIS_Idle) {
          mGunMotion->PlayPasAnim(SamusGun::kAS_Idle, mgr, 0.f, false);
          mIdleState = kIS_Idle;
        }
        mCamBob.SetState(CPlayerCameraBob::kCBS_WalkNoBob, mgr);
        break;
      case CFidget::kS_MinorFidget:
      case CFidget::kS_MajorFidget:
      case CFidget::kS_HolsterBeam:
        if (mIdleState != kIS_NotIdle) {
          mGunMotion->BasePosition(false);
          mIdleState = kIS_NotIdle;
        }
        AsyncLoadFidget(mgr);
        break;
      case CFidget::kS_Loading:
        if (IsFidgetLoaded())
          EnterFidget(mgr);
        break;
      case CFidget::kS_StillMinorFidget:
      case CFidget::kS_StillMajorFidget: {
        mCamBob.SetState(CPlayerCameraBob::kCBS_Walk, mgr);
        mNotFidgeting = false;
        mAnimPlaying =
            mGunMotionFidgeting
                ? mGunMotion->IsAnimPlaying()
                : mCurrentBeam->GetSolidModelData().GetAnimationData()->IsAnimTimeRemaining(
                      0.001f, rstl::string_l("Whole Body"));
        if (!mAnimPlaying) {
          mFidget.ResetMinor();
          ReturnToRestPose();
        }
        break;
      }
      default:
        break;
      }
    }
    mCamBob.Update(dt, mgr);
  }
}

void CPlayerGun::CMotionState::Update(bool firing, float dt, CTransform4f& xf, CStateManager& mgr) {
  if (firing) {
    mFireState = kFS_StartFire;
    mFireTime = 0.f;
  } else if (mFireState != kFS_NotFiring) {
    if (mFireTime > dt)
      mFireState = kFS_Firing;
    mFireTime += dt;
  }

  if (mExtendParabola && mState == kMS_LockOn) {
    float extendT = mCurExtendDist * (1.0f / gGunExtendDistance);
    CTransform4f other =
        CTransform4f::RotateZ(CRelAngle::FromDegrees(extendT * -4.f * (extendT - 1.f) * 15.f));
    other.SetTranslation(CVector3f(0.f, mCurExtendDist, 0.f));
    xf = xf * other;
  } else if (mFireState == kFS_StartFire || mFireState == kFS_Firing) {
    if (fabs(mRotationT - 1.f) < 0.1f) {
      mStartRotation = mEndRotation;
      mRotationT = 0.f;
      if (mFireState == kFS_StartFire) {
        mEndRotation = CCast::StoF(mgr.Random()->Next() % 15);
        mEndRotation *= (mgr.Random()->Next() % 100) > 45 ? 1.f : -1.f;
      } else {
        mEndRotation = 0.f;
        if (mStartRotation == mEndRotation) {
          mCurRotation = mEndRotation;
          mFireState = kFS_NotFiring;
        }
      }
    } else {
      mCurRotation = (mEndRotation - mStartRotation) * mRotationT + mStartRotation;
    }

    mRotationT += (10.f * dt) * (0.8f * (1.f - mRotationT));

    const CRelAngle angle = CRelAngle::FromDegrees(mCurRotation);
    CQuaternion quat = CQuaternion::AxisAngle(CUnitVector3f(xf.GetForward()), angle);

    CTransform4f tmpXf = quat.BuildTransform4f() * xf.GetRotation();
    tmpXf.SetTranslation(xf.GetTranslation());
    xf = tmpXf * CTransform4f::Translate(0.f, mCurExtendDist, 0.f);
  } else {
    xf *= CTransform4f::Translate(0.f, mCurExtendDist, 0.f);
  }

  switch (mState) {
  case kMS_LockOn:
    mCurExtendDist += 3.f * dt;
    if (mCurExtendDist > gGunExtendDistance) {
      mCurExtendDist = gGunExtendDistance;
      mState = kMS_One;
      mExtendParabola = false;
    }
    break;
  case kMS_CancelLockOn:
    mCurExtendDist -= 3.f * dt;
    if (mCurExtendDist < 0.f) {
      mCurExtendDist = 0.f;
      mState = kMS_Zero;
    }
    break;
  default:
    break;
  }

  if (mExtendParabola != true) {
    if (mExtendParabolaDelayTimer < 30.f) {
      mExtendParabolaDelayTimer += dt;
    } else {
      mExtendParabola = true;
      mExtendParabolaDelayTimer = 0.f;
    }
  }
}

void CPlayerGun::DamageRumble(const CVector3f& location, float damage, const CStateManager&) {
  mDamageAmt = damage;
  mDamageLocation = location;
}

void CPlayerGun::TakeDamage(bool bigStrike, bool notFromMetroid, CStateManager& mgr) {
  const CPlayer& player = *mgr.GetPlayer();
  bool hasStrikeAngle = false;
  float angle = 0.f;
  if (mDamageAmt >= 10.f && !bigStrike && !IsWeaponStateSet(0x10) && !mComboFiring &&
      mGunStrikeDelayTimer <= 0.f) {
    mGunStrikeDelayTimer = 20.f;
    mGunStrikeCoolTimer = 0.75f;
    if (mMorph.GetGunState() == CGunMorph::kGS_OutWipeDone) {
      CVector3f localDamageLoc = player.GetTransform().TransposeRotate(mDamageLocation);
      angle =
          CAbsAngle::FromRadians(atan2(localDamageLoc.GetY(), localDamageLoc.GetX())).AsDegrees();
      hasStrikeAngle = true;
    }
  }

  if (hasStrikeAngle || bigStrike) {
    if (mgr.GetPlayerState()->GetCurrentVisor() != CPlayerState::kPV_Scan) {
      mGunMotion->PlayPasAnim(SamusGun::kAS_Struck, mgr, angle, bigStrike);
      if ((bigStrike && notFromMetroid) || mInFreeLook)
        mGrappleArm->EnterStruck(mgr, angle, bigStrike, !mInFreeLook);
    }
  }

  mDamageAmt = 0.f;
  mDamageLocation = CVector3f::Zero();
}

void CPlayerGun::StopChargeSound(CStateManager& mgr) {
  if (mChargeSfx) {
    CSfxManager::SfxStop(mChargeSfx);
    mChargeSfx.Clear();
  }
  if (mChargeRumbleHandle != -1) {
    mgr.GetRumbleManager()->StopRumble(mChargeRumbleHandle);
    mChargeRumbleHandle = -1;
  }
}

void CPlayerGun::CancelFiring(CStateManager& mgr) {
  if (mChargePhase == kCP_ComboFireDone)
    ReturnArmAndGunToDefault(mgr, true);

  if (IsWeaponStateSet(0x10)) {
    StopContinuousBeam(mgr, true);
    ResetToBeam();
  }

  if (mChargePhase != kCP_NotCharging) {
    mCurrentBeam->ActivateCharge(false, false);
    SetGunLightActive(false, mgr);
    ResetCharge(mgr, true);
  }

  Reset(mgr, IsWeaponStateSet(0x2));
}

void CPlayerGun::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId sender, CStateManager& mgr) {
  const CPlayer& player = *mgr.GetPlayer();
  const CPlayerState& playerState = *mgr.GetPlayerState();
  const bool isUnmorphed = player.GetMorphballTransitionState() == CPlayer::kMS_Unmorphed;

  switch (msg) {
  case kSM_Registered: {
    CreateGunLight(mgr);
    const CPlayerState::EBeamId currentBeam = playerState.GetCurrentBeam();
#if VERSION >= VERSION_GM8P_00
    LoadBeam(currentBeam, mgr);
#else
    const CPlayerState::EBeamId beam = mCurrentBeamId[currentBeam];
    mCurrentAuxBeam = beam;
    mNextBeamId = beam;
    mEquippedBeamId = beam;
    mCurrentBeam = mSelectableBeams[mEquippedBeamId];
    mNextBeam = mCurrentBeam;
    mCurrentBeam->Load(mgr, true);
    mCurrentBeam->mRainSplashGenerator = mRainSplashGenerator.get();
    mAuxWeapon->Load(mEquippedBeamId, mgr);
#endif
    mRightHandModel.AnimationData()->SetAnimation(
        CAnimPlaybackParms(mHandAnimId[currentBeam], -1, 1.f, true), false);
    break;
  }
  case kSM_Deleted:
    DeleteGunLight(mgr);
    break;
  case kSM_UpdateSplashInhabitant:
    if (playerState.HasPowerUp(CPlayerState::kIT_PhazonSuit) && isUnmorphed) {
      if (const CScriptWater* water = TCastToConstPtr< CScriptWater >(mgr.GetObjectById(sender))) {
        if (water->GetFluidPlane().GetFluidType() == CFluidPlane::kFT_PhazonFluid) {
          mCanFirePhazon = true;
          mInPhazonBeam = true;
        }
      }
    }

    if (player.GetDistanceUnderWater() > player.GetEyeHeight()) {
      mUnderwater = true;
      if (mAuxWeapon->IsComboFxActive(mgr) && mEquippedBeamId != CPlayerState::kBI_Wave) {
        StopContinuousBeam(mgr, false);
      }
    } else {
      mUnderwater = false;
    }
    break;
  case kSM_RemoveSplashInhabitant:
    mUnderwater = false;
    mCanFirePhazon = false;
    break;
  case kSM_AddPhazonPoolInhabitant:
    mInPhazonPool = true;
    if (playerState.HasPowerUp(CPlayerState::kIT_PhazonSuit) && isUnmorphed) {
      mCanFirePhazon = true;
    }
    break;
  case kSM_UpdatePhazonPoolInhabitant:
    mInPhazonPool = true;
    if (playerState.HasPowerUp(CPlayerState::kIT_PhazonSuit) && isUnmorphed) {
      mCanFirePhazon = true;
      mInPhazonBeam = true;
      CPhazonBeam* phazon;
      if (mPhazonBeamActive) {
        phazon = static_cast< CPhazonBeam* >(mCurrentBeam);
      } else {
        break;
      }
      if (phazon->IsFiring(mgr)) {
        if (CEntity* ent = TCastToPtr< CEntity >(mgr.ObjectById(sender))) {
          mgr.DeliverScriptMsg(ent, mPlayerId, kSM_Decrement);
        }
      }
    }
    break;
  case kSM_RemovePhazonPoolInhabitant:
    mInPhazonPool = false;
    mCanFirePhazon = false;
    break;
  case kSM_Damage: {
    bool bigStrike = false;
    bool metroidAttached = false;

    const CEnergyProjectile* proj = TCastToConstPtr< CEnergyProjectile >(mgr.GetObjectById(sender));
    if (proj && (proj->GetAttribField() & CWeapon::kPA_BigStrike) == CWeapon::kPA_BigStrike) {
      mDamageTimer = proj->GetDamageDuration();
      bigStrike = true;
    } else if (const CPatterned* ai = TCastToConstPtr< CPatterned >(mgr.GetObjectById(sender))) {
      if (ai->IsMakingBigStrike()) {
        mDamageTimer = ai->GetDamageDuration();
        bigStrike = true;
        const TUniqueId attachedActor = player.GetAttachedActor();
        if (attachedActor != kInvalidUniqueId) {
          metroidAttached = PATTERNED_CAST_TO(CMetroid, const_cast< CEntity* >(mgr.GetObjectById(attachedActor))) != nullptr;
        }
      }
    }

    if (!mInBigStrike) {
      if (bigStrike) {
        mGunMotionInFidgetBasePosition = false;
        CancelFiring(mgr);
      }
      TakeDamage(static_cast< const bool& >(bigStrike), !metroidAttached, mgr);
      mInBigStrike = bigStrike;
    }
    break;
  }
  case kSM_OnFloor:
    if (player.GetControlsFrozen() && !mInBigStrike) {
      mFireButtonStates = 0;
      mLastFireButtonStates = 0;
      CancelFiring(mgr);
      TakeDamage(true, false, mgr);
      mDamageTimer = 0.75f;
      mInBigStrike = true;
    }
    break;
  default:
    break;
  }

  mGrappleArm->AcceptScriptMsg(msg, sender, mgr);
  CPlasmaBeam* plasmaBeam = mPlasmaBeam.get();
  plasmaBeam->AcceptScriptMsg(msg, sender, mgr);
  CPhazonBeam* phazonBeam = mPhazonBeam.get();
  phazonBeam->AcceptScriptMsg(msg, sender, mgr);
  mAuxWeapon->AcceptScriptMsg(msg, sender, mgr);
}

void CPlayerGun::StopContinuousBeam(CStateManager& mgr, bool stopSfx) {
  if (IsWeaponStateSet(0x10)) {
    ReturnArmAndGunToDefault(mgr, false);
    mAuxWeapon->StopComboFx(mgr, stopSfx);

    bool doFx = true;
    if (mEquippedBeamId != CPlayerState::kBI_Ice) {
      if (mEquippedBeamId < CPlayerState::kBI_Ice) {
        if (mEquippedBeamId < CPlayerState::kBI_Power) {
          return;
        }
      } else if (mEquippedBeamId >= CPlayerState::kBI_Phazon) {
        return;
      }

      if (mEquippedBeamId == CPlayerState::kBI_Power && !mPhazonBeamActive) {
        doFx = false;
      }
      if (doFx) {
        mCurrentBeam->EnableSecondaryFx(stopSfx ? CGunWeapon::kSFT_None
                                                    : CGunWeapon::kSFT_CancelCharge);
      }
    }
  } else if (mPhazonBeamActive) {
    CPhazonBeam* beam = static_cast< CPhazonBeam* >(mCurrentBeam);
    if (beam->IsFiring(mgr)) {
      beam->StopBeam(mgr, stopSfx);
    }
  } else if (mEquippedBeamId == CPlayerState::kBI_Plasma) {
    CPlasmaBeam* beam = static_cast< CPlasmaBeam* >(mCurrentBeam);
    if (beam->IsFiring(mgr)) {
      beam->StopBeam(mgr, static_cast< const bool >(stopSfx));
    }
  }
}

void CPlayerGun::RenderEnergyDrainEffects(const CStateManager& mgr) const {
  const CEnergyDrainSource* it;
  const CPlayer* const player = TCastToConstPtr< CPlayer >(mgr.GetObjectById(mPlayerId));
  if (player != nullptr) {
    it = player->GetPlayerEnergyDrain().GetEnergyDrainSources().data();
    while (it != player->GetPlayerEnergyDrain().GetEnergyDrainSources().data() +
                     player->GetPlayerEnergyDrain().GetEnergyDrainSources().size()) {
      CMetroidBeta* metroid = PATTERNED_CAST_TO(CMetroidBeta, const_cast< CEntity* >(mgr.GetObjectById(it->GetEnergyDrainSourceId())));
      if (metroid != nullptr) {
        metroid->RenderHitGunEffect();
        return;
      }
      ++it;
    }
  }
}

void CPlayerGun::DoUserAnimEvents(float dt, CStateManager& mgr) {
  const int aid = mgr.GetPlayer()->GetCurrentAreaId().Value();
  const CAnimData& animData = *mCurrentBeam->GetSolidModelData().GetAnimationData();
  const CGameCamera& camera = mgr.GetCameraManager()->GetCurrentCamera(mgr);
  const CVector3f origin = mXf.GetTranslation();
  const CVector3f posToCam = camera.GetTranslation() - origin;

  int soundNodeCount = 0;
  const CSoundPOINode* soundNodes = animData.GetSoundPOIList(soundNodeCount);
  if (soundNodeCount > 0) {
    for (int i = 0; i < soundNodeCount; ++i) {
      const CSoundPOINode* soundNode = &soundNodes[i];
      const int charIdx = soundNode->GetCharacterIndex();
      if (soundNode->GetPoiType() != kPT_Sound)
        continue;
      if (charIdx != -1 && charIdx != animData.GetCharacterIndex())
        continue;
      NWeaponTypes::do_sound_event(mAnimSfx, mAnimSfxPitch, false, soundNode->GetSoundId(),
                                   soundNode->GetWeight(), soundNode->GetFlags(),
                                   soundNode->GetFallOff(), soundNode->GetMaxDistance(), 0x14,
                                   CAudioSys::kMaxVolume, posToCam, origin, aid, mgr);
    }
  }

  int intNodeCount = 0;
  const CInt32POINode* intNodes = animData.GetInt32POIList(intNodeCount);
  if (intNodeCount > 0) {
    for (int i = 0; i < intNodeCount; ++i) {
      const CInt32POINode* intNode = &intNodes[i];
      switch (intNode->GetPoiType()) {
      case kPT_UserEvent:
        DoUserAnimEvent(dt, mgr, *intNode, static_cast< EUserEventType >(intNode->GetValue()));
        break;
      case kPT_SoundInt32: {
        const int charIdx = intNode->GetCharacterIndex();
        if (charIdx != -1 && charIdx != animData.GetCharacterIndex())
          break;
        NWeaponTypes::do_sound_event(mAnimSfx, mAnimSfxPitch, false, intNode->GetValue(),
                                     intNode->GetWeight(), intNode->GetFlags(), 0.1f, 150.f, 0x14,
                                     CAudioSys::kMaxVolume, posToCam, origin, aid, mgr);
        break;
      }
      default:
        break;
      }
    }
  }
}

void CPlayerGun::DoUserAnimEvent(float dt, CStateManager& mgr, const CInt32POINode&,
                                 EUserEventType type) {
  switch (type) {
  case kUE_Projectile: {
    if (mChargePhase != kCP_ComboFireDone) {
      return;
    }

    uchar fireSecondary = 0;
    if (mEquippedBeamId != CPlayerState::kBI_Wave &&
        mEquippedBeamId != CPlayerState::kBI_Plasma) {
      fireSecondary = 1;
    }

    bool doFireSecondary = fireSecondary ? true : (mLastFireButtonStates & 0x1);
    if (doFireSecondary) {
      FireSecondary(dt, mgr);
    }
    if (!IsWeaponStateSet(0x10)) {
      EnableWeaponState(0x10);
    }
    CancelCharge(mgr, true);
    if (doFireSecondary) {
      mCurrentBeam->EnableSecondaryFx(CGunWeapon::kSFT_ToCombo);
    }
    break;
  }
  case kUE_Delete:
  case kUE_DamageOn:
  default:
    break;
  }
}

void CPlayerGun::CancelCharge(CStateManager& mgr, bool withEffect) {
  if (withEffect) {
    mChargePhase = kCP_ChargeCooldown;
    mCurrentBeam->EnableSecondaryFx(CGunWeapon::kSFT_CancelCharge);
  } else {
    mCurrentBeam->EnableSecondaryFx(CGunWeapon::kSFT_None);
  }

  mCharging = false;
  mChargeCooldownTimer = 0.f;
  mCurrentBeam->ActivateCharge(false, false);
  SetGunLightActive(false, mgr);
}

void CPlayerGun::EnterFreeLook(CStateManager& mgr) {
  if (!mRequestReturnToDefault)
    mGunMotion->PlayPasAnim(SamusGun::kAS_FreeLook, mgr, 0.f, false);

  const int setId = mGunMotion->GetFreeLookSetId();
  mGrappleArm->EnterFreeLook(mInPhazonBeam ? 1 : mEquippedBeamId, setId, mgr);
}

void CPlayerGun::EnterFidget(CStateManager& mgr) {
  SamusGun::EFidgetType type = mFidget.GetType();
  int animSet = mFidget.GetAnimSet();

  if ((mFidgetAnimBits & 0x1) == 0x1) {
    mGunMotion->EnterFidget(mgr, type, animSet);
    mGunMotionFidgeting = true;
  } else {
    mGunMotionFidgeting = false;
  }

  if ((mFidgetAnimBits & 0x2) == 0x2) {
    mCurrentBeam->EnterFidget(mgr, type, animSet);
  }

  if ((mFidgetAnimBits & 0x4) == 0x4) {
    mGrappleArm->EnterFidget(mgr, type, type != SamusGun::kFT_Minor ? mEquippedBeamId : 0,
                                 animSet);
  }

  UnLoadFidget();
  mFidget.DoneLoading();
}

void CPlayerGun::UpdateLeftArmTransform(const CModelData& modelData, const CStateManager&) {
  CVector3f elbowOffset(-0.9f, -0.4f, 0.4f);
  CTransform4f& auxXf = mGrappleArm->AuxTransform();

  if (mAnimPlaying) {
    auxXf = CTransform4f::Identity();
  } else {
    GetLctrWithShake(auxXf, modelData, rstl::string_l("elbow"), true, false);
  }

  const CVector3f elbowPos = auxXf * elbowOffset;
  auxXf.SetTranslation(elbowPos);
  mGrappleArm->SetTransform(mXf);
}

void CPlayerGun::ReturnArmAndGunToDefault(CStateManager& mgr, bool returnToDefault) {
  if (returnToDefault || !mInFreeLook) {
    mGunMotion->ReturnToDefault(mgr, false);
    mGrappleArm->ReturnToDefault(mgr, 0.f, false);
  }
  if (!mGunMotionFidgeting)
    mCurrentBeam->ReturnToDefault(mgr);
  mGunMotionFidgeting = false;
}

void CPlayerGun::UpdateAuxWeapons(float dt, const CTransform4f& targetXf, CStateManager& mgr) {
  const CVector3f firePoint = GetGunMotionTransform() * mBeamLocalXf.GetTranslation();
  const CVector3f camPos = mgr.GetCameraManager()->GetGlobalCameraTranslation(mgr);
  bool done = mAuxWeapon->UpdateComboFx(dt, kScaleVector, firePoint + camPos, targetXf, mgr);
  if (IsWeaponStateSet(0x10)) {
    if (mEquippedBeamId == CPlayerState::kBI_Wave &&
        mAuxWeapon->HasTarget(mgr) == kInvalidUniqueId) {
      TUniqueId targetId = GetTargetId(mgr);
      if (targetId == kInvalidUniqueId) {
        targetId = mgr.GetPlayer()->GetAimTargetId();
      }
      mAuxWeapon->SetNewTarget(targetId, mgr);
    }
    if (done == true) {
      return;
    }

    bool comboDone =
        mEquippedBeamId == CPlayerState::kBI_Wave || mEquippedBeamId == CPlayerState::kBI_Plasma;
    if (comboDone != true) {
      if (mCurrentBeam->ComboFireOver()) {
        comboDone = true;
      }
    }

    mCurrentBeam->EnableSecondaryFx(CGunWeapon::kSFT_CancelCharge);
    if (comboDone) {
      mChargePhase = kCP_ChargeDone;
      ReturnArmAndGunToDefault(mgr, false);
      ResetToBeam();
    }
  } else {
    if (mPhazonBeamActive) {
      CPhazonBeam* phazonBeam = static_cast< CPhazonBeam* >(mCurrentBeam);
      phazonBeam->UpdateBeam(dt, targetXf, mBeamLocalXf.GetTranslation(), mgr);
    } else if (mEquippedBeamId == CPlayerState::kBI_Plasma) {
      CPlasmaBeam* plasmaBeam = static_cast< CPlasmaBeam* >(mCurrentBeam);
      plasmaBeam->UpdateBeam(dt, targetXf, mBeamLocalXf.GetTranslation(), mgr);
    }
  }
}

void CPlayerGun::CancelLockOn() {
  if (mLockedOn) {
    mLockedOn = false;
    mMotionState.SetState(CMotionState::kMS_CancelLockOn);
    if (mChargePhase == kCP_NotCharging && int(mComboAmmoIdx) != 1) {
      PlayAnim(NWeaponTypes::kGAT_BasePosition, false);
    }
  }
}

void CPlayerGun::CreateGunLight(CStateManager& mgr) {
  if (mLightId != kInvalidUniqueId) {
    return;
  }

  mLightId = mgr.AllocateUniqueId();
  const uint lightId = mLightId.Value();
  mgr.AddObject(rs_new CGameLight(
      mLightId, kInvalidAreaId, false, rstl::string_l("GunLite"), mXf, mPlayerId,
      CLight::BuildDirectional(CVector3f::Forward(), CColor::Black()), lightId, 0, 0.f));
}

void CPlayerGun::DeleteGunLight(CStateManager& mgr) {
  if (mLightId == kInvalidUniqueId) {
    return;
  }

  mgr.DeleteObjectRequest(mLightId);
  mLightId = kInvalidUniqueId;
}

void CPlayerGun::UpdateGunLight(const CTransform4f& xf, CStateManager& mgr) {
  if (mLightId == kInvalidUniqueId) {
    return;
  }
  if (mChargePhase == kCP_NotCharging) {
    return;
  }

  if (CGameLight* light = TCastToPtr< CGameLight >(mgr.ObjectById(mLightId))) {
    if (light->GetActive()) {
      CElementGen* chargeFx = mCurrentBeam->GetChargeMuzzleFx();
      light->SetTransform(xf);
      light->SetTranslation(xf.GetTranslation());
      if (chargeFx != NULL && chargeFx->SystemHasLight()) {
        CLight genLight = chargeFx->GetLight();
        genLight.SetColor(
            CColor::Lerp(0, genLight.GetColor().GetColor_u32(), mChargeBeamFactor));
        light->SetLight(genLight);
      }
    }
  }
}

void CPlayerGun::SetGunLightActive(bool active, CStateManager& mgr) {
  if (mLightId == kInvalidUniqueId) {
    return;
  }

  if (CGameLight* light = TCastToPtr< CGameLight >(mgr.ObjectById(mLightId))) {
    light->SetActive(active);
    if (active) {
      if (CElementGen* chargeFx = mCurrentBeam->GetChargeMuzzleFx()) {
        if (chargeFx->SystemHasLight()) {
          CLight genLight = chargeFx->GetLight();
          genLight.SetColor(CColor::Black());
          light->SetLight(genLight);
        }
      }
    }
  }
}

void CPlayerGun::LoadHandAnimTokens() {
  CAnimData& animData = *mRightHandModel.AnimationData();
  rstl::set< CPrimitive > prims;
  for (int i = 0; i < 3; ++i) {
    CAnimPlaybackParms parms(i, -1, 1.f, true);
    animData.GetAnimationPrimitives(parms, prims);
  }
  CAnimData::PrimitiveSetToTokenVector(prims, mHandAnimTokens, true);
}

void CPlayerGun::ProcessPhazonGunMorph(float dt, CStateManager& mgr) {
  if (mPhazonBeamMorphing) {
    if (mIntoPhazonBeam) {
      mPhazonMorphT += 15.f * dt;
      if (mPhazonMorphT > 1.f) {
        mPhazonMorphT = 1.f;
      }
    } else {
      mPhazonMorphT -= 2.f * dt;
      if (mPhazonMorphT < 0.f) {
        mPhazonBeamMorphing = false;
        mPhazonMorphT = 0.f;
      }
    }
  }

  switch (mPhazonBeamState) {
  case kPBS_Entering:
    if (mPhazonBeam.get() != NULL) {
      mPhazonBeam->Update(dt, mgr);
      if (mPhazonBeam->IsLoaded()) {
        StartPhazonBeamTransition(true, mgr, *mgr.PlayerState());
        SetPhazonBeamMorph(false);
        mPhazonBeamState = kPBS_Active;
        mNextState = kNS_SetupBeam;
      }
    }
    break;
  case kPBS_Exiting:
    if (mNextBeam != NULL) {
      mNextBeam->Update(dt, mgr);
      if (mNextBeam->IsLoaded()) {
        mInPhazonBeam = false;
        StartPhazonBeamTransition(false, mgr, *mgr.PlayerState());
        SetPhazonBeamMorph(false);
        mPhazonBeamState = kPBS_Inactive;
        mNextState = kNS_SetupBeam;
      }
    }
    break;
  default:
    break;
  }
}

void CPlayerGun::ProcessGunMorph(float dt, CStateManager& mgr) {
  const CGunMorph::EGunState gunState = mMorph.GetGunState();
  const bool isUnmorphed = mgr.GetPlayer()->GetMorphballTransitionState() == CPlayer::kMS_Unmorphed;
  CPlayerState* playerState = mgr.PlayerState();

  switch (gunState) {
  case CGunMorph::kGS_InWipeDone:
    if (mEquippedBeamId != mNextBeamId && mLoadingBeam != NULL) {
      if (!isUnmorphed) {
        mLoadingBeam->Touch(mgr);
      }
      if (mLoadingBeam->IsLoaded() && mAuxWeapon->IsLoaded()) {
        mOutgoingBeam = mLoadingBeam != mCurrentBeam ? mCurrentBeam : NULL;
        mLoadingBeam = NULL;
        mEquippedBeamId = mNextBeamId;
        mCurrentAuxBeam = mNextBeamId;
        mCanShowAuxMuzzleEffect = true;
        mCurrentBeam = mSelectableBeams[mNextBeamId];
        mNextBeam = mCurrentBeam;
        mMorph.SetWeaponChanged();
        playerState->SetCurrentBeam(mNextBeamId);
      }
    }
    break;
  case CGunMorph::kGS_InWipe:
  case CGunMorph::kGS_OutWipe:
    mHoloTransitionGen->SetGlobalScale(kScaleVector);
    mHoloTransitionGen->SetGlobalTranslation(CVector3f(0.f, mMorph.GetYLerp(), 0.f));
    mHoloTransitionGen->Update(dt);
    break;
  default:
    break;
  }

  switch (mMorph.Update(0.2f, 1.292392f, dt)) {
  case CGunMorph::kME_InWipeDone:
    CSfxManager::SfxStart(SFXwpn_morph_in_wipe_done, 0x7f, 0x40, true, CSfxManager::kMaxPriority,
                          false, CSfxManager::kAllAreas);
    break;
  case CGunMorph::kME_OutWipeDone:
    if (mOutgoingBeam != NULL) {
      if (mOutgoingBeam != mCurrentBeam) {
        mOutgoingBeam->Unload(mgr);
        mOutgoingBeam = NULL;
      }
    }
    if (isUnmorphed) {
      const ushort* const intoBeamSounds = mIntoBeamSound;
      const short pan = 0x4a;
      NWeaponTypes::play_sfx(intoBeamSounds[mEquippedBeamId], mUnderwater, false, pan);
    }
    mCurrentBeam->mRainSplashGenerator = mRainSplashGenerator.get();
    mCurrentBeam->EnableFx(true);
    PlayAnim(NWeaponTypes::kGAT_ToBeam, false);
    if (mInFreeLook) {
      EnterFreeLook(mgr);
    } else if (mRequestReturnToDefault) {
      ReturnArmAndGunToDefault(mgr, false);
    }
    if (mRequestImmediateRecharge || (mLastFireButtonStates & 0x1) != 0) {
      if (playerState->GetCurrentVisor() != CPlayerState::kPV_Scan) {
        mChargePhase = kCP_ChargeRequested;
      }
      mRequestImmediateRecharge = false;
    }
    mRequestReturnToDefault = false;
    mNextState = kNS_SetupBeam;
    break;
  default:
    break;
  }
}

void CPlayerGun::AsyncLoadFidget(CStateManager& mgr) {
  SamusGun::EFidgetType type = mFidget.GetType();
  int animSet = mFidget.GetAnimSet();
  SamusGun::EFidgetType beamType = type;
  bool beamOnly = mFidget.GetState() == CFidget::kS_HolsterBeam;

  SetFidgetAnimBits(animSet, beamOnly);

  if ((mFidgetAnimBits & 0x1) == 0x1) {
    mGunMotion->GunController().LoadFidgetAnimAsync(mgr, type, mEquippedBeamId, animSet);
  }

  if ((mFidgetAnimBits & 0x2) == 0x2) {
    mCurrentBeam->AsyncLoadFidget(mgr, beamOnly ? SamusGun::kFT_Minor : beamType, animSet);
    mInRestPose = false;
  }

  if ((mFidgetAnimBits & 0x4) == 0x4) {
    if (CGunController* gc = mGrappleArm->GunController()) {
      gc->LoadFidgetAnimAsync(mgr, type, type != SamusGun::kFT_Minor ? mEquippedBeamId : 0,
                              animSet);
    }
  }
}

void CPlayerGun::UnLoadFidget() {
  if ((mFidgetAnimBits & 0x1) == 0x1)
    mGunMotion->GunController().UnLoadFidget();
  if ((mFidgetAnimBits & 0x2) == 0x2)
    mCurrentBeam->UnLoadFidget();
  if ((mFidgetAnimBits & 0x4) == 0x4)
    if (CGunController* gc = mGrappleArm->GunController())
      gc->UnLoadFidget();
  mFidgetAnimBits = 0;
}

bool CPlayerGun::IsFidgetLoaded() {
  uint loadFlags = 0;
  if ((mFidgetAnimBits & 0x1) == 0x1 && mGunMotion->GunController().IsFidgetLoaded()) {
    loadFlags |= 0x1;
  }
  if ((mFidgetAnimBits & 0x2) == 0x2 && mCurrentBeam->IsFidgetLoaded()) {
    loadFlags |= 0x2;
  }
  if ((mFidgetAnimBits & 0x4) == 0x4) {
    if (CGunController* gc = mGrappleArm->GunController()) {
      if (gc->IsFidgetLoaded()) {
        loadFlags |= 0x4;
      }
    }
  }
  return loadFlags == mFidgetAnimBits;
}

void CPlayerGun::SetFidgetAnimBits(int animSet, bool beamOnly) {
  mFidgetAnimBits = 0;
  if (beamOnly) {
    mFidgetAnimBits = 2;
    return;
  }

  switch (mFidget.GetType()) {
  case SamusGun::kFT_Minor:
    mFidgetAnimBits = 1;
    if (animSet <= 0) {
      return;
    }
    if (animSet >= 2) {
      return;
    }
    mFidgetAnimBits |= 4;
    return;
  case SamusGun::kFT_Major:
    if (animSet >= 6 || animSet < 4) {
      mFidgetAnimBits = 2;
    } else {
      mFidgetAnimBits = 1;
    }
    mFidgetAnimBits |= 4;
    return;
  default:
    return;
  }
}

void CPlayerGun::AsyncLoadSuit(CStateManager& mgr) {
  mCurrentBeam->AsyncLoadSuitArm(mgr);
  mGrappleArm->AsyncLoadSuit(mgr);
}

void CPlayerGun::ReturnToRestPose() {
  if (mInRestPose == true) {
    return;
  }

  if (IsWeaponStateSet(0x1)) {
    PlayAnim(NWeaponTypes::kGAT_BasePosition, false);
  } else if (IsWeaponStateSet(0x4)) {
    PlayAnim(NWeaponTypes::kGAT_ToMissile, false);
  }

  mInRestPose = true;
}

TUniqueId CPlayerGun::DropPowerBomb(CStateManager& mgr) const {
  const CDamageInfo dInfo = mgr.GetPlayer()->GetDeathTime() <= 0.f
                                ? gpTweakPlayerGun->mPowerBomb
                                : CDamageInfo(CWeaponMode::PowerBomb(), 0.f, 0.f, 0.f);
  const float ballHalfExtent = gpTweakPlayer->GetPlayerBallHalfExtent();

  TUniqueId uid = mgr.AllocateUniqueId();
  float zero = 0.f;
  CPowerBomb* pBomb =
      rs_new CPowerBomb(mBombEffects[1][0], uid, kInvalidAreaId, mPlayerId,
                        CTransform4f::Translate(mgr.GetPlayer()->GetTranslation() +
                                                CVector3f(zero, zero, ballHalfExtent)),
                        dInfo);
  mgr.AddObject(*pBomb);
  return uid;
}

void CPlayerGun::SetPhazonBeamFeedback(bool active) {
  const bool fadeOut = !active;
  CSamusHud::DisplayHudMemo(rstl::wstring_l(gpStringTable->GetString(21)),
                            CHUDMemoParms(5.f, true, fadeOut, false));

  if (CSfxManager::IsPlaying(mPhazonBeamSfx)) {
    CSfxManager::SfxStop(mPhazonBeamSfx);
  }

  mPhazonBeamSfx.Clear();
  if (active) {
    mPhazonBeamSfx =
        NWeaponTypes::play_sfx(SFXsam_a_phazup_lp_00, mUnderwater, false, 0x4a);
  }
}

#if VERSION >= VERSION_GM8E_02
void CPlayerGun::SetBeam(CPlayerState::EItemType item, CStateManager& mgr) {
  CPlayerState::EBeamId beam = CPlayerState::kBI_Power;
  switch (item) {
  case CPlayerState::kIT_IceBeam:
    beam = CPlayerState::kBI_Ice;
    break;
  case CPlayerState::kIT_WaveBeam:
    beam = CPlayerState::kBI_Wave;
    break;
  case CPlayerState::kIT_PlasmaBeam:
    beam = CPlayerState::kBI_Plasma;
    break;
  }

  if (mgr.GetPlayerState()->HasPowerUp(item) && beam != mEquippedBeamId) {
    mCurrentBeam->Unload(mgr);
    mgr.PlayerState()->SetCurrentBeam(beam);
    mMorph =
        CGunMorph(gpTweakPlayerGun->GetGunTransformTime(), gpTweakPlayerGun->GetHoloHoldTime());
    LoadBeam(beam, mgr);
  }
}

void CPlayerGun::LoadBeam(CPlayerState::EBeamId beamId, CStateManager& mgr) {
  const CPlayerState::EBeamId& beam = beamId;
  mEquippedBeamId = mNextBeamId = mCurrentAuxBeam = mCurrentBeamId[beam];
  mCurrentBeam = mSelectableBeams[beam];
  mNextBeam = mCurrentBeam;
  mNextState = kNS_StatusQuo;
  mCurrentBeam->Load(mgr, true);
  mCurrentBeam->mRainSplashGenerator = mRainSplashGenerator.get();
  mAuxWeapon->Load(beam, mgr);
  DisableWeaponState(0x8);
  ResetToBeam();
}
#endif
