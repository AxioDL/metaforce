#ifndef _CPLAYERGUN
#define _CPLAYERGUN

#include "types.h"

#include "MetroidPrime/CActor.hpp"
#include "MetroidPrime/CActorLights.hpp"
#include "MetroidPrime/CModelData.hpp"
#include "MetroidPrime/Player/CFidget.hpp"
#include "MetroidPrime/Player/CPlayerCameraBob.hpp"
#include "MetroidPrime/Player/CPlayerState.hpp"
#include "MetroidPrime/Weapons/WeaponCommon.hpp"

#include "Kyoto/Audio/CSfxHandle.hpp"
#include "Kyoto/Math/CAABox.hpp"
#include "Kyoto/Math/CTransform4f.hpp"
#include "Kyoto/TOneStatic.hpp"

#include "rstl/pair.hpp"
#include "rstl/reserved_vector.hpp"
#include "rstl/single_ptr.hpp"
#include "rstl/string.hpp"
#include "rstl/vector.hpp"

enum EChargeState {
  kCS_Normal,
  kCS_Charged,
};

class CGunWeapon;
class CGunMotion;
class CGrappleArm;
class CAuxWeapon;
class CRainSplashGenerator;
class CPowerBeam;
class CIceBeam;
class CWaveBeam;
class CPlasmaBeam;
class CPhazonBeam;
class CElementGen;
class CWorldShadow;
class CGenDescription;
class CFinalInput;
class CGameCamera;

class CPlayerGun;

class CPlayerGun : public TOneStatic< CPlayerGun > {
  static float kTractorBeamFactor;
  static const float kGunScale;
  static CVector3f kScaleVector;
  static const uint mHandAnimId[4];

public:
  enum EMissileMode {
    kMM_Inactive,
    kMM_Active,
  };
  enum EBWeapon {
    kBW_Bomb,
    kBW_PowerBomb,
  };
  enum EPhazonBeamState {
    kPBS_Inactive,
    kPBS_Entering,
    kPBS_Exiting,
    kPBS_Active,
  };
  enum EChargePhase {
    kCP_NotCharging,
    kCP_ChargeRequested,
    kCP_AnimAndSfx,
    kCP_FxGrowing,
    kCP_FxGrown,
    kCP_ComboXfer,
    kCP_ComboXferDone,
    kCP_ComboFire,
    kCP_ComboFireDone,
    kCP_ChargeCooldown,
    kCP_ChargeDone,
  };
  enum ENextState {
    kNS_StatusQuo,
    kNS_EnterMissile,
    kNS_ExitMissile,
    kNS_MissileShotDone,
    kNS_MissileReload,
    kNS_ChangeWeapon,
    kNS_SetupBeam,
    kNS_Seven,
    kNS_EnterPhazonBeam,
    kNS_ExitPhazonBeam,
  };
  enum EIdleState {
    kIS_NotIdle,
    kIS_Wander,
    kIS_Idle,
    kIS_Three,
    kIS_Four,
  };

  CPlayerGun(TUniqueId);
  ~CPlayerGun();

