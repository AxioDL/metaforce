#ifndef _CSAMUSHUD
#define _CSAMUSHUD

#include "types.h"

#include "Kyoto/Audio/CSfxHandle.hpp"
#include "Kyoto/Graphics/CColor.hpp"
#include "Kyoto/Math/CMatrix3f.hpp"
#include "Kyoto/Math/CVector2f.hpp"
#include "Kyoto/SObjectTag.hpp"
#include "MetroidPrime/CInGameGuiManagerCommon.hpp"
#include "MetroidPrime/CTargetReticles.hpp"
#include "MetroidPrime/Cameras/CCameraFilterPass.hpp"
#include "MetroidPrime/HUD/CHUDMemoParms.hpp"
#include "MetroidPrime/Player/CPlayerGun.hpp"
#include "rstl/optional_object.hpp"
#include "rstl/reserved_vector.hpp"
#include "rstl/single_ptr.hpp"
#include "rstl/string.hpp"

class CActorLights;
class CFinalInput;
class CGuiFrame;
class CGuiLight;
class CGuiModel;
class CGuiTextPane;
class CGuiWidget;
class CHudBallInterface;
class CHudBossEnergyInterface;
class CHudEnergyInterface;
class CHudHelmetInterface;
class CHudMissileInterface;
class CHudRadarInterface;
class CHudVisorBeamMenu;
class IHudDecoInterface;
class IHudFreeLookInterface;
class CLight;
class CStringTable;
class CUnitVector3f;

#if VERSION < VERSION_GM8E_02
#define ACTUAL_IHudThreatInterface IHudThreatInterface
class IHudThreatInterface;
#else
#define ACTUAL_IHudThreatInterface CHudThreatInterface
class CHudThreatInterface;
#endif

enum EHudState {
  kHS_Combat,
  kHS_XRay,
  kHS_Thermal,
  kHS_Scan,
  kHS_Ball,
  kHS_None,
};

class CSamusHud {
public:
  virtual ~CSamusHud();

  explicit CSamusHud(const CStateManager& mgr);
  void Update(float dt, const CStateManager& mgr, uint helmetVis, bool hudVis,
              bool targetingManager);
  void Draw(const CStateManager& mgr, float alpha, uint helmetVis, bool hudVis,
            bool targetingManager) const;
  void DrawHelmet(const CStateManager& mgr, float camYOff);
  void ProcessControllerInput(const CFinalInput& input);
  void UpdateStateTransition(float dt, const CStateManager& mgr);
  bool CheckLoadComplete(const CStateManager& mgr);
  void OnNewInGameGuiState(EInGameGuiState state, const CStateManager& mgr);
  void RefreshHudOptions();
  void Touch() const;
  const CTargetingManager& GetTargetingManager() const;
  float GetDesiredViewportScaleX() const { return mViewportScaleX; }
  float GetDesiredViewportScaleY() const { return mViewportScaleY; }
  CGuiFrame* GetBaseHudFrame() const { return mLoadedFrmeBaseHud; }

  static CTransform4f BuildFinalCameraTransform(const CQuaternion& rot, const CVector3f& pos,
                                                const CVector3f& camPos);
  static void DisplayHudMemo(const rstl::wstring& text, const CHUDMemoParms& info);
  static void DeferHintMemo(uint strg, uint hintNum, const CHUDMemoParms& info);
  static void ClearHudMemo() {
    DisplayHudMemo(rstl::wstring_l(L""), CHUDMemoParms(0.f, true, true, true));
  }
  void InternalDisplayHudMemo(const rstl::wstring& text, const CHUDMemoParms& info);
  void SetMessage(const rstl::wstring& text, const CHUDMemoParms& info);
  void _DeferHintMemo(uint strg, uint hintNum, const CHUDMemoParms& info);

private:
  enum ELoadPhase { kLP_Zero, kLP_One, kLP_Two, kLP_Three };
  enum ETransitionState { kTS_NotTransitioning, kTS_Countdown, kTS_Loading, kTS_Transitioning };

  struct SCachedHudLight {
    CVector3f mPos;
    CColor mColor;
    float mDistC;
    float mDistL;
    float mDistQ;
    float mFader;

