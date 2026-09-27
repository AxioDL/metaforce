#ifndef _CSCRIPTSPECIALFUNCTION
#define _CSCRIPTSPECIALFUNCTION

#include "MetroidPrime/CActor.hpp"
#include "MetroidPrime/CDamageInfo.hpp"

#include "Kyoto/Graphics/CColor.hpp"
#include "Kyoto/Graphics/CTexture.hpp"
#include "Kyoto/Math/CAABox.hpp"
#include "Kyoto/Math/CTransform4f.hpp"
#include "Kyoto/Math/CVector3f.hpp"
#include "Kyoto/TAverage.hpp"

#include "MetroidPrime/Player/CPlayerState.hpp"

class CScriptSpecialFunction : public CActor {
public:
  enum ESpecialFunction {
    kSF_What = 0,
    kSF_PlayerFollowLocator = 1,
    kSF_SpinnerController = 2,
    kSF_ObjectFollowLocator = 3,
    kSF_ChaffTarget = 4,
    kSF_InventoryActivator = 5,
    kSF_MapStation = 6,
    kSF_SaveStation = 7,
    kSF_IntroBossRingController = 8,
    kSF_ViewFrustumTester = 9,
    kSF_ShotSpinnerController = 10,
    kSF_EscapeSequence = 11,
    kSF_BossEnergyBar = 12,
    kSF_EndGame = 13,
    kSF_HUDFadeIn = 14,
    kSF_CinematicSkip = 15,
    kSF_ScriptLayerController = 16,
    kSF_RainSimulator = 17,
    kSF_AreaDamage = 18,
    kSF_ObjectFollowObject = 19,
    kSF_RedundantHintSystem = 20,
    kSF_DropBomb = 21,
    kSF_ScaleActor = 22,
    kSF_MissileStation = 23,
    kSF_Billboard = 24,
    kSF_PlayerInAreaRelay = 25,
    kSF_HUDTarget = 26,
    kSF_FogFader = 27,
    kSF_EnterLogbook = 28,
    kSF_PowerBombStation = 29,
    kSF_Ending = 30,
    kSF_FusionRelay = 31,
    kSF_WeaponSwitch = 32,
    kSF_FogVolume = 47,
    kSF_RadialDamage = 48,
    kSF_EnvFxDensityController = 49,
    kSF_RumbleEffect = 50
  };

  enum ESpinnerControllerMode {
    kSCM_Zero,
    kSCM_One,
  };

  enum ERingState { kRS_Scramble, kRS_Rotate, kRS_Stopped, kRS_Breakup };

  struct SRingController {
    TUniqueId mId;
    float mRotateSpeed;
    bool mReachedTarget;
    CVector3f xc_;

    SRingController(TUniqueId uid, float rotateSpeed, bool reachedTarget)
    : mId(uid)
    , mRotateSpeed(rotateSpeed)
    , mReachedTarget(reachedTarget)
    , xc_(CVector3f::Zero()) {}
  };

private:
  ESpecialFunction mFunction;
  rstl::string mLocatorName;
  float mFloat1;
  float mFloat2;
  float mFloat3;
  float mFloat4;
  CVector3f mVector3f;
  CColor mColor;
  CDamageInfo mDamageInfo;
  float x138_;
  CTransform4f mSpinnerInitialXf;
  float x16c_;
  ushort mSfx1;
  ushort mSfx2;
  ushort mSfx3;
  CSfxHandle mSfxHandle;
  uint x17c_;
  float x180_;
  TAverage< float > x184_;
  float x194_;
  rstl::vector< SRingController > mRingControllers;
  ERingState mRingState;
  CVector3f mRingRotateTarget;
  bool mRingReverse;
  int mAreaSaveId;
  int mLayerIdx;
  CPlayerState::EItemType mItem;
  rstl::optional_object< CAABox > mTouchBounds;
  bool mSpinnerInitializedXf : 1;
  bool mSpinnerCanMove : 1;
  bool mSfx2Played : 1;
  bool mSfx3Played : 1;
  bool mFrustumEntered : 1;
  bool mFrustumExited : 1;
  bool x1e4_30_ : 1;
  bool mInAreaDamage : 1;
  bool mDoSave : 1;
  bool mPlayerInArea : 1;
  bool mDisplayBillboard : 1;
  rstl::optional_object< CToken > x1e8_;

public:
  CScriptSpecialFunction(TUniqueId, const rstl::string&, const CEntityInfo&, const CTransform4f&,
                         ESpecialFunction, const rstl::string&, float, float, float, float,
                         const CVector3f&, const CColor&, const bool, const CDamageInfo&, int, int,
                         CPlayerState::EItemType, const ushort, const ushort, const ushort);

  DECLARE_TYPES_MATCH_OR_ACCEPT;
  void Think(float, CStateManager&) override;
  void AcceptScriptMsg(EScriptObjectMessage, TUniqueId, CStateManager&) override;
  void PreRender(CStateManager&, const CFrustumPlanes&) override;
  void AddToRenderer(const CFrustumPlanes&, const CStateManager&) const override;
  void Render(const CStateManager&) const override;
  rstl::optional_object< CAABox > GetTouchBounds() const override { return mTouchBounds; }

  void SkipCinematic(CStateManager&);
  void RingScramble(CStateManager&);
  void ThinkIntroBossRingController(float, CStateManager&);
  void ThinkPlayerFollowLocator(float, CStateManager&);
  void ThinkSpinnerController(float, CStateManager&, ESpinnerControllerMode);
  void ThinkObjectFollowLocator(float, CStateManager&);
  void ThinkObjectFollowObject(float, CStateManager&);
  void ThinkChaffTarget(float, CStateManager&);
  void ThinkActorScale(float, CStateManager&);
  void ThinkSaveStation(float, CStateManager&);
  void ThinkRainSimulator(float, CStateManager&);
  void ThinkAreaDamage(float, CStateManager&);
  void ThinkPlayerInArea(float, CStateManager&);

  bool ShouldSkipCinematic(CStateManager& stateMgr) const;

  void DeleteEmitter(CSfxHandle& handle);
  int GetSpecialEnding(const CStateManager&) const;
  void AddOrUpdateEmitter(float pitch, CSfxHandle& handle, ushort id, CVector3f pos, uchar vol);
};
CHECK_SIZEOF(CScriptSpecialFunction, (VERSION >= VERSION_GM8E_02 ? 0x208 : 0x1f8))

#endif // _CSCRIPTSPECIALFUNCTION