  void AddToRenderer(const CFrustumPlanes& frustum, const CStateManager& mgr) const;
  void PreRender(CStateManager&, const CFrustumPlanes&, const CVector3f&);
  void TouchModel(const CStateManager&) const;
  CVector3f ConvertToScreenSpace(const CVector3f& pos, const CGameCamera&) const;
  void DrawArm(const CStateManager&, const CVector3f&, const CModelFlags&) const;
  void Render(const CStateManager&, const CVector3f&, const CModelFlags&) const;
  void GetLctrWithShake(CTransform4f& xfOut, const CModelData&, const rstl::string&, bool, bool);
  void PlayAnim(NWeaponTypes::EGunAnimType type, bool);
  void Update(float, float, float, CStateManager&);
  void ProcessInput(const CFinalInput&, CStateManager&);
  void ProcessChargeState(int, int, CStateManager&, float);
  void ResetNormal(CStateManager&);
  void ResetCharged(float, CStateManager&);
  void ProcessNormalState(int, int, CStateManager&, float);
  bool ExitMissile();
  void UpdateNormalShotCycle(float, CStateManager&);
  void FireSecondary(float, CStateManager&);
  void DropBomb(CPlayerGun::EBWeapon, CStateManager&);
  void ActivateCombo(CStateManager&);
  void EnableChargeFx(CPlayerState::EChargeStage, CStateManager&);
  void UpdateChargeState(float, CStateManager&);
  void Reset(CStateManager& mgr, bool);
  void ResetCharge(CStateManager&, bool);
  void ResetBeamParams(CStateManager&, const CPlayerState&, bool);
  void ChangeWeapon(const CPlayerState&, CStateManager&);
  void StartPhazonBeamTransition(bool, CStateManager&, CPlayerState&);
  void HandleWeaponChange(const CFinalInput&, CStateManager&);
  void HandleBeamChange(const CFinalInput&, CStateManager&);
  void SetPhazonBeamMorph(bool);
  void HandlePhazonBeamChange(CStateManager&);
  void InitBeamData();
#if VERSION >= VERSION_GM8E_02
  void SetBeam(CPlayerState::EItemType item, CStateManager& mgr);
  void LoadBeam(CPlayerState::EBeamId beam, CStateManager& mgr);
#endif
  void InitBombData();
  void InitMuzzleData();
  void InitCTData();
  float GetBeamVelocity() const;
  TUniqueId GetTargetId(CStateManager&);
  void UpdateWeaponFire(float, CPlayerState&, CStateManager&);
  void ResetIdle(CStateManager&);
  void UpdateGunIdle(bool, float, float, CStateManager&);
  void DamageRumble(const CVector3f&, float damage, const CStateManager&);
  void TakeDamage(bool, bool, CStateManager&);
  void StopChargeSound(CStateManager&);
  void CancelFiring(CStateManager&);
  void AcceptScriptMsg(EScriptObjectMessage, TUniqueId, CStateManager&);
  void StopContinuousBeam(CStateManager&, bool);
  void RenderEnergyDrainEffects(const CStateManager&) const;
  void DoUserAnimEvents(float, CStateManager&);
  void DoUserAnimEvent(float, CStateManager&, const CInt32POINode&, EUserEventType);
  void CancelCharge(CStateManager&, bool);
  void EnterFreeLook(CStateManager&);
  void EnterFidget(CStateManager&);
  void UpdateLeftArmTransform(const CModelData&, const CStateManager&);
  void ReturnArmAndGunToDefault(CStateManager&, bool);
  void UpdateAuxWeapons(float, const CTransform4f&, CStateManager&);
  void CancelLockOn();
  void CreateGunLight(CStateManager&);
  void DeleteGunLight(CStateManager&);
  void UpdateGunLight(const CTransform4f&, CStateManager&);
  void SetGunLightActive(bool, CStateManager&);
  void LoadHandAnimTokens();
  void ProcessPhazonGunMorph(float, CStateManager&);
  void ProcessGunMorph(float, CStateManager&);
  void AsyncLoadFidget(CStateManager&);
  void UnLoadFidget();
  bool IsFidgetLoaded();
  void SetFidgetAnimBits(int, bool);
  void AsyncLoadSuit(CStateManager&);
  void ReturnToRestPose();
  TUniqueId DropPowerBomb(CStateManager&) const;
  void SetPhazonBeamFeedback(bool);
  void SetAssistAimTransform(const CTransform4f& xf) { mAssistAimXf = xf; }

  const CTransform4f& GetTransform() const { return mXf; }
  float GetChargePercentage() const { return mCharging ? mChargeBeamFactor : 0.f; }
  EChargeState GetChargeState() const { return mChargeState; }
  CPlayerState::EBeamId GetPrimaryWeaponId() const { return mEquippedBeamId; }
  CAuxWeapon& AuxWeapon() { return *mAuxWeapon; }
  CPlayerState::EBeamId GetPrimaryDestWeaponId() const { return mNextBeamId; }
  uint GetSecondaryItemId() const { return mComboAmmoIdx; }
  int GetBombsPending() const { return mBombCount; }
  bool IsBombReady() const { return mBombReady; }
  bool IsPowerBombReady() const { return mPowerBombReady; }
  int GetFiring() const { return mLastFireButtonStates; }
  EMissileMode GetMissileMode() const { return mMissileMode; }
  float GetHoloTransitionFactor() const { return mMorph.GetTransitionFactor(); }
  TUniqueId GetPowerBombId() const { return mPowerBomb; }

  static float GetTractorBeamFactor() { return kTractorBeamFactor; }
  static float GetGunScale() { return kGunScale; }