    SCachedHudLight(const CVector3f& pos, const CColor& color, float distC, float distL,
                    float distQ, float fader)
    : mPos(pos)
    , mColor(color)
    , mDistC(distC)
    , mDistL(distL)
    , mDistQ(distQ)
    , mFader(fader) {}
  };

  struct SVideoBand {
    CGuiModel* mVideoband;
    float mRandA;
    float mRandB;
  };

  struct SProfileInfo {
    u64 mUpdateUsec;
    u64 mDrawUsec;
    SProfileInfo() : mUpdateUsec(0), mDrawUsec(0) {}
  };

  friend class CInGameGuiManager;
  static const char* const skHudElementNames[15];
  ELoadPhase mLoadPhase;
  CTargetingManager mTargetingMgr;
  TLockedToken< CGuiFrame > mFrmeHelmet;
  CGuiFrame* mLoadedFrmeHelmet;
  TLockedToken< CGuiFrame > mFrmeBaseHud;
  CGuiFrame* mLoadedFrmeBaseHud;
  rstl::optional_object< TCachedToken< CGuiFrame > > mSelectedHud;
  CGuiFrame* mLoadedSelectedHud;
  rstl::single_ptr< CHudEnergyInterface > mEnergyIntf;
  rstl::single_ptr< ACTUAL_IHudThreatInterface > mThreatIntf;
  rstl::single_ptr< CHudMissileInterface > mMissileIntf;
  rstl::single_ptr< IHudFreeLookInterface > mFreeLookIntf;
  rstl::single_ptr< IHudDecoInterface > mDecoIntf;
  rstl::single_ptr< CHudHelmetInterface > mHelmetIntf;
  rstl::single_ptr< CHudVisorBeamMenu > mVisorMenu;
  rstl::single_ptr< CHudVisorBeamMenu > mBeamMenu;
  rstl::single_ptr< CHudRadarInterface > mRadarIntf;
  rstl::single_ptr< CHudBallInterface > mBallIntf;
  rstl::single_ptr< CHudBossEnergyInterface > mBossEnergyIntf;
  EHudState mCurState;
  EHudState mNextState;
  EHudState mSetState;
  ETransitionState mActiveTransState;
  float mTransT;
  int mPreLoadCountdown;
  float mPlayerHealth;
  int mTotalEnergyTanks;
  int mMissileAmount;
  int mMissileCapacity;
  bool mInFreeLook : 1;
  bool mLookControlHeld : 1;
  bool mLatestFirstPerson : 1;
  bool mEnergyLow : 1;
  uint x2e4_;
  uint x2e8_;
  CPlayerGun::EMissileMode mMissileMode;
  float mVisorBeamMenuAlpha;
  float x2f4_;
  CVector3f mFpCamDirA;
  CVector3f mBasewidgetIdlePos;
  CVector3f mCameraPos;
  CQuaternion mHudLag;
  CQuaternion mInvHudLag;
  rstl::single_ptr< CActorLights > mLights;
  rstl::reserved_vector< SCachedHudLight, 3 > mHudLights;
  CSfxHandle mDamageSfx;
  CCameraFilterPass mCamFilter;
  CGuiLight* mDamageLight;
  rstl::vector< CTransform4f > mLightTransforms;
  float mDamageTime;
  float mDamageLightPulser;
  float mDamageFilterAmtInit;
  float mDamageFilterAmt;
  float mDamageFilterAmtGain;
  float mHudDamagePracticalsInit;
  float mHudDamagePracticals;
  float mHudDamagePracticalsGain;
  CVector3f mDamagerToPlayerNorm;
  float mDecoShakeTranslateAmt;
  float mDecoShakeTranslateAmtVel;
  CVector3f mDecoShakeTranslate;
  CMatrix3f mDecoShakeRotate;
  CQuaternion mHudLagShakeRot;
  float mDecoShakeAmtInit;
  float mDecoShakeAmt;
  float mDecoShakeAmtGain;
  int x468_;
  rstl::reserved_vector< CTransform4f, 3 > x46c_;
  float mViewportScaleX;
  float mViewportScaleY;
  CSfxHandle mStaticSfxHi;
  CSfxHandle mStaticSfxLo;
  float mStaticInterp;
  float mStaticCycleTimerHi;
  float mStaticCycleTimerLo;
  CCameraFilterPass mCamFilter2;
  CHUDMemoParms mHudMemoParms;
  rstl::single_ptr< TToken< CStringTable > > mHudMemoString;
  int mHudMemoIdx;
  float mMessageTextTime;
  float mLastSfxChars;
  float mMessageTextScale;
  CSfxHandle mFreeLookSfx;
  CVector3f mFpCamDirB;
  float mLookDeltaDot;
  float mFreeLookSfxCycleTimer;
  float mEnergyLowTimer;
  float mEnergyLowPulse;
  float mAbuttonPulse;
  CGuiWidget* mBase_basewidget_pivot;
  CGuiWidget* mHelmet_BaseWidget_Pivot;
  CGuiModel* mBase_Model_AutoMapper;
  CGuiTextPane* mBase_textpane_counter;
  CGuiWidget* mBase_basewidget_message;
  CGuiTextPane* mBase_textpane_message;
  CGuiModel* mBase_model_abutton;
  rstl::reserved_vector< SVideoBand, 4 > mVideoBands;
  rstl::reserved_vector< CGuiLight*, 4 > mGuiLights;
  float mCamFovTweaks[16];
  float mCamYTweaks[64];
  float mCamZTweaks[32];
  mutable rstl::reserved_vector< SProfileInfo, 15 > mProfileInfo;

  rstl::reserved_vector< bool, 4 > BuildPlayerHasVisors(const CStateManager& mgr) const;
  rstl::reserved_vector< bool, 4 > BuildPlayerHasBeams(const CStateManager& mgr) const;
  void InitializeFrameGluePermanent(const CStateManager& mgr);
  void InitializeFrameGlueMutable(const CStateManager& mgr);
  void UninitializeFrameGlueMutable();
  void InitializeDamageLight();
  void UpdateEnergy(float dt, const CStateManager& mgr, bool init);
  void UpdateFreeLook(float dt, const CStateManager& mgr);
  void UpdateMissile(float dt, const CStateManager& mgr, bool init);
  void UpdateVideoBands(float dt, const CStateManager& mgr);
  void UpdateBallMode(const CStateManager& mgr, bool init);
  void UpdateThreatAssessment(float dt, const CStateManager& mgr);
  void UpdateVisorAndBeamMenus(float dt, const CStateManager& mgr);
  void UpdateCameraDebugSettings();
  void UpdateEnergyLow(float dt, const CStateManager& mgr);
  void ApplyClassicLag(const CUnitVector3f& lookDir, CQuaternion& rot, const CStateManager& mgr,
                       float dt, bool invert);
  void UpdateHudLag(float dt, const CStateManager& mgr);
  bool IsCachedLightInAreaLights(const SCachedHudLight& light,
                                 const CActorLights& areaLights) const;
  bool IsAreaLightInCachedLights(const CLight& light) const;
  int FindEmptyHudLightSlot(const CLight& light) const;
  CColor GetVisorHudLightColor(const CColor& color, const CStateManager& mgr);
  void UpdateHudDynamicLights(float dt, const CStateManager& mgr);
  void UpdateHudDamage(float dt, const CStateManager& mgr, uint helmetVis);
  void UpdateStaticSfx(CSfxHandle& handle, float& cycleTimer, ushort sfxId, float dt,
                       float oldStaticInterp, float staticThreshold);
  void UpdateStaticInterference(float dt, const CStateManager& mgr);
  int GetRelativeDirection(const CVector3f& position, const CStateManager& mgr) const;
  void ShowDamage(CVector3f position, float dam, float prevDam, const CStateManager& mgr);
  void EnterFirstPerson(const CStateManager& mgr);
  void LeaveFirstPerson(const CStateManager& mgr);
  void DrawAttachedEnemyEffect(const CStateManager& mgr) const;
  EHudState GetDesiredHudState(const CStateManager& mgr) const;
};
CHECK_SIZEOF(CSamusHud, 0x8a0)

#endif // _CSAMUSHUD