  int GetStateFlags() const { return mStateFlags; }
  void SetStateFlags(int flags) { mStateFlags = flags; }
  bool IsWeaponStateSet(int state) const { return (mStateFlags & state) == state; }
  void EnableWeaponState(int state) { mStateFlags |= state; }
  void DisableWeaponState(int state) { mStateFlags &= ~state; }
  void ResetToBeam() {
    if (!IsWeaponStateSet(0x8)) {
      EnableWeaponState(0x1);
      DisableWeaponState(0x16);
    }
    mComboAmmoIdx = 0;
    mMissileMode = kMM_Inactive;
  }
  void ResetToMissile() {
    DisableWeaponState(0x1);
    EnableWeaponState(0x6);
    mComboAmmoIdx = 1;
    mMissileMode = kMM_Active;
  }
  bool CanCycleShoot() const {
    return mChargePhase == kCP_NotCharging && int(mComboAmmoIdx) != 1;
  }
  bool IsCharging() const { return mCharging; }
  void SetTransform(CTransform4f xf) { mXf = xf; }
  CTransform4f GetGunMotionTransform() const { return mGunWorldXf; }
  CGrappleArm& GrappleArm() { return *mGrappleArm.get(); }
  CGrappleArm& GetGrappleArm() const { return *mGrappleArm.get(); }
  bool IsFidgeting() const { return mNotFidgeting; }

  void SetActorAttached(bool attached) { mActorAttached = attached; } // name?

private:
  class CGunMorph {
  public:
    enum EGunState {
      kGS_InWipeDone,
      kGS_OutWipeDone,
      kGS_InWipe,
      kGS_OutWipe,
    };
    enum EMorphEvent {
      kME_None,
      kME_InWipeDone,
      kME_OutWipeDone,
    };
    enum EDir {
      kD_In,
      kD_Out,
      kD_Done,
    };

    CGunMorph(float gunTransformTime, float holoHoldTime);
    EMorphEvent Update(float inY, float outY, float dt);
    void StartWipe(EDir dir);

    float GetYLerp() const { return mYLerp; }
    float GetTransitionFactor() const { return mTransitionFactor; }
    EGunState GetGunState() const { return mGunState; }
    void SetWeaponChanged() { mWeaponChanged = true; }

  private:
    float mYLerp;
    float mGunTransformTime;
    float mRemTime;
    float mSpeed;
    float mHoloHoldTime;
    float mRemHoldTime;
    float mTransitionFactor;
    EDir mDir;
    EGunState mGunState;
    bool mMorphing : 1;
    bool mWeaponChanged : 1;
  };

  class CMotionState {
  public:
    enum EMotionState {
      kMS_Zero,
      kMS_One,
      kMS_LockOn,
      kMS_CancelLockOn,
    };
    enum EFireState {
      kFS_NotFiring,
      kFS_StartFire,
      kFS_Firing,
    };

    CMotionState()
    : mExtendParabola(true)
    , mExtendParabolaDelayTimer(0.f)
    , mFireTime(0.f)
    , mCurExtendDist(0.f)
    , mCurRotation(0.f)
    , mRotationT(0.f)
    , mStartRotation(0.f)
    , mEndRotation(0.f)
    , mState(kMS_Zero)
    , mFireState(kFS_NotFiring) {}

    static void SetExtendDistance(float d) { gGunExtendDistance = d; }

    void SetState(EMotionState state) { mState = state; }
    void Update(bool firing, float dt, CTransform4f& xf, CStateManager& mgr);

  private:
    static float gGunExtendDistance;

    bool mExtendParabola : 1;
    float mExtendParabolaDelayTimer;
    float mFireTime;
    float mCurExtendDist;
    float mCurRotation;
    float mRotationT;
    float mStartRotation;
    float mEndRotation;
    EMotionState mState;
    EFireState mFireState;
  };

  CActorLights mLights;
  CSfxHandle mChargeSfx;
  CSfxHandle mInvalidSfx;
  CSfxHandle mPhazonBeamSfx;
  // 0x1: FireOrBomb, 0x2: MissileOrPowerBomb
  uint mLastFireButtonStates;
  uint mPressedFireButtonStates;
  uint mFireButtonStates;
  // 0x1: beam mode, 0x2: missile mode, 0x4: missile ready, 0x8: morphing, 0x10: combo fire
  int mStateFlags;
  int mFidgetAnimBits;
  uint mRemainingMissiles;
  uint x304_;
  int mBombCount;
  int mRapidFireShots;
  CPlayerState::EBeamId mEquippedBeamId;
  CPlayerState::EBeamId mNextBeamId;
  uint mComboAmmoIdx;
  EMissileMode mMissileMode;
  CPlayerState::EBeamId mCurrentAuxBeam;
  EIdleState mIdleState;
  int mAnimSfxPitch;
  EChargePhase mChargePhase;
  EChargeState mChargeState;
  uint x334_;
  ENextState mNextState;
  EPhazonBeamState mPhazonBeamState;
  float mChargeBeamFactor;
  float mComboXferTimer;
  float mChargeCooldownTimer;
  float mShakeX;
  float mShakeZ;
  float mBombFuseTime;
  float mBombDropDelayTime;
  float mBombTime;
  float x360_;
  float mGunStrikeCoolTimer;
  float mIdleWanderDelayTimer;
  float x36c_;
  float mGunMotionSpeedMult;
  float x374_;
  float mShotSmokeStartTimer;
  float mRapidFireShotsDecayTimer;
  float mShotSmokeTimer;
  float mGunStrikeDelayTimer;
  float mEnterFreeLookDelayTimer;
  float mMuzzleEffectVisTimer;
  float mCooldown;
  float mDamageTimer;
  float mDamageAmt;
  float mPhazonMorphT;
  float mMissileExitTimer;
  CFidget mFidget;
  CVector3f mDamageLocation;
  CTransform4f mXf;
  CTransform4f mBeamLocalXf;
  CTransform4f mElbowWorldXf;
  CTransform4f mAssistAimXf;
  CTransform4f mGunWorldXf;
  CTransform4f mGunLocalXf;
  CTransform4f mElbowLocalXf;
  TUniqueId mPlayerId;
  TUniqueId mPowerBomb;
  TUniqueId mLightId;
  rstl::vector< CToken > mHandAnimTokens;
  CPlayerCameraBob mCamBob;
  uint x658_;
  float x65c_;
  float x660_;
  float x664_;
  float mAimVerticalSpeed;
  float mAimHorizontalSpeed;
  rstl::pair< ushort, CSfxHandle > mAnimSfx;
  CGunMorph mMorph;
  CMotionState mMotionState;
  CAABox mHologramClipCube;
  CModelData mRightHandModel;
  CGunWeapon* mCurrentBeam;
  CGunWeapon* mOutgoingBeam;
  CGunWeapon* mLoadingBeam;
  CGunWeapon* mNextBeam;
  rstl::single_ptr< CGunMotion > mGunMotion;
  rstl::single_ptr< CGrappleArm > mGrappleArm;
  rstl::single_ptr< CAuxWeapon > mAuxWeapon;
  rstl::single_ptr< CRainSplashGenerator > mRainSplashGenerator;
  rstl::single_ptr< CPowerBeam > mPowerBeam;
  rstl::single_ptr< CIceBeam > mIceBeam;
  rstl::single_ptr< CWaveBeam > mWaveBeam;
  rstl::single_ptr< CPlasmaBeam > mPlasmaBeam;
  rstl::single_ptr< CPhazonBeam > mPhazonBeam;
  rstl::reserved_vector< CGunWeapon*, 4 > mSelectableBeams;
  rstl::auto_ptr< CElementGen > mHoloTransitionGen;
  rstl::auto_ptr< CElementGen > mComboXferGen;
  rstl::reserved_vector< rstl::reserved_vector< TLockedToken< CGenDescription >, 2 >, 2 >
      mBombEffects;
  rstl::reserved_vector< TLockedToken< CGenDescription >, 5 > mAuxMuzzleEffects;
  rstl::reserved_vector< rstl::auto_ptr< CElementGen >, 5 > mAuxMuzzleGenerators;
  rstl::single_ptr< CWorldShadow > mShadow;
  short mChargeRumbleHandle;

  bool mCoolingCharge : 1;
  bool mChargeEffectVisible : 1;
  bool mComboFiring : 1;
  bool mChargeAnimStarted : 1;
  bool mReadyForShot : 1;
  bool mLockedOn : 1;
  bool mRequestReturnToDefault : 1;
  bool mInRestPose : 1;

  bool mNotFidgeting : 1;
  bool x833_25_ : 1;
  bool x833_26_ : 1;
  bool x833_27_ : 1;
  bool mPhazonBeamActive : 1;
  bool mPointBlankWorldSurface : 1;
  bool mCanShowAuxMuzzleEffect : 1;
  bool mInFreeLook : 1;

  bool mCharging : 1;
  bool mGunMotionFidgeting : 1;
  bool mAnimPlaying : 1;
  bool mUnderwater : 1;
  bool mRequestImmediateRecharge : 1;
  bool mFrozen : 1;
  bool mInBigStrike : 1;
  bool mGunMotionInFidgetBasePosition : 1;

  bool mCanFirePhazon : 1;
  bool mInPhazonBeam : 1;
  bool mPhazonBeamMorphing : 1;
  bool mIntoPhazonBeam : 1;
  bool mBombReady : 1;
  bool mPowerBombReady : 1;
  bool mInPhazonPool : 1;
  bool mActorAttached : 1;
  // bool x835_32_unk : 1;
};
CHECK_SIZEOF(CPlayerGun, 0x838)

#endif // _CPLAYERGUN
